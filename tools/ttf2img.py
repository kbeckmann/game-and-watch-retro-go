#!/usr/bin/env python3

import argparse
import struct
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import sys


def Paint_Fontogn(font_name, priname, font_size:int, out_size:int, xoffset:int, yoffset:int):
    print("Process:" + font_name)
    h_s = out_size // 2
    s_w = out_size * 4
    s_h = out_size * 4
    #gset = ((out_size * 2) - out_size) // 2
    #new image
    img = Image.new('RGB', (s_w * 16, s_h * 16) , 0)
    img1 = Image.new('RGB', (s_w * 16, s_h * 16) , 0)
    tipfnt = ImageFont.truetype("cour.ttf", out_size)
    outfnt = ImageFont.truetype("fonts/" + font_name, font_size)
    draw = ImageDraw.Draw(img)
    draw1 = ImageDraw.Draw(img1)
    for x in range(16):
        for y in range(16):
            #paint grid ---
            draw.line((0, y * s_h, s_w * 16, y * s_h), width=1, fill="#404040")
            draw.line((0, y * s_h + out_size * 2 - 1, s_w * 16, y * s_h + out_size * 2 - 1), width=2, fill="#404040")
            draw.line((0, y * s_h + s_h - 1, s_w * 16, y * s_h + s_h - 1), width=1, fill="#404040")
            #paint grid |||
            draw.line((x * s_w, y * s_h, x * s_w, y * s_h + s_h - 1), width=1, fill="#404040")
            draw.line((x * s_w + out_size * 2 - 1, y * s_h + out_size * 2, x * s_w + out_size * 2 - 1, y * s_h + s_h - 1), width=2, fill="#404040")
            draw.line((x * s_w + s_w - 1, y * s_h, x * s_w + s_w - 1, y * s_h + s_h - 1), width=1, fill="#404040")
            #paint a fixed grid;
            draw.rectangle((x * s_w + out_size * 2 + h_s, y * s_h + out_size * 2 + h_s, x * s_w + out_size * 2 + h_s + out_size - 1, y * s_h + out_size * 2 + h_s + out_size - 1), outline="#101010")

            draw.text((x * s_w + h_s, y * s_h + h_s), "%03d"%(y*16+x), font=tipfnt, fill="#A0A0A0")
            draw1.text((x * s_w + h_s, y * s_h + h_s), "%03d"%(y*16+x), font=tipfnt, fill="#A0A0A0")
            #skip 0-31,128-159
            chrno = y*16+x
            if ((chrno > 31) and (chrno < 128)) or ((chrno > 159) and (chrno < 256)):
                draw.text((x * s_w + h_s, y * s_h + out_size * 2 + h_s), chr(y*16+x), font=tipfnt, fill=(120,120,120))
                draw.text((x * s_w + h_s + out_size * 2 + xoffset, y * s_h + out_size * 2 + h_s + yoffset), chr(y*16+x), font=outfnt, fill=(255,255,255))
                draw1.text((x * s_w + h_s, y * s_h + out_size * 2 + h_s), chr(y*16+x), font=tipfnt, fill=(120,120,120))
                draw1.text((x * s_w + h_s + out_size * 2 + xoffset, y * s_h + out_size * 2 + h_s + yoffset), chr(y*16+x), font=outfnt, fill=(255,255,255))
    bmp_file = "fontview" + "/" + priname + str((Path(font_name)).stem + ".bmp")
    img.save(bmp_file, "BMP")
    #bmp_file = "fontimgs" + "/" + priname + str((Path(font_name)).stem + ".bmp")
    #img1.save(bmp_file, "BMP")

def process_onefile(filename, fontdef):
    fontdef.setdefault(filename, {})
    fdef = fontdef[filename]
    fdef.setdefault("fontsize", "12")  #
    fdef.setdefault("fixedsize", "12") #
    fdef.setdefault("resize", "12")
    fdef.setdefault("xoffset", "0")
    fdef.setdefault("yoffset", "0")
    fdef.setdefault("check", "75")
    fdef.setdefault("_a_", "_")
    #paint priview image
    #Paint_Fontogn(font_name, img_floder, font_size, out_size, xoffset, yoffset, withline):
    Paint_Fontogn(filename, fdef["_a_"] + "_", int(fdef["fontsize"]), int(fdef["fixedsize"]), int(fdef["xoffset"]), int(fdef["yoffset"]))  #ogn font image


def main():
    import json
    jsonfile = "fonts/fonts.json"
    if Path(jsonfile).exists():
        with open(jsonfile,'r') as load_f:
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

    if (len(sys.argv) > 1):
        process_onefile(sys.argv[1], fontdef)
    else:
        for key in fontdef:
            process_onefile(key, fontdef)
        

if __name__ == "__main__":
    main()