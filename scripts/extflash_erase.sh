#!/bin/bash
set -x

. ./scripts/common.sh

if [[ $# -lt 1 ]]; then
    echo "Usage: $(basename $0) <EXTFLASH_SIZE>"
    echo "This will erase the whole extflash"
    exit 1
fi

EXTFLASH_SIZE=$1

DUMMY_FILE=$(mktemp /tmp/retro_go_dummy.XXXXXX)
if [[ ! -e "${DUMMY_FILE}" ]]; then
    echo "Can't create tempfile!"
    exit 1
fi

# Create dummy file with 0xFF of the size extflash
/usr/bin/env python3 -c "with open('${DUMMY_FILE}', 'wb') as f: f.write(b'\xFF'*${EXTFLASH_SIZE})"

# Flash it to start of the extflash
${FLASHAPP} "${DUMMY_FILE}"  0

# Reset the device and disable clocks from running when device is suspended
reset_and_disable_debug

# Clean up
rm -f "${DUMMY_FILE}"

echo ""
echo ""
echo "Extflash have been erased."
echo ""
echo ""
