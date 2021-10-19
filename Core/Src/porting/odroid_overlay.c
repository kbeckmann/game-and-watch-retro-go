#if 0

// #include "bitmaps/font_basic.h"
#include "odroid_system.h"
#include "odroid_overlay.h"

int odroid_overlay_game_settings_menu(odroid_dialog_choice_t *extra_options)
{
    return 0;
}

int odroid_overlay_game_debug_menu(void)
{
    return 0;
}

int odroid_overlay_game_menu()
{
    return 0;
}

#else

#if !defined(COVERFLOW)
#define COVERFLOW 0
#endif /* COVERFLOW */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "gw_buttons.h"
#include "bitmaps/font_basic.h"
#include "rg_i18n.h"
#include "gw_lcd.h"
#include "odroid_system.h"
#include "odroid_overlay.h"
#include "main.h"
#include "rom_manager.h"

// static uint16_t *overlay_buffer = NULL;
static uint16_t overlay_buffer[ODROID_SCREEN_WIDTH * 32 * 2] __attribute__((aligned(4)));
static short dialog_open_depth = 0;
static short font_size = 8;

void odroid_overlay_init()
{
    // overlay_buffer = (uint16_t *)rg_alloc(ODROID_SCREEN_WIDTH * 32 * 2, MEM_SLOW);
    odroid_overlay_set_font_size(odroid_settings_FontSize_get());
}

void odroid_overlay_set_font_size(int size)
{
    font_size = MAX(8, MIN(32, size));
    odroid_settings_FontSize_set(font_size);
}

int odroid_overlay_get_font_size()
{
    return font_size;
}

int odroid_overlay_get_font_width()
{
    return 8;
}

/* (128 X 14 )*/
/*
static const uint8_t img_logo[] =
    {
        0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, //
        0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, //
        0x20, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC4, //
        0x40, 0xF8, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0xEF, 0xDE, 0x87, 0x9F, 0xBD, 0xED, 0x18, 0x6E, 0xF2, //
        0x80, 0xCC, 0x01, 0x80, 0x00, 0x1E, 0xF7, 0xCF, 0x9C, 0xBF, 0x6F, 0xB9, 0xD9, 0xB7, 0xAE, 0xF9, //
        0x80, 0xCC, 0x03, 0xC0, 0x00, 0x1D, 0xFF, 0xAF, 0x5A, 0xBF, 0x4F, 0xB9, 0xD5, 0xAF, 0xEE, 0xFD, //
        0x80, 0xCC, 0x71, 0x8F, 0x38, 0x3D, 0xFF, 0xAF, 0x5A, 0xBF, 0xB3, 0xB5, 0xB5, 0xAF, 0xE0, 0xFD, //
        0x80, 0xF8, 0xD9, 0x8C, 0x6C, 0x3D, 0xC3, 0x0E, 0xD6, 0x87, 0x0F, 0xAD, 0x61, 0xAF, 0xEE, 0xFD, //
        0x81, 0x99, 0xF3, 0x9C, 0xCC, 0x7D, 0xFA, 0xED, 0xCE, 0xBF, 0x6F, 0xAD, 0x5D, 0xAF, 0xEE, 0xFD, //
        0x81, 0x99, 0x83, 0x18, 0xD8, 0x7E, 0xF6, 0xED, 0xCE, 0xBF, 0x23, 0x9C, 0xDD, 0xB7, 0xAE, 0xF9, //
        0x41, 0x98, 0xF3, 0x18, 0x70, 0xFF, 0x0D, 0xEB, 0xDE, 0x87, 0x9F, 0xBD, 0xBD, 0xB8, 0x6E, 0xF2, //
        0x20, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC4, //
        0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, //
        0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, //
};
*/
/* (112 X 12 )*/
/*
static const uint8_t img_logo[] =
    {
        0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, //
        0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, //
        0x40, 0x00, 0x00, 0x00, 0x02, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE2, //
        0x87, 0xC0, 0x0C, 0x00, 0x02, 0xDC, 0x7D, 0xED, 0x0F, 0x7D, 0xB7, 0x47, 0x1B, 0xB1, //
        0x86, 0x63, 0x9E, 0x79, 0xC5, 0xBB, 0xB9, 0xC9, 0x7E, 0xBD, 0xB6, 0x6E, 0xEB, 0xB5, //
        0x86, 0x66, 0xCC, 0x63, 0x65, 0xB7, 0xF5, 0xA5, 0x7F, 0x6D, 0x2D, 0x6D, 0xF8, 0x35, //
        0x87, 0xCF, 0x8C, 0xE6, 0x6B, 0x76, 0x11, 0xA5, 0x0E, 0x1D, 0x2C, 0x6D, 0xFB, 0xB5, //
        0x8C, 0xCC, 0x18, 0xC6, 0xCB, 0x7B, 0xAD, 0x6D, 0x7D, 0xAC, 0x9B, 0x6E, 0xEB, 0xB5, //
        0x8C, 0xC7, 0x98, 0xC3, 0x96, 0xFC, 0x6D, 0x6D, 0x0E, 0x1D, 0xBB, 0x6F, 0x1B, 0xB1, //
        0x40, 0x00, 0x00, 0x00, 0x16, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE2, //
        0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, //
        0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, //
};
*/
static const uint8_t img_logo[] =
    {
        0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, //
        0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, //
        0xD8, 0x00, 0x00, 0x00, 0x00, 0x25, 0xC3, 0xFB, 0xF7, 0xA1, 0xCF, 0xDE, 0xF6, 0x8C, 0x37, 0x7B, //
        0xA0, 0xF8, 0x01, 0x80, 0x00, 0x25, 0xBD, 0xF3, 0xE7, 0x2F, 0xB7, 0xDC, 0xEC, 0xDB, 0xD7, 0x75, //
        0x40, 0xCC, 0x73, 0xCF, 0x38, 0x4B, 0x7F, 0xEB, 0xD6, 0xAF, 0xAF, 0xDC, 0xEA, 0xD7, 0xF7, 0x6A, //
        0x48, 0xCC, 0xD9, 0x8C, 0x6C, 0x4B, 0x7F, 0xEB, 0xD6, 0xAF, 0xD9, 0xDA, 0xDA, 0xD7, 0xF0, 0x76, //
        0x48, 0xF9, 0xF1, 0x9C, 0xCC, 0x97, 0x70, 0xC3, 0xB5, 0xA1, 0x87, 0xD6, 0xB0, 0xD7, 0xF7, 0x76, //
        0x41, 0x99, 0x83, 0x18, 0xD8, 0x97, 0x7E, 0xBB, 0x73, 0xAF, 0xB7, 0xD6, 0xAE, 0xD7, 0xF7, 0x6A, //
        0xA1, 0x98, 0xF3, 0x18, 0x71, 0x2F, 0xBD, 0xBB, 0x73, 0xAF, 0x91, 0xCE, 0x6E, 0xDB, 0xD7, 0x75, //
        0xD8, 0x00, 0x00, 0x00, 0x01, 0x2F, 0xC3, 0x7A, 0xF7, 0xA1, 0xCF, 0xDE, 0xDE, 0xDC, 0x37, 0x7B, //
        0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, //
        0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, //
};

