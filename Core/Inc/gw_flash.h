#ifndef _GW_FLASH_H_
#define _GW_FLASH_H_

#include "stm32h7xx_hal.h"


typedef enum {
    SPI_MODE  = 0x00,
    QUAD_MODE = 0x01,
    HALF_QUAD_MODE = 0x02 // almost quad as used on the MX25U8035F
} quad_mode_t;

void OSPI_Init(OSPI_HandleTypeDef *hospi, quad_mode_t quad_mode);
void OSPI_EnableMemoryMappedMode(OSPI_HandleTypeDef *hospi1);
void OSPI_DisableMemoryMapped(OSPI_HandleTypeDef *hospi);
void OSPI_Read(OSPI_HandleTypeDef *hospi, uint32_t address, uint8_t *buffer, size_t buffer_size);
void OSPI_NOR_WriteEnable(OSPI_HandleTypeDef *hospi);
void OSPI_ChipErase(OSPI_HandleTypeDef *hospi);
void OSPI_Program(OSPI_HandleTypeDef *hospi, uint32_t address, const uint8_t *buffer, size_t buffer_size);
void OSPI_BlockErase(OSPI_HandleTypeDef *hospi, uint32_t address);

#endif
