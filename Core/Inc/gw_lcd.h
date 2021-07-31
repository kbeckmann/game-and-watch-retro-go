#ifndef _LCD_H_
#define _LCD_H_

#include "stm32h7xx_hal.h"
#include <stdint.h>

#define GW_LCD_WIDTH  320
#define GW_LCD_HEIGHT 240

#ifdef GW_LCD_MODE_LUT8
extern uint8_t framebuffer1[GW_LCD_WIDTH * GW_LCD_HEIGHT]  __attribute__((section (".lcd1"))) __attribute__ ((aligned (16)));
extern uint8_t framebuffer2[GW_LCD_WIDTH * GW_LCD_HEIGHT]  __attribute__((section (".lcd2"))) __attribute__ ((aligned (16)));
typedef uint8_t pixel_t;
#else
extern uint16_t framebuffer1[GW_LCD_WIDTH * GW_LCD_HEIGHT]  __attribute__((section (".lcd1"))) __attribute__ ((aligned (16)));
extern uint16_t framebuffer2[GW_LCD_WIDTH * GW_LCD_HEIGHT]  __attribute__((section (".lcd2"))) __attribute__ ((aligned (16)));
typedef uint16_t pixel_t;
#endif // GW_LCD_MODE_LUT8


// To be shared between NES and GB. NES is larger.
extern uint8_t emulator_framebuffer[(256 + 8 + 8) * 240]  __attribute__((section (".emulator_data")));


// 0 => framebuffer1
// 1 => framebuffer2
extern uint32_t active_framebuffer;


void lcd_deinit(SPI_HandleTypeDef *spi);
void lcd_init(SPI_HandleTypeDef *spi, LTDC_HandleTypeDef *ltdc);
void lcd_backlight_set(uint8_t brightness);
void lcd_backlight_on();
void lcd_backlight_off();
void lcd_swap(void);
void lcd_sync(void);
void* lcd_get_active_buffer(void);
void* lcd_get_inactive_buffer(void);
void lcd_set_buffers(uint16_t *buf1, uint16_t *buf2);
void lcd_wait_for_vblank(void);

// To be used by fault handlers
void lcd_reset_active_buffer(void);

#endif
