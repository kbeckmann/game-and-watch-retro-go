import argparse
import struct
from ffdata import fontdata, fwidth, fheight, outwidth



def writestring(file, ss):
    "Write string data to file"
    file.write(bytes(ss, encoding="ASCII",errors = "ignore"))
    
def write_fontpixels(fn):
    with open(fn, "wb") as f:
        charCode=idx=glyph_width=charCode=adjYOffset=width=height=xOffset=xDelta=int(0);
        flen = len(fontdata)
        chrsize = (fwidth + 7) // 8
        ecbytes = chrsize * fheight
        for idx in range(256):
            writestring(f,"    # ")
            if idx >= 0x20:
                f.write(struct.pack("B",idx))
            writestring(f," #" + "%d"%idx + " width " + "%d"%outwidth+"\n")
            writestring(f,"    # Option: fixed,fixed(outheight*2),width,xoffset, Data:")
            for x in range(chrsize * fheight):
                writestring(f," 0x%02x,"%fontdata[idx * ecbytes + x])
            writestring(f,"\n")
            maxspace = 4
            maxpixel = 0
            for y in range(fheight):
                v = 0
                if (chrsize > 1):
                    v = (fontdata[idx * ecbytes + 2 * y] << 8) + fontdata[idx * ecbytes + 2 * y + 1]
                else:
                    v = fontdata[idx * ecbytes + y] << 8
                m = 1 << 15
                allspece = 1
                for k in range(16):
                    if v & m:
                        if (allspece == 1) :
                            maxspace = k if (maxspace > k) else maxspace
                        allspece = 0
                        maxpixel = (k + 1) if (maxpixel <= k) else maxpixel
                    v = v << 1
            maxpixel = maxpixel - maxspace + 1
            if (maxpixel < 2):
                maxpixel = 2
            writestring(f,"    0x%02x"%idx+", 0x%02x"%(fheight*2)+ ", 0x%02x"%maxpixel + ", 0x%02x,\n"%maxspace)
            for y in range(fheight):
                v = 0
                if (chrsize > 1):
                    v = (fontdata[idx * ecbytes + 2 * y] << 8) + fontdata[idx * ecbytes + 2 * y + 1]
                else:
                    v = fontdata[idx * ecbytes + y] << 8
                m = 1 << 15
                writestring(f,"    ")
                for k in range(16):
                    if (k == 8):
                        writestring(f,", ")
                    if v & m:
                        writestring(f,"X")
                    else:
                        writestring(f,"_")
                    v = v << 1
                writestring(f,",\n")
            	

def main():
    import sys
    #filepath.stem
    write_fontpixels(sys.argv[0] + ".txt")

if __name__ == "__main__":
    main()
