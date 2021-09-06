#include <odroid_system.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rg_emulators.h"
#include "rg_favorites.h"
#include "gui.h"
#include "githash.h"
#include "main.h"
#include "gw_buttons.h"
#include "gw_flash.h"
#include "rg_rtc.h"
#include "rg_i18n.h"

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

    if (repeat > threshold) {
        step = fast_step;
    }

    if (event == ODROID_DIALOG_PREV) {
        if (timeout - step < 10) {
            // Lower than 10 seconds doesn't make sense. set to 0 = disabled
            odroid_settings_MainMenuTimeoutS_set(0);
            return false;
        }

        odroid_settings_MainMenuTimeoutS_set(timeout - step);
        gui_redraw();
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (timeout == 0) {
            odroid_settings_MainMenuTimeoutS_set(10);
            gui_redraw();
            return false;
        }
        else if (timeout == 0xffff) {
            return false;
        }
        
        if (timeout > (0xffff - step)) {
            step = 0xffff - timeout;
        }

        odroid_settings_MainMenuTimeoutS_set(timeout + step);
        gui_redraw();
    }
    sprintf(option->value, "%d %s", odroid_settings_MainMenuTimeoutS_get(), s_Second_Unit);
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

    for (int i = 0; i < ODROID_INPUT_MAX; i++) {
        if (gui.joystick.values[i]) 
		    last_key = i;
    }

    gui.selected = odroid_settings_MainMenuSelectedTab_get();
    // gui.theme      = odroid_settings_int32_get(KEY_GUI_THEME, 0);
    // gui.show_empty = odroid_settings_int32_get(KEY_SHOW_EMPTY, 1);
    // gui.show_cover = odroid_settings_int32_get(KEY_SHOW_COVER, 1);

    while (true)
    {
        wdog_refresh();

        if (gui.idle_start == 0) {
            gui.idle_start = uptime_get();
        }

        idle_s = uptime_get() - gui.idle_start;

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

        if (idle_s > 0 && gui.joystick.bitmask == 0)
        {
            gui_event(TAB_IDLE, tab);

            if (idle_s % 10 == 0)
                gui_draw_status(tab);
        }

        if ((last_key < 0) || ((repeat >= 30) && (repeat % 5 == 0))) {
            for (int i = 0; i < ODROID_INPUT_MAX; i++)
                if (gui.joystick.values[i]) last_key = i;

            bool hori_view = false;
            int key_up = ODROID_INPUT_UP;
            int key_down = ODROID_INPUT_DOWN;
            int key_left = ODROID_INPUT_LEFT;
            int key_right = ODROID_INPUT_RIGHT;
#if COVERFLOW == 1
            hori_view = odroid_settings_theme_get()  == 2;
            if (hori_view)
            {
                key_up = ODROID_INPUT_LEFT;
                key_down = ODROID_INPUT_RIGHT;
                key_left = ODROID_INPUT_UP;
                key_right = ODROID_INPUT_DOWN;
            }
#endif

            if (last_key == ODROID_INPUT_START) {
                odroid_dialog_choice_t choices[] = {
                    {0, s_Version, GIT_HASH, 1, NULL},
                    {0, s_Author, "ducalex", 1, NULL},
                    {0, s_Author_, "kbeckmann", 1, NULL},
                    {0, s_Author_, "stacksmashing", 1, NULL},
                    {0, s_UI_Mod, "orzeus", -1, NULL},
                    ODROID_DIALOG_CHOICE_SEPARATOR,
                    {0, s_Lang, s_LangAuthor, -1, NULL},
                    ODROID_DIALOG_CHOICE_SEPARATOR,
                    {2, s_Debug_menu, "", 1, NULL},
                    {1, s_Reset_settings, "", 1, NULL},
                    ODROID_DIALOG_CHOICE_SEPARATOR,
                    {0, s_Close, "", 1, NULL},
                    ODROID_DIALOG_CHOICE_LAST};

                int sel = odroid_overlay_dialog(s_Retro_Go, choices, -1);
                if (sel == 1) {
                    // Reset settings
                    if (odroid_overlay_confirm(s_Confirm_Reset_settings, false) == 1) {
                        odroid_settings_reset();
                        odroid_system_switch_app(0); // reset
                    }
                } else if (sel == 2) {
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
                        {0, s_Flash_JEDEC_ID, (char *)jedec_id_str, 1, NULL},
                        {0, s_Flash_Name, (char *)OSPI_GetFlashName(), 1, NULL},
                        {0, s_Flash_SR, (char *)status_str, 1, NULL},
                        {0, s_Flash_CR, (char *)config_str, 1, NULL},
                        {0, s_Smallest_erase, erase_size_str, 1, NULL},
                        ODROID_DIALOG_CHOICE_SEPARATOR,
                        {0, s_DBGMCU_IDCODE, dbgmcu_id_str, 1, NULL},
                        {1, s_Enable_DBGMCU_CK, dbgmcu_cr_str, 1, NULL},
                        {2, s_Disable_DBGMCU_CK, "", 1, NULL},
                        ODROID_DIALOG_CHOICE_SEPARATOR,
                        {0, s_Close, "", 1, NULL},
                        ODROID_DIALOG_CHOICE_LAST};

                    int sel = odroid_overlay_dialog(s_Debug_Title, debuginfo, -1);
                    switch (sel) {
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
            else if (last_key == ODROID_INPUT_VOLUME) {
                char timeout_value[32];
                odroid_dialog_choice_t choices[] = {
                    ODROID_DIALOG_CHOICE_SEPARATOR,
                    {0, s_Idle_power_off, timeout_value, 1, &main_menu_timeout_cb},
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
            else if (last_key == ODROID_INPUT_SELECT) {

                char time_str[14];
                char date_str[24];

                odroid_dialog_choice_t rtcinfo[] = {
                    {0, s_Time, time_str, 1, &time_display_cb},
                    {1, s_Date, date_str, 1, &date_display_cb},
                    ODROID_DIALOG_CHOICE_LAST};
                int sel = odroid_overlay_dialog(s_Time_Title, rtcinfo, 0);

                if (sel == 0) {
                    static char hour_value[8];
                    static char minute_value[8];
                    static char second_value[8];

                    // Time setup
                    odroid_dialog_choice_t timeoptions[32] = {
                        {0, s_Hour, hour_value, 1, &hour_update_cb},
                        {1, s_Minute, minute_value, 1, &minute_update_cb},
                        {2, s_Second, second_value, 1, &second_update_cb},
                        ODROID_DIALOG_CHOICE_LAST};
                    sel = odroid_overlay_dialog(s_Time_setup, timeoptions, 0);
                }
                else if (sel == 1) {

                    static char day_value[8];
                    static char month_value[8];
                    static char year_value[8];
                    static char weekday_value[8];

                    // Date setup
                    odroid_dialog_choice_t dateoptions[32] = {
                        {0, s_Day, day_value, 1, &day_update_cb},
                        {1, s_Month, month_value, 1, &month_update_cb},
                        {2, s_Year, year_value, 1, &year_update_cb},
                        {3, s_Weekday, weekday_value, 1, &weekday_update_cb},
                        ODROID_DIALOG_CHOICE_LAST};
                    sel = odroid_overlay_dialog(s_Date_setup, dateoptions, 0);
                }

                (void)sel;
                gui_redraw();
            }
            else if (last_key == key_up) {
                gui_scroll_list(tab, LINE_UP);
                repeat++;
            }
            else if (last_key == key_down) {
                gui_scroll_list(tab, LINE_DOWN);
                repeat++;
            }
            else if (last_key == key_left) {
                gui.selected--;
                if (gui.selected < 0) {
                    gui.selected = gui.tabcount - 1;
                }
                repeat++;
            }
            else if (last_key == key_right) {
                gui.selected++;
                if (gui.selected >= gui.tabcount) {
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
                odroid_system_sleep();
            }
        }
        if (repeat > 0)
            repeat++;
        if (last_key >= 0) {
            if (!gui.joystick.values[last_key]) {
                last_key = -1;
                repeat = 0;
            }
            gui.idle_start = uptime_get();
        }

        idle_s = uptime_get() - gui.idle_start;
        if (odroid_settings_MainMenuTimeoutS_get() != 0 &&
            (idle_s > odroid_settings_MainMenuTimeoutS_get())) {
          printf("Idle timeout expired\n");
          odroid_system_sleep();
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

    // Start the previously running emulator directly if it's a valid pointer.
    // If the user holds down the TIME button during startup,start the retro-go 
    // gui instead of the last ROM as a fallback.
    retro_emulator_file_t *file = odroid_settings_StartupFile_get();
    if (emulator_is_file_valid(file) && ((GW_GetBootButtons() & B_TIME) == 0)) {
        emulator_start(file, true, true);
    } else {
        retro_loop();
    }
}
