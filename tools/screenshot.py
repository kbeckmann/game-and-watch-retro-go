#!/usr/bin/env python3

import argparse
import struct
import subprocess

from datetime import datetime
from elftools.elf.elffile import ELFFile
from openocd import OpenOCD
from PIL import Image
from time import sleep

def get_symbol_by_symbol_name(elffile, symbol_name):
    return elffile.get_section_by_name('.symtab').get_symbol_by_name(symbol_name)[0]

def get_screenshot(args):
    # Find address of framebuffer storage location
    with open(args.elf, "rb") as f:
        elffile = ELFFile(f)
        fb_address = get_symbol_by_symbol_name(elffile, "framebuffer_capture").entry.st_value
    
    buffer_size = 320 * 240 * 2

    with OpenOCD(host=args.host, port=args.port) as ocd:
        ocd.send(f"dump_image {args.output}.bin {hex(fb_address)} {hex(buffer_size)}; resume; exit")

    with open(f"{args.output}.bin", "rb") as fd:
        data = fd.read()

    # Convert raw RGB565 pixel data to PNG
    img = Image.new("RGB", (args.width, args.height))
    pixels = img.load()
    index = 0
    for y in range(0, args.height):
        for x in range(0, args.width):
            color, = struct.unpack('<H', data[index:index+2])
            red =   int(((color & 0b1111100000000000) >> 11) / 31.0 * 255.0)
            green = int(((color & 0b0000011111100000) >>  5) / 63.0 * 255.0)
            blue =  int(((color & 0b0000000000011111)      ) / 31.0 * 255.0)
            pixels[x, y] = (red, green, blue)
            index += 2

    img.save(f"{args.output}.png")

    print(f"Screenshot saved as {args.output}.png")

def main():
    parser = argparse.ArgumentParser(description="Grabs the framebuffer and renders it to a video4l2 device")
    parser.add_argument(
        "--elf",
        type=str,
        default="build/gw_retro_go.elf",
        help="Game and Watch Retro-Go ELF file",
    )
    parser.add_argument(
        "--host",
        type=str,
        default="127.0.0.1",
        help="OpenOCD TCL hostname",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=6666,
        help="OpenOCD TCL port",
    )
    parser.add_argument("--width",  type=int, default=320)
    parser.add_argument("--height", type=int, default=240)
    parser.add_argument(
        "--output",
        type=str,
        default=f"screenshot-{datetime.now().strftime('%Y%m%dT%H%M%S')}",
        help="Screenshot filename (default: screenshot-YYYYMMDDTHHMMSS)",
    )

    try:
        get_screenshot(parser.parse_args())
        return
    except ConnectionRefusedError:
        pass

    # Attempt to automagically launch openocd
    try:
        p = subprocess.Popen(
            ["make", "openocd"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        )
        sleep(1)  # Give openocd some time to launch
        get_screenshot(parser.parse_args())
    except KeyboardInterrupt:
        pass

    try:
        p.terminate()
    except OSError:
        pass
    p.wait()


if __name__ == "__main__":
    main()