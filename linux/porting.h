#pragma once

#define IEXTFLASH_ATTR
#define DEXTFLASH_ATTR

#define IRAM_ATTR
#define DRAM_ATTR

#define rg_alloc(x, y) malloc(x)
#define rg_free(x) free(x)



#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <malloc.h>
#include <math.h>
#include <limits.h>


uint32_t HAL_GetTick(void);
