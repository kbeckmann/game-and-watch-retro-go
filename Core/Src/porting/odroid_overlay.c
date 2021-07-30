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


#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "gw_buttons.h"
#include "gw_lcd.h"
#include "bitmaps/font_basic.h"
#include "bitmaps/asc6x12.h"
#include "bitmaps/hzk12x12.h"
#include "odroid_system.h"
#include "odroid_overlay.h"
#include "odroid_overlay_ex.h"
#include "main.h"
#include "rom_manager.h"

// static uint16_t *overlay_buffer = NULL;
static uint16_t overlay_buffer[ODROID_SCREEN_WIDTH * 32 * 2]  __attribute__ ((aligned (4)));
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

int odroid_overlay_draw_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg)
{
    int font_height = 8; //odroid_overlay_get_font_size();
    int font_width = 8; //odroid_overlay_get_font_width();
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
            {
                overlay_buffer[offset + x] = (glyph[y] & (1 << x)) ? color : color_bg;
            }
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

    if (text == NULL || text[0] == 0) {
        text = " ";
    }

    text_len = strlen(text);

    if (width < 1) {
        width = text_len * odroid_overlay_get_font_width();
    }

    if (width > (ODROID_SCREEN_WIDTH - x_pos)) {
        width = (ODROID_SCREEN_WIDTH - x_pos);
    }

    int line_len = width / odroid_overlay_get_font_width();
    char buffer[line_len + 1];

    for (int pos = 0; pos < text_len;)
    {
        sprintf(buffer, "%.*s", line_len, text + pos);
        if (strchr(buffer, '\n')) *(strchr(buffer, '\n')) = 0;
        height += odroid_overlay_draw_text_line(x_pos, y_pos + height, width, buffer, color, color_bg);
        pos += strlen(buffer);
        if (*(text + pos) == 0 || *(text + pos) == '\n') pos++;
    }

    return height;
}

void odroid_overlay_draw_rect(int x, int y, int width, int height, int border, uint16_t color)
{
    if (width == 0 || height == 0 || border == 0)
        return;

    int pixels = (width > height ? width : height) * border;
    for (int i = 0; i < pixels; i++)
    {
        overlay_buffer[i] = color;
    }
    odroid_display_write(x, y, width, border, overlay_buffer); // T
    odroid_display_write(x, y + height - border, width, border, overlay_buffer); // B
    odroid_display_write(x, y, border, height, overlay_buffer); // L
    odroid_display_write(x + width - border, y, border, height, overlay_buffer); // R
}

void odroid_overlay_draw_fill_rect(int x, int y, int width, int height, uint16_t color)
{
    if (width == 0 || height == 0)
        return;

    for (int i = 0; i < width * 16; i++)
    {
        overlay_buffer[i] = color;
    }

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
            odroid_overlay_draw_fill_rect(x_pos + 22/2 - 1, y_pos + 10/2 - 3, 2, 6, color_battery);
        case ODROID_BATTERY_CHARGE_STATE_DISCHARGING:
            odroid_overlay_draw_fill_rect(x_pos + 22/2 - 3, y_pos + 10/2 - 1, 6, 2, color_battery);
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
    {
        // if (memcmp(&last, options + i, sizeof(last))) {
        if (options[i].id == last.id && options[i].enabled == last.enabled) {
            return i;
        }
    }
    return 0;
}

uint16_t get_darken_pixel(uint16_t color, uint16_t darken)
{
    int16_t r = (int16_t)((color & 0b1111100000000000) * darken / 100) & 0b1111100000000000;
    int16_t g = (int16_t)((color & 0b0000011111100000) * darken / 100) & 0b0000011111100000;
    int16_t b = (int16_t)((color & 0b0000000000011111) * darken / 100) & 0b0000000000011111;
    return r | g | b;
}


