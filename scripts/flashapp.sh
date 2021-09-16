#!/bin/bash

. ./scripts/common.sh

if [[ "$VERBOSE" == "1" ]]; then
    set -ex
else
    set -e
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ELF=${DIR}/../build/gw_retro_go.elf
ADDRESS=0

BOOT_MAGIC_FLASHAPP="0xf1a5f1a5"

FLASHAPP_IDLE="00000001"
FLASHAPP_TEST_NEXT="0000000b"
FLASHAPP_FINAL="0000000d"
FLASHAPP_ERROR="0000000e"

STATUS_BAD_HASH_RAM="bad00001"
STATUS_BAD_HAS_FLASH="bad00002"
STATUS_NOT_ALIGNED="bad00003"
STATUS_IDLE="cafe0000"
STATUS_DONE="cafe0001"
STATUS_BUSY="cafe0002"

VAR_boot_magic=$(              printf '0x%08x\n' $(get_symbol "boot_magic"))
VAR_framebuffer2=$(            printf '0x%08x\n' $(get_symbol "framebuffer2"))
VAR_flashapp_state=$(          printf '0x%08x\n' $(get_symbol "flashapp_state"))
VAR_program_start=$(           printf '0x%08x\n' $(get_symbol "program_start"))
VAR_program_status=$(          printf '0x%08x\n' $(get_symbol "program_status"))
VAR_program_size=$(            printf '0x%08x\n' $(get_symbol "program_size"))
VAR_program_address=$(         printf '0x%08x\n' $(get_symbol "program_address"))
VAR_program_erase=$(           printf '0x%08x\n' $(get_symbol "program_erase"))
VAR_program_erase_bytes=$(     printf '0x%08x\n' $(get_symbol "program_erase_bytes"))
VAR_program_chunk_idx=$(       printf '0x%08x\n' $(get_symbol "program_chunk_idx"))
VAR_program_chunk_count=$(     printf '0x%08x\n' $(get_symbol "program_chunk_count"))
VAR_program_expected_sha256=$( printf '0x%08x\n' $(get_symbol "program_expected_sha256"))

INTFLASH_BANK=${INTFLASH_BANK:-1}
if [ $INTFLASH_BANK -eq 2 ]; then
    INTFLASH_ADDRESS=0x08100000
else
    INTFLASH_ADDRESS=0x08000000
fi

# $1: file to hash
# $2: file to write hash to in hex
function calc_sha256sum() {
    SHA256SUM=${SHA256SUM:-$(which sha256sum || true)}
    OPENSSL=${OPENSSL:-$(which openssl || true)}

    if [[ ! -z "${SHA256SUM}" ]]; then
        ${SHA256SUM} "$1" | cut -d " " -f1 > "$2"
    elif [[ ! -z "${OPENSSL}" ]]; then
        ${OPENSSL} sha256 "$1" | cut -d " " -f2 > "$2"
    else
        echo "Cannot find 'sha256sum' or 'openssl' in the PATH. You can set the environment variables 'SHA256SUM' or 'OPENSSL' to manually specify the location to either tool. Only one of the tools are needed."
        exit 2
    fi
}

function read_word() {
    ${OPENOCD} -f ${DIR}/interface_${ADAPTER}.cfg -c "init; mdw $1" -c "exit;" 2>&1 | grep $1 | cut -d" " -f2
}

function state_to_string() {
    if   [[ "$1" == "00000000" ]]; then echo "FLASHAPP_INIT"
    elif [[ "$1" == "00000001" ]]; then echo "FLASHAPP_IDLE"
    elif [[ "$1" == "00000002" ]]; then echo "FLASHAPP_START"
    elif [[ "$1" == "00000003" ]]; then echo "FLASHAPP_CHECK_HASH_RAM_NEXT"
    elif [[ "$1" == "00000004" ]]; then echo "FLASHAPP_CHECK_HASH_RAM"
    elif [[ "$1" == "00000005" ]]; then echo "FLASHAPP_ERASE_NEXT"
    elif [[ "$1" == "00000006" ]]; then echo "FLASHAPP_ERASE"
    elif [[ "$1" == "00000007" ]]; then echo "FLASHAPP_PROGRAM_NEXT"
    elif [[ "$1" == "00000008" ]]; then echo "FLASHAPP_PROGRAM"
    elif [[ "$1" == "00000009" ]]; then echo "FLASHAPP_CHECK_HASH_FLASH_NEXT"
    elif [[ "$1" == "0000000a" ]]; then echo "FLASHAPP_CHECK_HASH_FLASH"
    elif [[ "$1" == "0000000b" ]]; then echo "FLASHAPP_TEST_NEXT"
    elif [[ "$1" == "0000000c" ]]; then echo "FLASHAPP_TEST"
    elif [[ "$1" == "0000000d" ]]; then echo "FLASHAPP_FINAL"
    elif [[ "$1" == "0000000e" ]]; then echo "FLASHAPP_ERROR"
    else echo "UNKNOWN"
    fi
}

function wait_for_idle() {
    # Wait for the idle state
    while true; do
        STATE_REG=$(read_word ${VAR_flashapp_state})
        if [[ "$STATE_REG" == "$FLASHAPP_IDLE" ]]; then
            echo "Ready!"
            break;
        else
            echo "State: $(state_to_string $STATE_REG)"
        fi
        sleep 1
    done
}

