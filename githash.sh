#!/bin/bash

GITHASH=$(git rev-parse --short HEAD 2>/dev/null || echo "NOGIT")

echo -e "#ifndef GIT_HASH\n#define GIT_HASH \""${GITHASH}\""\n#endif"
