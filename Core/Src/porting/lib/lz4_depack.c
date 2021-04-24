//#include <string.h>
#include <stdlib.h>
//#include <stdio.h>
//#include <assert.h>

#include "lz4_depack.h"

/*********************************/
/*
 * blz4 - Example of LZ4 compression with BriefLZ algorithms
 *
 * C depacker
 *
 * Copyright (c) 2018 Joergen Ibsen
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must
 *      not claim that you wrote the original software. If you use this
 *      software in a product, an acknowledgment in the product
 *      documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must
 *      not be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any source
 *      distribution.
 */

unsigned long
lz4_depack(const void *src, void *dst, unsigned long packed_size)
{
	const unsigned char *in = (unsigned char *) src;
	unsigned char *out = (unsigned char *) dst;
	unsigned long dst_size = 0;
	unsigned long cur = 0;
	unsigned long prev_match_start = 0;

	if (in[0] == 0) {
		return 0;
	}

	/* Main decompression loop */
	while (cur < packed_size) {
		unsigned long token = in[cur++];
		unsigned long lit_len = token >> 4;
		unsigned long len = (token & 0x0F) + 4;
		unsigned long offs;
		unsigned long i;

		/* Read extra literal length bytes */
		if (lit_len == 15) {
			while (in[cur] == 255) {
				lit_len += 255;
				++cur;
			}
			lit_len += in[cur++];
		}

		/* Copy literals */
		for (i = 0; i < lit_len; ++i) {
			out[dst_size++] = in[cur++];
		}

		/* Check for last incomplete sequence */
		if (cur == packed_size) {
			/* Check parsing restrictions */
			if (dst_size >= 5 && lit_len < 5) {
				return 0;
			}

			if (dst_size > 12 && dst_size - prev_match_start < 12) {
				return 0;
			}

			break;
		}

		/* Read offset */
		offs = (unsigned long) in[cur] | ((unsigned long) in[cur + 1] << 8);
		cur += 2;

		/* Read extra length bytes */
		if (len == 19) {
			while (in[cur] == 255) {
				len += 255;
				++cur;
			}
			len += in[cur++];
		}

		prev_match_start = dst_size;

		/* Copy match */
		for (i = 0; i < len; ++i) {
			out[dst_size] = out[dst_size - offs];
			++dst_size;
		}
	}

	/* Return decompressed size */
	return dst_size;
}