void odroid_overlay_draw_dialog(const char *header, odroid_dialog_choice_t *options, int sel)
{
    int width = header ? strlen(header) : 8;
    int padding = 0;
    int len = 0;

    int row_margin = 1;
    int row_height = odroid_overlay_get_chn_font_size() + row_margin * 2;

    int box_width = 64;
    int box_height = 64;
    int box_padding = 6;
    int box_color = C_BLACK;
    int box_border_color = C_GW_OPAQUE_YELLOW;
    int box_text_color = C_GW_YELLOW;
    odroid_dialog_choice_t separator = ODROID_DIALOG_CHOICE_SEPARATOR;

    //if (dialog_open_depth <= 0) {   //darken bg
    //    uint16_t *dst_img = lcd_get_active_buffer();
    //    for (int y = 0; y < ODROID_SCREEN_HEIGHT; y++) {
    //        for (int x = 0; x < ODROID_SCREEN_WIDTH; x++)
    //            dst_img[y * ODROID_SCREEN_WIDTH + x] = get_darken_pixel(dst_img[y * ODROID_SCREEN_WIDTH + x], 40);
    //    }
    //}

    int options_count = get_dialog_items_count(options);

    char *rows = rg_alloc(options_count * 256, MEM_ANY);

    for (int i = 0; i < options_count; i++)
    {
        if (options[i].value[0]) {
            len = strlen(options[i].label);
            padding = (len > padding) ? len : padding;
        }
    }

    for (int i = 0; i < options_count; i++)
    {
        if (options[i].update_cb != NULL) {
            options[i].update_cb(&options[i], ODROID_DIALOG_INIT, 0);
        }
        if (options[i].value[0]) {
            len = sprintf(rows + i * 256, " %*s %s ", -padding, options[i].label, options[i].value);
        } else {
            len = sprintf(rows + i * 256, " %s ", options[i].label);
        }
        width = len > width ? len : width;
    }

    if (width > 44) width = 44;

    box_width = (odroid_overlay_get_chn_font_width() * width) + box_padding * 2;
    box_height = (row_height * options_count) + (header ? row_height + 8 : 0) + box_padding * 2;

    int box_x = (ODROID_SCREEN_WIDTH - box_width) / 2;
    int box_y = (ODROID_SCREEN_HEIGHT - box_height) / 2;

    int x = box_x + box_padding;
    int y = box_y + box_padding;

    uint16_t fg, bg, color, inner_width = box_width - (box_padding * 2);
    if (header)
    {
        //int pad = (0.5f * (width - strlen(header)) * odroid_overlay_get_chn_font_width());
        odroid_overlay_draw_rect(box_x - 1, box_y - 1, box_width + 2, row_height + 8, 1, box_border_color);
        //odroid_overlay_draw_rect(box_x, box_y, box_width, row_height + 7, 1, C_GW_OPAQUE_YELLOW);
        odroid_overlay_draw_fill_rect(box_x, box_y, box_width, row_height + 7, C_GW_RED);
        odroid_overlay_draw_chn_text_line(x , box_y + 3, inner_width, header, C_GW_YELLOW, C_GW_RED);
        odroid_overlay_draw_fill_rect(x + inner_width, box_y + 4, 2, 4, C_GW_YELLOW);
        odroid_overlay_draw_fill_rect(x + inner_width, box_y + 10, 2, 4, C_GW_OPAQUE_YELLOW);
        y += row_height + 8;
    }

    for (int i = 0; i < options_count; i++)
    {
        color = options[i].enabled == 1 ? box_text_color : C_GW_OPAQUE_YELLOW;
        if (options[i].enabled == 1) {
            fg = (i == sel) ? box_color : color;
            bg = (i == sel) ? color : box_color;
        }
        else {
            fg = color;
            bg = C_BLACK;
        }

        if (options[i].id == separator.id) {
            odroid_overlay_draw_fill_rect(x, y, inner_width, row_height + 3 * row_margin, bg);
            odroid_overlay_draw_fill_rect(x + 6, y + row_height / 2 - row_margin, inner_width - 12, 1, box_border_color);
        }
        else {
            row_height = odroid_overlay_draw_chn_text(x, y + row_margin, inner_width, rows + i * 256, fg, bg);
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

    odroid_overlay_draw_dialog(header, options, sel);
    lcd_sync();

    dialog_open_depth++;

    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY))
        wdog_refresh();

    while (1)
    {
        wdog_refresh();
        odroid_input_read_gamepad(&joystick);
        if (last_key < 0 || ((repeat >= 30) && (repeat % 5 == 0))) {
            if (joystick.values[ODROID_INPUT_UP]) {
                last_key = ODROID_INPUT_UP;
                if (--sel < 0) sel = options_count - 1;
                repeat++;
            }
            else if (joystick.values[ODROID_INPUT_DOWN]) {
                last_key = ODROID_INPUT_DOWN;
                if (++sel > options_count - 1) sel = 0;
                repeat++;
            }
            else if (joystick.values[ODROID_INPUT_B]) {
                last_key = ODROID_INPUT_B;
                sel = -1;
                break;
            }
            else if (joystick.values[ODROID_INPUT_VOLUME]) {
                last_key = ODROID_INPUT_VOLUME;
                sel = -1;
                break;
            }
            else if (joystick.values[ODROID_INPUT_MENU]) {
                last_key = ODROID_INPUT_MENU;
                sel = -1;
                break;
            }
            else if (joystick.values[ODROID_INPUT_POWER]) {
                sel = -1;
                odroid_system_emu_save_state(0);
                odroid_system_sleep();
                break;
            }
            if (options[sel].enabled) {
                select = false;
                if (joystick.values[ODROID_INPUT_LEFT]) {
                    last_key = ODROID_INPUT_LEFT;
                    if (options[sel].update_cb != NULL) {
                        select = options[sel].update_cb(&options[sel], ODROID_DIALOG_PREV, repeat);
                        sel_old = -1;
                    }
                    repeat++;
                }
                else if (joystick.values[ODROID_INPUT_RIGHT]) {
                    last_key = ODROID_INPUT_RIGHT;
                    if (options[sel].update_cb != NULL) {
                        select = options[sel].update_cb(&options[sel], ODROID_DIALOG_NEXT, repeat);
                        sel_old = -1;
                    }
                    repeat++;
                }
                else if (joystick.values[ODROID_INPUT_A]) {
                    last_key = ODROID_INPUT_A;
                    if (options[sel].update_cb != NULL) {
                        select = options[sel].update_cb(&options[sel], ODROID_DIALOG_ENTER, 0);
                        sel_old = -1;
                    } else {
                        select = true;
                    }
                }

                if (select) {
                    break;
                }
            }
        }
        if (repeat > 0)
            repeat++;
        if (last_key >= 0) {
            if (!joystick.values[last_key]) {
                last_key = -1;
                repeat = 0;
            }
        }
        if (sel_old != sel)
        {
            int dir = sel - sel_old;
            while (options[sel].enabled == -1 && sel_old != sel)
            {
                sel = (sel + dir) % options_count;
            }
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
        {1, "是", "○", 1, NULL},
        {0, "否", "×", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST
    };
    return odroid_overlay_dialog("请选择", choices, yes_selected ? 0 : 1);
}

void odroid_overlay_alert(const char *text)
{
    odroid_dialog_choice_t choices[] = {
        {0, text, "", -1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {1, "确定", "○", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST
    };
    odroid_overlay_dialog("信息确认", choices, 1);
}

bool odroid_overlay_dialog_is_open(void)
{
    return dialog_open_depth > 0;
}

static bool volume_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t level = odroid_audio_volume_get();
    int8_t min = 1;
    int8_t max = 9;
    unsigned char sout[max * 2 + 2];

    if (event == ODROID_DIALOG_PREV && level > 0) {
        odroid_audio_volume_set(--level);
    }

    if (event == ODROID_DIALOG_NEXT && level < max) {
        odroid_audio_volume_set(++level);
    }

    for (int i = 0; i <= max; i++) {
        sout[i * 2] = 0xA1;
        sout[i * 2 + 1] = (i <= level) ? 0xF6 : 0xF5;
    }

    sprintf(option->value, "%.*s", max * 2 + 2, sout);
    return event == ODROID_DIALOG_ENTER;
    return false;
}

static bool brightness_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t level = odroid_display_get_backlight();
    int8_t max = 9;
    unsigned char sout[max * 2 + 2];

    if (event == ODROID_DIALOG_PREV && level > 0) {
        odroid_display_set_backlight(--level);
    }

    if (event == ODROID_DIALOG_NEXT && level < max) {
        odroid_display_set_backlight(++level);
    }
    for (int i = 0; i <= max; i++) {
        sout[i * 2] = 0xA1;
        sout[i * 2 + 1] = (i <= level) ? 0xF6 : 0xF5;
    }

    sprintf(option->value, "%.*s", max * 2 + 2, sout);
    return event == ODROID_DIALOG_ENTER;
}

static bool filter_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    // int8_t max = ODROID_DISPLAY_FILTER_COUNT - 1;
    // int8_t mode = odroid_display_get_filter_mode();
    // int8_t prev = mode;

    // if (event == ODROID_DIALOG_PREV && --mode < 0) mode = max; // 0;
    // if (event == ODROID_DIALOG_NEXT && ++mode > max) mode = 0; // max;

    // if (mode != prev)
    // {
    //     odroid_display_set_filter_mode(mode);
    // }

    // if (mode == ODROID_DISPLAY_FILTER_OFF)      strcpy(option->value, "Off  ");
    // if (mode == ODROID_DISPLAY_FILTER_LINEAR_X) strcpy(option->value, "Horiz");
    // if (mode == ODROID_DISPLAY_FILTER_LINEAR_Y) strcpy(option->value, "Vert ");
    // if (mode == ODROID_DISPLAY_FILTER_BILINEAR) strcpy(option->value, "Both ");

    // return event == ODROID_DIALOG_ENTER;
    return false;
}

static bool scaling_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    // int8_t max = ODROID_DISPLAY_SCALING_COUNT - 1;
    // int8_t mode = odroid_display_get_scaling_mode();
    // int8_t prev = mode;

    // if (event == ODROID_DIALOG_PREV && --mode < 0) mode =  max; // 0;
    // if (event == ODROID_DIALOG_NEXT && ++mode > max) mode = 0;  // max;

    // if (mode != prev) {
    //     odroid_display_set_scaling_mode(mode);
    // }

    // if (mode == ODROID_DISPLAY_SCALING_OFF)  strcpy(option->value, "Off  ");
    // if (mode == ODROID_DISPLAY_SCALING_FIT)  strcpy(option->value, "Fit ");
    // if (mode == ODROID_DISPLAY_SCALING_FILL) strcpy(option->value, "Full ");

    // return event == ODROID_DIALOG_ENTER;
    return false;
}

