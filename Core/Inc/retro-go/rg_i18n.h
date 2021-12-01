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
#else
    #include "i18n/rg_i18n_en_us.h"
#endif



#define ODROID_DIALOG_CHOICE_SEPARATOR {0x0F0F0F0E, "-", "-", -1, NULL}

int  odroid_overlay_get_local_font_size();
int  odroid_overlay_get_local_font_width();
int  odroid_overlay_draw_local_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg, uint16_t* outlength, char transparent);
int  odroid_overlay_draw_local_text(uint16_t x, uint16_t y, uint16_t width, const char *text, uint16_t color, uint16_t color_bg, char transparent);


int8_t odroid_settings_theme_get();
void odroid_settings_theme_set(int8_t theme);
int8_t odroid_settings_colors_get();
void odroid_settings_colors_set(int8_t colors);




