#if CODEPAGE==950

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "rg_i18n.h"
#include "odroid_system.h"
#include "odroid_overlay.h"
#include "i18n/font_i18n_zh_tw.h"


static uint16_t overlay_buffer[ODROID_SCREEN_WIDTH * 12 * 2]  __attribute__ ((aligned (4)));


int odroid_overlay_get_local_font_size()
{
    return 12;
}

int odroid_overlay_get_local_font_width()
{
    return 6;
}

void odroid_overlay_read_screen_rect(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height)
{
    uint16_t *dst_img = lcd_get_active_buffer();
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            overlay_buffer[x + y * width] = dst_img [(y + y_pos) * ODROID_SCREEN_WIDTH + x_pos + x];
}

void do_point_data(uint16_t *data, char checked, char transparent, uint16_t color, uint16_t color_bg)
{
    if (checked)
        *data = color;
    else if (!transparent)
        *data = color_bg;
}

int odroid_overlay_draw_local_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg, uint16_t* outlength, char transparent)
{
    int font_height = odroid_overlay_get_local_font_size();
    int font_width = odroid_overlay_get_local_font_width();
    int x_offset = 0;
    //float scale = 1; //(float)font_height / 8;中文字体
    int text_len = strlen(text);
    //int posr = 0;
    uint8_t c1, c2, cc;
    uint32_t location;
    int max_len = width / font_width;

    for (int i = 0; i < max_len; i++)
    {
        c1 = (i < text_len) ? (((max_len < text_len) && (i > (max_len - 3))) ? 0x2e : text[i]) : 0x20;
        if (c1 < 0x80) {//ASCII
            //location = (c1 - 0x20) * 12;  //every 12 bytes
            location = c1 * 12;  //every 12 bytes no offset
            for (int y = 0; y < font_height; y++) {  //height :12;
                int offset = x_offset + (width * y);
                cc = hzk_asc6x12[location + y];
                do_point_data(&overlay_buffer[offset + 5], cc & 0x04, transparent, color, color_bg);
                do_point_data(&overlay_buffer[offset + 4], cc & 0x08, transparent, color, color_bg);
                do_point_data(&overlay_buffer[offset + 3], cc & 0x10, transparent, color, color_bg);
                do_point_data(&overlay_buffer[offset + 2], cc & 0x20, transparent, color, color_bg);
                do_point_data(&overlay_buffer[offset + 1], cc & 0x40, transparent, color, color_bg);
                do_point_data(&overlay_buffer[offset + 0], cc & 0x80, transparent, color, color_bg);
            }
            x_offset += 6;
        }
        else {
            //the last char?
            if (i == (max_len - 1)) {
                continue;
            } 
            else {
                c2 = (i < text_len - 1) ? text[i + 1] : 0x20;

                location = (c1 < 0xc9) ? ((c1 - 0xa1) * 158 + (c2 - 0x40 - ((c2 > 0xa0) ? 0x22 : 0))) * 24 : (5846 + (c1 - 0xc9) * 158 + (c2 - 0x40 - ((c2 > 0xa0) ? 0x22 : 0))) * 24;
                //location = (c1 < 0xc9) ?
                // ((c1 - 0xa1) * 158 + (c2 - 0x40 - ((c2 > 0xa0) ? 2 : 0))) * 24 : 
                //(5846 + (c1 - 0xc9) * 158 + (c2 - 0x40 - ((c2 > 0xa0) ? 2 : 0))) * 24;
                
                //draw double char
                for (int y = 0; y < font_height; y++) {  //height :12;
                    int offset = x_offset + (width * y);
                    cc = hzk12x12[location + y * 2];
                    
                    do_point_data(&overlay_buffer[offset + 7], cc & 0x01, transparent, color, color_bg);
                    do_point_data(&overlay_buffer[offset + 6], cc & 0x02, transparent, color, color_bg);
                    do_point_data(&overlay_buffer[offset + 5], cc & 0x04, transparent, color, color_bg);
                    do_point_data(&overlay_buffer[offset + 4], cc & 0x08, transparent, color, color_bg);
                    do_point_data(&overlay_buffer[offset + 3], cc & 0x10, transparent, color, color_bg);
                    do_point_data(&overlay_buffer[offset + 2], cc & 0x20, transparent, color, color_bg);
                    do_point_data(&overlay_buffer[offset + 1], cc & 0x40, transparent, color, color_bg);
                    do_point_data(&overlay_buffer[offset + 0], cc & 0x80, transparent, color, color_bg);

                    cc = hzk12x12[location + y * 2 + 1];

                    do_point_data(&overlay_buffer[offset + 11], cc & 0x10, transparent, color, color_bg);
                    do_point_data(&overlay_buffer[offset + 10], cc & 0x20, transparent, color, color_bg);
                    do_point_data(&overlay_buffer[offset +  9], cc & 0x40, transparent, color, color_bg);
                    do_point_data(&overlay_buffer[offset +  8], cc & 0x80, transparent, color, color_bg);
                }
                x_offset += 12;
                i++;
            }

        }
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
        if (strchr(buffer, '\n')) *(strchr(buffer, '\n')) = 0;
        int outlength = line_len;
        height += odroid_overlay_draw_local_text_line(x_pos, y_pos + height, width, buffer, color, color_bg, &outlength, transparent);
        pos += outlength; 
        if (*(text + pos) == 0 || *(text + pos) == '\n') pos++;
    }
    return height;
}

#endif
