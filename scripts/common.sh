#!/bin/bash

if [[ "$VERBOSE" == "1" ]]; then
    set -ex
else
    set -e
fi

if [[ "${GCC_PATH}" != "" ]]; then
    DEFAULT_OBJDUMP=${GCC_PATH}/arm-none-eabi-objdump
    DEFAULT_GDB=${GCC_PATH}/arm-none-eabi-gdb
else
    DEFAULT_OBJDUMP=arm-none-eabi-objdump
    DEFAULT_GDB=arm-none-eabi-gdb
fi

FLSHLD_DIR=${OBJDUMP:-../game-and-watch-flashloader}
FLASHLOADER=${FLSHLD_DIR}/flash_multi.sh

OBJDUMP=${OBJDUMP:-$DEFAULT_OBJDUMP}
GDB=${GDB:-$DEFAULT_GDB}

ADAPTER=${ADAPTER:-stlink}
OPENOCD=${OPENOCD:-$(which openocd || true)}

RESET_DBGMCU=${RESET_DBGMCU:-1}

if [[ -z ${OPENOCD} ]]; then
  echo "Cannot find 'openocd' in the PATH. You can set the environment variable 'OPENOCD' to manually specify the location"
  exit 2
fi

function get_symbol {
    name=$1
    objdump_cmd="$OBJDUMP -t $ELF"
    size=$($objdump_cmd | grep " $name" | cut -d " " -f1 | tr 'a-f' 'A-F')
    printf "$((16#${size}))\n"
}

function get_number_of_saves {
    prefix=$1
    objdump_cmd="${OBJDUMP} -t ${ELF}"
    echo $(${objdump_cmd} | grep " $prefix" | wc -l)
}

function reset_and_disable_debug {
    if [[ "$RESET_DBGMCU" -eq 1 ]]; then
        ${OPENOCD} -f ${FLSHLD_DIR}/interface_${ADAPTER}.cfg -c "init; reset halt; mww 0x5C001004 0x00000000; resume; exit;"
    else
        ${OPENOCD} -f ${FLSHLD_DIR}/interface_${ADAPTER}.cfg -c "init; reset run; exit;"
    fi
}
