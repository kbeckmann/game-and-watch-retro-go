#include <assert.h>
#include <string.h>

#include "odroid_system.h"
#include "odroid_settings.h"
#include "main.h"
#include "rg_i18n.h"
#include "appid.h"
#include "gui.h"

#define CONFIG_MAGIC 0xcafef00d
#define ODROID_APPID_COUNT 4

#if !defined  (COVERFLOW)
  #define COVERFLOW 0
#endif /* COVERFLOW */
// Global

#if !defined (CODEPAGE)
#define CODEPAGE 1252
#endif
#if !defined (UICODEPAGE)
#define UICODEPAGE 1252
#endif
static const char* Key_RomFilePath  = "RomFilePath";
static const char* Key_AudioSink    = "AudioSink";

// Per-app
static const char* Key_DispRotation = "DistRotation";

typedef struct app_config {
    uint8_t region;
    uint8_t palette;
    uint8_t disp_scaling;
    uint8_t disp_filter;
    uint8_t disp_overscan;
    uint8_t sprite_limit;
} app_config_t;

typedef struct persistent_config {
    uint32_t magic;
    uint8_t version;

    uint8_t backlight;
    uint8_t start_action;
    uint8_t volume;
    uint8_t font_size;
    uint8_t theme;
    uint8_t colors;
    uint8_t font;
    uint8_t lang;
    uint8_t romlang;
    uint8_t splashani;
    uint8_t startup_app;
    void *startup_file;

    uint16_t main_menu_timeout_s;
    uint16_t main_menu_selected_tab;
    uint16_t main_menu_cursor;

    app_config_t app[APPID_COUNT];

    uint32_t crc32;
} persistent_config_t;

static const persistent_config_t persistent_config_default = {
    .magic = CONFIG_MAGIC,
    .version = 4,

    .backlight = ODROID_BACKLIGHT_LEVEL6,
    .start_action = ODROID_START_ACTION_RESUME,
    .volume = ODROID_AUDIO_VOLUME_MAX / 2, // Too high volume can cause brown out if the battery isn't connected.
    .font_size = 8,
    .theme = 2, //use as theme index
    .colors = 0,
    .font = 8,
#if CODEPAGE==12521
    .lang = 1,
#elif CODEPAGE==12522
    .lang = 2,
#elif CODEPAGE==932
    .lang = 6,
#elif CODEPAGE==936
    .lang = 3,
#elif CODEPAGE==949
    .lang = 5,
#elif CODEPAGE==950
    .lang = 4,
#else
    .lang = 0,
#endif

#if UICODEPAGE==12521
    .romlang = 1,
#elif UICODEPAGE==12522
    .romlang = 2,
#elif UICODEPAGE==932
    .romlang = 6,
#elif UICODEPAGE==936
    .romlang = 3,
#elif UICODEPAGE==949
    .romlang = 5,
#elif UICODEPAGE==950
    .romlang = 4,
#else
    .romlang = 0,
#endif
    .splashani = 1,
    .startup_app = 0,
    .main_menu_timeout_s = 60 * 10, // Turn off after 10 minutes of idle time in the main menu
    .main_menu_selected_tab = 0,
    .main_menu_cursor = 0,
    .app = {
        {0}, // Launcher
        {
            .region = 0,
            .palette = 2,
            .disp_scaling = ODROID_DISPLAY_SCALING_FULL,
            .disp_filter = ODROID_DISPLAY_FILTER_SHARP,
            .disp_overscan = 0,
            .sprite_limit = 0,
        }, // GB
        {
            .disp_scaling = ODROID_DISPLAY_SCALING_CUSTOM,
            .disp_filter = ODROID_DISPLAY_FILTER_SHARP,
        }, // NES
        {0}, // SMS
        {0}, // PCE
        {0}, // GW
    },
};

__attribute__((section (".configflash"))) __attribute__((aligned(4096))) persistent_config_t persistent_config_flash;
persistent_config_t persistent_config_ram;

void odroid_settings_init()
{
    memcpy(&persistent_config_ram, &persistent_config_flash, sizeof(persistent_config_t));

    if (persistent_config_ram.magic != CONFIG_MAGIC) {
        printf("Config: Magic mismatch. Expected 0x%08x, got 0x%08lx\n", CONFIG_MAGIC, persistent_config_ram.magic);
        odroid_settings_reset();
        return;
    }

    if (persistent_config_ram.version != persistent_config_default.version) {
        printf("Config: New config version, resetting settings.\n");
        odroid_settings_reset();
        return;
    }

    // Calculate crc32 of the whole struct with the crc32 value set to 0
    persistent_config_ram.crc32 = 0;
    persistent_config_ram.crc32 = crc32_le(0, (unsigned char *) &persistent_config_ram, sizeof(persistent_config_t));

    if (persistent_config_ram.crc32 != persistent_config_flash.crc32) {
        printf("Config: CRC32 mismatch. Expected 0x%08lx, got 0x%08lx\n", persistent_config_ram.crc32, persistent_config_flash.crc32);
        odroid_settings_reset();
        return;
    }
    //set colors;
    curr_colors = (colors_t *)(&gui_colors[persistent_config_flash.colors]);
    //set font
    curr_font = (char *)gui_fonts[persistent_config_flash.font];
    //set lang
    curr_lang = (lang_t *)gui_lang[odroid_settings_lang_get()];
    curr_romlang = (lang_t *)gui_lang[odroid_settings_romlang_get()];
}

