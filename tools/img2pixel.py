import argparse
from pathlib import Path

def parse_args():
    """Parse bmp file to bin txt"""
    parser = argparse.ArgumentParser()
    parser.add_argument("bmp", type=Path)
    args = parser.parse_args()
    return args

def write_pixels(fi, fn):
    from PIL import Image, ImageOps
    img = Image.open(fi).convert(mode="RGB")
    pixels = list(img.getdata())
    with open(fn, "wb") as f:
        # TODO: this header could probably be a bit shorter, didn't really investigate
        #create a array;
        binData = [];
        #print("// width" + str(img.width) + ", height:" + str(img.height));
        f.write(str.encode("// width" + str(img.width) + ", height:" + str(img.height) + "\n", "utf-8"))
        for x in range((img.width+7) // 8):
            binData.append(0);

        for y in range(img.height):
        #for pix in pixels:
            s_pix = ""
            for x in range((img.width+7) // 8):
               binData[x] = 0;
            for x in range(img.width):
            #img.pixels[x,y]
                b_b = binData[(x+8) // 8 - 1];
                b_i = 7 - (x % 8);
                b_p = pixels[(y * img.width) + x][0] + pixels[(y * img.width) + x][1] + pixels[(y * img.width) + x][2];
                #print("x:" + str(x) + ", y:" + str(y) + ", color:" + str(b_p));
                if (b_p > 100):
                    b_b = b_b | (1 << b_i)
                    binData[(x+8) // 8 -1] = b_b
                    s_pix = s_pix + "#"
                else:
                    s_pix = s_pix + "_"
                #f.write(str.encode(f"0x{px:04X},", "utf-8"))
            for x in range((img.width+7) // 8):
               #binData[x] = 0;
               f.write(str.encode(f"0x{binData[x]:02x}, ", "utf-8"))
            f.write(str.encode("//  {s}\n".format(s=s_pix), "utf-8"))

def main():
    args = parse_args()
    #filepath.stem
    write_pixels(args.bmp, args.bmp.stem + ".txt")

if __name__ == "__main__":
    main()
