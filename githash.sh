#!/bin/bash
echo -ne '#ifndef GIT_HASH\n#define GIT_HASH "'

GITHASH=$(git describe --always --dirty | tr -d "\n")

if [[ "$GITHASH" == "" ]]; then
    echo "nogit"
fi

echo -ne '"\n#endif'