#include <odroid_system.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "lupng.h"
#include "gui.h"
#include "gw_lcd.h"
#include "rg_i18n.h"
#include "bitmaps.h"

#if !defined(COVERFLOW)
#define COVERFLOW 0
#endif /* COVERFLOW */

#define IMAGE_LOGO_WIDTH (47)
#define IMAGE_LOGO_HEIGHT (51)
#define IMAGE_BANNER_WIDTH (ODROID_SCREEN_WIDTH)
#define IMAGE_BANNER_HEIGHT (32)
#define STATUS_HEIGHT (33)
#define HEADER_HEIGHT (47)

#define CRC_WIDTH (104)
#define CRC_X_OFFSET (ODROID_SCREEN_WIDTH - CRC_WIDTH)
#define CRC_Y_OFFSET (STATUS_HEIGHT)

#define LIST_WIDTH (ODROID_SCREEN_WIDTH)
#define LIST_HEIGHT (ODROID_SCREEN_HEIGHT - STATUS_HEIGHT - HEADER_HEIGHT)
#define LIST_LINE_HEIGHT (odroid_overlay_get_font_size() + 2)
#define LIST_LINE_COUNT (LIST_HEIGHT / LIST_LINE_HEIGHT)
#define LIST_X_OFFSET (0)
#define LIST_Y_OFFSET (STATUS_HEIGHT)

#define COVER_MAX_HEIGHT (184)
#define COVER_MAX_WIDTH (184)

theme_t gui_themes[] = {
    {0, C_GRAY, C_WHITE, C_AQUA},
    {0, C_GRAY, C_GREEN, C_AQUA},
    {0, C_WHITE, C_GREEN, C_AQUA},

    {5, C_GRAY, C_WHITE, C_AQUA},
    {5, C_GRAY, C_GREEN, C_AQUA},
    {5, C_WHITE, C_GREEN, C_AQUA},

    {11, C_GRAY, C_WHITE, C_AQUA},
    {11, C_GRAY, C_GREEN, C_AQUA},
    {11, C_WHITE, C_GREEN, C_AQUA},

    {16, C_GRAY, C_WHITE, C_AQUA},
    {16, C_GRAY, C_GREEN, C_AQUA},
    {16, C_WHITE, C_GREEN, C_AQUA},
};
int gui_themes_count = 12;

static char str_buffer[128];

retro_gui_t gui;

void gui_event(gui_event_t event, tab_t *tab)
{
    if (tab->event_handler)
        (*tab->event_handler)(event, tab);
}

tab_t *gui_add_tab(const char *name, const void *logo, const void *header, void *arg, void *event_handler)
{
    tab_t *tab = rg_calloc(1, sizeof(tab_t));

    sprintf(tab->name, "%s", name);
    sprintf(tab->status, "Loading...");

    tab->event_handler = event_handler;
    tab->img_header = header;
    tab->img_logo = logo ?: (void *)tab;
    tab->initialized = false;
    tab->is_empty = false;
    tab->arg = arg;

    gui.tabs[gui.tabcount++] = tab;

    printf("gui_add_tab: Tab '%s' added at index %d\n", tab->name, gui.tabcount - 1);

    return tab;
}

void gui_init_tab(tab_t *tab)
{
    if (tab->initialized)
        return;

    tab->initialized = true;
    // tab->status[0] = 0;

    sprintf(str_buffer, "Sel.%.11s", tab->name);
    // tab->listbox.cursor = odroid_settings_int32_get(str_buffer, 0);
    tab_t *selected_tab = gui_get_tab(odroid_settings_MainMenuSelectedTab_get());
    if (tab->name == selected_tab->name)
    {
        tab->listbox.cursor = odroid_settings_MainMenuCursor_get();
    }

    gui_event(TAB_INIT, tab);

    tab->listbox.cursor = MIN(tab->listbox.cursor, tab->listbox.length - 1);
    tab->listbox.cursor = MAX(tab->listbox.cursor, 0);
}

tab_t *gui_get_tab(int index)
{
    return (index >= 0 && index < gui.tabcount) ? gui.tabs[index] : NULL;
}

tab_t *gui_get_current_tab()
{
    return gui_get_tab(gui.selected);
}