bool speedup_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    rg_app_desc_t *app = odroid_system_get_app();
    if (event == ODROID_DIALOG_PREV && --app->speedupEnabled <= SPEEDUP_MIN) app->speedupEnabled = SPEEDUP_MAX - 1;
    if (event == ODROID_DIALOG_NEXT && ++app->speedupEnabled >= SPEEDUP_MAX) app->speedupEnabled = SPEEDUP_MIN + 1;

    switch(app->speedupEnabled){
        case SPEEDUP_0_5x:
            sprintf(option->value, "%20s", "0.50 倍");
            break;
        case SPEEDUP_0_75x:
            sprintf(option->value, "%20s", "0.75 倍");
            break;
        case SPEEDUP_1x:
            sprintf(option->value, "%20s", "1.00 倍");
            break;
        case SPEEDUP_1_25x:
            sprintf(option->value, "%20s", "1.25 倍");
            break;
        case SPEEDUP_1_5x:
            sprintf(option->value, "%20s", "1.50 倍");
            break;
        case SPEEDUP_2x:
            sprintf(option->value, "%20s", "2.00 倍");
            break;
        case SPEEDUP_3x:
            sprintf(option->value, "%20s", "3.00 倍");
            break;
    }

    return event == ODROID_DIALOG_ENTER;
}

