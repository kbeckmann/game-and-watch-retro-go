Decompression code from the official [LZMA SDK 19.00](https://www.7-zip.org/sdk.html)

`7zTypes.h` is slightly modified so that the alloc struct uses the stack instead of
global/heap.

`lzma.c` and `lzma.h` are novel code to simplify the API.
