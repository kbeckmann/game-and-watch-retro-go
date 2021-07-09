#!/bin/bash
set -xe

apt install lz4

# Would be nice to clone the repo with submodules. If you know how to do this, please make a PR!
git submodule update --init --recursive

./.ci_prepare_roms.sh

make -j $(nproc)
