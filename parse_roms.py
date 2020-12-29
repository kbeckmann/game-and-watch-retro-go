#!/usr/bin/env python3

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
\t.roms_count = {variable_name}_count
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
        while True:
            current_data = rom_data[:8]
            char_array += "\t"
            for b in current_data:
                char_array += hex(b) + ", "
            rom_data = rom_data[8:]
            char_array += "\n"
            if(len(rom_data) == 0):
                break
        char_array += "};\n"

        return char_array


    def generate_save_entry(self, name: str, save_size: int) -> str:
        return f"uint8_t {name}[{save_size}]  __attribute__((section (\".saveflash\"))) __attribute__((aligned(65536)));\n"

    def get_gameboy_save_size(self, file: str):
        total_size = 4096

        with open(file, "rb") as f:
            # cgb
            f.seek(0x143)
            cgb = ord(f.read(1))

            if cgb == 0x80 or cgb == 0xc0:
                total_size += 8 * 4096 # irl
                total_size += 4 * 4096 # vrl
            else:
                total_size += 2 * 4096 # irl
                total_size += 2 * 4096 # vrl

            # Sram size
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

        if folder == "nes":
            save_size = 64 * 1024
        else:
            save_size = 0

        for i in range(len(roms)):
            rom = roms[i]
            if folder == "gb":
                save_size = self.get_gameboy_save_size(rom.path)

            # Aligned
            aligned_size = 64 * 1024
            total_save_size += ((save_size + aligned_size) // (aligned_size)) * aligned_size

            f.write(self.generate_char_array(data_prefix + str(i), rom))
            f.write(self.generate_save_entry(save_prefix + str(i), save_size))

        rom_entries = self.generate_rom_entries(folder + "_roms", roms, data_prefix, save_prefix)
        f.write(rom_entries)

        f.write(SYSTEM_TEMPLATE.format(name=variable_name, system_name=system_name, variable_name=folder + "_roms", extension =folder))
        f.close()

        return total_save_size


    def generate_saveflash(self, file: str, size: int):
        f = open(file, "w")
        f.write(f"__SAVEFLASH_LENGTH__ = {size};\n")
        f.close()


    def parse(self):
        save_size = 0
        save_size += self.generate_system("Core/Src/retro-go/gb_roms.c", "Nintendo Gameboy", "gb_system", "gb", ["gb", "gbc"], "ROM_GB_", "SAVE_GB_")
        save_size += self.generate_system("Core/Src/retro-go/nes_roms.c", "Nintendo Entertainment System", "nes_system", "nes", ["nes"], "ROM_NES_", "SAVE_NES_")

        self.generate_saveflash("saveflash.ld", save_size)


if __name__ == "__main__":
    ROMParser().parse()