tab_t *gui_set_current_tab(int index)
{
    index %= gui.tabcount;

    if (index < 0)
        index += gui.tabcount;

    gui.selected = index;

    return gui_get_tab(gui.selected);
}

void gui_save_current_tab()
{
    tab_t *tab = gui_get_current_tab();

    sprintf(str_buffer, "Sel.%.11s", tab->name);
    // odroid_settings_int32_set(str_buffer, tab->listbox.cursor);
    odroid_settings_MainMenuCursor_set(tab->listbox.cursor);
    // odroid_settings_int32_set("SelectedTab", gui.selected);
    odroid_settings_MainMenuSelectedTab_set(gui.selected);
    odroid_settings_commit();
}

listbox_item_t *gui_get_selected_item(tab_t *tab)
{
    listbox_t *list = &tab->listbox;

    if (list->cursor >= 0 && list->cursor < list->length)
        return &list->items[list->cursor];

    return NULL;
}

static int list_comparator(const void *p, const void *q)
{
    return strcasecmp(((listbox_item_t *)p)->text, ((listbox_item_t *)q)->text);
}

void gui_sort_list(tab_t *tab, int sort_mode)
{
    if (tab->listbox.length == 0)
        return;

    qsort((void *)tab->listbox.items, tab->listbox.length, sizeof(listbox_item_t), list_comparator);
}

void gui_resize_list(tab_t *tab, int new_size)
{
    int cur_size = tab->listbox.length;

    if (new_size == cur_size)
        return;

    if (new_size == 0)
    {
        rg_free(tab->listbox.items);
        tab->listbox.items = NULL;
    }
    else
    {
        tab->listbox.items = rg_realloc(tab->listbox.items, new_size * sizeof(listbox_item_t));
        for (int i = cur_size; i < new_size; i++)
            memset(&tab->listbox.items[i], 0, sizeof(listbox_item_t));
    }

    tab->listbox.length = new_size;
    tab->listbox.cursor = MIN(tab->listbox.cursor, tab->listbox.length - 1);
    tab->listbox.cursor = MAX(tab->listbox.cursor, 0);

    printf("gui_resize_list: Resized list '%s' from %d to %d items\n", tab->name, cur_size, new_size);
}

void gui_scroll_list(tab_t *tab, scroll_mode_t mode)
{
    listbox_t *list = &tab->listbox;

    if (list->length == 0 || list->cursor > list->length) {
        return;
    }

    int cur_cursor = list->cursor;
    int old_cursor = list->cursor;

    if (mode == LINE_UP){
        cur_cursor--;
    }
    else if (mode == LINE_DOWN) {
        cur_cursor++;
    }
    else if (mode == PAGE_UP) {
        char st = ((char *)list->items[cur_cursor].text)[0];
        int max = LIST_LINE_COUNT - 2;
        while (--cur_cursor > 0 && max-- > 0)
        {
            if (st != ((char *)list->items[cur_cursor].text)[0])
                break;
        }
    }
    else if (mode == PAGE_DOWN) {
        char st = ((char *)list->items[cur_cursor].text)[0];
        int max = LIST_LINE_COUNT - 2;
        while (++cur_cursor < list->length - 1 && max-- > 0)
        {
            if (st != ((char *)list->items[cur_cursor].text)[0])
                break;
        }
    }

    if (cur_cursor < 0)
        cur_cursor = list->length - 1;
    if (cur_cursor >= list->length)
        cur_cursor = 0;

    list->cursor = cur_cursor;

    if (cur_cursor != old_cursor)
    {
        gui_draw_notice(" ", C_BLACK);
        gui_draw_list(tab);
        gui_event(TAB_SCROLL, tab);
    }
}

void gui_redraw()
{
    tab_t *tab = gui_get_current_tab();
    gui_draw_header(tab);
    gui_draw_status(tab);
    gui_draw_list(tab);
    gui_event(TAB_REDRAW, tab);

    lcd_swap();
}

void gui_draw_navbar()
{
    for (int i = 0; i < gui.tabcount; i++)
    {
        odroid_display_write(i * IMAGE_LOGO_WIDTH, 0, IMAGE_LOGO_WIDTH, IMAGE_LOGO_HEIGHT, gui.tabs[i]->img_logo);
    }
}

