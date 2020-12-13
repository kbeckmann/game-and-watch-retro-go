#!/usr/bin/env python3

import os

ROM_ENTRIES_TEMPLATE = """
const rom_entry {name}[] = {{
    {body}
}};
const uint32_t {name}_count = {rom_count};
"""

ROM_ENTRY_TEMPLATE = """\t{{.rom_name = "{name}", .flash_address = (uint32_t)&{variable_name}[0], .size={size}}},"""

SYSTEM_TEMPLATE = """
const rom_system {name} = {{
    .system_name = "{system_name}",
    .roms = {variable_name},
    .extension = "{extension}",
    .roms_count = {variable_name}_count
}};
"""

class ROM:
    def __init__(self, system_name: str, filepath: str):
        self.name = os.path.basename(filepath).split(".")[0]
        self.path = filepath
        self.size = os.path.getsize(filepath)

    def __str__(self) -> str:
        return self.name + " " + str(self.size)

    def __repr__(self):
        return str(self)
    
    def data(self) -> bytes:
        with open(self.path, "rb") as f:
            return f.read()


def find_roms(system_name: str, folder: str, extension: str) -> [ROM]:
    if not extension.startswith("."):
        extension = "." + extension
    script_path = os.path.dirname(os.path.realpath(__file__))
    roms_path = os.path.join(script_path, "roms", folder)
    rom_files = os.listdir(roms_path)

    found_roms = []

    for rom in rom_files:
        if not rom.endswith(extension):
            print("cont")
            print(rom)
            continue

        rom_path = os.path.join(roms_path, rom)
        found_roms.append(ROM(system_name, rom_path))
    
    print(found_roms)
    return found_roms

def generate_rom_entries(name: str, roms: [ROM], variable_prefix: str) -> str:
    body = ""
    for i in range(len(roms)):
        rom = roms[i]
        variable_name = variable_prefix + str(i)
        body += ROM_ENTRY_TEMPLATE.format(name = rom.name, size=rom.size, variable_name=variable_name)
        body += "\n"

    return ROM_ENTRIES_TEMPLATE.format(name=name, body=body, rom_count=len(roms))

def generate_char_array(name: str, ROM: ROM) -> str:
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
    char_array += "};"
    return char_array


def generate_system(file: str, system_name: str, variable_name: str, extension: str, data_prefix: str) -> str:
    f = open(file, "w")
    roms = find_roms(system_name, extension, extension)

    for i in range(len(roms)):
        rom = roms[i]
        f.write(generate_char_array(data_prefix + str(i), rom))

    rom_entries = generate_rom_entries(extension + "_roms", roms, data_prefix)
    f.write(rom_entries)

    f.write(SYSTEM_TEMPLATE.format(name=variable_name, system_name=system_name, variable_name=extension + "_roms", extension =extension))
    f.close()


generate_system("Core/Src/retro-go/gb_roms.c", "Nintendo Gameboy", "gb_system", "gb", "ROM_GB_")
generate_system("Core/Src/retro-go/nes_roms.c", "Nintendo Entertainment System", "nes_system", "nes", "ROM_NES_")
