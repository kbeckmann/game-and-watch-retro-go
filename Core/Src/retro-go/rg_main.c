#include <odroid_system.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rg_emulators.h"
#include "rg_favorites.h"
#include "gui.h"
#include "githash.h"
#include "main.h"

#if 0
#define KEY_SELECTED_TAB  "SelectedTab"
#define KEY_GUI_THEME     "ColorTheme"
#define KEY_SHOW_EMPTY    "ShowEmptyTabs"
#define KEY_SHOW_COVER    "ShowGameCover"

static bool font_size_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int font_size = odroid_overlay_get_font_size();
    if (event == ODROID_DIALOG_PREV && font_size > 8) {
        odroid_overlay_set_font_size(font_size -= 4);
        gui_redraw();
    }
    if (event == ODROID_DIALOG_NEXT && font_size < 16) {
        odroid_overlay_set_font_size(font_size += 4);
        gui_redraw();
    }
    sprintf(option->value, "%d", font_size);
    if (font_size ==  8) strcpy(option->value, "Small ");
    if (font_size == 12) strcpy(option->value, "Medium");
    if (font_size == 16) strcpy(option->value, "Large ");
    return event == ODROID_DIALOG_ENTER;
}

static bool show_empty_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        gui.show_empty = !gui.show_empty;
        odroid_settings_int32_set(KEY_SHOW_EMPTY, gui.show_empty);
    }
    strcpy(option->value, gui.show_empty ? "Yes" : "No");
    return event == ODROID_DIALOG_ENTER;
}

static bool startup_app_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int startup_app = odroid_settings_StartupApp_get();
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        startup_app = startup_app ? 0 : 1;
        odroid_settings_StartupApp_set(startup_app);
    }
    strcpy(option->value, startup_app == 0 ? "Launcher" : "LastUsed");
    return event == ODROID_DIALOG_ENTER;
}

static bool show_cover_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV) {
        if (--gui.show_cover < 0) gui.show_cover = 2;
        odroid_settings_int32_set(KEY_SHOW_COVER, gui.show_cover);
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (++gui.show_cover > 2) gui.show_cover = 0;
        odroid_settings_int32_set(KEY_SHOW_COVER, gui.show_cover);
    }
    if (gui.show_cover == 0) strcpy(option->value, "No");
    if (gui.show_cover == 1) strcpy(option->value, "Slow");
    if (gui.show_cover == 2) strcpy(option->value, "Fast");
    return event == ODROID_DIALOG_ENTER;
}

static bool color_shift_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int max = gui_themes_count - 1;
    if (event == ODROID_DIALOG_PREV) {
        if (--gui.theme < 0) gui.theme = max;
        odroid_settings_int32_set(KEY_GUI_THEME, gui.theme);
        gui_redraw();
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (++gui.theme > max) gui.theme = 0;
        odroid_settings_int32_set(KEY_GUI_THEME, gui.theme);
        gui_redraw();
    }
    sprintf(option->value, "%d/%d", gui.theme + 1, max + 1);
    return event == ODROID_DIALOG_ENTER;
}

#endif
static inline bool tab_enabled(tab_t *tab)
{
    int disabled_tabs = 0;

    if (gui.show_empty)
        return true;

    // If all tabs are disabled then we always return true, otherwise it's an endless loop
    for (int i = 0; i < gui.tabcount; ++i)
        if (gui.tabs[i]->initialized && gui.tabs[i]->is_empty)
            disabled_tabs++;

    return (disabled_tabs == gui.tabcount) || (tab->initialized && !tab->is_empty);
}