void gui_draw_header(tab_t *tab)
{
    if (tab->img_header)
        odroid_display_write(0, ODROID_SCREEN_HEIGHT - IMAGE_BANNER_HEIGHT - 15, IMAGE_BANNER_WIDTH, IMAGE_BANNER_HEIGHT, tab->img_header);

    odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - 15, ODROID_SCREEN_WIDTH, 1, C_GW_OPAQUE_YELLOW);
    odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - 13, ODROID_SCREEN_WIDTH, 4, C_GW_RED);
    odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - 10, ODROID_SCREEN_WIDTH, 2, C_BLACK);
    odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - 8, ODROID_SCREEN_WIDTH, 2, C_GW_RED);
    odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - 6, ODROID_SCREEN_WIDTH, 2, C_BLACK);
    odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - 4, ODROID_SCREEN_WIDTH, 1, C_GW_RED);
    odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - 3, ODROID_SCREEN_WIDTH, 2, C_BLACK);
    odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - 1, ODROID_SCREEN_WIDTH, 1, C_GW_RED);
}

// void gui_draw_notice(tab_t *tab)
void gui_draw_notice(const char *text, uint16_t color)
{
    odroid_overlay_draw_text(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, text, color, C_BLACK);
}

void gui_draw_status(tab_t *tab)
{
    odroid_overlay_draw_fill_rect(0, 0, ODROID_SCREEN_WIDTH, STATUS_HEIGHT, C_GW_RED);
    odroid_overlay_draw_fill_rect(0, 1, ODROID_SCREEN_WIDTH, 2, C_BLACK);
    odroid_overlay_draw_fill_rect(0, 4, ODROID_SCREEN_WIDTH, 2, C_BLACK);
    odroid_overlay_draw_fill_rect(0, 8, ODROID_SCREEN_WIDTH, 2, C_BLACK);

    int max_len = (ODROID_SCREEN_WIDTH - 12) / odroid_overlay_get_local_font_width();

    odroid_overlay_draw_local_text_line(
        6,
        16,
        max_len * odroid_overlay_get_local_font_width(),
        tab->status,
        C_GW_YELLOW,
        C_GW_RED,
        NULL);

    odroid_overlay_draw_battery(ODROID_SCREEN_WIDTH - 32, 17);
}

listbox_item_t *gui_get_selected_prior_item(tab_t *tab)
{
    listbox_t *list = &tab->listbox;

    int x = list->cursor - 1;
    if (x < 0)
        x = list->length - 1;

    if (x >= 0 && x < list->length)
        return &list->items[x];

    return NULL;
}

listbox_item_t *gui_get_selected_next_item(tab_t *tab)
{
    listbox_t *list = &tab->listbox;

    int x = list->cursor + 1;
    if (x >= list->length)
        x = 0;

    if (x >= 0 && x < list->length)
        return &list->items[x];

    return NULL;
}

listbox_item_t *gui_get_item_by_index(tab_t *tab, int *index)
{
    listbox_t *list = &tab->listbox;
    int x = *index;

    if (x < 0)
        x = list->length - 1;

    if (x >= list->length)
        x = 0;

    if (x >= 0 && x < list->length)
    {
        *index = x;
        return &list->items[x];
    }

    return NULL;
}
/*
uint16_t get_darken_pixel(uint16_t color, uint16_t darken)
{
    int16_t r = (int16_t)((color & 0b1111100000000000) * darken / 100) & 0b1111100000000000;
    int16_t g = (int16_t)((color & 0b0000011111100000) * darken / 100) & 0b0000011111100000;
    int16_t b = (int16_t)((color & 0b0000000000011111) * darken / 100) & 0b0000000000011111;
    return r | g | b;
}
*/

#if COVERFLOW == 1