int odroid_overlay_settings_menu(odroid_dialog_choice_t *extra_options)
{
    static char bright_value[8];
    static char volume_value[8];

    odroid_dialog_choice_t options[32] = {
        {0, "亮度", bright_value, 1, &brightness_update_cb},
        {1, "音量", volume_value, 1, &volume_update_cb},
        ODROID_DIALOG_CHOICE_LAST
    };

    if (extra_options) {
        int options_count = get_dialog_items_count(options);
        int extra_options_count = get_dialog_items_count(extra_options);
        memcpy(&options[options_count], extra_options, (extra_options_count + 1) * sizeof(odroid_dialog_choice_t));
    }

    int ret = odroid_overlay_dialog("系统设置", options, 0);

    odroid_settings_commit();

    return ret;
}

static void draw_game_status_bar(runtime_stats_t stats)
{
    int width = ODROID_SCREEN_WIDTH, height = 18;
    int pad_text = (height - odroid_overlay_get_chn_font_size()) / 2;
    char bottom[50], header[40];

    const char *romPath = (ACTIVE_FILE) ? ACTIVE_FILE->name : 'N/A';
        //odroid_system_get_app()->romPath;

    snprintf(header, 40, "帧率: %d.%d (%d.%d) / 负载（CPU）: %d.%d%%",
        (int) stats.totalFPS,    (int) fmod(stats.totalFPS * 10, 10),
        (int) stats.skippedFPS,  (int) fmod(stats.skippedFPS * 10, 10),
        (int) stats.busyPercent, (int) fmod(stats.busyPercent * 10, 10));
    snprintf(bottom, 50, "%s", romPath);

    odroid_overlay_draw_fill_rect(0, 0, width, height, C_GW_RED);
    odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - height, width, height, C_GW_RED);
    odroid_overlay_draw_chn_text(0, pad_text, width, header, C_GW_YELLOW, C_GW_RED);
    odroid_overlay_draw_chn_text(0, ODROID_SCREEN_HEIGHT - height + pad_text, width, bottom, C_GW_YELLOW, C_GW_RED);
    odroid_overlay_draw_battery(width - 26, 4);
}

