#pragma GCC diagnostic ignored "-Wstack-usage="
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wchar-subscripts"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


#if !defined (INCLUDED_ZH_CN)
#define INCLUDED_ZH_CN 0
#endif
#if !defined (INCLUDED_ZH_TW)
#define INCLUDED_ZH_TW 0
#endif
#if !defined (INCLUDED_JA_JP)
#define INCLUDED_JA_JP 0
#endif
#if !defined (INCLUDED_KO_KR)
#define INCLUDED_KO_KR 0
#endif
#if !defined (INCLUDED_ES_ES)
#define INCLUDED_ES_ES 1
#endif
#if !defined (INCLUDED_PT_PT)
#define INCLUDED_PT_PT 1
#endif

#if !defined (BIG_BANK)
#define BIG_BANK 1
#endif

#include "rg_i18n.h"
#include "rg_i18n_lang.h"
#include "gw_lcd.h"
#include "main.h"
#include "odroid_system.h"
#include "odroid_overlay.h"

#if BIG_BANK == 1
#define FONT_DATA 
#else
#define FONT_DATA __attribute__((section(".extflash_font")))
#endif

#if INCLUDED_JA_JP == 1
#include "fonts/font_ja_jp.h"
#endif
#if INCLUDED_ZH_CN == 1
#include "fonts/font_zh_cn.h"
#endif
#if INCLUDED_KO_KR == 1
#include "fonts/font_ko_kr.h"
#endif
#if INCLUDED_ZH_TW == 1
#include "fonts/font_zh_tw.h"
#endif

#if BIG_BANK == 1
#define LANG_DATA 
#else
#define LANG_DATA __attribute__((section(".extflash_emu_data")))
#endif

#include "rg_i18n_en_us.c"
#include "rg_i18n_es_es.c"
#include "rg_i18n_pt_pt.c"
#include "rg_i18n_zh_cn.c"
#include "rg_i18n_zh_tw.c"
#include "rg_i18n_ko_kr.c"
#include "rg_i18n_ja_jp.c"

static uint16_t overlay_buffer[ODROID_SCREEN_WIDTH * 12 * 2] __attribute__((aligned(4)));

