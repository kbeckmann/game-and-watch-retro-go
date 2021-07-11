#!/bin/bash -e

if [ -z $1 ]; then
    echo "Usage: $(basename $0) <githash.h>"
    exit 1
fi

githashfile="$1"

TMPFILE=$(mktemp build/githash.XXXXXX)
if [[ ! -e $TMPFILE ]]; then
    echo "Can't create tempfile!"
    exit 1
fi

GITHASH=$(git describe --always --dirty 2> /dev/null || echo "NOGIT")

echo -e "#ifndef GIT_HASH\n#define GIT_HASH \""${GITHASH}\""\n#endif" > "${TMPFILE}"

if ! diff -q ${TMPFILE} ${githashfile} > /dev/null 2> /dev/null; then
    echo "Updating git hash file ${githashfile}"
    cp -f "${TMPFILE}" "${githashfile}"
fi

rm -f "${TMPFILE}"
