#!/bin/bash

. ./scripts/common.sh

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

# Start processing

mkdir -p "$INDIR"

for emu in gb gg nes pce sms; do
    mkdir -p "${INDIR}/${emu}"
    COUNT=$(get_number_of_saves SAVE_$(echo ${emu} | awk '{print toupper($0)}')_)
    for (( i = 0; i < COUNT; i++ )); do
        name=$(${GDB} "${ELF}" --batch -q -ex "printf \"%s\n\", ${emu}_roms[${i}].name")
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

# Reset the device and disable clocks from running when device is suspended
reset_and_disable_debug
