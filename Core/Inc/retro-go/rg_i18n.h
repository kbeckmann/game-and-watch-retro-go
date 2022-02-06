#pragma once

#include "stdbool.h"
#include "stdint.h"

#if !defined  (CODEPAGE)
  #define CODEPAGE 1252
#endif /* CODEPAGE */

#if CODEPAGE==932
    #include "i18n/rg_i18n_ja_jp.h"
#elif CODEPAGE==936
    #include "i18n/rg_i18n_zh_cn.h"
#elif CODEPAGE==949
    #include "i18n/rg_i18n_ko_kr.h"
#elif CODEPAGE==950
    #include "i18n/rg_i18n_zh_tw.h"
#elif CODEPAGE==12521
    #include "i18n/rg_i18n_es_es.h"
#elif CODEPAGE==12522
    #include "i18n/rg_i18n_pt_pt.h"
#else
    #include "i18n/rg_i18n_en_us.h"
#endif

#define ODROID_DIALOG_CHOICE_SEPARATOR {0x0F0F0F0E, "-", "-", -1, NULL}

extern char *curr_font;
extern const char* gui_fonts[];
extern const int gui_font_count;

int i18n_get_text_height();

int  i18n_get_text_width(const char *text);
int  i18n_get_text_lines(const char *text, const int fix_width);

int  i18n_draw_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg, char transparent);
int  i18n_draw_text(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t max_height, const char *text, uint16_t color, uint16_t color_bg, char transparent);

void odroid_overlay_clock(int x_pos, int y_pos);


int8_t odroid_settings_theme_get();
void odroid_settings_theme_set(int8_t theme);

int8_t odroid_settings_colors_get();
void odroid_settings_colors_set(int8_t colors);

int8_t odroid_settings_splashani_get();
void odroid_settings_splashani_set(int8_t splashani);

int8_t odroid_settings_font_get();
void odroid_settings_font_set(int8_t font);


