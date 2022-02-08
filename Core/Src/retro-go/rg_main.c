#include <odroid_system.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "appid.h"
#include "rg_emulators.h"
#include "gui.h"
#include "githash.h"
#include "main.h"
#include "gw_lcd.h"
#include "gw_buttons.h"
#include "gw_flash.h"
#include "rg_rtc.h"
#include "rg_i18n.h"
#include "bitmaps.h"

#if 0
#define KEY_SELECTED_TAB "SelectedTab"
#define KEY_GUI_THEME "ColorTheme"
#define KEY_SHOW_EMPTY "ShowEmptyTabs"
#define KEY_SHOW_COVER "ShowGameCover"

static bool font_size_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
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

static bool show_empty_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        gui.show_empty = !gui.show_empty;
        odroid_settings_int32_set(KEY_SHOW_EMPTY, gui.show_empty);
    }
    strcpy(option->value, gui.show_empty ? "Yes" : "No");
    return event == ODROID_DIALOG_ENTER;
}

static bool startup_app_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int startup_app = odroid_settings_StartupApp_get();
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        startup_app = startup_app ? 0 : 1;
        odroid_settings_StartupApp_set(startup_app);
    }
    strcpy(option->value, startup_app == 0 ? "Launcher" : "LastUsed");
    return event == ODROID_DIALOG_ENTER;
}

static bool show_cover_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
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

static bool color_shift_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
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

#if !defined(COVERFLOW)
#define COVERFLOW 0
#endif /* COVERFLOW */

static bool main_menu_timeout_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    uint16_t timeout = odroid_settings_MainMenuTimeoutS_get();
    int step = 1;
    const int threshold = 10;
    const int fast_step = 10;

    if (repeat > threshold)
        step = fast_step;

    if (event == ODROID_DIALOG_PREV)
    {
        if (timeout - step < 10)
        {
            // Lower than 10 seconds doesn't make sense. set to 0 = disabled
            odroid_settings_MainMenuTimeoutS_set(0);
            return false;
        }

        odroid_settings_MainMenuTimeoutS_set(timeout - step);
        gui_redraw();
    }
    if (event == ODROID_DIALOG_NEXT)
    {
        if (timeout == 0)
        {
            odroid_settings_MainMenuTimeoutS_set(10);
            gui_redraw();
            return false;
        }
        else if (timeout == 0xffff)
            return false;

        if (timeout > (0xffff - step))
            step = 0xffff - timeout;

        odroid_settings_MainMenuTimeoutS_set(timeout + step);
        gui_redraw();
    }
    sprintf(option->value, "%d %s", odroid_settings_MainMenuTimeoutS_get(), curr_lang->s_Second_Unit);
    return event == ODROID_DIALOG_ENTER;
}


#if COVERFLOW != 0

static bool theme_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    const char *GW_Themes[] = {curr_lang->s_Theme_sList, curr_lang->s_Theme_CoverV, curr_lang->s_Theme_CoverH, curr_lang->s_Theme_CoverLightH, curr_lang->s_Theme_CoverLightV};
    int8_t theme = odroid_settings_theme_get();

    if (event == ODROID_DIALOG_PREV)
    {
        if (theme > 0)
            odroid_settings_theme_set(--theme);
        else
        {
            theme = 4;
            odroid_settings_theme_set(4);
        }
    }
    else if (event == ODROID_DIALOG_NEXT)
    {
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

static bool colors_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t colors = odroid_settings_colors_get();

    if (event == ODROID_DIALOG_PREV)
    {
        if (colors > 0)
            odroid_settings_colors_set(--colors);
        else
        {
            colors = gui_colors_count - 1;
            odroid_settings_colors_set(gui_colors_count - 1);
        }
    }
    else if (event == ODROID_DIALOG_NEXT)
    {
        if (colors < gui_colors_count - 1)
            odroid_settings_colors_set(++colors);
        else
        {
            colors = 0;
            odroid_settings_colors_set(0);
        }
    }
    curr_colors = (colors_t *)(&gui_colors[colors]);
    option->value[0] = 0;
    option->value[10] = 0;
    memcpy(option->value + 2, curr_colors, sizeof(colors_t));
    //sprintf(option->value, "%s", curr_colors->name);
    return event == ODROID_DIALOG_ENTER;
}
#if (FONT_COUNT > 1)
static bool font_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t font = odroid_settings_font_get();

    if (event == ODROID_DIALOG_PREV)
    {
        if (font > 0)
            odroid_settings_font_set(--font);
        else
        {
            font = gui_font_count - 1;
            odroid_settings_font_set(gui_font_count - 1);
        }
    }
    else if (event == ODROID_DIALOG_NEXT)
    {
        if (font < gui_font_count - 1)
            odroid_settings_font_set(++font);
        else
        {
            font = 0;
            odroid_settings_font_set(0);
        }
    }
    curr_font = (char *)gui_fonts[font];
    sprintf(option->value, "%d/%d Wg", font + 1, gui_font_count);
    return event == ODROID_DIALOG_ENTER;
}
#endif

