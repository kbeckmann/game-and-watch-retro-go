#include <stdlib.h>
#include <stdint.h>

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

void rg_free(void *_p)
{
    uint32_t *p = ((uint32_t *) _p) - 1;

    _total_alloc_bytes -= p[0];
    _total_alloc_bytes_actual -= p[0] + sizeof(uint32_t);

    printf("F %d %d %d %p\n", p[0], _total_alloc_bytes, _total_alloc_bytes_actual, p);

    free(p);
}

#endif
