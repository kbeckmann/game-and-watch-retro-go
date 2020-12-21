#!/usr/bin/env python3

import os

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
            body += ROM_ENTRY_TEMPLATE.format(
                name=rom.name,
                size=rom.size,
                rom_entry=rom_prefix + str(i),
                save_entry=save_prefix + str(i),
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
        return f"uint8_t {name}[{save_size}]  __attribute__((section (\".saveflash\")));\n"

    def generate_system(self, file: str, system_name: str, variable_name: str, extension: str, data_prefix: str, save_prefix: str) -> int:
        f = open(file, "w")
        roms = self.find_roms(system_name, extension, extension)

        if extension == "gb":
            save_size = 192*1024
        elif extension == "nes":
            save_size = 64 * 1024
        else:
            save_size = 0

        for i in range(len(roms)):
            rom = roms[i]
            f.write(self.generate_char_array(data_prefix + str(i), rom))
            f.write(self.generate_save_entry(save_prefix + str(i), save_size))

        rom_entries = self.generate_rom_entries(extension + "_roms", roms, data_prefix, save_prefix)
        f.write(rom_entries)

        f.write(SYSTEM_TEMPLATE.format(name=variable_name, system_name=system_name, variable_name=extension + "_roms", extension =extension))
        f.close()

        return len(roms) * save_size


    def generate_saveflash(self, file: str, size: int):
        f = open(file, "w")
        f.write(f"__SAVEFLASH_LENGTH__ = {size};\n")
        f.close()


    def parse(self):
        save_size = 0
        save_size += self.generate_system("Core/Src/retro-go/gb_roms.c", "Nintendo Gameboy", "gb_system", "gb", "ROM_GB_", "SAVE_GB_")
        save_size += self.generate_system("Core/Src/retro-go/nes_roms.c", "Nintendo Entertainment System", "nes_system", "nes", "ROM_NES_", "SAVE_NES_")

        self.generate_saveflash("saveflash.ld", save_size)


if __name__ == "__main__":
    ROMParser().parse()