void gui_draw_prior_cover(retro_emulator_file_t *file)
{
    if (file == NULL)
        return;
    uint16_t *src_img = NULL;
    uint16_t *dst_img = lcd_get_active_buffer();
    if (file->img_size == 0)
        src_img = &cover_missed;
    else
        src_img = (uint16_t *)(file->img_address) + 0x46 / 2;
    for (int y = 0; y < 96; y++)
    {
        for (int x = 0; x < 80; x++)
            dst_img[(y + 61) * 320 + 0 + x] = get_darken_pixel(src_img[y * 128 + x + 48], x + 20);
    }
    sprintf(str_buffer, "%s", file->name);
    size_t max_len = 80 / odroid_overlay_get_local_font_width();
    size_t width = strlen(str_buffer) * odroid_overlay_get_local_font_width();
    if (width > 80)
        width = max_len * odroid_overlay_get_local_font_width();
    odroid_overlay_draw_local_text_line(
        80 - width,
        47,    //top
        width, //width
        str_buffer,
        C_GW_OPAQUE_YELLOW,
        C_BLACK,
        NULL);
}

void gui_draw_next_cover(retro_emulator_file_t *file)
{
    if (file == NULL)
        return;
    uint16_t *src_img = NULL;
    uint16_t *dst_img = lcd_get_active_buffer();
    if (file->img_size == 0)
        src_img = &cover_missed;
    else
        src_img = (uint16_t *)(file->img_address) + 0x46 / 2;
    for (int y = 0; y < 96; y++)
    {
        for (int x = 0; x < 80; x++)
            dst_img[(y + 61) * 320 + 240 + x] = get_darken_pixel(src_img[y * 128 + x], 99 - x);
    }
    sprintf(str_buffer, "%s", file->name);
    size_t max_len = 80 / odroid_overlay_get_local_font_width();
    size_t width = strlen(str_buffer) * odroid_overlay_get_local_font_width();
    if (width > 80)
        width = max_len * odroid_overlay_get_local_font_width();
    odroid_overlay_draw_local_text_line(
        240,
        47,    //top
        width, //width
        str_buffer,
        C_GW_OPAQUE_YELLOW,
        C_BLACK,
        NULL);
}

/*33	top	
10	eff	33
4	space	43
6	text	47
8	line-6txt	53
96	pic	61
8	line	157
2	space	165
12	txt	167
4	spae	179
10	eff	183
47	bottom	193
240		240
*/

void gui_draw_current_cover(retro_emulator_file_t *file)
{
    if (file == NULL)
        return;
    uint16_t *cover_buffer = NULL;
    if (file->img_size == 0)
        cover_buffer = &cover_missed;
    else
        cover_buffer = (uint16_t *)(file->img_address) + 0x46 / 2;
    odroid_display_write_rect(96, 61, 128, 96, 128, cover_buffer);
    odroid_overlay_draw_rect(88, 53, 144, 112, 1, C_GW_OPAQUE_YELLOW);
    odroid_overlay_draw_rect(90, 55, 140, 108, 2, C_GW_YELLOW);
    odroid_overlay_draw_rect(93, 58, 134, 102, 1, C_GW_OPAQUE_YELLOW);
    sprintf(str_buffer, "%s", file->name);
    size_t len = strlen(str_buffer);
    size_t max_len = (ODROID_SCREEN_WIDTH - 24) / odroid_overlay_get_local_font_width();
    if (len > max_len)
        len = max_len;
    size_t width = len * odroid_overlay_get_local_font_width();
    odroid_overlay_draw_local_text_line(
        (ODROID_SCREEN_WIDTH - width) / 2,
        167,
        width,
        str_buffer,
        C_GW_YELLOW,
        C_BLACK,
        NULL);
}

void gui_draw_current_cover_h(retro_emulator_file_t *file)
{
    if (file == NULL)
        return;
    uint16_t *cover_buffer = NULL;
    if (file->img_size == 0)
        cover_buffer = &cover_missed;
    else
        cover_buffer = (uint16_t *)(file->img_address) + 0x46 / 2;
    odroid_display_write_rect(4, 65, 128, 96, 128, cover_buffer);
    odroid_overlay_draw_rect(1, 62, 134, 102, 2, C_GW_YELLOW);
}