static bool lang_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t lang = odroid_settings_lang_get();

    if (event == ODROID_DIALOG_PREV)
    {
        lang = odroid_settings_get_prior_lang(lang);
        odroid_settings_lang_set(lang);
    }
    else if (event == ODROID_DIALOG_NEXT)
    {
        lang = odroid_settings_get_next_lang(lang);
        odroid_settings_lang_set(lang);
    }
    curr_lang = (lang_t *)gui_lang[lang];
    sprintf(option->value, "%s", curr_lang->s_LangName);
    //sprintf(option->label, "%s", curr_lang->s_Lang);
    return event == ODROID_DIALOG_ENTER;
}


static bool romlang_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t lang = odroid_settings_romlang_get();

    if (event == ODROID_DIALOG_PREV)
    {
        lang = odroid_settings_get_prior_lang(lang);
        odroid_settings_romlang_set(lang);
    }
    else if (event == ODROID_DIALOG_NEXT)
    {
        lang = odroid_settings_get_next_lang(lang);
        odroid_settings_romlang_set(lang);
    }
    curr_romlang = (lang_t *)gui_lang[lang];
    sprintf(option->value, "%s", curr_romlang->s_LangName);
    //sprintf(option->label, "%s", curr_lang->s_Lang);
    return event == ODROID_DIALOG_ENTER;
}