void odroid_settings_commit()
{
    // Calculate crc32 of the whole struct with the crc32 value set to 0
    persistent_config_ram.crc32 = 0;
    persistent_config_ram.crc32 = crc32_le(0, (unsigned char *) &persistent_config_ram, sizeof(persistent_config_t));

    store_save((const uint8_t *) &persistent_config_flash, (const uint8_t *) &persistent_config_ram, sizeof(persistent_config_t));
}

void odroid_settings_reset()
{
    memcpy(&persistent_config_ram, &persistent_config_default, sizeof(persistent_config_t));

    // odroid_settings_commit();
}

char* odroid_settings_string_get(const char *key, const char *default_value)
{
    return (char *) default_value;
}

void odroid_settings_string_set(const char *key, const char *value)
{
}

int32_t odroid_settings_int32_get(const char *key, int32_t default_value)
{
    return default_value;
}

void odroid_settings_int32_set(const char *key, int32_t value)
{
}

int8_t odroid_settings_splashani_get()
{
    return persistent_config_ram.splashani;
}

void odroid_settings_splashani_set(int8_t splashani)
{
    persistent_config_ram.splashani = splashani;
}

int8_t odroid_settings_colors_get()
{
    int colors = persistent_config_ram.colors;
    if (colors < 0)
        persistent_config_ram.colors = 0;
    else if (colors >= gui_colors_count)
        persistent_config_ram.colors = gui_colors_count - 1;
    return persistent_config_ram.colors;
}

void odroid_settings_colors_set(int8_t colors)
{
    if (colors < 0)
        colors = 0;
    else if (colors >= gui_colors_count)
        colors = gui_colors_count - 1;
    persistent_config_ram.colors = colors;
}


int8_t odroid_settings_font_get()
{
    int font = persistent_config_ram.font;
    if (font < 0)
        persistent_config_ram.font = 0;
    else if (font >= gui_font_count)
        persistent_config_ram.font = gui_font_count - 1;
    return persistent_config_ram.font;
}

void odroid_settings_font_set(int8_t font)
{
    if (font < 0)
        font = 0;
    else if (font >= gui_font_count)
        font = gui_font_count - 1;
    persistent_config_ram.font = font;
}


int8_t odroid_settings_get_next_lang(uint8_t cur)
{
    lang_t* next_lang = NULL;
    int ret = cur;
    while (!next_lang)
    {
        ret ++;
        if (ret >= gui_lang_count)
            ret = 0;
        next_lang = (lang_t *)gui_lang[ret];  
    }
    return ret;
}

int8_t odroid_settings_get_prior_lang(uint8_t cur)
{
    lang_t* prior_lang = NULL;
    int ret = cur;
    while (!prior_lang)
    {
        ret --;
        if (ret < 0)
            ret = gui_lang_count - 1;
        prior_lang = (lang_t *)gui_lang[ret];  
    }
    return ret;
}

int8_t odroid_settings_lang_get()
{
    int lang = persistent_config_ram.lang;
    return odroid_settings_get_prior_lang(lang + 1);
}


void odroid_settings_lang_set(int8_t lang)
{
    if (lang < 0)
        lang = 0;
    else if (lang >= gui_lang_count)
        lang = gui_lang_count - 1;
    persistent_config_ram.lang = lang;
}


int8_t odroid_settings_romlang_get()
{
    int lang = persistent_config_ram.romlang;
    return odroid_settings_get_prior_lang(lang + 1);
}

void odroid_settings_romlang_set(int8_t lang)
{
    if (lang < 0)
        lang = 0;
    else if (lang >= gui_lang_count)
        lang = gui_lang_count - 1;
    persistent_config_ram.romlang = lang;
}

#if COVERFLOW != 0
int8_t odroid_settings_theme_get()
{
    int theme = persistent_config_ram.theme;
    if (theme < 0)
        persistent_config_ram.theme = 0;
    else if (theme > 4)
        persistent_config_ram.theme = 4;
    return persistent_config_ram.theme;
}
void odroid_settings_theme_set(int8_t theme)
{
    if (theme < 0)
        theme = 0;
    else if (theme > 4)
        theme = 4;
    persistent_config_ram.theme = theme;
}
#endif

int32_t odroid_settings_app_int32_get(const char *key, int32_t default_value)
{
    return default_value;
}

