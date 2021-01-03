#!/bin/bash -e

if [ -z $2 ]; then
    echo "Usage: `basename $0` <hash file> <rom directory> [<rom directory>] ..."
    exit 1
fi

hashfile="$1"

update_needed=0

if [ ! -f $hashfile ]; then
    update_needed=1
else
    # Any file updated or removed?
	while read line; do
		file=`echo $line | sed 's/[^ ]* *//'`
		hash=`echo $line | sed 's/\([^ ]*\) .*/\1/'`

		if [ ! -f $file ]; then
			# File removed
			update_needed=1
		else
			h=`sha1sum $file | sed 's/\([^ ]*\) .*/\1/'`

			if [ "$h" != "$hash" ]; then
				# File updated
				update_needed=1
			fi
		fi
	done < "$hashfile"

    # Any new files?
    for dir in ${@:2}; do
        while read file; do
            grep $file\$ $hashfile > /dev/null || update_needed=1
        done <<< `find $dir -type f`
    done
fi

if [ $update_needed -ne 0 ]; then
    echo "Updating hash file"
    find ${@:2} -type f -exec sha1sum {} \; > $hashfile 2> /dev/null
fi