void gui_draw_prior_cover_h(retro_emulator_file_t *file)
{
    if (file == NULL)
        return;
    uint16_t *cover_buffer = NULL;
    if (file->img_size == 0)
        cover_buffer = &cover_missed;
    else
        cover_buffer = (uint16_t *)(file->img_address) + 0x46 / 2;
    uint16_t *dst_img = lcd_get_active_buffer();
    odroid_overlay_draw_rect(2, 40, 132, 22, 1, C_GW_OPAQUE_YELLOW);
    odroid_overlay_draw_rect(2, 61, 132, 1, 1, C_BLACK);
    odroid_overlay_draw_rect(4, 38, 128, 1, 1, get_darken_pixel(C_GW_OPAQUE_YELLOW, 40));

    for (int y = 0; y < 20; y++)
    {
        for (int x = 0; x < 128; x++)
            dst_img[(y + 41) * 320 + 4 + x] = get_darken_pixel(cover_buffer[y * 128 + x], y + 30);
    }

}

void gui_draw_next_cover_h(retro_emulator_file_t *file)
{
    if (file == NULL)
        return;
    uint16_t *cover_buffer = NULL;
    if (file->img_size == 0)
        cover_buffer = &cover_missed;
    else
        cover_buffer = (uint16_t *)(file->img_address) + 0x46 / 2;
    uint16_t *dst_img = lcd_get_active_buffer();
    odroid_overlay_draw_rect(2, 164, 132, 22, 1, C_GW_OPAQUE_YELLOW);
    odroid_overlay_draw_rect(2, 164, 132, 1, 1, C_BLACK);
    odroid_overlay_draw_rect(4, 187, 128, 1, 1, get_darken_pixel(C_GW_OPAQUE_YELLOW, 80));
    for (int y = 0; y < 20; y++)
    {
        for (int x = 0; x < 128; x++)
            dst_img[(y + 165) * 320 + 4 + x] = get_darken_pixel(cover_buffer[(y + 76) * 128 + x], 50 - y);
    }
}

#endif

void gui_draw_item_postion(int posx, int cur, int size)
{
    int len = strlen(str_buffer);
    int height = len * odroid_overlay_get_font_size();
    uint16_t *dst_img = lcd_get_active_buffer();
    int posy = (cur * (160 - height)) / (size + 1);

    for (int y = 33; y <= posy + 33; y++)
    {
        dst_img[y * ODROID_SCREEN_WIDTH + posx] = get_darken_pixel(C_GW_YELLOW, ((y - 33) * 100) / posy);
    }
    for (int y = posy + 33 + height; y <= 193; y++)
    {
        dst_img[y * ODROID_SCREEN_WIDTH + posx] = get_darken_pixel(C_GW_YELLOW, ((193 - y) * 100) / (160 - posy - height));
    }

    odroid_overlay_draw_fill_rect(
        posx - odroid_overlay_get_font_width() / 2 - 1,
        33 + posy - 2,
        odroid_overlay_get_font_size() + 2, height + 4, C_GW_YELLOW);

    for (int y = 0; y < len; y++)
        odroid_overlay_draw_text_line(
            posx - odroid_overlay_get_font_width() / 2,
            33 + posy + y * odroid_overlay_get_font_size(), //top
            odroid_overlay_get_font_width(),
            &str_buffer[y],
            C_BLACK,
            C_GW_YELLOW);
}

void gui_draw_simple_list(int posx, tab_t *tab)
{
    listbox_t *list = &tab->listbox;
    if (list->cursor >= 0 && list->cursor < list->length)
    {
        //draw currpostion
        sprintf(str_buffer, "%d/%d", list->cursor + 1, list->length);
        gui_draw_item_postion(ODROID_SCREEN_WIDTH - 5, list->cursor + 1, list->length);
        int w = (ODROID_SCREEN_WIDTH - posx - 14) / odroid_overlay_get_local_font_width();
        w = w * odroid_overlay_get_local_font_width();
        listbox_item_t *item = &list->items[list->cursor];
        if (item) //drawcurrent title
            //gui_draw_current_cover((retro_emulator_file_t *)item->arg);
            //&item->text
            //list->items[entry].text
            odroid_overlay_draw_local_text_line(posx, 107, w, list->items[list->cursor].text, C_GW_YELLOW, C_BLACK, NULL);

        int index_next = list->cursor + 1;
        int index_proior = list->cursor - 1;
        //up & down
        int h1 = 107;
        int h2 = 107;
        for (int i = 0; i < 5; i++)
        {
            listbox_item_t *next_item = gui_get_item_by_index(tab, &index_next);
            h1 = h1 + 12 + 4 - i;
            if (next_item)
                odroid_overlay_draw_local_text_line(
                    posx,
                    h1,
                    w,
                    //&(next_item).text,
                    list->items[index_next].text,
                    get_darken_pixel(C_GW_YELLOW, 60 - i * 10),
                    C_BLACK,
                    NULL);
            index_next++;
            listbox_item_t *prior_item = gui_get_item_by_index(tab, &index_proior);
            h2 = h2 - 12 - 4 + i;
            if (prior_item)
                odroid_overlay_draw_local_text_line(
                    posx,
                    h2,
                    w,
                    //&(prior_item).text,
                    list->items[index_proior].text,
                    get_darken_pixel(C_GW_YELLOW, 60 - i * 10),
                    C_BLACK,
                    NULL);
            index_proior--;
        }
    }
}

