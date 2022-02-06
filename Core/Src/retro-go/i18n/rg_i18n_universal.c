#pragma GCC diagnostic ignored "-Wstack-usage=" 
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wchar-subscripts"

#if !((CODEPAGE == 932) || (CODEPAGE == 936) || (CODEPAGE == 949) || (CODEPAGE == 950))

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "rg_i18n.h"
#include "main.h"
#include "gw_lcd.h"
#include "odroid_system.h"
#include "odroid_overlay.h"
#include "fonts/font_un_en.h"

static uint16_t overlay_buffer[ODROID_SCREEN_WIDTH * 12 * 2] __attribute__((aligned(4)));

const char *gui_fonts[21] = {
    font_un_01,
    font_un_02,
    font_un_03,
    font_un_04,
    font_un_05,
    font_un_06,
    font_un_07,
    font_un_08,
    font_un_09,
    font_un_10,
    font_un_11,
    font_un_12,
    font_un_13,
    font_un_14,
    font_un_15,
    font_un_16,
    font_un_17,
    font_un_18,
    font_un_19,
    font_un_20,
    font_un_21    
};

char *curr_font = font_un_09;

const int gui_font_count = 21;

int i18n_get_text_height()
{
    return 12;
}

int i18n_get_text_width(const char *text)
{
    int text_len = strlen(text);
    int ret = 0;
    for (int i = 0; i < text_len; i++)
        ret += curr_font[text[i]];
    return ret;
}

int i18n_get_text_lines(const char *text, const int fix_width)
{
    int text_len = strlen(text);
    int w = 0;
    int ret = (text[0]) ? 1 : 0;
    for (int i = 0; i < text_len; i++)
    {
        if (text[i] == 13)
            ret += 1;
        else if (text[i] == 10)
            w = 0;
        else if ((fix_width - w) < curr_font[text[i]])
        {
            w = 0;
            ret += 1;
        };
        w += curr_font[text[i]];
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

int i18n_draw_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg, char transparent)
{
    int font_height = 12;
    int x_offset = 0;
    char realtxt[161];

    if (transparent)
        odroid_overlay_read_screen_rect(x_pos, y_pos, width, font_height);
    else
    {
        for (int x = 0; x < width; x++)
            for (int y = 0; y < font_height; y++)
                overlay_buffer[x + y * width] = color_bg;
    }
    int w = i18n_get_text_width(text);
    sprintf(realtxt, "%.*s", 160, text); 
    if (w > width)
    {
        w = 0;
        int i = 0;
        while (w <= width) 
        {
            w += curr_font[realtxt[i]];
            i ++;
        }
        realtxt[i - 1] = 0;
        //paint end point
        overlay_buffer[width * (font_height - 3) - 1] = get_darken_pixel(color, 80);
        overlay_buffer[width * (font_height - 3) - 3] = get_darken_pixel(color, 80);
        overlay_buffer[width * (font_height - 3) - 6] = get_darken_pixel(color, 80);
    };

    int text_len = strlen(realtxt);

    for (int i = 0; i < text_len; i++)
    {
        uint8_t c1 = realtxt[i];
        int cw = curr_font[c1]; //width;
        if ((x_offset + cw) > width)
            break;
        if (cw != 0)
        {
            int d_pos = curr_font[c1 * 2 + 0x100] + curr_font[c1 * 2 + 0x101] * 0x100; //data pos
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
    odroid_display_write(x_pos, y_pos, width, font_height, overlay_buffer);

    return font_height;
}

int i18n_draw_text(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t max_height, const char *text, uint16_t color, uint16_t color_bg, char transparent)
{
    int text_len = 1;
    int height = 0;

    if (text == NULL || text[0] == 0)
        text = " ";

    text_len = strlen(text);
    if (x_pos < 0)
        x_pos = ODROID_SCREEN_WIDTH + x_pos;

    if (width < 1)
        width = i18n_get_text_width(text);

    if (width > (ODROID_SCREEN_WIDTH - x_pos))
        width = (ODROID_SCREEN_WIDTH - x_pos);

    int line_len = 160; //min width is 2, max 160 char everline;
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
            else if ((width - w) < curr_font[buffer[x]])
            {
                buffer[x] = 0;
                break;
            }
            w += curr_font[buffer[x]];
        }
        height += i18n_draw_text_line(x_pos, y_pos + height, width, buffer, color, color_bg, transparent);
        pos += strlen(buffer);
        if (*(text + pos) == 0 || *(text + pos) == '\n')
            pos++;
    }
    return height;
}

#endif
