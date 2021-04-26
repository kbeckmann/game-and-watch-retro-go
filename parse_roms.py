#!/usr/bin/env python3

import argparse
import os
import subprocess
from typing import List

ROM_ENTRIES_TEMPLATE = """
const retro_emulator_file_t {name}[] = {{
{body}
}};
const uint32_t {name}_count = {rom_count};
"""

ROM_ENTRY_TEMPLATE = """\t{{
\t\t.name = "{name}",
\t\t.ext = "{extension}",
\t\t.address = {rom_entry},
\t\t.size = {size},
\t\t.save_address = {save_entry},
\t\t.save_size = sizeof({save_entry}),
\t\t.system = &{system},
\t\t.region = {region},
\t}},"""

SYSTEM_PROTO_TEMPLATE = """
extern const rom_system_t {name};
"""

SYSTEM_TEMPLATE = """
const rom_system_t {name} = {{
\t.system_name = "{system_name}",
\t.roms = {variable_name},
\t.extension = "{extension}",
\t.roms_count = {roms_count},
}};
"""

class ROM:
    def __init__(self, system_name: str, filepath: str, extension: str):
        # Remove .lz4 from the name in case it ends with that
        stripped_filepath = os.path.splitext(os.path.basename(filepath))[0] \
                            if filepath.endswith(".lz4") else os.path.basename(filepath)

        self.name = os.path.splitext(stripped_filepath)[0]
        self.path = filepath
        self.size = os.path.getsize(filepath)
        self.ext = extension
        obj = "".join([i if i.isalnum() else "_" for i in os.path.basename(filepath)])
        symbol_path = os.path.dirname(filepath) + "/" + obj
        self.obj_path = "build/roms/" + obj + ".o"
        self.symbol = "_binary_" + "".join([i if i.isalnum() else "_" for i in symbol_path]) + "_start"

    def __str__(self) -> str:
        return f"name: {self.name} size: {self.size} ext: {self.ext}"

    def __repr__(self):
        return str(self)


