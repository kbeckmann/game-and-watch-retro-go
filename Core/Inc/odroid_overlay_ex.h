#pragma once

#include "stdbool.h"
#include "stdint.h"

int  odroid_overlay_get_chn_font_size();
int  odroid_overlay_get_chn_font_width();
int  odroid_overlay_draw_chn_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg);
int  odroid_overlay_draw_chn_text(uint16_t x, uint16_t y, uint16_t width, const char *text, uint16_t color, uint16_t color_bg);
uint16_t get_darken_pixel(uint16_t color, uint16_t darken);