int odroid_overlay_game_settings_menu(odroid_dialog_choice_t *extra_options)
{
    char speedup_value[8];

    odroid_dialog_choice_t options[32] = {
        {200, "缩放", "全屏", 1, &scaling_update_cb},
        {210, "过滤", "无", 1, &filter_update_cb}, // Interpolation
        {220, "速度", speedup_value, 1, &speedup_update_cb},

        ODROID_DIALOG_CHOICE_LAST
    };

    if (extra_options) {
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
        ODROID_DIALOG_CHOICE_LAST
    };

    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY))
        wdog_refresh();
    return odroid_overlay_dialog("Debugging", options, 0);
}

int odroid_overlay_game_menu(odroid_dialog_choice_t *extra_options)
{
    odroid_dialog_choice_t choices[] = {
        // {0, "Continue", "",  1, NULL},
        {10, "保存进度", "■",  1, NULL},
        {20, "保存后退出", "×", 1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {30, "重载", "∞", 1, NULL},
        {40, "选项", "◎", 1, NULL},
        // {50, "Tools", "", 1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {90, "休眠", "∽", 1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {100, "退出游戏", "×", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST
    };

    // Collect stats before freezing emulation with wait_all_keys_released()
    runtime_stats_t stats = odroid_system_get_stats();

    odroid_audio_mute(true);
    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY))
        wdog_refresh();
    draw_game_status_bar(stats);

    lcd_sync();

    int r = odroid_overlay_dialog("游戏选项", choices, 0);

    // Clear startup file so we boot into the retro-go gui
    odroid_settings_StartupFile_set(NULL);

    switch (r)
    {
        case 10: odroid_system_emu_save_state(0); break;
        case 20: odroid_system_emu_save_state(0); odroid_system_switch_app(0); break;
        case 30: odroid_system_emu_load_state(0); break; // TODO: Reload emulator?
        case 40: odroid_overlay_game_settings_menu(extra_options); break;
        case 50: odroid_overlay_game_debug_menu(); break;
        case 90: odroid_system_sleep(); break;
        case 100: odroid_system_switch_app(0); break;
    }

    // Required to reset the timestamps (we don't run a background stats task)
    (void) odroid_system_get_stats();

    odroid_audio_mute(false);

    return r;
}

int odroid_overlay_get_chn_font_size()
{
    return 12;
}

int odroid_overlay_get_chn_font_width()
{
    return 6;
}