#if ONEFONT == 1
#include "fonts/font_un_01.h"
const char *gui_fonts[1] = { font_un_01 };
char *curr_font = font_un_01;
#elif ONEFONT == 2
#include "fonts/font_un_02.h"
const char *gui_fonts[1] = { font_un_02 };
char *curr_font = font_un_02;
#elif ONEFONT == 3
#include "fonts/font_un_03.h"
const char *gui_fonts[1] = { font_un_03 };
char *curr_font = font_un_03;
#elif ONEFONT == 4
#include "fonts/font_un_04.h"
const char *gui_fonts[1] = { font_un_04 };
char *curr_font = font_un_04;
#elif ONEFONT == 5
#include "fonts/font_un_05.h"
const char *gui_fonts[1] = { font_un_05 };
char *curr_font = font_un_05;
#elif ONEFONT == 6
#include "fonts/font_un_06.h"
const char *gui_fonts[1] = { font_un_06 };
char *curr_font = font_un_06;
#elif ONEFONT == 7
#include "fonts/font_un_07.h"
const char *gui_fonts[1] = { font_un_07 };
char *curr_font = font_un_07;
#elif ONEFONT == 8
#include "fonts/font_un_08.h"
const char *gui_fonts[1] = { font_un_08 };
char *curr_font = font_un_08;
#elif ONEFONT == 9
#include "fonts/font_un_09.h"
const char *gui_fonts[1] = { font_un_09 };
char *curr_font = font_un_09;
#elif ONEFONT == 10
#include "fonts/font_un_10.h"
const char *gui_fonts[1] = { font_un_10 };
char *curr_font = font_un_10;
#elif ONEFONT == 11
#include "fonts/font_un_11.h"
const char *gui_fonts[1] = { font_un_11 };
char *curr_font = font_un_11;
#elif ONEFONT == 12
#include "fonts/font_un_12.h"
const char *gui_fonts[1] = { font_un_12 };
char *curr_font = font_un_12;
#elif ONEFONT == 13
#include "fonts/font_un_13.h"
const char *gui_fonts[1] = { font_un_13 };
char *curr_font = font_un_13;
#elif ONEFONT == 14
#include "fonts/font_un_14.h"
const char *gui_fonts[1] = { font_un_14 };
char *curr_font = font_un_14;
#elif ONEFONT == 15
#include "fonts/font_un_15.h"
const char *gui_fonts[1] = { font_un_15 };
char *curr_font = font_un_15;
#elif ONEFONT == 16
#include "fonts/font_un_16.h"
const char *gui_fonts[1] = { font_un_16 };
char *curr_font = font_un_16;
#elif ONEFONT == 17
#include "fonts/font_un_17.h"
const char *gui_fonts[1] = { font_un_17 };
char *curr_font = font_un_17;
#elif ONEFONT == 18
#include "fonts/font_un_18.h"
const char *gui_fonts[1] = { font_un_18 };
char *curr_font = font_un_18;
#elif ONEFONT == 19
#include "fonts/font_un_19.h"
const char *gui_fonts[1] = { font_un_19 };
char *curr_font = font_un_19;
#elif ONEFONT == 20
#include "fonts/font_un_20.h"
const char *gui_fonts[1] = { font_un_20 };
#elif ONEFONT == 21
#include "fonts/font_un_21.h"
const char *gui_fonts[1] = { font_un_21 };
#else
#include "fonts/font_un_01.h"
#include "fonts/font_un_02.h"
#include "fonts/font_un_03.h"
#include "fonts/font_un_04.h"
#include "fonts/font_un_05.h"
#include "fonts/font_un_06.h"
#include "fonts/font_un_07.h"
#include "fonts/font_un_08.h"
#include "fonts/font_un_09.h"
#include "fonts/font_un_10.h"
#include "fonts/font_un_11.h"
#include "fonts/font_un_12.h"
#include "fonts/font_un_13.h"
#include "fonts/font_un_14.h"
#include "fonts/font_un_15.h"
#include "fonts/font_un_16.h"
#include "fonts/font_un_17.h"
#include "fonts/font_un_18.h"
#include "fonts/font_un_19.h"
#include "fonts/font_un_20.h"
#include "fonts/font_un_21.h"
const char *gui_fonts[21] = {
    font_un_01,    font_un_02,    font_un_03,    font_un_04,    font_un_05,    font_un_06,    font_un_07,
    font_un_08,    font_un_09,    font_un_10,    font_un_11,    font_un_12,    font_un_13,    font_un_14,
    font_un_15,    font_un_16,    font_un_17,    font_un_18,    font_un_19,    font_un_20,    font_un_21};
char *curr_font = font_un_09;
#endif

const int gui_font_count = FONT_COUNT;

const lang_t *gui_lang[7] = {
    &lang_en_us,
#if INCLUDED_ES_ES == 1
    &lang_es_es,
#else
    NULL,
#endif
#if INCLUDED_PT_PT == 1
    &lang_pt_pt,
#else
    NULL,
#endif
#if INCLUDED_ZH_CN == 1
    &lang_zh_cn,
#else
    NULL,
#endif
#if INCLUDED_ZH_TW == 1
    &lang_zh_tw,
#else
    NULL,
#endif
#if INCLUDED_KO_KR == 1
    &lang_ko_kr,
#else
    NULL,
#endif
#if INCLUDED_JA_JP == 1
    &lang_ja_jp,
#else
    NULL,
#endif
};

lang_t *curr_lang = &lang_en_us;
lang_t *curr_romlang = &lang_en_us;
const int gui_lang_count = 7;

int i18n_get_text_height()
{
    return 12;
}

bool IS_CJK(const lang_t* lang)
{
    return (lang->codepage == 932) || (lang->codepage == 936) || (lang->codepage == 949) || (lang->codepage == 950);
};

int i18n_get_text_width(const char *text, const lang_t* lang)
{
    if (text == NULL || text[0] == 0)
        return 0;
    int text_len = strlen(text);
    int ret = 0;
    bool is_cjk = IS_CJK(lang);
    for (int i = 0; i < text_len; i++)
    {
        if ((text[i] > 0xA0) && is_cjk)
        {
            ret += 12;
            i++;
        }
        else
            ret += curr_font[text[i]];
    }
    return ret;
}

