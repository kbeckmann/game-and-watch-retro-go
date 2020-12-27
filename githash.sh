#!/bin/bash

GITHASH=$(git describe --always --dirty 2> /dev/null || echo "NOGIT")

echo -e "#ifndef GIT_HASH\n#define GIT_HASH \""${GITHASH}\""\n#endif"