void odroid_overlay_draw_logo(uint16_t x_pos, uint16_t y_pos, uint16_t color)
{
    uint16_t *dst_img = lcd_get_active_buffer();
    for (int i = 0; i < 16; i++)     //128=16*8
        for (int y = 0; y < 12; y++) //height=12
        {
            const char glyph = ~img_logo[y * 16 + i]; //every line 16 byte; wtf the tools chg bytes.
            for (int x = 0; x < 8; x++)
                if (glyph & (0x80 >> x))
                    dst_img[(y + y_pos) * 320 + i * 8 + x + x_pos] = color;
        }
}

int odroid_overlay_draw_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg)
{
    int font_height = 8; //odroid_overlay_get_font_size();
    int font_width = 8;  //odroid_overlay_get_font_width();
    int x_offset = 0;
    //float scale = 1; //(float)font_height / 8;
    int text_len = strlen(text);

    for (int i = 0; i < (width / font_width); i++)
    {
        const char *glyph = font8x8_basic[(i < text_len) ? text[i] : ' '];
        for (int y = 0; y < font_height; y++)
        {
            int offset = x_offset + (width * y);
            for (int x = 0; x < 8; x++)
                overlay_buffer[offset + x] = (glyph[y] & (1 << x)) ? color : color_bg;
        }
        x_offset += font_width;
    }

    odroid_display_write(x_pos, y_pos, width, font_height, overlay_buffer);

    return font_height;
}

int odroid_overlay_draw_text(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg)
{
    int text_len = 1;
    int height = 0;

    if (text == NULL || text[0] == 0)
        text = " ";

    text_len = strlen(text);

    if (width < 1)
        width = text_len * odroid_overlay_get_font_width();

    if (width > (ODROID_SCREEN_WIDTH - x_pos))
        width = (ODROID_SCREEN_WIDTH - x_pos);

    int line_len = width / odroid_overlay_get_font_width();
    char buffer[line_len + 1];

    for (int pos = 0; pos < text_len;)
    {
        sprintf(buffer, "%.*s", line_len, text + pos);
        if (strchr(buffer, '\n'))
            *(strchr(buffer, '\n')) = 0;
        height += odroid_overlay_draw_text_line(x_pos, y_pos + height, width, buffer, color, color_bg);
        pos += strlen(buffer);
        if (*(text + pos) == 0 || *(text + pos) == '\n')
            pos++;
    }

    return height;
}