int i18n_get_text_lines(const char *text, const int fix_width, const lang_t* lang)
{
    if (text == NULL || text[0] == 0)
        return 0;
    int text_len = strlen(text);
    int w = 0;
    int ret = (text[0]) ? 1 : 0;
    int chr_width = 0;
    bool is_cjk = IS_CJK(lang);
    for (int i = 0; i < text_len; i++)
    {
        if (text[i] == 13)
            ret += 1;
        else if (text[i] == 10)
            w = 0;
        else
        {
            chr_width = ((text[i] > 0xA0) && is_cjk) ? 12 : curr_font[text[i]];
            if ((fix_width - w) < chr_width)
            {
                w = 0;
                ret += 1;
            };
            if ((text[i] > 0xA0) && is_cjk)
                i++;
        }
        w += chr_width;
    }
    return ret;
}

void odroid_overlay_read_screen_rect(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height)
{
    uint16_t *dst_img = (uint16_t *)(lcd_get_active_buffer());
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            overlay_buffer[x + y * width] = dst_img[(y + y_pos) * ODROID_SCREEN_WIDTH + x_pos + x];
}

int i18n_draw_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg, char transparent, const lang_t* lang)
{
    if (text == NULL || text[0] == 0)
        return 0;
    int font_height = 12;
    int x_offset = 0;
    char realtxt[161];
    uint8_t cc;
    bool is_cjk = IS_CJK(lang);

    if (transparent)
        odroid_overlay_read_screen_rect(x_pos, y_pos, width, font_height);
    else
    {
        for (int x = 0; x < width; x++)
            for (int y = 0; y < font_height; y++)
                overlay_buffer[x + y * width] = color_bg;
    }
    int w = i18n_get_text_width(text, lang);
    sprintf(realtxt, "%.*s", 160, text);
    bool l_is_cjk = false;
    if (w > width)
    {
        w = 0;
        int i = 0;
        while (w < width)
        {
            l_is_cjk = (realtxt[i] > 0xA0) && is_cjk;
            w += l_is_cjk ? 12 : curr_font[realtxt[i]];
            i += l_is_cjk ? 2 : 1;
        }
        realtxt[i - (l_is_cjk ? 2 : 1)] = 0;
        // paint end point
        overlay_buffer[width * (font_height - 3) - 1] = get_darken_pixel(color, 80);
        overlay_buffer[width * (font_height - 3) - 3] = get_darken_pixel(color, 80);
        overlay_buffer[width * (font_height - 3) - 6] = get_darken_pixel(color, 80);
    };

    int text_len = strlen(realtxt);

    for (int i = 0; i < text_len; i++)
    {
        uint8_t c1 = realtxt[i];
        if ((! is_cjk) || (c1 < 0xA1))
        {
            int cw = curr_font[c1]; // width;
            if ((x_offset + cw) > width)
                break;
            if (cw != 0)
            {
                int d_pos = curr_font[c1 * 2 + 0x100] + curr_font[c1 * 2 + 0x101] * 0x100; // data pos
                int line_bytes = (cw + 7) / 8;
                for (int y = 0; y < font_height; y++)
                {
                    uint32_t *pixels_data = (uint32_t *)&(curr_font[0x300 + d_pos + y * line_bytes]);
                    int offset = x_offset + (width * y);

                    for (int x = 0; x < cw; x++)
                    {
                        if (pixels_data[0] & (1 << x))
                            overlay_buffer[offset + x] = color;
                    }
                }
            }
            x_offset += cw;
        }
        else
        {
            uint32_t location = 0;
            uint8_t c2 = text[i + 1];
            if (lang->codepage == 936) // zh_cn
            //#elif (CODEPAGE == 936) // zh_cn
                location = (c1 < 0xb0) ? ((c1 - 0xa1) * 94 + (c2 - 0xa1)) * 24 : (9 * 94 + (c1 - 0xb0) * 94 + (c2 - 0xa1)) * 24;
            else if (lang->codepage == 950) // zh_tw
            //#elif (CODEPAGE == 950) // zh_tw
                location = (c1 < 0xc9) ? ((c1 - 0xa1) * 158 + (c2 - 0x40 - ((c2 > 0xa0) ? 0x22 : 0))) * 24 : (5846 + (c1 - 0xc9) * 158 + (c2 - 0x40 - ((c2 > 0xa0) ? 0x22 : 0))) * 24;
            else if (lang->codepage == 949) // ko_kr
            //#elif (CODEPAGE == 949) // ko_kr
                location = (c1 < 0xb0) ? ((c1 - 0xa1) * 94 + (c2 - 0xa1)) * 24 : ((c1 < 0xca) ? (12 * 94 + (c1 - 0xb0) * 94 + (c2 - 0xa1)) * 24 : (37 * 94 + (c1 - 0xca) * 94 + (c2 - 0xa1)) * 24);
            else if (lang->codepage == 932) // ja_jp
            //#if (CODEPAGE == 932)
                location = ((c1 - 0xa1) * 94 + (c2 - 0xa1)) * 24;
            //#endif
            // draw double char
            for (int y = 0; y < font_height; y++)
            { // height :12;
                int offset = x_offset + (width * y);
                cc = lang->extra_font[location + y * 2];

                if (cc & 0x01)
                    overlay_buffer[offset + 7] = color;
                if (cc & 0x02)
                    overlay_buffer[offset + 6] = color;
                if (cc & 0x04)
                    overlay_buffer[offset + 5] = color;
                if (cc & 0x08)
                    overlay_buffer[offset + 4] = color;
                if (cc & 0x10)
                    overlay_buffer[offset + 3] = color;
                if (cc & 0x20)
                    overlay_buffer[offset + 2] = color;
                if (cc & 0x40)
                    overlay_buffer[offset + 1] = color;
                if (cc & 0x80)
                    overlay_buffer[offset + 0] = color;

                cc = lang->extra_font[location + y * 2 + 1];

                if (cc & 0x10)
                    overlay_buffer[offset + 11] = color;
                if (cc & 0x20)
                    overlay_buffer[offset + 10] = color;
                if (cc & 0x40)
                    overlay_buffer[offset + 9] = color;
                if (cc & 0x80)
                    overlay_buffer[offset + 8] = color;
            }
            x_offset += 12;
            i++;
        }
    }
    odroid_display_write(x_pos, y_pos, width, font_height, overlay_buffer);

    return font_height;
}