static bool splashani_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t splashani = odroid_settings_splashani_get();

    if ((event == ODROID_DIALOG_PREV) || (event == ODROID_DIALOG_NEXT))
    {
        splashani = splashani ? 0 : 1;
        odroid_settings_splashani_set(splashani);
    };

    sprintf(option->value, "%s", splashani ? curr_lang->s_Splash_On : curr_lang->s_Splash_Off);
    return event == ODROID_DIALOG_ENTER;
}


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
    uint32_t idle_s;

    // Read the initial state as to not trigger on button held down during boot
    odroid_input_read_gamepad(&gui.joystick);

    for (int i = 0; i < ODROID_INPUT_MAX; i++)
        if (gui.joystick.values[i])
            last_key = i;

    gui.selected = odroid_settings_MainMenuSelectedTab_get();
    // gui.theme      = odroid_settings_int32_get(KEY_GUI_THEME, 0);
    // gui.show_empty = odroid_settings_int32_get(KEY_SHOW_EMPTY, 1);
    // gui.show_cover = odroid_settings_int32_get(KEY_SHOW_COVER, 1);

    while (true)
    {
        wdog_refresh();

        if (gui.idle_start == 0)
            gui.idle_start = uptime_get();

        idle_s = uptime_get() - gui.idle_start;

        if (gui.selected != selected_tab_last)
        {
            int direction = (gui.selected - selected_tab_last) < 0 ? -1 : 1;

            tab = gui_set_current_tab(gui.selected);
            if (!tab->initialized)
            {
                gui_init_tab(tab);
                gui_redraw();
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

        if (idle_s > 0 && gui.joystick.bitmask == 0)
        {
            gui_event(TAB_IDLE, tab);
        }

        if ((last_key < 0) || ((repeat >= 30) && (repeat % 5 == 0)))
        {
            for (int i = 0; i < ODROID_INPUT_MAX; i++)
                if (gui.joystick.values[i])
                    last_key = i;

            int hori_view = false;
            int key_up = ODROID_INPUT_UP;
            int key_down = ODROID_INPUT_DOWN;
            int key_left = ODROID_INPUT_LEFT;
            int key_right = ODROID_INPUT_RIGHT;
#if COVERFLOW != 0
            hori_view = odroid_settings_theme_get();
            if ((hori_view== 2) | (hori_view==3))
            {
                key_up = ODROID_INPUT_LEFT;
                key_down = ODROID_INPUT_RIGHT;
                key_left = ODROID_INPUT_UP;
                key_right = ODROID_INPUT_DOWN;
            }
#endif

            if ((last_key == ODROID_INPUT_START) || (last_key == ODROID_INPUT_X))
            {
                odroid_dialog_choice_t choices[] = {
                    {9, curr_lang->s_Version, GIT_HASH, 1, NULL},
                    {9, curr_lang->s_Author, "ducalex", 1, NULL},
                    {9, curr_lang->s_Author_, "kbeckmann", 1, NULL},
                    {9, curr_lang->s_Author_, "stacksmashing", 1, NULL},
                    {9, curr_lang->s_UI_Mod, "orzeus", 1, NULL},
                    ODROID_DIALOG_CHOICE_SEPARATOR,
                    {1, curr_lang->s_Lang, curr_lang->s_LangAuthor, 1, NULL},
                    ODROID_DIALOG_CHOICE_SEPARATOR,
                    {2, curr_lang->s_Debug_menu, "", 1, NULL},
                    {1, curr_lang->s_Reset_settings, "", 1, NULL},
                    ODROID_DIALOG_CHOICE_SEPARATOR,
                    {0, curr_lang->s_Close, "", 1, NULL},
                    ODROID_DIALOG_CHOICE_LAST};

                int sel = odroid_overlay_dialog(curr_lang->s_Retro_Go, choices, -1);
                if (sel == 1)
                {
                    // Reset settings
                    if (odroid_overlay_confirm(curr_lang->s_Confirm_Reset_settings, false) == 1)
                    {
                        odroid_settings_reset();
                        odroid_system_switch_app(0); // reset
                    }
                }
                else if (sel == 2)
                {
                    // Debug menu
                    uint8_t jedec_id[3];
                    char jedec_id_str[16];
                    uint8_t status;
                    char status_str[8];
                    uint8_t config;
                    char config_str[8];
                    char erase_size_str[32];
                    char dbgmcu_id_str[16];
                    char dbgmcu_cr_str[16];

                    // Read jedec id and status register from the external flash
                    OSPI_DisableMemoryMappedMode();
                    OSPI_ReadJedecId(&jedec_id[0]);
                    OSPI_ReadSR(&status);
                    OSPI_ReadCR(&config);
                    OSPI_EnableMemoryMappedMode();

                    snprintf(jedec_id_str, sizeof(jedec_id_str), "%02X %02X %02X", jedec_id[0], jedec_id[1], jedec_id[2]);
                    snprintf(status_str, sizeof(status_str), "0x%02X", status);
                    snprintf(config_str, sizeof(config_str), "0x%02X", config);
                    snprintf(erase_size_str, sizeof(erase_size_str), "%ld kB", OSPI_GetSmallestEraseSize() / 1024);
                    snprintf(dbgmcu_id_str, sizeof(dbgmcu_id_str), "0x%08lX", DBGMCU->IDCODE);
                    snprintf(dbgmcu_cr_str, sizeof(dbgmcu_cr_str), "0x%08lX", DBGMCU->CR);

                    odroid_dialog_choice_t debuginfo[] = {
                        {0, curr_lang->s_Flash_JEDEC_ID, (char *)jedec_id_str, 1, NULL},
                        {0, curr_lang->s_Flash_Name, (char *)OSPI_GetFlashName(), 1, NULL},
                        {0, curr_lang->s_Flash_SR, (char *)status_str, 1, NULL},
                        {0, curr_lang->s_Flash_CR, (char *)config_str, 1, NULL},
                        {0, curr_lang->s_Smallest_erase, erase_size_str, 1, NULL},
                        ODROID_DIALOG_CHOICE_SEPARATOR,
                        {0, curr_lang->s_DBGMCU_IDCODE, dbgmcu_id_str, 1, NULL},
                        {1, curr_lang->s_Enable_DBGMCU_CK, dbgmcu_cr_str, 1, NULL},
                        {2, curr_lang->s_Disable_DBGMCU_CK, "", 1, NULL},
                        ODROID_DIALOG_CHOICE_SEPARATOR,
                        {0, curr_lang->s_Close, "", 1, NULL},
                        ODROID_DIALOG_CHOICE_LAST};

                    int sel = odroid_overlay_dialog(curr_lang->s_Debug_Title, debuginfo, -1);
                    switch (sel)
                    {
                    case 1:
                        // Enable debug clocks explicitly
                        SET_BIT(DBGMCU->CR,
                                DBGMCU_CR_DBG_SLEEPCD |
                                    DBGMCU_CR_DBG_STOPCD |
                                    DBGMCU_CR_DBG_STANDBYCD |
                                    DBGMCU_CR_DBG_TRACECKEN |
                                    DBGMCU_CR_DBG_CKCDEN |
                                    DBGMCU_CR_DBG_CKSRDEN);
                    case 2:
                        // Disable debug clocks explicitly
                        CLEAR_BIT(DBGMCU->CR,
                                  DBGMCU_CR_DBG_SLEEPCD |
                                      DBGMCU_CR_DBG_STOPCD |
                                      DBGMCU_CR_DBG_STANDBYCD |
                                      DBGMCU_CR_DBG_TRACECKEN |
                                      DBGMCU_CR_DBG_CKCDEN |
                                      DBGMCU_CR_DBG_CKSRDEN);
                        break;
                    default:
                        break;
                    }
                }

                gui_redraw();
            }
            else if ((last_key == ODROID_INPUT_VOLUME) || (last_key == ODROID_INPUT_Y))
            {
                char splashani_value[16];
#if (FONT_COUNT > 1)
                char font_value[16];
#endif
                char timeout_value[16];
                char theme_value[16];
                char colors_value[16];
                char UIlang_value[64];
                char lang_value[64];

                odroid_dialog_choice_t choices[] = {
                    ODROID_DIALOG_CHOICE_SEPARATOR,
#if COVERFLOW != 0
                     //ODROID_DIALOG_CHOICE_SEPARATOR,
                    {0, curr_lang->s_Theme_Title, theme_value, 1, &theme_update_cb},
#endif
                    {0x0F0F0E0E, curr_lang->s_Colors, colors_value, 1, &colors_update_cb},
                    {0, curr_lang->s_Splash_Option, splashani_value, 1, &splashani_update_cb},
                    ODROID_DIALOG_CHOICE_SEPARATOR,
#if (FONT_COUNT > 1)
                    {0, curr_lang->s_Font, font_value, 1, &font_update_cb},
#endif
                    {0, curr_lang->s_LangUI, UIlang_value, 1, &lang_update_cb},
                    {0, curr_lang->s_LangTitle, lang_value, 1, &romlang_update_cb},
                    ODROID_DIALOG_CHOICE_SEPARATOR,
                    {0, curr_lang->s_Idle_power_off, timeout_value, 1, &main_menu_timeout_cb},
                    // {0, "Color theme", "1/10", 1, &color_shift_cb},
                    // {0, "Font size", "Small", 1, &font_size_cb},
                    // {0, "Show cover", "Yes", 1, &show_cover_cb},
                    // {0, "Show empty", "Yes", 1, &show_empty_cb},
                    // {0, "---", "", -1, NULL},
                    // {0, "Startup app", "Last", 1, &startup_app_cb},
                    ODROID_DIALOG_CHOICE_LAST};
                odroid_overlay_settings_menu(choices);
                gui_redraw();
            }
            // TIME menu
            else if (last_key == ODROID_INPUT_SELECT)
            {

                char time_str[14];
                char date_str[24];

                odroid_dialog_choice_t rtcinfo[] = {
                    {0, curr_lang->s_Time, time_str, 1, &time_display_cb},
                    {1, curr_lang->s_Date, date_str, 1, &date_display_cb},
                    ODROID_DIALOG_CHOICE_LAST};
                int sel = odroid_overlay_dialog(curr_lang->s_Time_Title, rtcinfo, 0);

                if (sel == 0)
                {
                    static char hour_value[8];
                    static char minute_value[8];
                    static char second_value[8];

                    // Time setup
                    odroid_dialog_choice_t timeoptions[8] = {
                        {0, curr_lang->s_Hour, hour_value, 1, &hour_update_cb},
                        {1, curr_lang->s_Minute, minute_value, 1, &minute_update_cb},
                        {2, curr_lang->s_Second, second_value, 1, &second_update_cb},
                        ODROID_DIALOG_CHOICE_LAST};
                    sel = odroid_overlay_dialog(curr_lang->s_Time_setup, timeoptions, 0);
                }
                else if (sel == 1)
                {

                    static char day_value[8];
                    static char month_value[8];
                    static char year_value[8];
                    static char weekday_value[8];

                    // Date setup
                    odroid_dialog_choice_t dateoptions[8] = {
                        {0, curr_lang->s_Day, day_value, 1, &day_update_cb},
                        {1, curr_lang->s_Month, month_value, 1, &month_update_cb},
                        {2, curr_lang->s_Year, year_value, 1, &year_update_cb},
                        {3, curr_lang->s_Weekday, weekday_value, 1, &weekday_update_cb},
                        ODROID_DIALOG_CHOICE_LAST};
                    sel = odroid_overlay_dialog(curr_lang->s_Date_setup, dateoptions, 0);
                }

                (void)sel;
                gui_redraw();
            }
            else if (last_key == key_up)
            {
                gui_scroll_list(tab, LINE_UP);
                repeat++;
            }
            else if (last_key == key_down)
            {
                gui_scroll_list(tab, LINE_DOWN);
                repeat++;
            }
            else if (last_key == key_left)
            {
                gui.selected--;
                if (gui.selected < 0)
                {
                    gui.selected = gui.tabcount - 1;
                }
                repeat++;
            }
            else if (last_key == key_right)
            {
                gui.selected++;
                if (gui.selected >= gui.tabcount)
                {
                    gui.selected = 0;
                }
                repeat++;
            }
            else if (last_key == ODROID_INPUT_A)
            {
                gui_event(KEY_PRESS_A, tab);
            }
            else if (last_key == ODROID_INPUT_B)
            {
                gui_event(KEY_PRESS_B, tab);
            }
            else if (last_key == ODROID_INPUT_POWER)
            {
                if ((gui.joystick.values[ODROID_INPUT_UP]) || (gui.joystick.values[ODROID_INPUT_DOWN]) ||
                    (gui.joystick.values[ODROID_INPUT_LEFT]) || (gui.joystick.values[ODROID_INPUT_RIGHT]))
                {
                    odroid_system_switch_app(0);
                    return;
                }
                else
                    odroid_system_sleep();
            }
        }
        if (repeat > 0)
            repeat++;
        if (last_key >= 0)
        {
            if (!gui.joystick.values[last_key])
            {
                last_key = -1;
                repeat = 0;
            }
            gui.idle_start = uptime_get();
        }

        idle_s = uptime_get() - gui.idle_start;
        if (odroid_settings_MainMenuTimeoutS_get() != 0 &&
            (idle_s > odroid_settings_MainMenuTimeoutS_get()))
        {
            printf("Idle timeout expired\n");
            odroid_system_sleep();
        }

        gui_redraw();
        HAL_Delay(20);
    }
}

#define ODROID_APPID_LAUNCHER 0

void app_check_data_loop()
{
    static const uint8_t img_error[] = {
        0x0F, 0xFC, 0x12, 0x4A, 0x12, 0x4A, //
        0x20, 0x02, 0x24, 0x22, 0x4E, 0x72, //
        0x40, 0x02, 0x43, 0xC2, 0x47, 0xE2, //
        0x4C, 0x32, 0x40, 0x02, 0x40, 0x02, //
        0x40, 0x02, 0x3F, 0xFC,             //
    };

    char s[22];
    int idle_s = uptime_get();
    //     - gui.idle_start;

    printf("Flash Magic Check: %x at %p & %x at %p; \n", extflash_magic_sign, &extflash_magic_sign, intflash_magic_sign, &intflash_magic_sign);
    if (extflash_magic_sign != intflash_magic_sign)
    {
        //flash is not compare read;
        lcd_set_buffers(framebuffer1, framebuffer2);
        odroid_overlay_draw_fill_rect(0, 0, ODROID_SCREEN_WIDTH, ODROID_SCREEN_HEIGHT, curr_colors->bg_c);
        for (int y = 0; y < 14; y++)
        {
            uint8_t pt = img_error[2 * y];
            for (int x = 0; x < 8; x++)
                if (pt & (0x80 >> x))
                    odroid_overlay_draw_fill_rect((12 + x) * 8, (9 + y) * 8, 8, 8, curr_colors->main_c);
            pt = img_error[2 * y + 1];
            for (int x = 0; x < 8; x++)
                if (pt & (0x80 >> x))
                    odroid_overlay_draw_fill_rect((20 + x) * 8, (9 + y) * 8, 8, 8, curr_colors->main_c);
        }
        
        odroid_overlay_draw_logo((ODROID_SCREEN_WIDTH - logo_rgo.width) / 2, 42, (retro_logo_image *)(&logo_rgo), curr_colors->sel_c);
        odroid_overlay_draw_text_line(15 * 8, 20 * 8, 10 * 8, "DATA ERROR", C_RED, curr_colors->bg_c);
        odroid_overlay_draw_text_line(9 * 8, 24 * 8 - 4, 23 * 8, "It's seemed you need to", curr_colors->dis_c, curr_colors->bg_c);
        odroid_overlay_draw_text_line(9 * 8, 25 * 8, 23 * 8, "programs external flash", curr_colors->dis_c, curr_colors->bg_c);
        odroid_overlay_draw_text_line(ODROID_SCREEN_WIDTH - strlen(GIT_HASH) * 8 - 4, 29 * 8 - 4, strlen(GIT_HASH) * 8, GIT_HASH, curr_colors->sel_c, curr_colors->bg_c);
        odroid_gamepad_state_t joystick;
        while (1)
        {
            wdog_refresh();
            int steps = uptime_get() - idle_s;
            sprintf(s, "%ds to sleep", 600 - steps);
            odroid_overlay_draw_text_line(4, 29 * 8 - 4, strlen(s) * 8, s, C_RED, curr_colors->bg_c);

            lcd_sync();
            lcd_swap();
            //lcd_wait_for_vblank();
            HAL_Delay(10);
            if (steps >= 600)
                break;
            odroid_input_read_gamepad(&joystick);
            if (joystick.values[ODROID_INPUT_POWER])
                break;
        }
        for (int i=0; i<10; i++){
            wdog_refresh();
            HAL_Delay(10);
        }
        app_sleep_logo();
        GW_EnterDeepSleep();
        //odroid_system_sleep();
    }
}


void app_start_logo()
{
    if (! odroid_settings_splashani_get())
        return;
    odroid_overlay_draw_fill_rect(0, 0, ODROID_SCREEN_WIDTH, ODROID_SCREEN_HEIGHT, curr_colors->bg_c);
    //tab_t *tab = gui_get_tab(odroid_settings_MainMenuSelectedTab_get());
    //tab = tab ? tab : gui_get_tab(0);
    tab_t *tab = gui_set_current_tab(odroid_settings_MainMenuSelectedTab_get());
    retro_logo_image *l_top = (retro_logo_image *)(tab->img_header);
    retro_logo_image *l_bot = (retro_logo_image *)(tab->img_logo);

    const retro_logo_image* logos[] =   {&logo_nitendo, &logo_sega,     &logo_nitendo, &logo_sega,  &logo_nitendo, &logo_pce,    &logo_sega,  &logo_coleco};
    const retro_logo_image* headers[] = {&header_gb,    &header_sg1000, &header_nes,   &header_gg,  &header_gw,    &header_pce,  &header_sms, &header_col};
    for (int i = 0; i < 8; i++)
    {
        if (l_top == (retro_logo_image *)headers[i])
            l_bot = (retro_logo_image *)logos[i];
        odroid_overlay_draw_fill_rect(0, 0, ODROID_SCREEN_WIDTH, ODROID_SCREEN_HEIGHT, curr_colors->bg_c);
        odroid_overlay_draw_logo((ODROID_SCREEN_WIDTH - ((retro_logo_image *)(headers[i]))->width) / 2, 90, (retro_logo_image *)(headers[i]), curr_colors->sel_c);
        odroid_overlay_draw_logo((ODROID_SCREEN_WIDTH - ((retro_logo_image *)(logos[i]))->width) / 2, 160 + (40 - ((retro_logo_image *)(logos[i]))->height) / 2, (retro_logo_image *)(logos[i]), curr_colors->dis_c);
        lcd_sync();
        lcd_swap();
        for (int j = 0; j < 5; j++)
        {
            wdog_refresh();
            HAL_Delay(9);
        }
    }

    odroid_overlay_draw_fill_rect(0, 0, ODROID_SCREEN_WIDTH, ODROID_SCREEN_HEIGHT, curr_colors->bg_c);
    odroid_overlay_draw_logo((ODROID_SCREEN_WIDTH - l_top->width) / 2, 90, l_top, curr_colors->sel_c);
    odroid_overlay_draw_logo((ODROID_SCREEN_WIDTH - l_bot->width) / 2, 160 + (40 - l_bot->height) / 2, l_bot, curr_colors->dis_c);
    lcd_sync();
    lcd_swap();

    for (int i = 0; i < 30; i++)
    {
        wdog_refresh();
        HAL_Delay(10);
    }
}

void app_sleep_logo()
{
    lcd_set_buffers(framebuffer1, framebuffer2);
    odroid_overlay_draw_fill_rect(0, 0, ODROID_SCREEN_WIDTH, ODROID_SCREEN_HEIGHT, curr_colors->bg_c);
    
    odroid_overlay_draw_logo((ODROID_SCREEN_WIDTH - logo_gnw.width) / 2, 72, (retro_logo_image *)(&logo_gnw), curr_colors->sel_c);
    odroid_overlay_draw_logo((ODROID_SCREEN_WIDTH - logo_rgo.width) / 2, 200, (retro_logo_image *)(&logo_rgo), curr_colors->dis_c);
    for (int i = 0; i < 40; i++)
    {
        wdog_refresh();
        HAL_Delay(10);
    }
    for (int i = 10; i <= 100; i++)
    {
        odroid_overlay_draw_logo((ODROID_SCREEN_WIDTH - logo_gnw.width) / 2, 72,(retro_logo_image *)(&logo_gnw), 
            get_darken_pixel_d(curr_colors->sel_c, curr_colors->bg_c, 110 - i));

        odroid_overlay_draw_logo((ODROID_SCREEN_WIDTH - logo_rgo.width) / 2, 200, (retro_logo_image *)(&logo_rgo), 
           get_darken_pixel_d(curr_colors->dis_c,curr_colors->bg_c, 110 - i));

        lcd_sync();
        lcd_swap();
        wdog_refresh();
        HAL_Delay(i / 10);
    }
}

void app_main(void)
{

    lcd_set_buffers(framebuffer1, framebuffer2);
    odroid_system_init(ODROID_APPID_LAUNCHER, 32000);
    odroid_overlay_draw_fill_rect(0, 0, ODROID_SCREEN_WIDTH, ODROID_SCREEN_HEIGHT, curr_colors->bg_c);
    // odroid_display_clear(0);

    //check data;
    app_check_data_loop();
    emulators_init();

    app_start_logo();

    // favorites_init();

    // Start the previously running emulator directly if it's a valid pointer.
    // If the user holds down the TIME button during startup,start the retro-go
    // gui instead of the last ROM as a fallback.
    retro_emulator_file_t *file = odroid_settings_StartupFile_get();
    if (emulator_is_file_valid(file) && ((GW_GetBootButtons() & B_TIME) == 0)) {
        emulator_start(file, (file->save_address != 0), true);
    }
    else
    {
        retro_loop();
    }
}
