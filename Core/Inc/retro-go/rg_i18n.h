#pragma once

#include "stdbool.h"
#include "stdint.h"
#include "rg_i18n_lang.h"

#define ODROID_DIALOG_CHOICE_SEPARATOR {0x0F0F0F0E, "-", "-", -1, NULL}

extern char *curr_font;
extern const char* gui_fonts[];
extern const int gui_font_count;

extern lang_t* curr_lang;
extern lang_t *curr_romlang;
extern const lang_t* gui_lang[];
extern const int gui_lang_count;

int i18n_get_text_height();

int  i18n_get_text_width(const char *text, const lang_t* lang);
int  i18n_get_text_lines(const char *text, const int fix_width, const lang_t* lang);

int  i18n_draw_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg, char transparent, const lang_t* lang);
int  i18n_draw_text(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t max_height, const char *text, uint16_t color, uint16_t color_bg, char transparent, const lang_t* lang);

void odroid_overlay_clock(int x_pos, int y_pos);


int8_t odroid_settings_theme_get();
void odroid_settings_theme_set(int8_t theme);

int8_t odroid_settings_colors_get();
void odroid_settings_colors_set(int8_t colors);

int8_t odroid_settings_splashani_get();
void odroid_settings_splashani_set(int8_t splashani);

int8_t odroid_settings_font_get();
void odroid_settings_font_set(int8_t font);


int8_t odroid_settings_lang_get();
int8_t odroid_settings_get_next_lang(uint8_t cur);
int8_t odroid_settings_get_prior_lang(uint8_t cur);
void odroid_settings_lang_set(int8_t lang);

int8_t odroid_settings_romlang_get();
void odroid_settings_romlang_set(int8_t lang);
