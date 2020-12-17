#pragma once

#include "stm32h7xx_hal.h"


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

#define IEXTFLASH_ATTR
#define DEXTFLASH_ATTR 


#   define IRAM_ATTR
#   define DRAM_ATTR


#ifndef DEBUG_RG_ALLOC

#define rg_alloc(x, y) malloc(x)
#define rg_free(x) free(x)

#endif