#!/bin/bash
# Usage: ./size.sh app.elf

objdump=${OBJDUMP:-arm-none-eabi-objdump}
elf_file=$1

function get_symbol {
	name=$1
	objdump_cmd="$objdump -t $elf_file"
	size=$($objdump_cmd | grep " $name" | cut -d " " -f1 | tr 'a-f' 'A-F')
	printf "$((16#${size}))\n"
}

function get_section_length {
	name=$1
	start=$(get_symbol "__${name}_start__")
	end=$(get_symbol "__${name}_end__")
	echo $(( $end - $start ))
}

function print_usage {
	symbol=$1
	length_symbol=$2
    whitespace1=$3
    whitespace2=$4
	usage=$(get_section_length $symbol)
	length=$(get_symbol $length_symbol)
	free=$(( $length - $usage ))
	echo -e "$symbol\t$usage / $length\t($free bytes free)"
}

print_usage itcram   __ITCMRAM_LENGTH__

# DTCRAM is a special case
dtc_size=$(get_symbol __DTCMRAM_LENGTH__)
dtc_free=$(get_section_length dtc_padding)
dtc_usage=$(( dtc_size - dtc_free ))
echo -e "dtcram\t$dtc_usage / $dtc_size ($dtc_free bytes free)"

print_usage ram      __RAM_LENGTH__
print_usage ahbram   __AHBRAM_LENGTH__
print_usage flash    __FLASH_LENGTH__
print_usage extflash __EXTFLASH_LENGTH__
