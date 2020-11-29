#!/bin/bash

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <nes_rom.nes> [nes_rom.c]"
    echo "This will convert the nes rom into a .c file and update all the references"
    exit 1
fi

INFILE=$1
OUTFILE=roms/nes/loaded_nes_rom.c

if [[ ! -e "$(dirname $OUTFILE)" ]]; then
    mkdir -p "$(dirname $OUTFILE)"
fi

if [[ $# -gt 1 ]]; then
    OUTFILE=$2
fi

SIZE=$(stat -c%s "$INFILE")

echo "unsigned char cart_rom[] __attribute__((section (\".extflash_game_rom\"))) = {" > $OUTFILE
xxd -i < "$INFILE" >> $OUTFILE
echo "};" >> $OUTFILE
echo "unsigned int cart_rom_len = $SIZE;" >> $OUTFILE

echo "#define ROM_LENGTH $SIZE" > Core/Inc/rom_info.h

echo "Done!"