void odroid_overlay_draw_rect(int x, int y, int width, int height, int border, uint16_t color)
{
    if (width == 0 || height == 0 || border == 0)
        return;

    int pixels = (width > height ? width : height) * border;
    for (int i = 0; i < pixels; i++)
        overlay_buffer[i] = color;
    odroid_display_write(x, y, width, border, overlay_buffer);                   // T
    odroid_display_write(x, y + height - border, width, border, overlay_buffer); // B
    odroid_display_write(x, y, border, height, overlay_buffer);                  // L
    odroid_display_write(x + width - border, y, border, height, overlay_buffer); // R
}

void odroid_overlay_draw_fill_rect(int x, int y, int width, int height, uint16_t color)
{
    if (width == 0 || height == 0)
        return;

    for (int i = 0; i < width * 16; i++)
        overlay_buffer[i] = color;

    int y_pos = y;
    int y_end = y + height;

    while (y_pos < y_end)
    {
        int thickness = (y_end - y_pos >= 16) ? 16 : (y_end - y_pos);
        odroid_display_write(x, y_pos, width, thickness, overlay_buffer);
        y_pos += 16;
    }
}

void odroid_overlay_draw_battery(int x_pos, int y_pos)
{
    uint16_t percentage = odroid_input_read_battery().percentage;
    odroid_battery_charge_state_t battery_state = odroid_input_read_battery().state;
    uint16_t color_fill = C_GW_YELLOW;
    uint16_t color_border = C_GW_YELLOW;
    uint16_t color_empty = C_GW_RED;
    uint16_t color_battery = C_BLACK;
    uint16_t width_fill = 20.f / 100 * percentage;
    uint16_t width_empty = 20 - width_fill;

    if (percentage < 20)
        color_fill = C_RED;
    else if (percentage < 40)
        color_fill = C_ORANGE;

    odroid_overlay_draw_rect(x_pos, y_pos, 22, 10, 1, color_border);
    odroid_overlay_draw_rect(x_pos + 22, y_pos + 2, 2, 6, 1, color_border);
    odroid_overlay_draw_fill_rect(x_pos + 1, y_pos + 1, width_fill, 8, color_fill);
    odroid_overlay_draw_fill_rect(x_pos + 1 + width_fill, y_pos + 1, width_empty, 8, color_empty);

    switch (battery_state)
    {
    case ODROID_BATTERY_CHARGE_STATE_BATTERY_MISSING:
        break;
    case ODROID_BATTERY_CHARGE_STATE_CHARGING:
        odroid_overlay_draw_fill_rect(x_pos + 22 / 2 - 1, y_pos + 10 / 2 - 3, 2, 6, color_battery);
    case ODROID_BATTERY_CHARGE_STATE_DISCHARGING:
        odroid_overlay_draw_fill_rect(x_pos + 22 / 2 - 3, y_pos + 10 / 2 - 1, 6, 2, color_battery);
        break;
    case ODROID_BATTERY_CHARGE_STATE_FULL:
        break;
    }
}

static int get_dialog_items_count(odroid_dialog_choice_t *options)
{
    odroid_dialog_choice_t last = ODROID_DIALOG_CHOICE_LAST;

    if (options == NULL)
        return 0;

    for (int i = 0; i < 16; i++)
        if (options[i].id == last.id && options[i].enabled == last.enabled)
            return i;
    return 0;
}

uint16_t get_darken_pixel(uint16_t color, uint16_t darken)
{
    int16_t r = (int16_t)((color & 0b1111100000000000) * darken / 100) & 0b1111100000000000;
    int16_t g = (int16_t)((color & 0b0000011111100000) * darken / 100) & 0b0000011111100000;
    int16_t b = (int16_t)((color & 0b0000000000011111) * darken / 100) & 0b0000000000011111;
    return r | g | b;
}

uint16_t get_shined_pixel(uint16_t color, uint16_t shined)
{
    int16_t r = (int16_t)((color & 0b1111100000000000) + (0b1111100000000000 - (color & 0b1111100000000000)) / 100 * shined) & 0b1111100000000000;
    int16_t g = (int16_t)((color & 0b0000011111100000) + (0b0000011111100000 - (color & 0b0000011111100000)) / 100 * shined) & 0b0000011111100000;
    int16_t b = (int16_t)((color & 0b0000000000011111) + (0b0000000000011111 - (color & 0b0000000000011111)) * shined / 100) & 0b0000000000011111;
    return r | g | b;
}