int odroid_overlay_draw_chn_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg)
{
    int font_height = 12; //odroid_overlay_get_font_size();
    int font_width = 6; //odroid_overlay_get_font_width();
    int x_offset = 0;
    //float scale = 1; //(float)font_height / 8;中文字体
    int text_len = strlen(text);
    int posr = 0;
    uint8_t c1, c2, cc;
    uint32_t location;
    int maxlen = width / font_width;

    for (int i = 0; i < maxlen; i++)
    {
        //const char *glyph = font8x12_basic[(i < text_len) ? text[i] : ' '];
        c1 = (i < text_len) ? text[i] : 0x20;
        if (c1 < 0x80) {//ASCII
            location = (c1 - 0x20) * 12;  //every 12 bytes
            for (int y = 0; y < font_height; y++) {  //height :12;
                int offset = x_offset + (width * y);
                cc = asc6x12[location + y];
                //overlay_buffer[offset + 7] = (cc & 0x01) ? color : color_bg;
                //overlay_buffer[offset + 6] = (cc & 0x02) ? color : color_bg;
                overlay_buffer[offset + 5] = (cc & 0x04) ? color : color_bg;
                overlay_buffer[offset + 4] = (cc & 0x08) ? color : color_bg;
                overlay_buffer[offset + 3] = (cc & 0x10) ? color : color_bg;
                overlay_buffer[offset + 2] = (cc & 0x20) ? color : color_bg;
                overlay_buffer[offset + 1] = (cc & 0x40) ? color : color_bg;
                overlay_buffer[offset + 0] = (cc & 0x80) ? color : color_bg;
            }
            x_offset += 6;
        }
        else {
            //the last char?
            if (i == (maxlen - 1)) {
                posr = 1;
                continue;
            } 
            else {
                c2 = (i < text_len - 1) ? text[i + 1] : 0x20;
                location = (c1 < 0xb0) ? ((c1 - 0xa1) * 94 + (c2 - 0xa1)) * 24 : (9 * 94 + (c1 - 0xb0) * 94 + (c2 - 0xa1)) * 24;
                //draw double char
                for (int y = 0; y < font_height; y++) {  //height :12;
                    int offset = x_offset + (width * y);
                    cc = hzk12x12[location + y * 2];
                    overlay_buffer[offset + 7] = (cc & 0x01) ? color : color_bg;
                    overlay_buffer[offset + 6] = (cc & 0x02) ? color : color_bg;
                    overlay_buffer[offset + 5] = (cc & 0x04) ? color : color_bg;
                    overlay_buffer[offset + 4] = (cc & 0x08) ? color : color_bg;
                    overlay_buffer[offset + 3] = (cc & 0x10) ? color : color_bg;
                    overlay_buffer[offset + 2] = (cc & 0x20) ? color : color_bg;
                    overlay_buffer[offset + 1] = (cc & 0x40) ? color : color_bg;
                    overlay_buffer[offset + 0] = (cc & 0x80) ? color : color_bg;
                    cc = hzk12x12[location + y * 2 + 1];
                    //overlay_buffer[offset + 15] = (cc & 0x01) ? color : color_bg;
                    //overlay_buffer[offset + 14] = (cc & 0x02) ? color : color_bg;
                    //overlay_buffer[offset + 13] = (cc & 0x04) ? color : color_bg;
                    //overlay_buffer[offset + 12] = (cc & 0x08) ? color : color_bg;
                    overlay_buffer[offset + 11] = (cc & 0x10) ? color : color_bg;
                    overlay_buffer[offset + 10] = (cc & 0x20) ? color : color_bg;
                    overlay_buffer[offset + 9] = (cc & 0x40) ? color : color_bg;
                    overlay_buffer[offset + 8] = (cc & 0x80) ? color : color_bg;
                }
                x_offset += 12;
                i++;
            }

        }
    }
    odroid_display_write(x_pos, y_pos, width, font_height, overlay_buffer);
    return posr;
}


int odroid_overlay_draw_chn_text(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg)
{
    int text_len = 1;
    int height = 0;

    if (text == NULL || text[0] == 0) 
        text = " ";

    text_len = strlen(text);

    if (width < 1) 
        width = text_len * odroid_overlay_get_chn_font_width();

    if (width > (ODROID_SCREEN_WIDTH - x_pos)) 
        width = (ODROID_SCREEN_WIDTH - x_pos);

    int line_len = width / odroid_overlay_get_chn_font_width();
    char buffer[line_len + 1];

    for (int pos = 0; pos < text_len;)
    {
        sprintf(buffer, "%.*s", line_len, text + pos);
        if (strchr(buffer, '\n')) *(strchr(buffer, '\n')) = 0;
        int posr = odroid_overlay_draw_chn_text_line(x_pos, y_pos + height, width, buffer, color, color_bg);
        height += odroid_overlay_get_chn_font_size();
        pos += strlen(buffer) - posr; //need reset start pos
        if (*(text + pos) == 0 || *(text + pos) == '\n') pos++;
    }
    return height;
}


#endif
