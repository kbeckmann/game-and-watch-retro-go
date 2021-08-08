import argparse
from pathlib import Path

import imageio
import numpy as np


def parse_args():
    """Parse CLI arguments into an object and a dictionary"""
    parser = argparse.ArgumentParser()
    parser.add_argument("png", type=Path)
    parser.add_argument("--invert", action="store_true")
    args = parser.parse_args()
    return args


def main():
    args = parse_args()
    img = imageio.imread(args.png)
    if img.ndim == 3:
        if img.shape[-1] == 4:
            img = (img == [0, 0, 0, 255]).all(axis=-1)
            img = ~img
        else:
            raise NotImplementedError
    img = img.astype(bool)
    if args.invert:
        img = ~img
    print(f"// img shape: {img.shape}")
    img_packed = np.packbits(img)
    print("static const uint8_t img[] = {")
    for i in range(0, len(img_packed), 8):
        print(
            f"    0x{img_packed[i]:02X}, 0x{img_packed[i+1]:02X}, 0x{img_packed[i+2]:02X}, "
            f"0x{img_packed[i+3]:02X}, 0x{img_packed[i+4]:02X}, 0x{img_packed[i+5]:02X}, "
            f"0x{img_packed[i+6]:02X}, 0x{img_packed[i+7]:02X},"
        )
    print("};")


if __name__ == "__main__":
    main()