if [[ $# -lt 1 ]]; then
    echo "Usage: flashapp.sh <binary to flash> [address in flash] [size] [erase=1] [erase_bytes=0] [chunk_idx] [chunk_count]"
    echo "       flashapp.sh --test"
    echo "Note! Destination address must be aligned to 256 bytes."
    echo "'address in flash': Where to program to. 0x000000 is the start of the flash. "
    echo "'size': Size of the binary to flash. Should be aligned to 256 bytes."
    echo "'erase': If '0', chip erase will be skipped. Default '1'."
    echo "'erase_bytes': Number of bytes to erase, all if '0'. Default '0'."
    echo "--test: Performs a erase/write/read test"
    exit
fi

IMAGE=$1

if [[ ${IMAGE} == "--test" ]]; then
    # Run test and exit
    echo "FLASH TEST START"

    ${OPENOCD} -f ${DIR}/interface_${ADAPTER}.cfg \
        -c "init; reset halt;" \
        -c "mww ${VAR_boot_magic} ${BOOT_MAGIC_FLASHAPP}" \
        -c "resume;" \
        -c "exit;"

    wait_for_idle

    ${OPENOCD} -f ${DIR}/interface_${ADAPTER}.cfg \
    -c "init; halt;" \
    -c "echo \"Starting flash test\";" \
    -c "mww ${VAR_program_start} 2" \
    -c "resume; exit;"

    exit 0
fi

if [[ $# -gt 1 ]]; then
    ADDRESS=$2
fi

if [[ $# -gt 2 ]]; then
    SIZE=$3
else
    SIZE=$(( 512 * 1024 ))
fi

ERASE=1
if [[ $# -gt 3 ]]; then
    ERASE=$4
fi

ERASE_BYTES=0
if [[ $# -gt 4 ]]; then
    ERASE_BYTES=$5
fi

CHUNK_IDX=1
CHUNK_COUNT=1
if [[ $# -gt 6 ]]; then
    CHUNK_IDX=$6
    CHUNK_COUNT=$7
fi

HASH_HEX_FILE=$(mktemp /tmp/sha256_hash_hex.XXXXXX)
if [[ ! -e "${HASH_HEX_FILE}" ]]; then
    echo "Can't create tempfile!"
    exit 1
fi

HASH_FILE=$(mktemp /tmp/sha256_hash.XXXXXX)
if [[ ! -e "${HASH_FILE}" ]]; then
    echo "Can't create tempfile!"
    exit 1
fi
dd if="${IMAGE}" of="${HASH_FILE}" bs=1 count=$(( SIZE )) 2> /dev/null
calc_sha256sum "${HASH_FILE}" "${HASH_HEX_FILE}"
rm -f "${HASH_FILE}"

if [[ ${CHUNK_IDX} -eq "1" ]]; then
    ${OPENOCD} -f ${DIR}/interface_${ADAPTER}.cfg \
        -c "init; reset halt;" \
        -c "set MSP 0x[string range [mdw $INTFLASH_ADDRESS] 12 19]" \
        -c "set PC 0x[string range [mdw [format 0x%x [expr {$INTFLASH_ADDRESS + 0x4}]]] 12 19]" \
        -c 'reg msp $MSP' \
        -c 'reg pc $PC' \
        -c "mww ${VAR_boot_magic} ${BOOT_MAGIC_FLASHAPP}" \
        -c "mww ${VAR_program_chunk_idx} ${CHUNK_IDX}" \
        -c "mww ${VAR_program_chunk_count} ${CHUNK_COUNT}" \
        -c "echo \"Starting flash app\";" \
        -c "resume;" \
        -c "exit;"
fi

wait_for_idle

echo "Loading data"
    ${OPENOCD} -f ${DIR}/interface_${ADAPTER}.cfg \
    -c "init; halt;" \
    -c "echo \"Loading image into RAM\";" \
    -c "load_image ${IMAGE} ${VAR_framebuffer2};" \
    -c "mww ${VAR_program_size} ${SIZE}" \
    -c "mww ${VAR_program_address} ${ADDRESS}" \
    -c "mww ${VAR_program_erase} ${ERASE}" \
    -c "mww ${VAR_program_erase_bytes} ${ERASE_BYTES}" \
    -c "mww ${VAR_program_chunk_idx} ${CHUNK_IDX}" \
    -c "mww ${VAR_program_chunk_count} ${CHUNK_COUNT}" \
    -c "load_image ${HASH_HEX_FILE} ${VAR_program_expected_sha256};" \
    -c "echo \"Starting flash process\";" \
    -c "mww ${VAR_program_start} 1" \
    -c "resume; exit;"

# Remove the temporary hash files
rm -f "${HASH_HEX_FILE}"

echo "Please see the LCD for interactive status."

# Wait for the final or error state
while true; do
    STATE_REG=$(read_word ${VAR_flashapp_state})
    if [[ "$STATE_REG" == "$FLASHAPP_FINAL" ]]; then
        echo_green "Done!"
        exit 0
    elif [[ "$STATE_REG" == "$FLASHAPP_IDLE" ]]; then
        echo_green "Done, more chunks left!"
        exit 0
    elif [[ "$STATE_REG" == "$FLASHAPP_ERROR" ]]; then
        STATUS_REG=$(read_word ${VAR_program_status})
        if [[ "$STATUS_REG" == "$STATUS_BAD_HASH_RAM" ]]; then
            echo_red "Hash mismatch in RAM. Flashing failed."
            exit 3
        elif [[ "$STATUS_REG" == "$STATUS_BAD_HAS_FLASH" ]]; then
            echo_red "Hash mismatch in FLASH. Flashing failed."
            exit 3
        elif [[ "$STATUS_REG" == "$STATUS_NOT_ALIGNED" ]]; then
            echo_red "Address not 4k aligned. Flashing failed."
            exit 4
        else
            echo_red "Unknown error. Flashing failed. Status: $STATUS_REG"
            exit 5
        fi
    else
        echo "State: $(state_to_string $STATE_REG)"
    fi
    sleep 1
done
