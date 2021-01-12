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
FLASHLOADER=${FLASHLOADER:-${FLSHLD_DIR}/flash_multi.sh}

OBJDUMP=${OBJDUMP:-$DEFAULT_OBJDUMP}
GDB=${GDB:-$DEFAULT_GDB}

ADAPTER=${ADAPTER:-stlink}
OPENOCD=${OPENOCD:-$(which openocd || true)}

if [[ -z ${OPENOCD} ]]; then
  echo "Cannot find 'openocd' in the PATH. You can set the environment variable 'OPENOCD' to manually specify the location"
  exit 2
fi

if [[ $# -lt 1 ]]; then
    echo "Usage: $(basename $0) <currently_running_binary.elf> [backup directory]"
    echo "This will program all save states from the backup directory that match what is included in the elf file"
    exit 1
fi

ELF="$1"
INDIR=save_states

if [[ $# -gt 1 ]]; then
    INDIR="$2"
fi


function get_number_of_saves {
    prefix=$1
    objdump_cmd="${OBJDUMP} -t ${ELF}"
    echo $(${objdump_cmd} | grep " $prefix" | wc -l)
}


# Start processing

mkdir -p "$INDIR"

for emu in gb nes gg sms; do
    mkdir -p "${INDIR}/${emu}"
    COUNT=$(get_number_of_saves SAVE_$(echo ${emu} | awk '{print toupper($0)}')_)
    for i in $(seq 0 $(( COUNT - 1 ))); do
        name=$(${GDB} "${ELF}" --batch -q -ex "printf \"%s\n\", ${emu}_roms[${i}].rom_name")
        # Note that 0x90000000 is subtracted from the address.
        address=$(${GDB} "${ELF}" --batch -q -ex "printf \"%ld\n\", ${emu}_roms[${i}].save_address - 0x90000000")
        size=$(${GDB} "${ELF}" --batch -q -ex "printf \"%d\n\", ${emu}_roms[${i}].save_size")
        image="${INDIR}/${emu}/${name}.save"

        if [[ -e "$image" ]]; then
            echo ""
            echo ""
            echo "Programming save for:"
            echo "    rom_name=\"$name\""
            echo "    save_address=$address"
            echo "    save_size=$size"
            echo ""
            echo ""
            ${FLASHLOADER} "${image}" ${address} ${size}
        else
            echo ""
            echo ""
            echo "Skipping: (missing save file in backup directory)"
            echo "    rom_name=\"$name\""
            echo "    save_address=$address"
            echo "    save_size=$size"
            echo ""
            echo ""
        fi
    done
done

# Reset the device
${OPENOCD} -f ${FLSHLD_DIR}/interface_${ADAPTER}.cfg -c "init; halt; reset run; exit;"