void app_start_logo()
{
    lcd_set_buffers(framebuffer1, framebuffer1);
    odroid_overlay_draw_fill_rect(0, 0, 320, 240, C_BLACK);
    for (int i = 10; i <= 100; i++)
    {
        odroid_overlay_draw_logo(96, 100, get_darken_pixel(C_GW_YELLOW, i));
        lcd_sync();
        lcd_swap();
        wdog_refresh();
        HAL_Delay(4);
    }
    for (int i = 0; i < 40; i++)
    {
        wdog_refresh();
        HAL_Delay(10);
    }
    odroid_overlay_draw_fill_rect(0, 0, 320, 240, C_BLACK);
    lcd_sync();
}

void app_sleep_logo()
{
    lcd_set_buffers(framebuffer1, framebuffer1);
    odroid_overlay_draw_fill_rect(0, 0, 320, 240, C_BLACK);
    odroid_overlay_draw_logo(96, 100, C_GW_YELLOW);
    for (int i = 0; i < 40; i++)
    {
        wdog_refresh();
        HAL_Delay(10);
    }
    for (int i = 10; i <= 100; i++)
    {
        odroid_overlay_draw_logo(96, 100, get_darken_pixel(C_GW_YELLOW, 110 - i));
        lcd_sync();
        lcd_swap();
        wdog_refresh();
        HAL_Delay(i / 10);
    }
}

void odroid_overlay_darken_all()
{
    if (dialog_open_depth <= 0)
    { //darken bg
        uint16_t mgic = 0b0000100000100001;
        uint16_t *dst_img = lcd_get_active_buffer();
        if ((dst_img[0] == mgic) || is_lcd_swap_pending())
            return;
        for (int y = 0; y < ODROID_SCREEN_HEIGHT; y++)
            for (int x = 0; x < ODROID_SCREEN_WIDTH; x++)
                dst_img[y * ODROID_SCREEN_WIDTH + x] = get_darken_pixel(dst_img[y * ODROID_SCREEN_WIDTH + x], 40);

        dst_img[0] = mgic;
        lcd_sync();
    }
}

