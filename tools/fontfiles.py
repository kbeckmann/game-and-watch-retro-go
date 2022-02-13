#!/usr/bin/env python3
import argparse
import os
import shutil
import struct
from pathlib import Path
from tempfile import TemporaryDirectory
from typing import List

class FontParser:
    def find_fonts(self, fontdefs: dict, ext) -> dict:
        script_path = Path(__file__).parent
        fonts_folder = script_path / "fonts"

        # find all files that end with extension (case-insensitive)
        font_files = list(fonts_folder.iterdir())
        font_files = [r for r in font_files if r.name.lower().endswith(ext)]
        font_files.sort()
        for font_file in font_files :
            file_name = font_file.stem + ext
            #file_name = str(font_file)
            fontdefs.setdefault(file_name, {})
            fontdef = fontdefs[file_name]
            fontdef.setdefault("fontsize", "12")  #
            fontdef.setdefault("fixedsize", "12") #
            fontdef.setdefault("resize", "12")
            fontdef.setdefault("xoffset", "0")
            fontdef.setdefault("yoffset", "0")
            fontdef.setdefault("check", "75")
            fontdef.setdefault("_a_", "_")
            fontdef.setdefault("name", file_name)

        return fontdefs

    def parse_one(self, filename: str):
        import json;
        if Path(filename).exists():
            with open(filename,'r') as load_f:
                try:
                    fontdef = json.load(load_f)
                    #print("Rom define file loaded")
                    load_f.close()
                except: 
                    print("Fonts define file load failed")
                    fontdef = {}
                    load_f.close()
        else :
            fontdef = {};

        fontdef = self.find_fonts(fontdef, ".ttf");
        fontdef = self.find_fonts(fontdef, ".otf");
        with open(filename,'w', encoding ='utf-8') as dump_f:
            json.dump(fontdef, dump_f, ensure_ascii=False, indent=4, sort_keys=True)
            #print("Rom Define file saved ok!")
            dump_f.close()

    def parse(self):
        script_path = Path(__file__).parent

        json_file = script_path / "fonts" / "fonts.json"
        self.parse_one(json_file)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Define fontfiles params")

    try:
        FontParser().parse()
    except ImportError as e:
        print(e)
        print("Missing dependencies. Run:")
        print("    python -m pip install -r requirements.txt")
        exit(-1)
