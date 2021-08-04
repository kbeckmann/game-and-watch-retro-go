#!/bin/bash

. ./scripts/common.sh

if [[ "$VERBOSE" == "1" ]]; then
    set -ex
else
    set -e
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ELF=${DIR}/../build/gw_retro_go.elf

make openocd > /dev/null 2> /dev/null &

sleep 0.5

echo_green ""
echo_green "Dumping logs:"
echo_green ""

$GDB -batch "$ELF" \
    -ex "target extended-remote :3333" \
    -ex "bt" \
    -ex "echo \n" \
    -ex "printf \"%s\", logbuf" \
    -ex "mon resume" \
    -ex "mon shutdown"
