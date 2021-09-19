#include <assert.h>
#include <string.h>

#include "odroid_system.h"
#include "odroid_settings.h"
#include "main.h"

#define CONFIG_MAGIC 0xcafef00d
#define ODROID_APPID_COUNT 4

// Global
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
    uint8_t startup_app;
    void *startup_file;

    uint16_t main_menu_timeout_s;
    uint16_t main_menu_selected_tab;
    uint16_t main_menu_cursor;

    app_config_t app[ODROID_APPID_COUNT];

    uint32_t crc32;
} persistent_config_t;

static const persistent_config_t persistent_config_default = {
    .magic = CONFIG_MAGIC,
    .version = 2,

    .backlight = ODROID_BACKLIGHT_LEVEL6,
    .start_action = ODROID_START_ACTION_RESUME,
    .volume = ODROID_AUDIO_VOLUME_MAX / 2, // Too high volume can cause brown out if the battery isn't connected.
    .font_size = 8,
    .startup_app = 0,
    .main_menu_timeout_s = 60 * 10, // Turn off after 10 minutes of idle time in the main menu
    .main_menu_selected_tab = 0,
    .main_menu_cursor = 0,
    .app = {
        {0}, // Launcher
        {
            .region = 0,
            .palette = 2,
            .disp_scaling = 0,
            .disp_filter = 0,
            .disp_overscan = 0,
            .sprite_limit = 0,
        }, // GB
        {0}, // NES
        {0}, // SMS
    },
};

__attribute__((section (".configflash"))) __attribute__((aligned(4096))) persistent_config_t persistent_config_flash;
persistent_config_t persistent_config_ram;

void odroid_settings_init()
{
    memcpy(&persistent_config_ram, &persistent_config_flash, sizeof(persistent_config_t));

    if (persistent_config_ram.magic != CONFIG_MAGIC) {
        printf("Config: Magic mismatch. Expected 0x%08lx, got 0x%08lx\n", CONFIG_MAGIC, persistent_config_ram.magic);
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
    persistent_config_ram.crc32 = crc32_le(0, &persistent_config_ram, sizeof(persistent_config_t));

    if (persistent_config_ram.crc32 != persistent_config_flash.crc32) {
        printf("Config: CRC32 mismatch. Expected 0x%08lx, got 0x%08lx\n", persistent_config_ram.crc32, persistent_config_flash.crc32);
        odroid_settings_reset();
        return;
    }
}

void odroid_settings_commit()
{
    // Calculate crc32 of the whole struct with the crc32 value set to 0
    persistent_config_ram.crc32 = 0;
    persistent_config_ram.crc32 = crc32_le(0, &persistent_config_ram, sizeof(persistent_config_t));

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