void odroid_overlay_draw_dialog(const char *header, odroid_dialog_choice_t *options, int sel)
{
    int width = 0;
    int padding = 0;
    int value_padding = 0;
    int len = 0;
    int max_titlen = 8;
    int row_margin = 1;
    int row_height = odroid_overlay_get_local_font_size() + row_margin * 2;
    int box_width = 64;
    int box_height = 64;
    int box_padding = 6;
    int box_color = C_BLACK;
    int box_border_color = C_GW_OPAQUE_YELLOW;
    int box_text_color = C_GW_YELLOW;
    odroid_dialog_choice_t separator = ODROID_DIALOG_CHOICE_SEPARATOR;

    int options_count = get_dialog_items_count(options);

    char *rows = rg_alloc(options_count * 256, MEM_ANY);

    odroid_overlay_darken_all();

    for (int i = 0; i < options_count; i++)
        if (options[i].update_cb != NULL)
            options[i].update_cb(&options[i], ODROID_DIALOG_INIT, 0);

    for (int i = 0; i < options_count; i++)
    {
        len = strlen(options[i].label);
        if (options[i].value[0])
        {
            padding = (len > padding) ? len : padding;
            len = strlen(options[i].value);
            value_padding = (len > value_padding) ? len : value_padding;
        }
        else
            max_titlen = (len > max_titlen) ? len : max_titlen;
    }
    padding = padding + 1;
    value_padding = value_padding + 1;
    max_titlen = max_titlen + 2;
    max_titlen = (padding + value_padding + 1) > max_titlen ? padding + value_padding + 1 : max_titlen;
    width = (ODROID_SCREEN_WIDTH - 60) / odroid_overlay_get_local_font_width();
    max_titlen = (max_titlen > width) ? width : max_titlen;
    if ((max_titlen - padding - value_padding - 1) < 0)
    {
        value_padding = max_titlen - padding - 1;
        width = padding + value_padding + 1;
    }
    else
        width = max_titlen;

    for (int i = 0; i < options_count; i++)
    {
        if (options[i].value[0])
        {
            if (strlen(options[i].value) < value_padding)
                sprintf(rows + i * 256, " %*s %*s ", -(padding - 1), options[i].label, value_padding - 1, options[i].value);
            else
                sprintf(rows + i * 256, " %*s %s ", -(padding - 1), options[i].label, options[i].value);
        }
        else
            sprintf(rows + i * 256, " %s ", options[i].label);
    }

    box_width = (odroid_overlay_get_local_font_width() * width) + box_padding * 2;
    box_height = (row_height * options_count) + (header ? row_height + 8 : 0) + box_padding * 2;

    int box_x = (ODROID_SCREEN_WIDTH - box_width) / 2;
    int box_y = (ODROID_SCREEN_HEIGHT - box_height) / 2;

    int x = box_x + box_padding;
    int y = box_y + box_padding;

    uint16_t fg, bg, color, inner_width = box_width - (box_padding * 2);
    if (header)
    {
        odroid_overlay_draw_rect(box_x - 1, box_y - 1, box_width + 2, row_height + 8, 1, box_border_color);
        odroid_overlay_draw_fill_rect(box_x, box_y, box_width, row_height + 7, C_GW_RED);
        odroid_overlay_draw_local_text_line(x, box_y + 5, inner_width, header, C_GW_YELLOW, C_GW_RED, NULL, 0);
        odroid_overlay_draw_fill_rect(x + inner_width - 2, box_y + 5, 4, 4, C_GW_YELLOW);
        odroid_overlay_draw_fill_rect(x + inner_width, box_y + 11, 2, 4, C_GW_OPAQUE_YELLOW);
        y += row_height + 8;
    }

    for (int i = 0; i < options_count; i++)
    {
        color = options[i].enabled == 1 ? box_text_color : C_GW_OPAQUE_YELLOW;
        if (options[i].enabled == 1)
        {
            fg = (i == sel) ? box_color : color;
            bg = (i == sel) ? color : box_color;
        }
        else
        {
            fg = color;
            bg = C_BLACK;
        }

        if (options[i].id == separator.id)
        {
            odroid_overlay_draw_fill_rect(x, y, inner_width, row_height + 3 * row_margin, bg);
            odroid_overlay_draw_fill_rect(x + 6, y + row_height / 2 - row_margin, inner_width - 12, 1, box_border_color);
        }
        else
        {
            row_height = odroid_overlay_draw_local_text(x, y + row_margin, inner_width, rows + i * 256, fg, bg, 0);
            row_height += row_margin * 2;
            odroid_overlay_draw_rect(x, y, inner_width, row_height, row_margin, bg);
        }
        y += row_height;
    }

    box_y = header ? box_y + row_height + 8 : box_y;
    box_height = y - box_y + box_padding;
    odroid_overlay_draw_rect(box_x, box_y, box_width, box_height, box_padding, box_color);
    odroid_overlay_draw_rect(box_x - 1, box_y - 1, box_width + 2, box_height + 2, 1, box_border_color);

    rg_free(rows);
}

int odroid_overlay_dialog(const char *header, odroid_dialog_choice_t *options, int selected)
{
    int options_count = get_dialog_items_count(options);
    int sel = selected < 0 ? (options_count + selected) : selected;
    int sel_old = sel;
    int last_key = -1;
    int repeat = 0;
    bool select = false;
    odroid_gamepad_state_t joystick;

    lcd_sync();
    lcd_swap();
    HAL_Delay(20);
    odroid_overlay_draw_dialog(header, options, sel);
    dialog_open_depth++;

    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY))
        wdog_refresh();

    while (1)
    {
        wdog_refresh();
        odroid_input_read_gamepad(&joystick);
        if (last_key < 0 || ((repeat >= 30) && (repeat % 5 == 0)))
        {
            if (joystick.values[ODROID_INPUT_UP])
            {
                last_key = ODROID_INPUT_UP;
                if (--sel < 0)
                    sel = options_count - 1;
                repeat++;
            }
            else if (joystick.values[ODROID_INPUT_DOWN])
            {
                last_key = ODROID_INPUT_DOWN;
                if (++sel > options_count - 1)
                    sel = 0;
                repeat++;
            }
            else if (joystick.values[ODROID_INPUT_B])
            {
                last_key = ODROID_INPUT_B;
                sel = -1;
                break;
            }
            else if (joystick.values[ODROID_INPUT_VOLUME])
            {
                last_key = ODROID_INPUT_VOLUME;
                sel = -1;
                break;
            }
            else if (joystick.values[ODROID_INPUT_MENU])
            {
                last_key = ODROID_INPUT_MENU;
                sel = -1;
                break;
            }
            else if (joystick.values[ODROID_INPUT_POWER])
            {
                sel = -1;
                odroid_system_emu_save_state(0);
                odroid_system_sleep();
                break;
            }
            if (options[sel].enabled)
            {
                select = false;
                if (joystick.values[ODROID_INPUT_LEFT])
                {
                    last_key = ODROID_INPUT_LEFT;
                    if (options[sel].update_cb != NULL)
                    {
                        select = options[sel].update_cb(&options[sel], ODROID_DIALOG_PREV, repeat);
                        sel_old = -1;
                    }
                    repeat++;
                }
                else if (joystick.values[ODROID_INPUT_RIGHT])
                {
                    last_key = ODROID_INPUT_RIGHT;
                    if (options[sel].update_cb != NULL)
                    {
                        select = options[sel].update_cb(&options[sel], ODROID_DIALOG_NEXT, repeat);
                        sel_old = -1;
                    }
                    repeat++;
                }
                else if (joystick.values[ODROID_INPUT_A])
                {
                    last_key = ODROID_INPUT_A;
                    if (options[sel].update_cb != NULL)
                    {
                        select = options[sel].update_cb(&options[sel], ODROID_DIALOG_ENTER, 0);
                        sel_old = -1;
                    }
                    else
                        select = true;
                }

                if (select)
                    break;
            }
        }
        if (repeat > 0)
            repeat++;
        if (last_key >= 0)
        {
            if (!joystick.values[last_key])
            {
                last_key = -1;
                repeat = 0;
            }
        }
        if (sel_old != sel)
        {
            int dir = sel - sel_old;
            while (options[sel].enabled == -1 && sel_old != sel)
                sel = (sel + dir) % options_count;

            sel_old = sel;
        }

        odroid_overlay_draw_dialog(header, options, sel);
        lcd_swap();
        HAL_Delay(20);
    }

    odroid_input_wait_for_key(last_key, false);

    odroid_display_force_refresh();

    dialog_open_depth--;

    return sel < 0 ? sel : options[sel].id;
}

