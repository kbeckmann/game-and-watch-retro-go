#!/bin/bash
set -xe

# Would be nice to clone the repo with submodules. If you know how to do this, please make a PR!
git submodule update --init --recursive

./.ci_prepare_roms.sh

# Install toolchain
# curl -L -o toolchain.tar.bz2 https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-mac.tar.bz2?revision=48a4e09a-eb5a-4eb8-8b11-d65d7e6370ff&la=en&hash=8AACA5F787C5360D2C3C50647C52D44BCDA1F73F
# Downloading from developer.arm.com is slow, use github mirror instead
curl -L -o toolchain.tar.bz2 https://github.com/kbeckmann/gcc-toolchains/raw/main/10-2020-q4-major/gcc-arm-none-eabi-10-2020-q4-major-mac.tar.bz2

tar xf toolchain.tar.bz2

export GCC_PATH=$(pwd)/gcc-arm-none-eabi-10-2020-q4-major/bin

make -j8 download_sdk

make -j$(sysctl -n hw.logicalcpu)
