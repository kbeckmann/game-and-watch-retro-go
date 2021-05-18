#!/usr/bin/env python3

import argparse
import os
import subprocess
import tempfile
import platform
import shutil

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

# TODO: Find a better way to find this before building
MAX_COMPRESSED_NES_SIZE = 0x00081000

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

        rom_file_raw = file.rstrip(".lz4")
        with open(rom_file_raw, "rb") as f:
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

    def generate_system(self, file: str, system_name: str, variable_name: str, folder: str, extensions: List[str], save_prefix: str, compress: bool=False,compress_gb_speed: bool=False) -> int:
        f = open(file, "w")

        roms_raw = []
        for e in extensions:
            roms_raw += self.find_roms(system_name, folder, e)

        roms_lz4 = []
        for e in extensions:
            roms_lz4 += self.find_roms(system_name, folder, e + ".lz4")

        def contains_rom_by_name(rom, roms):        
            for r in roms:
                if r.name == rom.name:
                    return True
            return False

        if compress:
            lz4_path = os.environ["LZ4_PATH"] if "LZ4_PATH" in os.environ else "lz4"
            split_path = os.environ["LZ4_PATH"] if "LZ4_PATH" in os.environ else "split"

            for r in roms_raw:
                if not contains_rom_by_name(r, roms_lz4):

                    #NES LZ4 compression
                    if  "nes_system" in variable_name:
                        if os.stat(r.path).st_size > MAX_COMPRESSED_NES_SIZE:
                            print(f"INFO: {r.name} is too large to compress, skipping compression!")
                            continue
                        subprocess.run([lz4_path, "-9", "--content-size", "--no-frame-crc", r.path, r.path + ".lz4"])

                    #GB/GBC LZ4 compression
                    if "gb_system" in variable_name:
                        if platform.system() == "Darwin":
                            split_exec = "gsplit"
                            if not shutil.which(split_exec):
                                print("Cannot find program \"gsplit\". Try:")
                                print("    brew install coreutils")
                                exit(-1)
                        else:
                            split_exec = "split"

                        # Create temp directory to build
                        tmp_dir_inst = tempfile.TemporaryDirectory(dir='./')
                        tmp_dir=tmp_dir_inst.name

                         # split the ROM in banks
                        prefix =  tmp_dir+"/bank_"
                        subprocess.run([split_exec, "-b16384","-d", r.path, prefix])

                         # Get all banks ordered by name
                        tmp_files_list = sorted(os.listdir(tmp_dir))
                        bank_files      = [tmp_dir+"/"+s for s in tmp_files_list]

                        # determine number of banks
                        banks_nb = len(bank_files)

                        #compress all banks
                        cmd=lz4_path + " -9 --content-size --no-frame-crc -m " + tmp_dir+"/*"
                        cout=subprocess.check_output(cmd,stderr=subprocess.STDOUT,shell=True)

                        # Get all banks compressed  ordered by name
                        tmp_files_list = sorted(os.listdir(tmp_dir))

                        #keep only lz4 files
                        lz4_files_short = [ flz4 for flz4 in tmp_files_list if os.path.splitext(flz4)[1] == ".lz4"]
                        lz4_files = [tmp_dir+"/"+s for s in lz4_files_short]

                        compress_it = [True] * banks_nb

                        # For ROM having continous bank switching we can use 'partial' compression
                        # a mix of comcompressed and uncompress
                        # compress empty banks and the bigger compress ratio

                        #keep bank0 uncompressed
                        compress_it[0] = False

                         # START : ALTERNATIVE COMPRESSION STRATEGY
                         # the larger banks only are compressed.
                         # It shoul fit exactly in the cache reducing the SWAP cache feequency to 0.
                         # any empty bank is compressed (=98bytes). considered never used by MBC.

                        ## Ths is the cache size used as a compression credit
                        # TODO : can we the value from the linker ?
                        compression_credit = 26

                        if  compress_gb_speed:
                            compress_size = [ os.path.getsize(flz4) for flz4 in lz4_files[1:]]

                            # to keep empty banks compressed (size=98)
                            compress_size = [i for i in compress_size if i > 98]

                            ordered_size = sorted(compress_size)

                            if compression_credit > len(ordered_size):
                                compression_credit = len(ordered_size) -1

                            compress_threshold = ordered_size[int(compression_credit)]

                            for idx, flz4 in enumerate(lz4_files):
                                if os.path.getsize(flz4)  >= compress_threshold:
                                    compress_it[idx] = False
                         # END : ALTERNATIVE COMPRESSION STRATEGY

                        #create rom lz4 file (concatenate all banks)
                        with open(r.path + ".lz4",'wb') as lz4_out_file:

                            for idx, bank_is_compressed in enumerate(compress_it):

                                # Add compressed bank
                                if bank_is_compressed == True:
                                    with open(lz4_files[idx],'rb') as bank_file:
                                        lz4_out_file.write(bank_file.read())

                                # Add uncompressed bank
                                else :
                                    with open(bank_files[idx],'rb') as bank_file:

                                        #### Write header ###
                                         # write MAGIC WORD \x04\x22\x4D\x18
                                        # write FLG,BD,HC \x68\x40\x00

                                        lz4_header =  b'\x04\x22\x4D\x18\x68\x40'
                                        lz4_out_file.write(lz4_header)

                                        #write uncompressed frame size
                                        content_size_hc = b'\x00\x40\x00\x00\x00\x00\x00\x00\x25'
                                        lz4_out_file.write(content_size_hc)

                                        #### Write block data ###
                                        #0x8000 flag uncompressed block
                                        #0x4000 16384 bytes
                                        block_size=b'\x00\x40\x00\x80'
                                        lz4_out_file.write(block_size)
                                        lz4_out_file.write(bank_file.read())

                                        #### Write footer ###
                                        # write END_MARK 0x0000
                                        lz4_footer =   b'\x00\x00\x00\x00'
                                        lz4_out_file.write(lz4_footer)

                        tmp_dir_inst.cleanup()

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
        elif folder == "col":
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

        save_size, rom_size = self.generate_system("Core/Src/retro-go/gb_roms.c", "Nintendo Gameboy", "gb_system", "gb", ["gb", "gbc"], "SAVE_GB_", args.compress, args.compress_gb_speed)
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

        save_size, rom_size = self.generate_system("Core/Src/retro-go/col_roms.c", "Colecovision", "col_system", "col", ["col"], "SAVE_COL_")
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_COL\n" if rom_size > 0 else ""

        save_size, rom_size = self.generate_system("Core/Src/retro-go/pce_roms.c", "PC Engine", "pce_system", "pce", ["pce"], "SAVE_PCE_")
        total_save_size += save_size
        total_rom_size += rom_size
        build_config += "#define ENABLE_EMULATOR_PCE\n" if rom_size > 0 else ""

        total_size = total_save_size + total_rom_size

        if total_size == 0:
            print("No roms found! Please add at least one rom to one of the the directories in roms/")
            exit(-1)

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
    parser.add_argument('--compress_gb_speed', dest='compress_gb_speed', action='store_true')
    parser.add_argument('--no-compress_gb_speed', dest='compress_gb_speed', action='store_false')

    parser.set_defaults(compress=True,compress_gb_speed=False)
    args = parser.parse_args()

    if not os.path.isdir("build/roms"):
        os.mkdir("build/roms", 0o755)

    ROMParser().parse(args)