int odroid_overlay_confirm(const char *text, bool yes_selected)
{
    odroid_dialog_choice_t choices[] = {
        {0, text, "", -1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {1, s_Yes, "", 1, NULL},
        {0, s_No, "", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST,
    };
    return odroid_overlay_dialog(s_PlsChose, choices, yes_selected ? 2 : 3);
}

void odroid_overlay_alert(const char *text)
{
    odroid_dialog_choice_t choices[] = {
        {0, text, "", -1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {1, s_OK, "", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST,
    };
    odroid_overlay_dialog(s_Confirm, choices, 2);
}

bool odroid_overlay_dialog_is_open(void)
{
    return dialog_open_depth > 0;
}

static bool volume_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t level = odroid_audio_volume_get();
    int8_t min = ODROID_AUDIO_VOLUME_MIN;
    int8_t max = ODROID_AUDIO_VOLUME_MAX;
    char volume_value[ODROID_AUDIO_VOLUME_MAX - ODROID_AUDIO_VOLUME_MIN + 2];

    if (event == ODROID_DIALOG_PREV && level > min)
        odroid_audio_volume_set(--level);

    if (event == ODROID_DIALOG_NEXT && level < max)
        odroid_audio_volume_set(++level);

    for (int i = ODROID_AUDIO_VOLUME_MIN; i <= ODROID_AUDIO_VOLUME_MAX; i++)
        volume_value[i - ODROID_AUDIO_VOLUME_MIN] = (i - ODROID_AUDIO_VOLUME_MIN) <= level ? s_Full : s_Fill;

    volume_value[ODROID_AUDIO_VOLUME_MAX + 1] = 0;
    sprintf(option->value, "%s", (char *)volume_value);
    return event == ODROID_DIALOG_ENTER;
}

#if COVERFLOW != 0
const char * GW_Themes[] = {s_Theme_sList, s_Theme_CoverV, s_Theme_CoverH,s_Theme_CoverLightH,s_Theme_CoverLightV};

static bool theme_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t theme = odroid_settings_theme_get();

    if (event == ODROID_DIALOG_PREV)
    {
        if (theme > 0)
            odroid_settings_theme_set(--theme);
        else {
            theme = 4;
            odroid_settings_theme_set(4);
        }
    }
    else if (event == ODROID_DIALOG_NEXT) {
        if (theme < 4) 
            odroid_settings_theme_set(++theme);
        else
        {
            theme = 0;
            odroid_settings_theme_set(0);
        }
    }
    sprintf(option->value, "%s", (char *)GW_Themes[theme]);
    return event == ODROID_DIALOG_ENTER;
}
#endif

static bool brightness_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t level = odroid_display_get_backlight();
    int8_t max = ODROID_BACKLIGHT_LEVEL_COUNT - 1;
    char bright_value[max + 1];

    if (event == ODROID_DIALOG_PREV && level > 0)
        odroid_display_set_backlight(--level);

    if (event == ODROID_DIALOG_NEXT && level < max)
        odroid_display_set_backlight(++level);

    for (int i = ODROID_BACKLIGHT_LEVEL0; i <= ODROID_BACKLIGHT_LEVEL9; i++)
        bright_value[i - ODROID_BACKLIGHT_LEVEL0] = (i - ODROID_BACKLIGHT_LEVEL0) <= level ? s_Full : s_Fill;

    bright_value[ODROID_BACKLIGHT_LEVEL9 + 1] = 0;
    sprintf(option->value, "%s", (char *)bright_value);
    return event == ODROID_DIALOG_ENTER;
}

static bool filter_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t max = ODROID_DISPLAY_FILTER_COUNT - 1;
    int8_t mode = odroid_display_get_filter_mode();
    int8_t prev = mode;

    if (event == ODROID_DIALOG_PREV && --mode < 0) mode = max; // 0;
    if (event == ODROID_DIALOG_NEXT && ++mode > max) mode = 0; // max;

    if (mode != prev)
    {
        odroid_display_set_filter_mode(mode);
    }

    if (mode == ODROID_DISPLAY_FILTER_OFF)   strcpy(option->value, s_FilteringOff);
    if (mode == ODROID_DISPLAY_FILTER_SHARP) strcpy(option->value, s_FilteringSharp);
    if (mode == ODROID_DISPLAY_FILTER_SOFT)  strcpy(option->value, s_FilteringSoft);

    return event == ODROID_DIALOG_ENTER;
}

