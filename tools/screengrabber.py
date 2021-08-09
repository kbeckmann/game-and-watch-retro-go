#!/usr/bin/env python3

import argparse
import os
import v4l2

from elftools.elf.elffile import ELFFile
from fcntl import ioctl
from openocd import OpenOCD

# 1. Create a v4l2 loopback and find out the name (/dev/video0 etc):
#    sudo modprobe v4l2loopback
# 2. Start openocd with a fast adapter speed separately:
#    openocd -f interface/jlink.cfg -c "transport select swd;" -f target/stm32h7x.cfg -c "adapter speed 10000; init;"
# 3. python3 tools/screengrabber.py

def get_symbol_by_symbol_name(elffile, symbol_name):
    return elffile.get_section_by_name('.symtab').get_symbol_by_name(symbol_name)[0]

def screengrabber(args):
    # Find address of framebuffer
    with open(args.elf, "rb") as f:
        elffile = ELFFile(f)
        fb_address = get_symbol_by_symbol_name(elffile, "framebuffer1").entry.st_value

    # Open camera driver
    fd = os.open(args.device, os.O_RDWR, 0)

    cap = v4l2.v4l2_capability()

    ioctl(fd, v4l2.VIDIOC_QUERYCAP, cap)

    BUFTYPE = v4l2.V4L2_BUF_TYPE_VIDEO_OUTPUT
    FRAME_FORMAT = v4l2.V4L2_PIX_FMT_RGB565

    # Set format
    width = args.width
    height = args.height
    sizeimage = width * height * 2
    linewidth = width

    fmt = v4l2.v4l2_format()
    fmt.type = BUFTYPE
    fmt.fmt.pix.width        = width
    fmt.fmt.pix.height       = height
    fmt.fmt.pix.pixelformat  = FRAME_FORMAT
    fmt.fmt.pix.sizeimage    = sizeimage
    fmt.fmt.pix.field        = v4l2.V4L2_FIELD_NONE
    fmt.fmt.pix.bytesperline = linewidth
    fmt.fmt.pix.colorspace   = v4l2.V4L2_COLORSPACE_SRGB

    ret = ioctl(fd, v4l2.VIDIOC_S_FMT, fmt)
    print("fcntl.ioctl(fd, v4l2.VIDIOC_S_FMT, fmt) = %d" % ret)

    buffer_size = fmt.fmt.pix.sizeimage

    with OpenOCD(host=args.host, port=args.port) as ocd:
        while True:
            fb = ocd.send(f"dump_image fb.bin {hex(fb_address)} {hex(buffer_size)}; exit")
            with open("fb.bin", "rb") as f_in:
                os.write(fd, f_in.read())

if __name__ == "__main__":
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
    parser.add_argument(
        "--device",
        type=str,
        default="/dev/video0",
        help="video4l2 device",
    )
    parser.add_argument("--width",  type=int, default=320)
    parser.add_argument("--height", type=int, default=240)

    screengrabber(parser.parse_args())
