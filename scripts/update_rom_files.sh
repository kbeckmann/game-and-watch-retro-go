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

find ${@:2} -type f | sort > "${TMPFILE}" 2> /dev/null

if ! diff -q ${TMPFILE} ${filelist} > /dev/null 2> /dev/null; then
    echo "Updating file list ${filelist}"
	cp -f "${TMPFILE}" "${filelist}"
fi

rm -f "${TMPFILE}"
