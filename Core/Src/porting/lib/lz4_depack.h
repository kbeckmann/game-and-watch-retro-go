#ifndef DEF_LZ4DEPACK
#define DEF_LZ4DEPACK

/* MAGIC WORD found as a magic word in LZ4 payload */
#define  ROM_LZ4_MAGIC       "\x04\x22\x4D\x18" // LE 0x184D2204U

/* LZ4 depack function */
unsigned long lz4_depack(const void *src, void *dst, unsigned long packed_size);

#endif /* _LZ4DEPACK */