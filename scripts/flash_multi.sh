#!/bin/bash

. ./scripts/common.sh

if [[ "$VERBOSE" == "1" ]]; then
    set -ex
else
    set -e
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ELF=${DIR}/build/gw_base.elf

if [[ $# -lt 1 ]]; then
    echo "Usage: flash_multi.sh <binary to flash> [address=0] [chip_erase=0]"
    echo ""
    echo "address:    The address to start writing to in the flash. Default is 0."
    echo "chip_erase: Forces the use of chip erase. Will erase the whole chip,"
    echo "            but may be faster for large flash chips."
    echo ""
    echo "Note! This will cut the binary in 832kB chunks and flash them to address and onwards"
    exit
fi

IMAGE="$1"
ADDRESS=0
if [[ $# -gt 1 ]]; then
    ADDRESS=$2
fi

CHIP_ERASE=0
if [[ $# -gt 2 ]]; then
    CHIP_ERASE=$3
fi

# stat on macOS has different flags
if [[ "$(uname -s)" == "Darwin" ]]; then
    FILESIZE=$(stat -f%z "${IMAGE}")
else 
    FILESIZE=$(stat -c%s "${IMAGE}")
fi

DEFAULT_CHUNK_SIZE_KB=$(( 832 ))
DEFAULT_CHUNK_SIZE=$(( DEFAULT_CHUNK_SIZE_KB * 1024 ))
CHUNKS=$(( (FILESIZE + DEFAULT_CHUNK_SIZE - 1) / (DEFAULT_CHUNK_SIZE) ))
SIZE=$((FILESIZE))
SECTOR_SIZE=$(( 4 * 1024 ))
ERASE_BYTES=0

ERASE=1
i=0
while [[ $SIZE -gt 0 ]]; do
    ADDRESS_HEX=$(printf "0x%08x" $(( ADDRESS + i * DEFAULT_CHUNK_SIZE )))
    if [[ $SIZE -gt $(( DEFAULT_CHUNK_SIZE )) ]]; then
        CHUNK_SIZE=$(( DEFAULT_CHUNK_SIZE ))
    else
        CHUNK_SIZE=${SIZE}
    fi
    SIZE_HEX=$(printf "0x%08x" ${CHUNK_SIZE})

    TMPFILE=$(mktemp /tmp/flash_chunk.XXXXXX)
    if [[ ! -e $TMPFILE ]]; then
        echo "Can't create tempfile!"
        exit 1
    fi

    echo_green "Preparing chunk $((i + 1)) / ${CHUNKS} in file ${TMPFILE}"
    dd if="${IMAGE}" of="${TMPFILE}" bs=1024 count=$(( (CHUNK_SIZE + 1023) / 1024 )) skip=$(( i * DEFAULT_CHUNK_SIZE_KB )) 2> /dev/null

    if [[ $CHUNK_SIZE -le 8 ]]; then
        echo "Chunk size <= 8 bytes, padding with zeros"
        dd if=/dev/zero of=${TMPFILE} bs=1 count=$(( 9 - CHUNK_SIZE )) seek=${CHUNK_SIZE} 2> /dev/null
    fi

    echo_green "Flashing!"
    if [[ $CHIP_ERASE == 1 ]]; then
        ERASE_BYTES=0
    else
        ERASE_BYTES=$(( (( SIZE + SECTOR_SIZE - 1 ) / SECTOR_SIZE) * SECTOR_SIZE ))
    fi

    # Try to flash 10 times, give up after that.
    COUNT=10
    for RETRY_COUNT in $(seq $COUNT); do
        if [[ $RETRY_COUNT -gt 1 ]]; then
            echo_red "Flashing chunk $i failed... power cycle unit and retry? (y/n)"
            read -n 1 -r
            if [[ ! $REPLY =~ ^[Yy]$ ]]; then
                echo "Aborted."
                exit 1
            fi

            echo ""
            echo "Retry count $RETRY_COUNT/10"
        fi

        ${DIR}/flashapp.sh ${TMPFILE} ${ADDRESS_HEX} ${SIZE_HEX} ${ERASE} ${ERASE_BYTES} $((i + 1)) ${CHUNKS} && break
    done

    if [[ $RETRY_COUNT -eq 10 ]]; then
        echo ""
        echo ""
        echo_red "Programming of the external flash FAILED after 3 tries."
        echo_red "Please check your debugger and wires connecting to the target."
        echo ""
        echo ""
        exit 1
    else 
        echo ""
        echo ""
        echo_green "Programming of chunk $((i + 1)) / ${CHUNKS} succeeded."
        echo ""
        echo ""
    fi

    # Skip erase the following iterations
    ERASE=0
    ERASE_BYTES=0

    rm -f ${TMPFILE}

    SIZE=$(( SIZE - CHUNK_SIZE ))
    i=$(( i + 1 ))
done

echo_green "Programming of the external flash succeeded."
echo ""
echo ""
