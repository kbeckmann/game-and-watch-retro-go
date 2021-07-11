#!/bin/bash

. ./scripts/common.sh

if [[ $# -lt 1 ]]; then
    echo "Usage: $(basename $0) <currently_running_binary.elf>"
    echo "This will erase all save states from the device"
    exit 1
fi

ELF="$1"

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

# Reset the device and disable clocks from running when device is suspended
reset_and_disable_debug

# Clean up
rm -f "${DUMMY_FILE}"

echo ""
echo ""
echo "Saves have been erased."
echo ""
echo ""
