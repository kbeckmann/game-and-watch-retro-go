#include "lzma.h"


static void *SzAlloc(ISzAllocPtr p, size_t size) {
    void* res = p->Mem;
    return res;
}

static void SzFree(ISzAllocPtr p, void *address) {
}

static const ISzAlloc g_Alloc = { SzAlloc, SzFree };

size_t lzma_inflate(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size){
    const uint8_t prop_data[5] = {0x5d, 0x00, 0x40, 0x00, 0x00};
    unsigned char lzma_heap[LZMA_BUF_SIZE];
    ISzAlloc allocs = {
        .Alloc=SzAlloc,
        .Free=SzFree,
        .Mem=lzma_heap,
    };

    ELzmaStatus lzmaStatus;
    LzmaDecode(dst, &dst_size, src, &src_size, prop_data, 5, LZMA_FINISH_ANY, &lzmaStatus, &allocs);
    return dst_size;
}