static bool scaling_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t max = ODROID_DISPLAY_SCALING_COUNT - 1;
    int8_t mode = odroid_display_get_scaling_mode();
    int8_t prev = mode;

    if (event == ODROID_DIALOG_PREV && --mode < 0) mode =  max; // 0;
    if (event == ODROID_DIALOG_NEXT && ++mode > max) mode = 0;  // max;

    if (mode != prev) {
        odroid_display_set_scaling_mode(mode);
    }

    if (mode == ODROID_DISPLAY_SCALING_OFF)     strcpy(option->value, s_SCalingOff);
    if (mode == ODROID_DISPLAY_SCALING_FIT)     strcpy(option->value, s_SCalingFit);
    if (mode == ODROID_DISPLAY_SCALING_FULL)    strcpy(option->value, s_SCalingFull);
    if (mode == ODROID_DISPLAY_SCALING_CUSTOM)  strcpy(option->value, s_SCalingCustom);

    return event == ODROID_DIALOG_ENTER;
}

bool speedup_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    rg_app_desc_t *app = odroid_system_get_app();
    if (event == ODROID_DIALOG_PREV && --app->speedupEnabled <= SPEEDUP_MIN)
        app->speedupEnabled = SPEEDUP_MAX - 1;
    if (event == ODROID_DIALOG_NEXT && ++app->speedupEnabled >= SPEEDUP_MAX)
        app->speedupEnabled = SPEEDUP_MIN + 1;

    switch (app->speedupEnabled)
    {
    case SPEEDUP_0_5x:
        sprintf(option->value, "0.5%s", s_Speed_Unit);
        break;
    case SPEEDUP_0_75x:
        sprintf(option->value, "0.75%s", s_Speed_Unit);
        break;
    case SPEEDUP_1x:
        sprintf(option->value, "1%s", s_Speed_Unit);
        break;
    case SPEEDUP_1_25x:
        sprintf(option->value, "1.25%s", s_Speed_Unit);
        break;
    case SPEEDUP_1_5x:
        sprintf(option->value, "1.5%s", s_Speed_Unit);
        break;
    case SPEEDUP_2x:
        sprintf(option->value, "2%s", s_Speed_Unit);
        break;
    case SPEEDUP_3x:
        sprintf(option->value, "3%s", s_Speed_Unit);
        break;
    }

    return event == ODROID_DIALOG_ENTER;
}

int odroid_overlay_settings_menu(odroid_dialog_choice_t *extra_options)
{
    static char bright_value[25];
    static char volume_value[25];
    static char theme_value[25];

    odroid_dialog_choice_t options[32] = {                         //
        {0, s_Brightness, bright_value, 1, &brightness_update_cb}, //
        {1, s_Volume, volume_value, 1, &volume_update_cb},
#if COVERFLOW != 0
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {2, s_Theme_Title, theme_value, 1, &theme_update_cb},
#endif

        ODROID_DIALOG_CHOICE_LAST, //
    };

    if (extra_options)
    {
        int options_count = get_dialog_items_count(options);
        int extra_options_count = get_dialog_items_count(extra_options);
        memcpy(&options[options_count], extra_options, (extra_options_count + 1) * sizeof(odroid_dialog_choice_t));
    }

    int ret = odroid_overlay_dialog(s_OptionsTit, options, 0);

    odroid_settings_commit();

    return ret;
}

