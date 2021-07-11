#!/bin/bash

if [[ "$VERBOSE" == "1" ]]; then
    set -ex
else
    set -e
fi

if [[ "${GCC_PATH}" != "" ]]; then
    DEFAULT_OBJDUMP=${GCC_PATH}/arm-none-eabi-objdump
else
    DEFAULT_OBJDUMP=arm-none-eabi-objdump
fi

ELF=$1

FLSHLD_DIR=${OBJDUMP:-../game-and-watch-flashloader}
FLASHLOADER=${FLSHLD_DIR}/flash_multi.sh

OBJDUMP=${OBJDUMP:-$DEFAULT_OBJDUMP}

ADAPTER=${ADAPTER:-stlink}
OPENOCD=${OPENOCD:-$(which openocd || true)}

if [[ -z ${OPENOCD} ]]; then
  echo "Cannot find 'openocd' in the PATH. You can set the environment variable 'OPENOCD' to manually specify the location"
  exit 2
fi

if [[ $# -lt 1 ]]; then
    echo "Usage: $(basename $0) <currently_running_binary.elf>"
    echo "This will erase all save states from the device"
    exit 1
fi

function get_symbol {
    name=$1
    objdump_cmd="$OBJDUMP -t $ELF"
    size=$($objdump_cmd | grep " $name" | cut -d " " -f1 | tr 'a-f' 'A-F')
    printf "$((16#${size}))\n"
}

saveflash_start=$(get_symbol __SAVEFLASH_START__)
saveflash_size=$(get_symbol __SAVEFLASH_LENGTH__)

DUMMY_FILE=$(mktemp /tmp/retro_go_dummy.XXXXXX)
if [[ ! -e "${DUMMY_FILE}" ]]; then
    echo "Can't create tempfile!"
    exit 1
fi

# Create dummy file with 0xFF of the size saveflash_size
/usr/bin/env python3 -c "with open('${DUMMY_FILE}', 'wb') as f: f.write(b'\xFF'*${saveflash_size})"

# Flash it to the saveflash_start
${FLASHLOADER} "${DUMMY_FILE}" $(( saveflash_start - 0x90000000 ))

# Reset the device
${OPENOCD} -f ${FLSHLD_DIR}/interface_${ADAPTER}.cfg -c "init; halt; reset run; exit;"

# Clean up
rm -f "${DUMMY_FILE}"

echo ""
echo ""
echo "Saves have been erased."
echo ""
echo ""
