#!/bin/bash
# Usage: ./extflash_size.sh app.elf

if [[ "${GCC_PATH}" != "" ]]; then
	DEFAULT_OBJDUMP=${GCC_PATH}/arm-none-eabi-objdump
else
	DEFAULT_OBJDUMP=arm-none-eabi-objdump
fi

OBJDUMP=${OBJDUMP:-$DEFAULT_OBJDUMP}

elf_file=$1

function get_symbol {
	name=$1
	objdump_cmd="$OBJDUMP -t $elf_file"
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
	usage=$(get_section_length $symbol)
	usagemb=$(printf "%.3f" "$(( ($usage * 1000000) / 1024 / 1024 ))e-6")
	length=$(get_symbol $length_symbol)
	lengthmb=$(printf "%.3f" "$(( ($length * 1000000) / 1024 / 1024 ))e-6")
	free=$(( $length - $usage ))
	freemb=$(printf "%.3f" "$(( ($free * 1000000) / 1024 / 1024 ))e-6")
	echo -e ""
	echo -e "External flash usage"
	printf  "    Capacity: %12d Bytes (%7.3f MB)\n" $length $lengthmb
	printf  "    Usage:    %12d Bytes (%7.3f MB)\n" $usage $usagemb
	printf  "    Free:     %12d Bytes (%7.3f MB)\n" $free $freemb
	echo -e ""
}

print_usage extflash __EXTFLASH_LENGTH__
