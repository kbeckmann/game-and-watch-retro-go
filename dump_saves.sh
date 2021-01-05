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

OBJDUMP=${OBJDUMP:-$DEFAULT_OBJDUMP}
GDB=${GDB:-$DEFAULT_GDB}

ADAPTER=${ADAPTER:-stlink}
OPENOCD=${OPENOCD:-$(which openocd || true)}

if [[ -z ${OPENOCD} ]]; then
  echo "Cannot find 'openocd' in the PATH. You can set the environment variable 'OPENOCD' to manually specify the location"
  exit 2
fi

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <currently_running_binary.elf> [backup directory]"
    echo "This will dump all save states from the device to the backup directory"
    exit 1
fi

ELF=$1
OUTDIR=save_states

if [[ $# -gt 1 ]]; then
    OUTDIR="$2"
fi


function get_number_of_saves {
    prefix=$1
    objdump_cmd="${OBJDUMP} -t ${ELF}"
    echo $(${objdump_cmd} | grep " $prefix" | wc -l)
}

# Start processing

mkdir -p "$OUTDIR"

for emu in nes gb; do
    mkdir -p "${OUTDIR}/${emu}"
    COUNT=$(get_number_of_saves SAVE_${emu^^}_)
    for i in $(seq 0 $(( COUNT - 1 ))); do
        name=$(${GDB} "${ELF}" --batch -q -ex "printf \"%s\n\", ${emu}_roms[${i}].rom_name")
        address=$(${GDB} "${ELF}" --batch -q -ex "printf \"0x%08x\n\", ${emu}_roms[${i}].save_address")
        size=$(${GDB} "${ELF}" --batch -q -ex "printf \"0x%08x\n\", ${emu}_roms[${i}].save_size")
        echo ""
        echo ""
        echo "Dumping save data for:"
        echo "    rom_name=\"$name\""
        echo "    save_address=$address"
        echo "    save_size=$size"
        echo ""
        echo ""
        image="${OUTDIR}/${emu}/${name}.save"
        # openocd does not handle [ and ] well in filenames.
        image_quoted=${image//\[/\\[}
        image_quoted=${image_quoted//\]/\\]}
        ${OPENOCD} -f ${FLSHLD_DIR}/interface_${ADAPTER}.cfg -c "init; halt; dump_image \"${image_quoted}\" ${address} ${size}; resume; exit;"
    done
done
