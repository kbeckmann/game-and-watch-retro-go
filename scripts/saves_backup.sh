#!/bin/bash

. ./scripts/common.sh

if [[ $# -lt 1 ]]; then
    echo "Usage: $(basename $0) <currently_running_binary.elf> [backup directory]"
    echo "This will dump all save states from the device to the backup directory"
    exit 1
fi

ELF="$1"
OUTDIR=save_states

if [[ $# -gt 1 ]]; then
    OUTDIR="$2"
fi

# Start processing

mkdir -p "$OUTDIR"

for emu in gb gg nes pce sms; do
    mkdir -p "${OUTDIR}/${emu}"
    COUNT=$(get_number_of_saves SAVE_$(echo ${emu} | awk '{print toupper($0)}')_)
    for i in $(seq 0 $(( COUNT - 1 ))); do
        name=$(${GDB} "${ELF}" --batch -q -ex "printf \"%s\n\", ${emu}_roms[${i}].name")
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
