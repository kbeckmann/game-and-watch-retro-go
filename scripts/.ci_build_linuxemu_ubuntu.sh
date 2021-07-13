#!/bin/bash
set -xe

sudo apt-get update -y
sudo apt-get upgrade -y
sudo apt-get install -y libsdl2-dev

./scripts/.ci_prepare_roms.sh

cd linux

# Build gb
./update_gb_rom.sh ../roms/gb/*.gb
make -j$(nproc) -f Makefile.gb
make -f Makefile.gb clean

# Build nes
./update_nes_rom.sh ../roms/nes/*.nes
make -j$(nproc) -f Makefile.nes
make -f Makefile.nes clean
