#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "gw_linker.h"


void *
_sbrk (incr)
     int incr;
{
    static char * heap_end;
    char *        prev_heap_end;

    if (heap_end == 0)
        heap_end = &_heap_start;

    assert((heap_end + incr) < (&_heap_end));

    prev_heap_end = heap_end;
    heap_end += incr;

    return (void *) prev_heap_end;
}

#ifdef DEBUG_RG_ALLOC

size_t   _total_alloc_bytes;
size_t   _total_alloc_bytes_actual;
uint32_t _total_alloc_num;

void *rg_alloc(size_t size, uint32_t caps)
{
    uint32_t *p = malloc(size + sizeof(uint32_t));

    _total_alloc_bytes += size;
    _total_alloc_bytes_actual += size + sizeof(uint32_t);

    printf("A %d %d %d %p\n", size, _total_alloc_bytes, _total_alloc_bytes_actual, p);

    p[0] = size;

    return &p[1];
}

void *rg_calloc(size_t nmemb, size_t size)
{
    uint8_t *p = rg_alloc(nmemb * size, 0);

    memset(p, '\x00', nmemb * size);

    return p;
}

void rg_free(void *ptr)
{
    assert(ptr != NULL);

    uint32_t *p = ((uint32_t *) ptr) - 1;

    _total_alloc_bytes -= p[0];
    _total_alloc_bytes_actual -= p[0] + sizeof(uint32_t);

    printf("F %d %d %d %p\n", p[0], _total_alloc_bytes, _total_alloc_bytes_actual, p);

    free(p);
}

void *rg_realloc(void *ptr, size_t size)
{
    if (ptr == NULL) {
        return rg_alloc(size, 0);
    }

    uint32_t *p = ((uint32_t *) ptr) - 1;

    size_t old_size = p[0];

    _total_alloc_bytes -= p[0];
    _total_alloc_bytes_actual -= p[0] + sizeof(uint32_t);

    _total_alloc_bytes += size;
    _total_alloc_bytes_actual += size + sizeof(uint32_t);

    printf("R %d=>%d %d %d %p\n", old_size, size, _total_alloc_bytes, _total_alloc_bytes_actual, p);

    return realloc(p, size);
}

#endif
