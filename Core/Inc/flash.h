#ifndef _FLASH_H_
#define _FLASH_H_

#include "stm32h7xx_hal.h"

typedef enum {
    SPI_MODE  = 0x00,
    QUAD_MODE = 0x01,
} quad_mode_t;

typedef enum {
    VENDOR_MX   = 0x00, // MX25U8035F, Nintendo Stock Flash
    VENDOR_ISSI = 0x01, // IS25WP128F, 128Mb large flash
} spi_chip_vendor_t;

void OSPI_Init(OSPI_HandleTypeDef *hospi, quad_mode_t quad_mode, spi_chip_vendor_t vendor);
void OSPI_EnableMemoryMappedMode(OSPI_HandleTypeDef *hospi1);


#endif