void retro_loop()
{
    tab_t *tab = gui_get_current_tab();
    int last_key = -1;
    int repeat = 0;
    int selected_tab_last = -1;

    // Read the initial state as to not trigger on button held down during boot
    odroid_input_read_gamepad(&gui.joystick);

    for (int i = 0; i < ODROID_INPUT_MAX; i++) {
        if (gui.joystick.values[i]) last_key = i;
    }

    // gui.selected   = odroid_settings_int32_get(KEY_SELECTED_TAB, 0);
    // gui.theme      = odroid_settings_int32_get(KEY_GUI_THEME, 0);
    // gui.show_empty = odroid_settings_int32_get(KEY_SHOW_EMPTY, 1);
    // gui.show_cover = odroid_settings_int32_get(KEY_SHOW_COVER, 1);

    while (true)
    {
        wdog_refresh();
        if (gui.selected != selected_tab_last)
        {
            int direction = (gui.selected - selected_tab_last) < 0 ? -1 : 1;

            tab = gui_set_current_tab(gui.selected);
            if (!tab->initialized)
            {
                gui_redraw();
                gui_init_tab(tab);
                if (tab_enabled(tab))
                {
                    gui_draw_status(tab);
                    gui_draw_list(tab);
                }
            }
            else if (tab_enabled(tab))
            {
                gui_redraw();
            }

            if (!tab_enabled(tab))
            {
                gui.selected += direction;
                continue;
            }

            selected_tab_last = gui.selected;
        }

        odroid_input_read_gamepad(&gui.joystick);

        if (gui.idle_counter > 0 && gui.joystick.bitmask == 0)
        {
            gui_event(TAB_IDLE, tab);

            if (gui.idle_counter % 100 == 0)
                gui_draw_status(tab);
        }

        if ((last_key < 0) || ((repeat >= 30) && (repeat % 5 == 0))) {
            for (int i = 0; i < ODROID_INPUT_MAX; i++)
                if (gui.joystick.values[i]) last_key = i;

            if (last_key == ODROID_INPUT_START) {
                odroid_dialog_choice_t choices[] = {
                    {0, "Ver.", GIT_HASH, 1, NULL},
                    {0, "By", "ducalex", 1, NULL},
                    {0, "", "kbeckmann", 1, NULL},
                    {0, "", "stacksmashing", 1, NULL},
                    {0, "", "", -1, NULL},
                    {1, "Reset settings", "", 1, NULL},
                    {0, "Close", "", 1, NULL},
                    ODROID_DIALOG_CHOICE_LAST
                };

                int sel = odroid_overlay_dialog("Retro-Go", choices, -1);
                if (sel == 1) {
                    if (odroid_overlay_confirm("Reset all settings? (TODO)", false) == 1) {
                        odroid_settings_reset();
                        odroid_system_switch_app(0); // reset
                    }
                }
                gui_redraw();
            }
            else if (last_key == ODROID_INPUT_VOLUME) {
                odroid_dialog_choice_t choices[] = {
                    // {0, "---", "", -1, NULL},
                    // {0, "Color theme", "1/10", 1, &color_shift_cb},
                    // {0, "Font size", "Small", 1, &font_size_cb},
                    // {0, "Show cover", "Yes", 1, &show_cover_cb},
                    // {0, "Show empty", "Yes", 1, &show_empty_cb},
                    // {0, "---", "", -1, NULL},
                    // {0, "Startup app", "Last", 1, &startup_app_cb},
                    ODROID_DIALOG_CHOICE_LAST
                };
                odroid_overlay_settings_menu(choices);
                gui_redraw();
            }
            else if (last_key == ODROID_INPUT_UP) {
                gui_scroll_list(tab, LINE_UP);
                repeat++;
            }
            else if (last_key == ODROID_INPUT_DOWN) {
                gui_scroll_list(tab, LINE_DOWN);
                repeat++;
            }
            else if (last_key == ODROID_INPUT_LEFT) {
                gui.selected--;
                if(gui.selected < 0) {
                    gui.selected = gui.tabcount - 1;
                }
                repeat++;
            }
            else if (last_key == ODROID_INPUT_RIGHT) {
                gui.selected++;
                if(gui.selected >= gui.tabcount) {
                    gui.selected = 0;
                }
                repeat++;
            }
            else if (last_key == ODROID_INPUT_A) {
                gui_event(KEY_PRESS_A, tab);
            }
            else if (last_key == ODROID_INPUT_B) {
                gui_event(KEY_PRESS_B, tab);
            }
            else if (last_key == ODROID_INPUT_POWER) {
                GW_EnterDeepSleep();
            }
        }
        if (repeat > 0)
            repeat++;
        if (last_key >= 0) {
            if (!gui.joystick.values[last_key]) {
                last_key = -1;
                repeat = 0;
            }
        }

        if (gui.joystick.bitmask) {
            gui.idle_counter = 0;
        } else {
            gui.idle_counter++;
        }

        gui_redraw();
        HAL_Delay(20);
    }
}

#define ODROID_APPID_LAUNCHER 0

void app_main(void)
{
    odroid_system_init(ODROID_APPID_LAUNCHER, 32000);
    // odroid_display_clear(0);

    emulators_init();
    // favorites_init();

    retro_loop();
}
