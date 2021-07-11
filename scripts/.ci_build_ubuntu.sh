#!/bin/bash
set -xe

apt install lz4

./scripts/.ci_prepare_roms.sh

make -j $(nproc)