int i18n_draw_text(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t max_height, const char *text, uint16_t color, uint16_t color_bg, char transparent, const lang_t* lang)
{
    int text_len = 1;
    int height = 0;
    bool is_cjk = IS_CJK(lang);

    if (text == NULL || text[0] == 0)
        text = " ";

    text_len = strlen(text);
    if (x_pos < 0)
        x_pos = ODROID_SCREEN_WIDTH + x_pos;

    if (width < 1)
        width = i18n_get_text_width(text, lang);

    if (width > (ODROID_SCREEN_WIDTH - x_pos))
        width = (ODROID_SCREEN_WIDTH - x_pos);

    int line_len = 160; // min width is 2, max 160 char everline;
    char buffer[line_len + 1];

    for (int pos = 0; pos < text_len;)
    {
        if ((height + i18n_get_text_height()) > max_height)
            break;
        sprintf(buffer, "%.*s", line_len, text + pos);
        if (strchr(buffer, '\n'))
            *(strchr(buffer, '\n')) = 0;
        int w = 0;
        for (int x = 0; x < line_len; x++)
        {
            if (buffer[x] == 0)
                break;
            bool l_is_cjk = (buffer[x] > 0xA0) && is_cjk;
            int chr_width = l_is_cjk ? 12 : curr_font[buffer[x]];
            if ((width - w) < chr_width)
            {
                buffer[x] = 0;
                break;
            }
            w += chr_width;
            if (l_is_cjk)
                x ++;
        }
        height += i18n_draw_text_line(x_pos, y_pos + height, width, buffer, color, color_bg, transparent, lang);
        pos += strlen(buffer);
        if (*(text + pos) == 0 || *(text + pos) == '\n')
            pos++;
    }
    return height;
}
