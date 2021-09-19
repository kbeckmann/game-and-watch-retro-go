/**
 * Simplifies lzma calling.
 */
#pragma once

#include "LzmaDec.h"
#include <stdint.h>

#define LZMA_BUF_SIZE    16256

extern const uint8_t lzma_prop_data[5];

void lzma_init_allocs(ISzAlloc *allocs, uint8_t *heap);

size_t lzma_inflate(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size);
