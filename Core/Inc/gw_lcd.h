#ifndef _LCD_H_
#define _LCD_H_

#include "stm32h7xx_hal.h"
#include <stdint.h>

#ifdef GW_LCD_MODE_LUT8
extern uint8_t framebuffer1[320 * 240]  __attribute__((section (".lcd")));
extern uint8_t framebuffer2[320 * 240]  __attribute__((section (".lcd")));
#else
extern uint16_t framebuffer1[320 * 240]  __attribute__((section (".lcd")));
extern uint16_t framebuffer2[320 * 240]  __attribute__((section (".lcd")));
#endif // GW_LCD_MODE_LUT8

#define GFX_MAX_WIDTH 320

#define GW_LCD_WIDTH  320
#define GW_LCD_HEIGHT 240

void lcd_init(SPI_HandleTypeDef *spi, LTDC_HandleTypeDef *ltdc);
void lcd_backlight_on();
void lcd_backlight_off();
#endif