class ROMParser():
    def find_roms(self, system_name: str, folder: str, extension: str) -> [ROM]:
        ext = extension
        if not extension.startswith("."):
            extension = "." + extension
        script_path = os.path.dirname(os.path.realpath(__file__))
        roms_path = os.path.join(script_path, "roms", folder)
        rom_files = os.listdir(roms_path)

        found_roms = []

        for rom in rom_files:
            if not rom.endswith(extension):
                continue

            rom_path = os.path.join(roms_path, rom)
            found_roms.append(ROM(system_name, rom_path, ext))

        return found_roms


    def generate_rom_entries(self, name: str, roms: [ROM], save_prefix: str, system: str) -> str:
        body = ""
        for i in range(len(roms)):
            rom = roms[i]
            is_pal = any(substring in rom.name for substring in ["(E)", "(Europe)", "(Sweden)", "(Germany)", "(Italy)", "(France)", "(A)", "(Australia)"])
            region = "REGION_PAL" if is_pal else "REGION_NTSC"
            body += ROM_ENTRY_TEMPLATE.format(
                name=rom.name,
                size=rom.size,
                rom_entry=rom.symbol,
                save_entry=save_prefix + str(i),
                region=region,
                extension=rom.ext,
                system=system,
            )
            body += "\n"

        return ROM_ENTRIES_TEMPLATE.format(name=name, body=body, rom_count=len(roms))


    def generate_object_file(self, ROM: ROM) -> str:
        # convert rom to an .o file and place the data in the .extflash_game_rom section
        prefix = ""
        if "GCC_PATH" in os.environ:
            prefix = os.environ["GCC_PATH"]
        subprocess.run([ os.path.join(prefix, "arm-none-eabi-objcopy"), "--rename-section", ".data=.extflash_game_rom,alloc,load,readonly,data,contents", \
                         "-I", "binary", "-O", "elf32-littlearm", "-B", "armv7e-m", ROM.path, ROM.obj_path ])
        subprocess.run([ os.path.join(prefix, "arm-none-eabi-ar"), "-cru", "build/roms.a", ROM.obj_path ])
        template = "extern const uint8_t {name}[];\n"
        return template.format(name=ROM.symbol)

    def generate_save_entry(self, name: str, save_size: int) -> str:
        return f"uint8_t {name}[{save_size}]  __attribute__((section (\".saveflash\"))) __attribute__((aligned(4096)));\n"

    def get_gameboy_save_size(self, file: str):
        total_size = 4096

        with open(file, "rb") as f:
            # cgb
            f.seek(0x143)
            cgb = ord(f.read(1))

            # 0x80 = Gameboy color but supports old gameboy
            # 0xc0 = Gameboy color only
            if cgb & 0x80 or cgb == 0xc0:
                # Bank 0 + 1-7 for gameboy color work ram
                total_size += 8 * 4096 # irl

                # Bank 0 + 1 for gameboy color video ram, 2*8KiB
                total_size += 4 * 4096 # vrl
            else:
                # Bank 0 + 1 for gameboy classic work ram
                total_size += 2 * 4096 # irl

                # Bank 0 only for gameboy classic video ram, 1*8KiB
                total_size += 2 * 4096 # vrl

            # Cartridge ram size
            f.seek(0x149)
            total_size += [1, 1, 1, 4, 16, 8][ord(f.read(1))] * 8 * 1024
            return total_size

        return 0

    def generate_system(self, file: str, system_name: str, variable_name: str, folder: str, extensions: List[str], save_prefix: str, compress: bool=False) -> int:
        f = open(file, "w")

        roms_raw = []
        for e in extensions:
            roms_raw += self.find_roms(system_name, folder, e)

        roms_lz4 = []
        for e in extensions:
            roms_lz4 += self.find_roms(system_name, folder, e + ".lz4")

      #  def contains_rom_by_name(rom: ROM, roms: list[ROM]):
        def contains_rom_by_name(rom, roms):        
            for r in roms:
                if r.name == rom.name:
                    return True
            return False

        if compress:
            lz4_path = os.environ["LZ4_PATH"] if "LZ4_PATH" in os.environ else "lz4"
            for r in roms_raw:
                print(r)
                if not contains_rom_by_name(r, roms_lz4):
                    subprocess.run([lz4_path, "--best", "--content-size", "--no-frame-crc", r.path, r.path + ".lz4"])
            # Re-generate the lz4 rom list
            roms_lz4 = []
            for e in extensions:
                roms_lz4 += self.find_roms(system_name, folder, e + ".lz4")


        # Create a list with all LZ4-compressed roms and roms that
        # don't have a compressed counterpart.
        roms = roms_lz4[:]
        for r in roms_raw:
            if not contains_rom_by_name(r, roms_lz4):
                roms.append(r)

        total_save_size = 0
        total_rom_size = 0

        if folder == "nes":
            save_size = 24 * 1024
        elif folder == "sms":
            save_size = 60 * 1024
        elif folder == "gg":
            save_size = 60 * 1024
        elif folder == "pce":
            save_size = 76 * 1024
        else:
            save_size = 0

        f.write(SYSTEM_PROTO_TEMPLATE.format(
            name=variable_name))

        for i in range(len(roms)):
            rom = roms[i]
            if folder == "gb":
                save_size = self.get_gameboy_save_size(rom.path)

            # Aligned
            aligned_size = 4 * 1024
            total_save_size += ((save_size + aligned_size - 1) // (aligned_size)) * aligned_size
            total_rom_size += rom.size

            f.write(self.generate_object_file(rom))
            f.write(self.generate_save_entry(save_prefix + str(i), save_size))

        rom_entries = self.generate_rom_entries(folder + "_roms", roms, save_prefix, variable_name)
        f.write(rom_entries)

        f.write(SYSTEM_TEMPLATE.format(
            name=variable_name,
            system_name=system_name,
            variable_name=folder + "_roms",
            extension=folder,
            roms_count=len(roms)))
        f.close()

        return total_save_size, total_rom_size

    def write_if_changed(self, path: str, data: str):
        old_data = None
        if os.path.exists(path):
            with open(path) as f:
                old_data = f.read()

        if data != old_data:
            with open(path, "w") as f:
                f.write(data)

    def parse(self, args):
        total_save_size = 0
        total_rom_size = 0
        build_config = ""

        save_size, rom_size = self.generate_system("Core/Src/retro-go/gb_roms.c", "Nintendo Gameboy", "gb_system", "gb", ["gb", "gbc"], "SAVE_GB_")
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_GB\n" if rom_size > 0 else ""

        save_size, rom_size = self.generate_system("Core/Src/retro-go/nes_roms.c", "Nintendo Entertainment System", "nes_system", "nes", ["nes"], "SAVE_NES_", args.compress)
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_NES\n" if rom_size > 0 else ""

        save_size, rom_size = self.generate_system("Core/Src/retro-go/sms_roms.c", "Sega Master System", "sms_system", "sms", ["sms"], "SAVE_SMS_")
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_SMS\n" if rom_size > 0 else ""

        save_size, rom_size = self.generate_system("Core/Src/retro-go/gg_roms.c", "Sega Game Gear", "gg_system", "gg", ["gg"], "SAVE_GG_")
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_GG\n" if rom_size > 0 else ""

        save_size, rom_size = self.generate_system("Core/Src/retro-go/pce_roms.c", "PC Engine", "pce_system", "pce", ["pce"], "SAVE_PCE_")
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_PCE\n" if rom_size > 0 else ""

        total_size = total_save_size + total_rom_size

        print(f"Save data:\t{total_save_size} bytes\nROM data:\t{total_rom_size} bytes\nTotal:\t\t{total_size} / {args.flash_size} bytes (plus some metadata).")
        if total_size > args.flash_size:
            print(f"Error: External flash will overflow!")
            exit(-1)

        self.write_if_changed("build/saveflash.ld", f"__SAVEFLASH_LENGTH__ = {total_save_size};\n")
        self.write_if_changed("build/config.h", build_config)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Import ROMs to the build environment')
    parser = argparse.ArgumentParser()
    parser.add_argument('--flash-size', '-s', type=int, default=1024*1024)
    parser.add_argument('--compress', dest='compress', action='store_true')
    parser.add_argument('--no-compress', dest='compress', action='store_false')
    parser.set_defaults(compress=True)
    args = parser.parse_args()

    if not os.path.isdir("build/roms"):
        os.mkdir("build/roms", 0o755)

    ROMParser().parse(args)
