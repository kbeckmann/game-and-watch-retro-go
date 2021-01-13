#!/usr/bin/env python3

import argparse
import os
from typing import List

ROM_ENTRIES_TEMPLATE = """
const rom_entry_t {name}[] = {{
{body}
}};
const uint32_t {name}_count = {rom_count};
"""

ROM_ENTRY_TEMPLATE = """\t{{
\t\t.rom_name = "{name}",
\t\t.flash_address = {rom_entry},
\t\t.size={size},
\t\t.save_address = {save_entry},
\t\t.save_size = sizeof({save_entry}),
\t\t.region = {region},
\t}},"""

SYSTEM_TEMPLATE = """
const rom_system_t {name} = {{
\t.system_name = "{system_name}",
\t.roms = {variable_name},
\t.extension = "{extension}",
\t.roms_count = {roms_count},
}};
"""

class ROM:
    def __init__(self, system_name: str, filepath: str):
        self.name = (os.path.splitext(os.path.basename(filepath))[0])
        self.path = filepath
        self.size = os.path.getsize(filepath)

    def __str__(self) -> str:
        return self.name + " " + str(self.size)

    def __repr__(self):
        return str(self)

    def data(self) -> bytes:
        with open(self.path, "rb") as f:
            return f.read()


class ROMParser():
    def find_roms(self, system_name: str, folder: str, extension: str) -> [ROM]:
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
            found_roms.append(ROM(system_name, rom_path))

        return found_roms


    def generate_rom_entries(self, name: str, roms: [ROM], rom_prefix: str, save_prefix: str) -> str:
        body = ""
        for i in range(len(roms)):
            rom = roms[i]
            variable_name = rom_prefix + str(i)
            is_pal = any(substring in rom.name for substring in ["(E)", "(Europe)", "(Sweden)", "(Germany)", "(Italy)", "(France)", "(A)", "(Australia)"])
            region = "REGION_PAL" if is_pal else "REGION_NTSC"
            body += ROM_ENTRY_TEMPLATE.format(
                name=rom.name,
                size=rom.size,
                rom_entry=rom_prefix + str(i),
                save_entry=save_prefix + str(i),
                region=region,
            )
            body += "\n"

        return ROM_ENTRIES_TEMPLATE.format(name=name, body=body, rom_count=len(roms))


    def generate_char_array(self, name: str, ROM: ROM) -> str:
        rom_data = ROM.data()
        template = "const uint8_t {name}[]  __attribute__((section (\".extflash_game_rom\"))) = {{\n"
        char_array = template.format(name=name)
        char_array += ", ".join([hex(x) for x in rom_data])
        char_array += "};\n"

        return char_array


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

    def generate_system(self, file: str, system_name: str, variable_name: str, folder: str, extensions: List[str], data_prefix: str, save_prefix: str) -> int:
        f = open(file, "w")
        roms = []
        for e in extensions:
            roms += self.find_roms(system_name, folder, e)

        total_save_size = 0
        total_rom_size = 0

        if folder == "nes":
            save_size = 24 * 1024
        elif folder == "sms":
            save_size = 60 * 1024
        elif folder == "gg":
            save_size = 60 * 1024
        else:
            save_size = 0

        for i in range(len(roms)):
            rom = roms[i]
            if folder == "gb":
                save_size = self.get_gameboy_save_size(rom.path)

            # Aligned
            aligned_size = 4 * 1024
            total_save_size += ((save_size + aligned_size - 1) // (aligned_size)) * aligned_size
            total_rom_size += rom.size

            f.write(self.generate_char_array(data_prefix + str(i), rom))
            f.write(self.generate_save_entry(save_prefix + str(i), save_size))

        rom_entries = self.generate_rom_entries(folder + "_roms", roms, data_prefix, save_prefix)
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

        save_size, rom_size = self.generate_system("Core/Src/retro-go/gb_roms.c", "Nintendo Gameboy", "gb_system", "gb", ["gb", "gbc"], "ROM_GB_", "SAVE_GB_")
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_GB\n" if rom_size > 0 else ""

        save_size, rom_size = self.generate_system("Core/Src/retro-go/nes_roms.c", "Nintendo Entertainment System", "nes_system", "nes", ["nes"], "ROM_NES_", "SAVE_NES_")
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_NES\n" if rom_size > 0 else ""

        save_size, rom_size = self.generate_system("Core/Src/retro-go/sms_roms.c", "Sega Master System", "sms_system", "sms", ["sms"], "ROM_SMS_", "SAVE_SMS_")
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_SMS\n" if rom_size > 0 else ""

        save_size, rom_size = self.generate_system("Core/Src/retro-go/gg_roms.c", "Sega Game Gear", "gg_system", "gg", ["gg"], "ROM_GG_", "SAVE_GG_")
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_GG\n" if rom_size > 0 else ""

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
    args = parser.parse_args()
    ROMParser().parse(args)
