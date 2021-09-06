#include "lzma.h"


static void *SzAlloc(ISzAllocPtr p, size_t size) {
    void* res = p->Mem;
    return res;
}

static void SzFree(ISzAllocPtr p, void *address) {
}

static const ISzAlloc g_Alloc = { SzAlloc, SzFree };

size_t lzma_inflate(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size){
    unsigned char lzma_heap[LZMA_BUF_SIZE];
    ISzAlloc allocs = {
        .Alloc=SzAlloc,
        .Free=SzFree,
        .Mem=lzma_heap,
    };

    ELzmaStatus lzmaStatus;
    src_size -= 13;
    LzmaDecode(dst, &dst_size, &src[13], &src_size, src, 5, LZMA_FINISH_ANY, &lzmaStatus, &allocs);
    return dst_size;
}