static void draw_game_status_bar(runtime_stats_t stats)
{
    int width = ODROID_SCREEN_WIDTH, height = 16;
    int pad_text = (height - odroid_overlay_get_local_font_size()) / 2;
    char bottom[40], header[40];

    snprintf(header, 40, "%s: %d.%d (%d.%d) / %s: %d.%d%%",
             s_FPS,
             (int)stats.totalFPS, (int)fmod(stats.totalFPS * 10, 10),
             (int)stats.skippedFPS, (int)fmod(stats.skippedFPS * 10, 10),
             s_BUSY,
             (int)stats.busyPercent, (int)fmod(stats.busyPercent * 10, 10));
    snprintf(bottom, 40, "%s", ACTIVE_FILE ? (ACTIVE_FILE->name) : "N/A");

    odroid_overlay_draw_fill_rect(0, 0, width, height, C_GW_RED);
    odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - height, width, height, C_GW_RED);
    odroid_overlay_draw_local_text(0, pad_text, width, header, C_GW_YELLOW, C_GW_RED, 0);
    odroid_overlay_draw_local_text(0, ODROID_SCREEN_HEIGHT - height + pad_text, width, bottom, C_GW_YELLOW, C_GW_RED, 0);
    odroid_overlay_draw_battery(width - 26, 3);
}

int odroid_overlay_game_settings_menu(odroid_dialog_choice_t *extra_options)
{
    char speedup_value[12];
    char scaling_value[12];
    char filtering_value[12];

    odroid_dialog_choice_t options[32] = {
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {200, s_Scaling, scaling_value, 1, &scaling_update_cb},
        {210, s_Filtering, filtering_value, 1, &filter_update_cb}, // Interpolation
        {220, s_Speed, speedup_value, 1, &speedup_update_cb},

        ODROID_DIALOG_CHOICE_LAST,
    };

    if (extra_options)
    {
        int options_count = get_dialog_items_count(options);
        int extra_options_count = get_dialog_items_count(extra_options);
        memcpy(&options[options_count], extra_options, (extra_options_count + 1) * sizeof(odroid_dialog_choice_t));
    }

    odroid_audio_mute(true);
    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY))
        wdog_refresh();

    int r = odroid_overlay_settings_menu(options);

    odroid_audio_mute(false);

    return r;
}

int odroid_overlay_game_debug_menu(void)
{
    odroid_dialog_choice_t options[12] = {
        {10, "Screen Res", "A", 1, NULL},
        {10, "Game Res", "B", 1, NULL},
        {10, "Scaled Res", "C", 1, NULL},
        {10, "Cheats", "C", 1, NULL},
        {10, "Rewind", "C", 1, NULL},
        {10, "Registers", "C", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST,
    };

    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY))
        wdog_refresh();
    return odroid_overlay_dialog("Debugging", options, 0);
}

int odroid_overlay_game_menu(odroid_dialog_choice_t *extra_options)
{
    odroid_dialog_choice_t choices[] = {
        // {0, "Continue", "",  1, NULL},
#if STATE_SAVING == 1
        {10, s_Save_Cont, "", 1, NULL},
        {20, s_Save_Quit, "", 1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {30, s_Reload, "", 1, NULL},
#endif
        {40, s_Options, "", 1, NULL},
        // {50, "Tools", "", 1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {90, s_Power_off, "", 1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {100, s_Quit_to_menu, "", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST,
    };

    // Collect stats before freezing emulation with wait_all_keys_released()
    runtime_stats_t stats = odroid_system_get_stats();

    odroid_audio_mute(true);
    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY))
        wdog_refresh();
    draw_game_status_bar(stats);

    lcd_sync();

    int r = odroid_overlay_dialog(s_Retro_Go_options, choices, 0);

    // Clear startup file so we boot into the retro-go gui
    odroid_settings_StartupFile_set(NULL);

    switch (r)
    {
    case 10:
        odroid_system_emu_save_state(0);
        break;
    case 20:
        odroid_system_emu_save_state(0);
        odroid_system_switch_app(0);
        break;
    case 30:
        odroid_system_emu_load_state(0);
        break; // TODO: Reload emulator?
    case 40:
        odroid_overlay_game_settings_menu(extra_options);
        break;
    case 50:
        odroid_overlay_game_debug_menu();
        break;
    case 90:
        odroid_system_sleep();
        break;
    case 100:
        odroid_system_switch_app(0);
        break;
    }

    // Required to reset the timestamps (we don't run a background stats task)
    (void)odroid_system_get_stats();

    odroid_audio_mute(false);

    return r;
}

#endif
