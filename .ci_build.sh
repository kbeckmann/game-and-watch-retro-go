#!/bin/bash
set -xe

sudo apt-get install -y gcc-arm-none-eabi binutils-arm-none-eabi

# Would be nice to clone the repo with submodules. If you know how to do this, please make a PR!
git submodule update --init --recursive

# NES
curl -L -o pwn.bin https://github.com/Vector35/PwnAdventureZ/blob/master/PwnAdventureZ-csaw-withkeys.bin?raw=true
./update_nes_rom.sh pwn.bin
make -f Makefile.nes -j clean
make -f Makefile.nes -j


# GB
curl -L -o matrix-rain.gb https://github.com/wtjones/matrix-rain-gb/releases/download/0.0.3/matrix-rain.gb
./update_gb_rom.sh matrix-rain.gb
make -f Makefile.gb -j clean
make -f Makefile.gb -j
