import argparse


from fcdata import fontdata 

class char:
    pass
    
def write_fontpixels(fn, height):
    with open(fn, "w") as f:
        chars = [None] * 256;
        for i in range(256):
            chars[i] = char()
            chars[i].idx = i
            chars[i].width = 0
            chars[i].size = 0
            chars[i].postion = 0
            chars[i].xoffset = 0
            chars[i].data = [0] * 36 #max 12 * 24 pixels width
            chars[i].mdata = [0] * height

        idx=0;
        flen = len(fontdata)
        while (idx < flen):
            charCode = fontdata[idx]
            chars[charCode].size = fontdata[idx + 1]
            chars[charCode].width = fontdata[idx + 2]
            chars[charCode].xoffset = fontdata[idx + 3]
            #print(fontdata[idx+4:idx+4+chars[charCode].size])
            for i in range(chars[charCode].size):
                chars[charCode].data[i] = fontdata[idx + 4 + i]
            idx = idx + chars[charCode].size + 4
            #Change Value Data
            chars[charCode].size = ((chars[charCode].width + 7) // 8) * height
            for i in range(height):
                m = int(("{:0>8b}".format(chars[charCode].data[i * 2]))[::-1], 2)
                m1 = int(("{:0>8b}".format(chars[charCode].data[i * 2 + 1]))[::-1], 2)
                m = m + (m1 << 8)
                #print("{:0>8b}".format(chars[charCode].data[i * 2]))
                #print("{:0>8b}".format(chars[charCode].data[i * 2 + 1]))
                #print("{:0>16b}".format(m))
                #m = chars[charCode].data[i * 2] + (chars[charCode].data[i * 2 + 1] << 8)
                m = m >> chars[charCode].xoffset
                chars[charCode].mdata[i] = m
            #print(chars[charCode].data)
            #print(chars[charCode].mdata)
        
        start_pos = 0;
        for i in range(256):
            chars[i].postion = start_pos
            start_pos += chars[i].size

        #write out file
        f.write("    // width data\n")
        for i in range(256):
            if (i % 16) == 0:
                f.write("    ")
            f.write("0x%02x, "%chars[i].width)
            if ((i + 1) % 16) == 0:
                f.write("//\n")

        f.write("\n")
        f.write("    // Data Postion\n")
        for i in range(256):
            if (i % 8) == 0:
                f.write("    ")
            pos = chars[i].postion
            f.write("0x%02x,"%(pos % 0x100))
            f.write("0x%02x,  "%(pos >> 8))
            if ((i + 1) % 8) == 0:
                f.write("//\n")
        f.write("\n")
        f.write("    // Real font's pixels data\n")
        for i in range(256):
            f.write("     /* %03d */ "%i)
            if (chars[i].width > 0):
                for j in range(height):
                    if (chars[i].width > 8):
                        f.write("0x%02x,"%(chars[i].mdata[j] % 0x100))
                        f.write("0x%02x,  "%(chars[i].mdata[j] >> 8))
                    else:
                        f.write("0x%02x, "%(chars[i].mdata[j] % 0x100))
            f.write("\n")

def main():
    import sys
    #filepath.stem
    write_fontpixels(sys.argv[0] + ".txt", 12)

if __name__ == "__main__":
    main()
