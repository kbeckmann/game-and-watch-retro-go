#!/bin/bash
echo -ne '#ifndef GIT_HASH\n#define GIT_HASH "'
git describe --always --dirty | tr -d "\n"
echo -ne '"\n#endif'