#!/bin/bash

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <gb_rom.gb> [gb_rom.c]"
    echo "This will convert the gb rom into a .c file and update all the references"
    exit 1
fi

INFILE=$1
OUTFILE=loaded_gb_rom.c

if [[ ! -e "$(dirname $OUTFILE)" ]]; then
    mkdir -p "$(dirname $OUTFILE)"
fi

if [[ $# -gt 1 ]]; then
    OUTFILE=$2
fi

SIZE=$(wc -c "$INFILE" | awk '{print $1}')

echo "const unsigned char ROM_DATA[] __attribute__((section (\".extflash_game_rom\"))) = {" > $OUTFILE
xxd -i < "$INFILE" >> $OUTFILE
echo "};" >> $OUTFILE
echo "unsigned int ROM_DATA_LENGTH = $SIZE;" >> $OUTFILE
echo "unsigned int cart_rom_len = $SIZE;" >> $OUTFILE

echo "Done!"
