#pragma GCC diagnostic ignored "-Wstack-usage="

#if !((CODEPAGE == 932) || (CODEPAGE == 936) || (CODEPAGE == 949) || (CODEPAGE == 950))

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "rg_i18n.h"
#include "gw_lcd.h"
#include "odroid_system.h"
#include "odroid_overlay.h"

#if CODEPAGE == 1252 
#include "fonts/font_en_us.h"
#else
#include "fonts/font_es_es.h"
#endif

static uint16_t overlay_buffer[ODROID_SCREEN_WIDTH * 12 * 2] __attribute__((aligned(4)));

int odroid_overlay_get_local_font_size()
{
    return 12;
}
int odroid_overlay_get_local_font_width()
{
    return 8;
}

void odroid_overlay_read_screen_rect(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height)
{
    uint16_t *dst_img = (uint16_t *)(lcd_get_active_buffer());
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            overlay_buffer[x + y * width] = dst_img[(y + y_pos) * ODROID_SCREEN_WIDTH + x_pos + x];
}

int odroid_overlay_draw_local_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg, uint16_t *outlength, char transparent)
{
    int font_height = odroid_overlay_get_local_font_size(); //odroid_overlay_get_font_size();
    int font_width = odroid_overlay_get_local_font_width(); //odroid_overlay_get_font_width();
    int x_offset = 0;
    //float scale = 1; //(float)font_height / 8;
    int text_len = strlen(text);
    int max_len = width / font_width;

    if (transparent)
        odroid_overlay_read_screen_rect(x_pos, y_pos, width, font_height);
    else
    {
        for (int x = 0; x < width; x++)
            for (int y = 0; y < font_height; y++)
                overlay_buffer[x + y * width] = color_bg;
    }

    for (int i = 0; i < max_len; i++)
    {
        uint8_t c1 = (i < text_len) ? (((max_len < text_len) && (i > (max_len - 3))) ? 0x2e : text[i]) : 0x20;
        const char *glyph = font8x12_basic[c1];
        for (int y = 0; y < font_height; y++)
        {
            int offset = x_offset + (width * y);
            for (int x = 0; x < font_width; x++)
            {
                if (glyph[y] & (1 << x))
#if CODEPAGE == 1252
                    overlay_buffer[offset + x] = color;
#else
                    overlay_buffer[offset + 8 - x] = color;
#endif
            }
        }
        x_offset += font_width;
    }
    odroid_display_write(x_pos, y_pos, width, font_height, overlay_buffer);
    if (outlength)
        *outlength = x_offset / font_width;
    return font_height;
}

int odroid_overlay_draw_local_text(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg, char transparent)
{
    int text_len = 1;
    int height = 0;
    int outlength = 0;

    if (text == NULL || text[0] == 0)
        text = " ";

    text_len = strlen(text);

    if (width < 1)
        width = text_len * odroid_overlay_get_local_font_width();

    if (width > (ODROID_SCREEN_WIDTH - x_pos))
        width = (ODROID_SCREEN_WIDTH - x_pos);

    int line_len = width / odroid_overlay_get_local_font_width();
    char buffer[line_len + 1];

    for (int pos = 0; pos < text_len;)
    {
        sprintf(buffer, "%.*s", line_len, text + pos);
        if (strchr(buffer, '\n'))
            *(strchr(buffer, '\n')) = 0;
        height += odroid_overlay_draw_local_text_line(x_pos, y_pos + height, width, buffer, color, color_bg, (uint16_t *)(&outlength), transparent);
        pos += outlength;
        if (*(text + pos) == 0 || *(text + pos) == '\n')
            pos++;
    }

    return height;
}

#endif
