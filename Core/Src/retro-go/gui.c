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

#if COVERFLOW != 0
#define COVER_HEIGHT (96)
#define COVER_WIDTH (128)
#endif


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

    if (list->length == 0 || list->cursor > list->length)
    {
        return;
    }

    int cur_cursor = list->cursor;
    int old_cursor = list->cursor;

    if (mode == LINE_UP)
    {
        cur_cursor--;
    }
    else if (mode == LINE_DOWN)
    {
        cur_cursor++;
    }
    else if (mode == PAGE_UP)
    {
        char st = ((char *)list->items[cur_cursor].text)[0];
        int max = LIST_LINE_COUNT - 2;
        while (--cur_cursor > 0 && max-- > 0)
        {
            if (st != ((char *)list->items[cur_cursor].text)[0])
                break;
        }
    }
    else if (mode == PAGE_DOWN)
    {
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

    if (tab->img_header)
    {
        int max_len = (ODROID_SCREEN_WIDTH - 12) / odroid_overlay_get_local_font_width();
        odroid_overlay_draw_local_text_line(
            6,
            16,
            max_len * odroid_overlay_get_local_font_width(),
            tab->status,
            C_GW_YELLOW,
            C_GW_RED,
            NULL,
            0);
    }
    else
    {
        int max_len = (ODROID_SCREEN_WIDTH - 12) / odroid_overlay_get_font_width();
        odroid_overlay_draw_text_line(
            6,
            18,
            max_len * odroid_overlay_get_font_width(),
            tab->status,
            C_GW_YELLOW,
            C_GW_RED);
    }
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

void gui_draw_item_postion_v(int posx, int starty, int endy, int cur, int size)
{
    sprintf(str_buffer, "%d", size);
    int len = strlen(str_buffer);
    sprintf(str_buffer, "%0*d/%0*d", len, cur, len, size);
    len = strlen(str_buffer);
    int height = len * odroid_overlay_get_font_size();
    uint16_t *dst_img = lcd_get_active_buffer();
    int posy = (cur * (endy - starty + 1 - height)) / (size + 1);
    //posy = (posy < 0) ? 0 : posy;

    for (int y = starty; y <= starty + posy; y++)
        dst_img[y * ODROID_SCREEN_WIDTH + posx] = get_darken_pixel(C_GW_YELLOW, ((y - starty + 1) * 90) / posy + 10);
    for (int y = posy + starty + height; y <= endy; y++)
        dst_img[y * ODROID_SCREEN_WIDTH + posx] = get_darken_pixel(C_GW_YELLOW, ((endy - y + 1) * 90) / (endy - starty - posy - height + 1) + 10);

    odroid_overlay_draw_fill_rect(
        posx - odroid_overlay_get_font_width() / 2 - 1,
        starty + posy - 1,
        odroid_overlay_get_font_size() + 2, height + 2, C_GW_YELLOW);
    odroid_overlay_draw_fill_rect(
        posx - odroid_overlay_get_font_width() / 2,
        starty + posy - 2,
        odroid_overlay_get_font_width(), 1, C_GW_OPAQUE_YELLOW);
    odroid_overlay_draw_fill_rect(
        posx - odroid_overlay_get_font_width() / 2,
        starty + posy + height + 1,
        odroid_overlay_get_font_width(), 1, C_GW_OPAQUE_YELLOW);

    for (int y = 0; y < len; y++)
        odroid_overlay_draw_text_line(
            posx - odroid_overlay_get_font_width() / 2,
            starty + posy + y * odroid_overlay_get_font_size(), //top
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
        int font_height = odroid_overlay_get_local_font_size();
        int w = (ODROID_SCREEN_WIDTH - posx - 10) / odroid_overlay_get_local_font_width();
        w = w * odroid_overlay_get_local_font_width();
        listbox_item_t *item = &list->items[list->cursor];
        int h1 = LIST_Y_OFFSET + (LIST_HEIGHT - font_height) / 2;
        if (item)
            odroid_overlay_draw_local_text_line(posx, h1, w, list->items[list->cursor].text, C_GW_YELLOW, C_BLACK, NULL, 0);

        int index_next = list->cursor + 1;
        int index_proior = list->cursor - 1;
        int max_line = (LIST_HEIGHT - font_height) / font_height / 2;
        int h2 = h1;
        h1++;
        for (int i = 0; i < max_line; i++)
        {
            listbox_item_t *next_item = gui_get_item_by_index(tab, &index_next);
            h1 = h1 + font_height + max_line - i;
            h2 = h2 - font_height - max_line + i;
            if (h2 < LIST_Y_OFFSET)   //out range;
                break;
            if (next_item)
                odroid_overlay_draw_local_text_line(
                    posx,
                    h1,
                    w,
                    list->items[index_next].text,
                    get_darken_pixel(C_GW_OPAQUE_YELLOW, (max_line - i) * 100 / max_line),
                    C_BLACK,
                    NULL,
                    0);
            index_next++;
            listbox_item_t *prior_item = gui_get_item_by_index(tab, &index_proior);
            if (prior_item)
                odroid_overlay_draw_local_text_line(
                    posx,
                    h2,
                    w,
                    list->items[index_proior].text,
                    get_darken_pixel(C_GW_OPAQUE_YELLOW, (max_line - i) * 100 / max_line),
                    C_BLACK,
                    NULL,
                    0);
            index_proior--;
        }
        //draw currpostion
        gui_draw_item_postion_v(ODROID_SCREEN_WIDTH - 5, LIST_Y_OFFSET + 4, LIST_Y_OFFSET + LIST_HEIGHT - 4, list->cursor + 1, list->length);
    }
}

#if COVERFLOW != 0

static void draw_centered_local_text_line(uint16_t y_pos,
                                          const char *text,
                                          uint16_t x1,
                                          uint16_t x2,
                                          uint16_t color,
                                          uint16_t color_bg)
{
    int width = strlen(text) * odroid_overlay_get_local_font_width();
    int x_pos = (x2 - x1) / 2 - width / 2;
    if (x_pos < 0)
        x_pos = 0;
    if (width > (x2 - x1))
        width = x2 - x1;

    odroid_overlay_draw_local_text_line(x_pos + x1, y_pos, width, text, color, color_bg, NULL, 0);
}

void gui_draw_item_postion_h(int posy, int startx, int endx, int cur, int size)
{
    sprintf(str_buffer, "%d", size);
    int len = strlen(str_buffer);
    sprintf(str_buffer, "%0*d/%0*d", len, cur, len, size);
    len = strlen(str_buffer);
    int width = len * odroid_overlay_get_font_width();
    uint16_t *dst_img = lcd_get_active_buffer();
    int posx = (cur * (endx - startx + 1 - width)) / (size + 1);

    for (int x = startx; x <= startx + posx; x++)
    {
        dst_img[(posy - 1) * ODROID_SCREEN_WIDTH + x] = get_darken_pixel(C_GW_OPAQUE_YELLOW, 100 - ((x - startx + 1) * 90) / posx);
        dst_img[(posy - 2) * ODROID_SCREEN_WIDTH + x] = get_darken_pixel(C_GW_YELLOW, 100 - ((x - startx + 1) * 90) / posx);
    }
    for (int x = posx + startx + width; x <= endx; x++)
    {
        dst_img[(posy - 1) * ODROID_SCREEN_WIDTH + x] = get_darken_pixel(C_GW_OPAQUE_YELLOW, 100 - ((endx - x + 1) * 90) / (endx - startx - posx - width + 1));
        dst_img[(posy - 2) * ODROID_SCREEN_WIDTH + x] = get_darken_pixel(C_GW_YELLOW, 100 - ((endx - x + 1) * 90) / (endx - startx - posx - width + 1));
    }

    odroid_overlay_draw_text_line(
        posx + startx,
        posy - odroid_overlay_get_font_size(), //top
        width,
        str_buffer,
        C_GW_YELLOW,
        C_BLACK);
}

void gui_draw_coverflow_h(tab_t *tab) //------------
{
    int font_height = odroid_overlay_get_local_font_size();
    int cover_height = COVER_HEIGHT;
    int cover_width = COVER_WIDTH;
    int space_width = 26;
    //left _|_|__|_(pl)__||_(main)_||__(pr)_|__|_|_ min 26 pixel space;
    int p_width = (ODROID_SCREEN_WIDTH - cover_width - space_width) / 2;
    //p_width must big than 1;
    p_width = (p_width > cover_width) ? cover_width : p_width; //space width than real width, draw full size;
    int start_xpos = (ODROID_SCREEN_WIDTH - ((p_width * 2) + cover_width + space_width)) / 2;
    //fisrt left point pos getted, get fisrt top point;
    int v_space = LIST_HEIGHT - (cover_height + 6);
    uint8_t draw_bot_title = v_space > (font_height + 5 + 8) ? 1 : 0;  //(12 = 3 * 4)
    uint8_t draw_top_title = v_space > (font_height * 2 + 12) ? 1 : 0; //(14 = 4 + 1 + 4 + 4)
    int cover_top = 0;
    int top_tit_pos = 0;
    int bot_tit_pos = 0;
    if (draw_top_title == 1)
    {
        top_tit_pos = STATUS_HEIGHT + (v_space - (font_height * 2) - 5) / 2;
        cover_top = top_tit_pos + font_height + 4;
    }
    else if (draw_bot_title == 1)
    {
        cover_top = STATUS_HEIGHT + (v_space - font_height - 5) / 3 + 8; // 8 = (tit pos)
    }
    else
    {
        cover_top = STATUS_HEIGHT + (v_space - 5) / 2 + 8; //
    }
    bot_tit_pos = cover_top + cover_height + 3 + ((v_space - font_height - 5) / 3 > 4 ? 4 : (v_space - font_height - 5) / 3); //(3+4)
    //let's start draw effect;
    uint16_t *dst_img = lcd_get_active_buffer();
    odroid_overlay_draw_fill_rect(start_xpos + 1, cover_top + 2, 1, cover_height - 6, get_darken_pixel(C_GW_OPAQUE_YELLOW, 70));
    odroid_overlay_draw_fill_rect(start_xpos + 3, cover_top, 1, cover_height - 1, get_darken_pixel(C_GW_OPAQUE_YELLOW, 80));
    odroid_overlay_draw_rect(start_xpos + 6, cover_top - 2, p_width + 2, cover_height + 4, 1, get_darken_pixel(C_GW_OPAQUE_YELLOW, 80));

    odroid_overlay_draw_rect(start_xpos + p_width + 10, cover_top - 3, cover_width + 6, cover_height + 6, 1, C_GW_YELLOW);
    odroid_overlay_draw_rect(start_xpos + p_width + 11, cover_top - 2, cover_width + 4, cover_height + 4, 1, C_GW_OPAQUE_YELLOW);

    odroid_overlay_draw_rect(start_xpos + p_width + cover_width + 19, cover_top - 2, p_width + 2, cover_height + 4, 1, get_darken_pixel(C_GW_OPAQUE_YELLOW, 80));
    odroid_overlay_draw_fill_rect(start_xpos + p_width * 2 + cover_width + 23, cover_top, 1, cover_height - 1, get_darken_pixel(C_GW_OPAQUE_YELLOW, 80));
    odroid_overlay_draw_fill_rect(start_xpos + p_width * 2 + cover_width + 25, cover_top + 2, 1, cover_height - 6, get_darken_pixel(C_GW_OPAQUE_YELLOW, 70));

    odroid_overlay_draw_fill_rect(start_xpos + p_width + 7, cover_top - 1, 1, cover_height + 2, C_BLACK);
    odroid_overlay_draw_fill_rect(start_xpos + p_width + cover_width + 19, cover_top - 1, 1, cover_height + 2, C_BLACK);

    listbox_t *list = &tab->listbox;
    if (list->cursor >= 0 && list->cursor < list->length)
        gui_draw_item_postion_h(cover_top - 1, start_xpos + p_width + 12, start_xpos + p_width + cover_width + 12, list->cursor + 1, list->length);
    else
        return;
    listbox_item_t *item = &list->items[list->cursor];
    uint16_t *cover_buffer = NULL;
    retro_emulator_file_t *file = NULL;
    if (item) //current page
    {
        file = (retro_emulator_file_t *)item->arg;
        if (file->img_size == 0)
        {
            draw_centered_local_text_line(cover_top + (cover_height - font_height) / 2,
                                          s_No_Cover,
                                          start_xpos + p_width + 13,
                                          start_xpos + p_width + 13 + cover_width,
                                          get_darken_pixel(C_GW_RED, 80),
                                          C_BLACK);

            if (!draw_bot_title)
            {
                sprintf(str_buffer, "%s", file->name);
                size_t len = strlen(str_buffer);
                size_t width = len * odroid_overlay_get_local_font_width();
                width = width > cover_width ? cover_width : width;
                width = (width / odroid_overlay_get_local_font_width()) * odroid_overlay_get_local_font_width();
                odroid_overlay_draw_local_text(
                    start_xpos + p_width + 14,
                    cover_top + 4,
                    width,
                    str_buffer,
                    C_GW_YELLOW,
                    C_BLACK,
                    0);
            }
        }
        else
        {
            cover_buffer = (uint16_t *)(file->img_address);
            odroid_display_write_rect(start_xpos + p_width + 13, cover_top, cover_width, cover_height, cover_width, cover_buffer);
        }
        if (draw_bot_title)
        {
            sprintf(str_buffer, "%s", file->name);
            size_t len = strlen(str_buffer);
            size_t max_len = (ODROID_SCREEN_WIDTH - 24) / odroid_overlay_get_local_font_width();
            if (len > max_len)
                len = max_len;
            size_t width = len * odroid_overlay_get_local_font_width();
            odroid_overlay_draw_local_text_line(
                (ODROID_SCREEN_WIDTH - width) / 2,
                bot_tit_pos,
                width,
                str_buffer,
                C_GW_YELLOW,
                C_BLACK,
                NULL,
                0);
        }
    }
    int index = list->cursor + 1;
    item = gui_get_item_by_index(tab, &index);
    if (item)
    {
        file = (retro_emulator_file_t *)item->arg;
        if (file->img_size == 0)
        {
            draw_centered_local_text_line(cover_top + (cover_height - font_height) / 2,
                                          s_No_Cover,
                                          start_xpos + p_width + cover_width + 19,
                                          start_xpos + p_width + cover_width + 19 + p_width,
                                          get_darken_pixel(C_GW_OPAQUE_YELLOW, 80),
                                          C_BLACK);
            if ((!draw_top_title) && (p_width > (odroid_overlay_get_local_font_width() * 4)))
            {
                sprintf(str_buffer, "%s", file->name);
                size_t len = strlen(str_buffer);
                size_t width = len * odroid_overlay_get_local_font_width();
                width = width > p_width ? p_width : width;
                width = (width / odroid_overlay_get_local_font_width()) * odroid_overlay_get_local_font_width();
                odroid_overlay_draw_local_text(
                    start_xpos + p_width + cover_width + 19,
                    cover_top + 4,
                    width,
                    str_buffer,
                    C_GW_OPAQUE_YELLOW,
                    C_BLACK,
                    0);
            }
        }
        else
        {
            cover_buffer = (uint16_t *)(file->img_address);
            for (int y = 0; y < cover_height; y++)
                for (int x = 0; x < p_width; x++)
                    dst_img[(y + cover_top) * ODROID_SCREEN_WIDTH + start_xpos + p_width + cover_width + 19 + x] =
                        get_darken_pixel(cover_buffer[y * cover_width + (cover_width - p_width) + x], 20 + x * 60 / p_width);
        };
        if (draw_top_title)
        {
            sprintf(str_buffer, "%s", file->name);
            size_t max_len = (p_width + 4) / odroid_overlay_get_local_font_width();
            size_t width = strlen(str_buffer) * odroid_overlay_get_local_font_width();
            if (width > (p_width + 4))
                width = max_len * odroid_overlay_get_local_font_width();
            odroid_overlay_draw_local_text_line(
                start_xpos + p_width * 2 + cover_width + 23 - width, //232,
                top_tit_pos,                                         //top
                width,                                               //width
                str_buffer,
                C_GW_OPAQUE_YELLOW,
                C_BLACK,
                NULL,
                0);
        };
    };
    index = list->cursor - 1;
    item = gui_get_item_by_index(tab, &index);
    if (item)
    {
        file = (retro_emulator_file_t *)item->arg;
        if (file->img_size == 0)
        {
            draw_centered_local_text_line(cover_top + (cover_height - font_height) / 2,
                                          s_No_Cover,
                                          start_xpos + 8,
                                          start_xpos + 8 + p_width,
                                          get_darken_pixel(C_GW_OPAQUE_YELLOW, 80),
                                          C_BLACK);
            if ((!draw_top_title) && (p_width > odroid_overlay_get_local_font_width() * 4))
            {
                sprintf(str_buffer, "%s", file->name);
                size_t len = strlen(str_buffer);
                size_t width = len * odroid_overlay_get_local_font_width();
                width = width > p_width ? p_width : width;
                width = (width / odroid_overlay_get_local_font_width()) * odroid_overlay_get_local_font_width();
                odroid_overlay_draw_local_text(
                    start_xpos + 8,
                    cover_top + 4,
                    width,
                    str_buffer,
                    C_GW_OPAQUE_YELLOW,
                    C_BLACK,
                    0);
            }
        }
        else
        {
            cover_buffer = (uint16_t *)(file->img_address);
            for (int y = 0; y < cover_height; y++)
                for (int x = 0; x < p_width; x++)
                    dst_img[(y + cover_top) * ODROID_SCREEN_WIDTH + start_xpos + 8 + x] =
                        get_darken_pixel(cover_buffer[y * cover_width + x], 80 - x * 60 / p_width);
        };
        if (draw_top_title)
        {
            sprintf(str_buffer, "%s", file->name);
            size_t max_len = (p_width + 4) / odroid_overlay_get_local_font_width();
            size_t width = strlen(str_buffer) * odroid_overlay_get_local_font_width();
            if (width > (p_width + 4))
                width = max_len * odroid_overlay_get_local_font_width();
            odroid_overlay_draw_local_text_line(
                start_xpos + 8,
                top_tit_pos, //top
                width,       //width
                str_buffer,
                C_GW_OPAQUE_YELLOW,
                C_BLACK,
                NULL,
                0);
        };
    };
};

void gui_draw_coverflow_v(tab_t *tab, int start_posx) // ||||||||
{
    int font_height = odroid_overlay_get_local_font_size();
    int cover_height = COVER_HEIGHT;
    int cover_width = COVER_WIDTH;
    int space_height = 40;
    //top ____|_|__|_(pl)__||_(main)_||__(pr)_|__|_|____ min 40;
    int p_height = (LIST_HEIGHT - cover_height - space_height) / 2;
    p_height = (p_height > cover_height) ? cover_height : p_height; //space width than real width, draw full size;
    p_height = p_height < 0 ? 0 : p_height;
    //real height = 32-8 = 24 //max = 136
    int start_ypos = STATUS_HEIGHT + (LIST_HEIGHT - ((p_height * 2) + cover_height + space_height)) / 2 + 4;
    //fisrt top point pos getted;
    start_ypos = start_ypos < 0 ? 0 : start_ypos;

    uint16_t *dst_img = lcd_get_active_buffer();

    odroid_overlay_draw_fill_rect(start_posx + 5, start_ypos + 4, cover_width - 6, 1, get_darken_pixel(C_GW_OPAQUE_YELLOW, 70));
    odroid_overlay_draw_fill_rect(start_posx + 3, start_ypos + 6, cover_width, 1, get_darken_pixel(C_GW_OPAQUE_YELLOW, 80));

    odroid_overlay_draw_rect(start_posx + 1, start_ypos + 9, cover_width + 4, p_height + 2, 1, get_darken_pixel(C_GW_OPAQUE_YELLOW, 80));

    odroid_overlay_draw_rect(start_posx, start_ypos + 13 + p_height, cover_width + 6, cover_height + 6, 1, C_GW_YELLOW);
    odroid_overlay_draw_rect(start_posx + 1, start_ypos + 14 + p_height, cover_width + 4, cover_height + 4, 1, C_GW_OPAQUE_YELLOW);

    odroid_overlay_draw_rect(start_posx + 1, start_ypos + p_height + cover_height + 21, cover_width + 4, p_height + 2, 1, get_darken_pixel(C_GW_OPAQUE_YELLOW, 80));

    odroid_overlay_draw_fill_rect(start_posx + 3, start_ypos + 2 * p_height + cover_height + 25, cover_width, 1, get_darken_pixel(C_GW_OPAQUE_YELLOW, 80));
    odroid_overlay_draw_fill_rect(start_posx + 5, start_ypos + 2 * p_height + cover_height + 27, cover_width - 6, 1, get_darken_pixel(C_GW_OPAQUE_YELLOW, 70));

    if (p_height)
    {
        odroid_overlay_draw_fill_rect(start_posx + 2, start_ypos + p_height + 10, cover_width + 2, 1, C_BLACK);
        odroid_overlay_draw_fill_rect(start_posx + 2, start_ypos + p_height + cover_height + 21, cover_width + 2, 1, C_BLACK);
    }

    listbox_t *list = &tab->listbox;
    listbox_item_t *item = &list->items[list->cursor];
    uint16_t *cover_buffer = NULL;
    retro_emulator_file_t *file = NULL;
    if (item) //current page
    {
        file = (retro_emulator_file_t *)item->arg;
        if (file->img_size == 0)
            draw_centered_local_text_line(start_ypos + p_height + 16 + (cover_height - font_height) / 2,
                                          s_No_Cover,
                                          start_posx + 3,
                                          start_posx + 3 + cover_width,
                                          get_darken_pixel(C_GW_RED, 80),
                                          C_BLACK);
        else
        {
            cover_buffer = (uint16_t *)(file->img_address);
            odroid_display_write_rect(start_posx + 3, start_ypos + p_height + 16, cover_width, cover_height, cover_width, cover_buffer);
        };
    }
    if (p_height)
    {
        int index = list->cursor + 1;
        item = gui_get_item_by_index(tab, &index);
        if (item)
        {
            file = (retro_emulator_file_t *)item->arg;
            if (file->img_size == 0)
            {
                if (p_height > font_height)
                    draw_centered_local_text_line(start_ypos + p_height + cover_height + 21 + (p_height - font_height) / 2,
                                                  s_No_Cover,
                                                  start_posx + 3,
                                                  start_posx + 3 + cover_width,
                                                  get_darken_pixel(C_GW_OPAQUE_YELLOW, 80),
                                                  C_BLACK);
            }
            else
            {
                cover_buffer = (uint16_t *)(file->img_address);
                for (int y = 0; y < p_height; y++)
                    for (int x = 0; x < cover_width; x++)
                        dst_img[(start_ypos + p_height + cover_height + 21 + y) * ODROID_SCREEN_WIDTH + start_posx + 3 + x] =
                            get_darken_pixel(cover_buffer[(cover_height - p_height + y) * cover_width + x], 40 + y * 40 / p_height);
            }
        }
        index = list->cursor - 1;
        item = gui_get_item_by_index(tab, &index);
        if (item)
        {
            file = (retro_emulator_file_t *)item->arg;
            if (file->img_size == 0)
            {
                if (p_height > font_height)
                    draw_centered_local_text_line(start_ypos + 11 + (p_height - font_height) / 2,
                                                  s_No_Cover,
                                                  start_posx + 3,
                                                  start_posx + 3 + cover_width,
                                                  get_darken_pixel(C_GW_OPAQUE_YELLOW, 80),
                                                  C_BLACK);
            }
            else
            {
                cover_buffer = (uint16_t *)(file->img_address);
                for (int y = 0; y < p_height; y++)
                    for (int x = 0; x < cover_width; x++)
                        dst_img[(start_ypos + 11 + y) * ODROID_SCREEN_WIDTH + start_posx + 3 + x] =
                            get_darken_pixel(cover_buffer[y * cover_width + x], 80 - y * 40 / p_height);
            }
        }
    }
    gui_draw_simple_list(start_posx + cover_width + 12, tab);
}

#endif

void gui_draw_list(tab_t *tab)
{
    odroid_overlay_draw_fill_rect(0, LIST_Y_OFFSET, LIST_WIDTH, LIST_HEIGHT, C_BLACK);

#if COVERFLOW != 0
    int theme_index = odroid_settings_theme_get();
    //theme_index = 0;
    switch (theme_index)
    {
    case 2:
        gui_draw_coverflow_h(tab);
        break;
    case 1:
        gui_draw_coverflow_v(tab, 4);
        break;
    default:
        gui_draw_simple_list(10, tab);
    }

#else
    gui_draw_simple_list(10, tab);
#endif
}

void gui_draw_cover(retro_emulator_file_t *file)
{
    //nothing
}