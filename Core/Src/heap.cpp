#pragma GCC optimize("O0")

extern "C" {

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gw_linker.h"

}

#include <cstddef>

size_t ram_end = (size_t) &__RAM_END__;
size_t snes_bss_start = (size_t) &_OVERLAY_SNES_BSS_START;
size_t snes_bss_end = (size_t) &_OVERLAY_SNES_BSS_END;

size_t heapsize = ram_end - snes_bss_end;
uint8_t *badheap = (uint8_t *) ((snes_bss_end + 0b1111) & ~0b1111);
size_t heap_offset;

#define DBG(...) printf(__VA_ARGS__)

void *operator new[](std::size_t s)
{
    void *ptr = &badheap[heap_offset];

    DBG("new[] %d\n", s);
    heap_offset += (s + 0b11) & ~0b11; // 32-bit
    assert(heap_offset <= heapsize);

    memset(ptr, 0, s);

    return ptr;
}

void *operator new(std::size_t s)
{
    void *ptr = &badheap[heap_offset];

    DBG("new %d\n", s);
    heap_offset += (s + 0b11) & ~0b11; // 32-bit
    assert(heap_offset <= heapsize);

    memset(ptr, 0, s);

    return ptr;
}

void operator delete(void *p)
{
    DBG("d 0x%08x\n", p);
}

void operator delete[](void *p)
{
    DBG("d[] 0x%08x\n", p);
}

extern "C" void cpp_heap_init(void)
{
    heap_offset = 0;
    heapsize = ram_end - snes_bss_end;
    badheap = (uint8_t *) ((snes_bss_end + 0b1111) & ~0b1111);
}
