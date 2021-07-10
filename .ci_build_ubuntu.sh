#!/bin/bash
set -xe

apt install lz4

./.ci_prepare_roms.sh

make -j $(nproc)