void odroid_settings_app_int32_set(const char *key, int32_t value)
{
    char app_key[16];
    sprintf(app_key, "%.12s.%ld", key, odroid_system_get_app()->id);
    odroid_settings_int32_set(app_key, value);
}


int32_t odroid_settings_FontSize_get()
{
    return persistent_config_ram.font_size;
}
void odroid_settings_FontSize_set(int32_t value)
{
    persistent_config_ram.font_size = value;
}


char* odroid_settings_RomFilePath_get()
{
  return odroid_settings_string_get(Key_RomFilePath, NULL);
}
void odroid_settings_RomFilePath_set(const char* value)
{
  odroid_settings_string_set(Key_RomFilePath, value);
}


int32_t odroid_settings_Volume_get()
{
    return persistent_config_ram.volume;
}
void odroid_settings_Volume_set(int32_t value)
{
    persistent_config_ram.volume = value;
}


int32_t odroid_settings_AudioSink_get()
{
  return odroid_settings_int32_get(Key_AudioSink, ODROID_AUDIO_SINK_SPEAKER);
}
void odroid_settings_AudioSink_set(int32_t value)
{
  odroid_settings_int32_set(Key_AudioSink, value);
}



int32_t odroid_settings_Backlight_get()
{
    return persistent_config_ram.backlight;
}
void odroid_settings_Backlight_set(int32_t value)
{
    persistent_config_ram.backlight = value;
}


ODROID_START_ACTION odroid_settings_StartAction_get()
{
    return persistent_config_ram.start_action;
}
void odroid_settings_StartAction_set(ODROID_START_ACTION value)
{
    persistent_config_ram.start_action = value;
}


int32_t odroid_settings_StartupApp_get()
{
    return persistent_config_ram.startup_app;
}
void odroid_settings_StartupApp_set(int32_t value)
{
    persistent_config_ram.startup_app = value;
}


void* odroid_settings_StartupFile_get()
{
    return persistent_config_ram.startup_file;
}
void odroid_settings_StartupFile_set(void *value)
{
    persistent_config_ram.startup_file = value;
}


uint16_t odroid_settings_MainMenuTimeoutS_get()
{
    return persistent_config_ram.main_menu_timeout_s;
}
void odroid_settings_MainMenuTimeoutS_set(uint16_t value)
{
    persistent_config_ram.main_menu_timeout_s = value;
}

uint16_t odroid_settings_MainMenuSelectedTab_get()
{
    return persistent_config_ram.main_menu_selected_tab;
}
void odroid_settings_MainMenuSelectedTab_set(uint16_t value)
{
    persistent_config_ram.main_menu_selected_tab = value;
}

uint16_t odroid_settings_MainMenuCursor_get()
{
    return persistent_config_ram.main_menu_cursor;
}
void odroid_settings_MainMenuCursor_set(uint16_t value)
{
    persistent_config_ram.main_menu_cursor = value;
}


int32_t odroid_settings_Palette_get()
{
    return persistent_config_ram.app[odroid_system_get_app()->id].palette;
}
void odroid_settings_Palette_set(int32_t value)
{
    persistent_config_ram.app[odroid_system_get_app()->id].palette = value;
}


int32_t odroid_settings_SpriteLimit_get()
{
    return persistent_config_ram.app[odroid_system_get_app()->id].sprite_limit;
}
void odroid_settings_SpriteLimit_set(int32_t value)
{
    persistent_config_ram.app[odroid_system_get_app()->id].sprite_limit = value;
}


ODROID_REGION odroid_settings_Region_get()
{
    return persistent_config_ram.app[odroid_system_get_app()->id].region;
}
void odroid_settings_Region_set(ODROID_REGION value)
{
    persistent_config_ram.app[odroid_system_get_app()->id].region = value;
}


int32_t odroid_settings_DisplayScaling_get()
{
    return persistent_config_ram.app[odroid_system_get_app()->id].disp_scaling;
}
void odroid_settings_DisplayScaling_set(int32_t value)
{
    persistent_config_ram.app[odroid_system_get_app()->id].disp_scaling = value;
}


int32_t odroid_settings_DisplayFilter_get()
{
    return persistent_config_ram.app[odroid_system_get_app()->id].disp_filter;
}
void odroid_settings_DisplayFilter_set(int32_t value)
{
    persistent_config_ram.app[odroid_system_get_app()->id].disp_filter = value;
}


int32_t odroid_settings_DisplayRotation_get()
{
  return odroid_settings_app_int32_get(Key_DispRotation, ODROID_DISPLAY_ROTATION_AUTO);
}
void odroid_settings_DisplayRotation_set(int32_t value)
{
  odroid_settings_app_int32_set(Key_DispRotation, value);
}


int32_t odroid_settings_DisplayOverscan_get()
{
    return persistent_config_ram.app[odroid_system_get_app()->id].disp_overscan;
}
void odroid_settings_DisplayOverscan_set(int32_t value)
{
    persistent_config_ram.app[odroid_system_get_app()->id].disp_overscan = value;
}
