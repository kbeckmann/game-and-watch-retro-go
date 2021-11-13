#!/usr/bin/env python3
import argparse
import os
import shutil
import struct
from pathlib import Path
from tempfile import TemporaryDirectory
from typing import List

class ROMParser:
    def find_roms(self, romdefs: dict, folder: str, extension: str) -> dict:
        extension = extension.lower()
        ext = extension
        if not extension.startswith("."):
            extension = "." + extension

        script_path = Path(__file__).parent
        roms_folder = script_path / "roms" / folder

        # find all files that end with extension (case-insensitive)
        romdefs.setdefault("_cover_width", 128)
        romdefs.setdefault("_cover_height", 96)
        w = romdefs["_cover_width"]
        h = romdefs["_cover_height"]
        print(f"{folder}: Cover image size is defined :{[w]} x {[h]}")
        rom_files = list(roms_folder.iterdir())
        rom_files = [r for r in rom_files if r.name.lower().endswith(extension)]
        rom_files.sort()
        for rom_file in rom_files :
            file_name = rom_file.stem
            romdefs.setdefault(file_name, {})
            romdef = romdefs[file_name]
            romdef.setdefault("name", file_name)
            romdef.setdefault("publish", "1")
            print(folder + ":" + file_name + " >> " + romdef["name"] + ",P: " + romdef["publish"])

        return romdefs

    def generate_system(self, romdef: dict, folder: str,extensions: List[str]) -> dict:
        roms_raw = []
        for e in extensions:
            roms_raw += self.find_roms(romdef, folder, e)
        return romdef

    def parse_one(self, filename: str, folder: str,extensions: List[str]):
        import json;
        if Path(filename).exists():
            with open(filename,'r') as load_f:
                try:
                    romdef = json.load(load_f)
                    #print("Rom define file loaded")
                    load_f.close()
                except: 
                    print("Rom define file load failed")
                    romdef = {}
                    load_f.close()
        else :
            romdef = {};

        romdef = self.generate_system(romdef, folder, extensions);
        with open(filename,'w', encoding ='utf-8') as dump_f:
            json.dump(romdef, dump_f, ensure_ascii=False, indent=4, sort_keys=True)
            #print("Rom Define file saved ok!")
            dump_f.close()

    def parse(self):
        script_path = Path(__file__).parent

        json_file = script_path / "roms" / "gb.json"
        self.parse_one(json_file,"gb",["gb", "gbc"])

        json_file = script_path / "roms" / "nes.json"
        self.parse_one(json_file, "nes",["nes"])

        json_file = script_path / "roms" / "sms.json"
        self.parse_one(json_file, "sms",["sms"])

        json_file = script_path / "roms" / "gg.json"
        self.parse_one(json_file, "gg",["gg"])

        json_file = script_path / "roms" / "col.json"
        self.parse_one(json_file, "col",["col"])

        json_file = script_path / "roms" / "sg.json"
        self.parse_one(json_file, "sg",["sg"])

        json_file = script_path / "roms" / "pce.json"
        self.parse_one(json_file, "pce",["pce"])

        json_file = script_path / "roms" / "gw.json"
        self.parse_one(json_file, "gw",["gw"])

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Define ROMs name & pulish to the build environment")

    try:
        ROMParser().parse()
    except ImportError as e:
        print(e)
        print("Missing dependencies. Run:")
        print("    python -m pip install -r requirements.txt")
        exit(-1)
