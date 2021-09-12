#!/bin/bash -e

if [ -z $2 ]; then
	echo "Usage: $(basename $0) <file list> <rom directory> [<rom directory>] ..."
	exit 1
fi

filelist="$1"

TMPFILE=$(mktemp build/hashes.XXXXXX)
if [[ ! -e $TMPFILE ]]; then
	echo "Can't create tempfile!"
	exit 1
fi

find ${@:2} -type f \( -iname \*.gb -o -iname \*.gbc -o -iname \*.nes -o -iname \*.gw -o -iname \*.sms -o -iname \*.gg -o -iname \*.sg -o -iname \*.col -o -iname \*.pce -o -iname \*.png -o -iname \*.jpg -o -iname \*.bmp \) | sort > "${TMPFILE}" 2> /dev/null
#find ${@:2} -type f | sort > "${TMPFILE}" 2> /dev/null

if ! diff -q ${TMPFILE} ${filelist} > /dev/null 2> /dev/null; then
    echo "Updating file list ${filelist}"
	cp -f "${TMPFILE}" "${filelist}"
fi

rm -f "${TMPFILE}"
