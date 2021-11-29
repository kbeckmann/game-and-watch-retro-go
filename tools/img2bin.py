import argparse
from pathlib import Path

def parse_args():
    """Parse bmp file to bin txt"""
    parser = argparse.ArgumentParser()
    parser.add_argument("bmp", type=Path)
    args = parser.parse_args()
    return args

def write_rgb565(fi, fn):
    from PIL import Image, ImageOps
    img = Image.open(fi).convert(mode="RGB")
    pixels = list(img.getdata())
    with open(fn, "wb") as f:
        # TODO: this header could probably be a bit shorter, didn't really investigate
        for y in range(img.height):
        #for pix in pixels:
            for x in range(img.width):
            #img.pixels[x,y]
                r = (pixels[(y * img.width) + x][0] >> 3) & 0x1F
                g = (pixels[(y * img.width) + x][1] >> 2) & 0x3F
                b = (pixels[(y * img.width) + x][2] >> 3) & 0x1F
                px = (r << 11) + (g << 5) + b
                f.write(str.encode(f"0x{px:04X},", "utf-8"))
            f.write(str.encode("//\n", "utf-8"))

def main():
    args = parse_args()
    #filepath.stem
    write_rgb565(args.bmp, args.bmp.stem + ".txt")


if __name__ == "__main__":
    main()
