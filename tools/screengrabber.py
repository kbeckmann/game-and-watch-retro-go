#!/usr/bin/python3

import subprocess

import os
import v4l2
import fcntl
from fcntl import ioctl


# 1. Create a v4l2 loopback and find out the name (/dev/video2 etc): sudo modprobe v4l2loopback
# 2. Start openocd with a fast adapter speed separately:
#    openocd -f interface/jlink.cfg -c "transport select swd;" -f target/stm32h7x.cfg -c "adapter speed 10000; init; "
# 3. python3 screengrabber.py

class ScreenGrabber():
    def __init__(self):
        pass

    def run(self):

        # Open camera driver
        fd = os.open('/dev/video2', os.O_RDWR, 0)

        cap = v4l2.v4l2_capability()

        ioctl(fd, v4l2.VIDIOC_QUERYCAP, cap)

        print(hex(cap.capabilities))

        BUFTYPE = v4l2.V4L2_BUF_TYPE_VIDEO_OUTPUT
        MEMTYPE = v4l2.V4L2_MEMORY_MMAP
        FRAME_FORMAT = v4l2.V4L2_PIX_FMT_RGB565

        # Set format
        width = 320
        height = 240
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
        print("buffer_size = " + str(buffer_size))

        fb_address = 0x24000000

        while True:
            subprocess.check_output(f"echo 'dump_image fb.bin {hex(fb_address)} {hex(buffer_size)}; exit' | nc localhost 4444", shell=True)
            with open("fb.bin", "rb") as f_in:
                os.write(fd, f_in.read())

if __name__ == "__main__":
    ScreenGrabber().run()