void gui_draw_list(tab_t *tab)
{
    odroid_overlay_draw_fill_rect(0, LIST_Y_OFFSET, LIST_WIDTH, LIST_HEIGHT, C_BLACK);
    /* 
    for (int y = 0; y < 8; y++) {
        odroid_overlay_draw_fill_rect(0, LIST_Y_OFFSET + y, LIST_WIDTH, 1, get_darken_pixel(C_GW_RED, 100 - (y + 1) * 10));
        odroid_overlay_draw_fill_rect(0, LIST_Y_OFFSET + LIST_HEIGHT - 1 - y, LIST_WIDTH, 1, get_darken_pixel(C_GW_RED, 100 - (y + 1) * 10));
    }
*/

#if COVERFLOW == 1
    int theme_index = odroid_overlay_get_font_size();
    odroid_overlay_set_font_size(0);
    theme_index = 0;
    switch (theme_index)
    {
    case 2:
    {
        listbox_t *list = &tab->listbox;
        if (list->cursor >= 0 && list->cursor < list->length)
        {
            //draw currpostion
            sprintf(str_buffer, "%d/%d", list->cursor + 1, list->length);
            //
            int width = strlen(str_buffer) * odroid_overlay_get_font_width();
            odroid_overlay_draw_fill_rect((ODROID_SCREEN_WIDTH - width) / 2 - 2, 42, width + 4, 12, C_GW_YELLOW);
            odroid_overlay_draw_text_line((ODROID_SCREEN_WIDTH - width) / 2, 44, width, str_buffer, C_BLACK, C_GW_YELLOW);
            listbox_item_t *item = &list->items[list->cursor];
            if (item)
                gui_draw_current_cover((retro_emulator_file_t *)item->arg);
            int index = list->cursor + 1;
            listbox_item_t *next_item = gui_get_item_by_index(tab, &index);
            if (next_item)
                gui_draw_next_cover((retro_emulator_file_t *)next_item->arg);
            index = list->cursor - 1;
            listbox_item_t *prior_item = gui_get_item_by_index(tab, &index);
            if (prior_item)
                gui_draw_prior_cover((retro_emulator_file_t *)prior_item->arg);
        }
    }
    break;
    case 1:
    {
        listbox_t *list = &tab->listbox;
        if (list->cursor >= 0 && list->cursor < list->length)
        {
            //draw currpostion
            listbox_item_t *item = &list->items[list->cursor];
            if (item)
                gui_draw_current_cover_h((retro_emulator_file_t *)item->arg);
            int index = list->cursor + 1;
            listbox_item_t *next_item = gui_get_item_by_index(tab, &index);
            if (next_item)
                gui_draw_next_cover_h((retro_emulator_file_t *)next_item->arg);
            index = list->cursor - 1;
            listbox_item_t *prior_item = gui_get_item_by_index(tab, &index);
            if (prior_item)
                gui_draw_prior_cover_h((retro_emulator_file_t *)prior_item->arg);

            gui_draw_simple_list(128 + 8 + 6, tab);
        }
    }
    break;
    default:
        //drawlist without image;
        gui_draw_simple_list(12, tab);
    }
#else
    gui_draw_simple_list(12, tab);

#endif
}

void gui_draw_cover(retro_emulator_file_t *file)
{
    //nothing
}