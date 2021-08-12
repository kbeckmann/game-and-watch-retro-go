#!/bin/bash
# Usage: ./size.sh app.elf

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
	length=$(get_symbol $length_symbol)
	free=$(( $length - $usage ))
	freehr=$(printf "%.3f" "$(( ($free * 1000000) / 1024 / 1024 ))e-6")
	echo -e "$symbol\t$usage / $length\t($free bytes free ($freehr MB))"
}

print_usage itcram   __ITCMRAM_LENGTH__

# DTCRAM is a special case
dtc_size=$(get_symbol __DTCMRAM_LENGTH__)
dtc_free=$(get_section_length dtc_padding)
dtc_usage=$(( dtc_size - dtc_free ))
echo -e "dtcram\t$dtc_usage / $dtc_size ($dtc_free bytes free)"

print_usage ram_uc   __RAM_UC_LENGTH__
print_usage ram      __RAM_CORE_LENGTH__
print_usage ram_emu_nes  __RAM_EMU_LENGTH__
print_usage ram_emu_gb  __RAM_EMU_LENGTH__
print_usage ram_emu_sms  __RAM_EMU_LENGTH__
print_usage ram_emu_pce  __RAM_EMU_LENGTH__
print_usage ram_emu_gw   __RAM_EMU_LENGTH__
print_usage ahbram   __AHBRAM_LENGTH__
print_usage flash    __FLASH_LENGTH__
print_usage extflash __EXTFLASH_LENGTH__
print_usage saveflash __SAVEFLASH_LENGTH__
