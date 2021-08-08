#include "common.h"
#include <odroid_system.h>

#include <string.h>
#include <nofrendo.h>
#include <bitmap.h>
#include <nes.h>
#include <nes_input.h>
#include <nes_state.h>
#include <nes_input.h>
#include <osd.h>
#include "main.h"
#include "gw_buttons.h"
#include "gw_lcd.h"
#include "gw_linker.h"


static void set_ingame_overlay(ingame_overlay_t type);

cpumon_stats_t cpumon_stats = {0};

uint32_t audioBuffer[AUDIO_BUFFER_LENGTH];
uint32_t audio_mute;


int16_t pendingSamples = 0;
int16_t audiobuffer_emulator[AUDIO_BUFFER_LENGTH] __attribute__((section (".audio")));
int16_t audiobuffer_dma[AUDIO_BUFFER_LENGTH * 2] __attribute__((section (".audio")));

dma_transfer_state_t dma_state;
uint32_t dma_counter;

const uint8_t volume_tbl[ODROID_AUDIO_VOLUME_MAX + 1] = {
    (uint8_t)(UINT8_MAX * 0.00f),
    (uint8_t)(UINT8_MAX * 0.06f),
    (uint8_t)(UINT8_MAX * 0.125f),
    (uint8_t)(UINT8_MAX * 0.187f),
    (uint8_t)(UINT8_MAX * 0.25f),
    (uint8_t)(UINT8_MAX * 0.35f),
    (uint8_t)(UINT8_MAX * 0.42f),
    (uint8_t)(UINT8_MAX * 0.60f),
    (uint8_t)(UINT8_MAX * 0.80f),
    (uint8_t)(UINT8_MAX * 1.00f),
};

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    dma_counter++;
    dma_state = DMA_TRANSFER_STATE_HF;
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    dma_counter++;
    dma_state = DMA_TRANSFER_STATE_TC;
}


bool odroid_netplay_quick_start(void)
{
    return true;
}

// TODO: Move to own file
void odroid_audio_mute(bool mute)
{
    if (mute) {
        for (int i = 0; i < sizeof(audiobuffer_dma) / sizeof(audiobuffer_dma[0]); i++) {
            audiobuffer_dma[i] = 0;
        }
    }

    audio_mute = mute;
}

common_emu_state_t common_emu_state = {
    .frame_time_10us = (uint16_t)(100000 / 60 + 0.5f),  // Reasonable default of 60FPS if not explicitly configured.
};


bool common_emu_frame_loop(void){
    rg_app_desc_t *app = odroid_system_get_app();
    static int32_t frame_integrator = 0;
    int16_t frame_time_10us = common_emu_state.frame_time_10us;
    int16_t elapsed_10us = 100 * get_elapsed_time_since(common_emu_state.last_sync_time);
    bool draw_frame = common_emu_state.skip_frames < 2;

    if( !cpumon_stats.busy_ms ) cpumon_busy();
    odroid_system_tick(!draw_frame, 0, cpumon_stats.busy_ms);
    cpumon_reset();

    common_emu_state.pause_frames = 0;
    common_emu_state.skip_frames = 0;

    common_emu_state.last_sync_time = get_elapsed_time();

    if(common_emu_state.startup_frames < 3) {
        common_emu_state.startup_frames++;
        return true;
    }

    switch(app->speedupEnabled){
        case SPEEDUP_0_5x:
            frame_time_10us *= 2;
            break;
        case SPEEDUP_0_75x:
            frame_time_10us *= 5;
            frame_time_10us /= 4;
            break;
        case SPEEDUP_1_25x:
            frame_time_10us *= 4;
            frame_time_10us /= 5;
            break;
        case SPEEDUP_1_5x:
            frame_time_10us *= 2;
            frame_time_10us /= 3;
            break;
        case SPEEDUP_2x:
            frame_time_10us /= 2;
            break;
        case SPEEDUP_3x:
            frame_time_10us /= 3;
            break;
    }
    frame_integrator += (elapsed_10us - frame_time_10us);
    if(frame_integrator > frame_time_10us << 1) common_emu_state.skip_frames = 2;
    else if(frame_integrator > frame_time_10us) common_emu_state.skip_frames = 1;
    else if(frame_integrator < -frame_time_10us) common_emu_state.pause_frames = 1;
    common_emu_state.skipped_frames += common_emu_state.skip_frames;

    return draw_frame;
}



/**
 * Common input/macro/menuing features inside all emu loops. This is to be called
 * after inputs are read into `joystick`, but before the actual emulation tick
 * is called.
 *
 */
void common_emu_input_loop(odroid_gamepad_state_t *joystick, odroid_dialog_choice_t *game_options) {
    rg_app_desc_t *app = odroid_system_get_app();
    static emu_speedup_t last_speedup = SPEEDUP_1_5x;
    static int8_t last_key = -1;
    static bool pause_pressed = false;
    static bool macro_activated = false;

    if(joystick->values[ODROID_INPUT_VOLUME]){  // PAUSE/SET button
        // PAUSE/SET has been pressed, checking additional inputs for macros
        pause_pressed = true;
        if(last_key < 0) {
            if (joystick->values[ODROID_INPUT_POWER]){
                // Do NOT save-state and then poweroff
                last_key = ODROID_INPUT_POWER;
                HAL_SAI_DMAStop(&hsai_BlockA1);
                odroid_system_sleep();
            }
            else if(joystick->values[ODROID_INPUT_START]){ // GAME button
                // Reserved for future use
                last_key = ODROID_INPUT_START;
            }
            else if(joystick->values[ODROID_INPUT_SELECT]){ // TIME button
                // Toggle Speedup
                last_key = ODROID_INPUT_SELECT;
                if(app->speedupEnabled == SPEEDUP_1x) {
                    app->speedupEnabled = last_speedup;
                }
                else {
                    last_speedup = app->speedupEnabled;
                    app->speedupEnabled = SPEEDUP_1x;
                }
                set_ingame_overlay(INGAME_OVERLAY_SPEEDUP);
            }
            else if(joystick->values[ODROID_INPUT_LEFT]){
                // Volume Up
                last_key = ODROID_INPUT_LEFT;
                int8_t level = odroid_audio_volume_get();
                if (level > ODROID_AUDIO_VOLUME_MIN) odroid_audio_volume_set(--level);
                set_ingame_overlay(INGAME_OVERLAY_VOLUME);
            }
            else if(joystick->values[ODROID_INPUT_RIGHT]){
                // Volume Down
                last_key = ODROID_INPUT_RIGHT;
                int8_t level = odroid_audio_volume_get();
                if (level < ODROID_AUDIO_VOLUME_MAX) odroid_audio_volume_set(++level);
                set_ingame_overlay(INGAME_OVERLAY_VOLUME);
            }
            else if(joystick->values[ODROID_INPUT_UP]){
                // Brightness Up
                last_key = ODROID_INPUT_UP;
                int8_t level = odroid_display_get_backlight();
                if (level < ODROID_BACKLIGHT_LEVEL_COUNT - 1) odroid_display_set_backlight(++level);
                set_ingame_overlay(INGAME_OVERLAY_BRIGHTNESS);
            }
            else if(joystick->values[ODROID_INPUT_DOWN]){
                // Brightness Down
                last_key = ODROID_INPUT_DOWN;
                int8_t level = odroid_display_get_backlight();
                if (level > 0) odroid_display_set_backlight(--level);
                set_ingame_overlay(INGAME_OVERLAY_BRIGHTNESS);
            }
            else if(joystick->values[ODROID_INPUT_A]){
                // Save State
                last_key = ODROID_INPUT_A;
                odroid_audio_mute(true);

                // Call ingame overlay so that the save icon gets displayed first.
                set_ingame_overlay(INGAME_OVERLAY_SAVE);
                common_ingame_overlay();
                lcd_sync();

                odroid_system_emu_save_state(0);
                odroid_audio_mute(false);
                common_emu_state.startup_frames = 0;
            }
            else if(joystick->values[ODROID_INPUT_B]){
                // Load State
                last_key = ODROID_INPUT_B;
                odroid_system_emu_load_state(0);
                common_emu_state.startup_frames = 0;
                set_ingame_overlay(INGAME_OVERLAY_LOAD);
            }
        }

        if (last_key >= 0) {
            macro_activated = true;
            if (!joystick->values[last_key]) {
                last_key = -1;
            }

            // Consume all inputs so it doesn't get passed along to the
            // running emulator
            memset(joystick, '\x00', sizeof(odroid_gamepad_state_t));
        }

        // Refresh the last_overlay_time so that it won't disappear until after
        // PAUSE/SET has been released.
        common_emu_state.last_overlay_time = get_elapsed_time();
    }
    else if (pause_pressed && !joystick->values[ODROID_INPUT_VOLUME] && !macro_activated){
        // PAUSE/SET has been released without performing any macro. Launch menu
        pause_pressed = false;

        odroid_overlay_game_menu(game_options);
        memset(framebuffer1, 0x0, sizeof(framebuffer1));
        memset(framebuffer2, 0x0, sizeof(framebuffer2));
        common_emu_state.startup_frames = 0;
        cpumon_stats.last_busy = 0;
    }
    else if (!joystick->values[ODROID_INPUT_VOLUME]){
        pause_pressed = false;
        macro_activated = false;
        last_key = -1;
    }

    if(get_elapsed_time_since(common_emu_state.last_overlay_time) > 1000){
        set_ingame_overlay(INGAME_OVERLAY_NONE);
    }

    if (joystick->values[ODROID_INPUT_POWER]) {
        // Save-state and poweroff
        HAL_SAI_DMAStop(&hsai_BlockA1);
        app->saveState("");
        odroid_system_sleep();
    }

    if (common_emu_state.pause_after_frames > 0) {
        (common_emu_state.pause_after_frames)--;
        if (common_emu_state.pause_after_frames == 0) {
            pause_pressed = true;
        }
    }
}

static void cpumon_common(bool sleep){
    uint t0 = get_elapsed_time();
    if(cpumon_stats.last_busy){
        cpumon_stats.busy_ms += t0 - cpumon_stats.last_busy;
    }
    else{
        cpumon_stats.busy_ms = 0;
    }
    if(sleep) __WFI();
    uint t1 = get_elapsed_time();
    cpumon_stats.last_busy = t1;
    cpumon_stats.sleep_ms += t1 - t0;
}


void cpumon_busy(void){
    cpumon_common(false);
}

void cpumon_sleep(void){
    cpumon_common(true);
}

void cpumon_reset(void){
    cpumon_stats.busy_ms = 0;
    cpumon_stats.sleep_ms = 0;
}

#define OVERLAY_COLOR_565 0xFFFF

static const uint8_t ROUND[] = {  // This is the top/left of a 8-pixel radius circle
    0b00000001,
    0b00000111,
    0b00001111,
    0b00011111,
    0b00111111,
    0b01111111,
    0b01111111,
    0b11111111,
};

// These must be multiples of 8
#define IMG_H 24
#define IMG_W 24
static const uint8_t IMG_SPEAKER[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x04, 0x00, 0x30, 0x42, 0x00,
    0x70, 0x21, 0x00, 0xF0, 0x11, 0x7F, 0xF1, 0x09,
    0xFF, 0xF0, 0x89, 0xFF, 0xF0, 0x49, 0xFF, 0xF0,
    0x49, 0xFF, 0xF0, 0x49, 0xFF, 0xF0, 0x49, 0xFF,
    0xF0, 0x49, 0xFF, 0xF0, 0x49, 0x7F, 0xF0, 0x89,
    0x00, 0xF1, 0x09, 0x00, 0x70, 0x11, 0x00, 0x30,
    0x21, 0x00, 0x00, 0x42, 0x00, 0x00, 0x04, 0x00,
    0x00, 0x08, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
};

static const uint8_t IMG_SUN[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x04,
    0x10, 0x40, 0x02, 0x00, 0x80, 0x01, 0x01, 0x00,
    0x00, 0x38, 0x00, 0x00, 0x7C, 0x00, 0x00, 0xFE,
    0x00, 0x1C, 0xFE, 0x70, 0x00, 0xFE, 0x00, 0x00,
    0x7C, 0x00, 0x00, 0x38, 0x00, 0x01, 0x00, 0x80,
    0x02, 0x00, 0x40, 0x04, 0x10, 0x20, 0x00, 0x10,
    0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t IMG_FOLDER[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x3E, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x63,
    0x80, 0x00, 0x61, 0xFF, 0xE0, 0x60, 0xFF, 0xF0,
    0x60, 0x00, 0x30, 0x60, 0x00, 0x30, 0x60, 0x00,
    0x30, 0x60, 0x00, 0x00, 0x60, 0x3F, 0xFE, 0x60,
    0x7F, 0xFE, 0x60, 0xFF, 0xFC, 0x60, 0xFF, 0xF8,
    0x61, 0xFF, 0xF8, 0x63, 0xFF, 0xF0, 0x63, 0xFF,
    0xE0, 0x67, 0xFF, 0xE0, 0x7F, 0xFF, 0xC0, 0x3F,
    0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t IMG_DISKETTE[] = {
    0x00, 0x00, 0x00, 0x3F, 0xFF, 0xE0, 0x7C, 0x00,
    0x70, 0x7C, 0x03, 0x78, 0x7C, 0x03, 0x7C, 0x7C,
    0x03, 0x7E, 0x7C, 0x00, 0x7E, 0x7F, 0xFF, 0xFE,
    0x7F, 0xFF, 0xFE, 0x7F, 0xFF, 0xFE, 0x7F, 0xFF,
    0xFE, 0x7F, 0xFF, 0xFE, 0x7F, 0xFF, 0xFE, 0x7E,
    0x00, 0x7E, 0x7C, 0x00, 0x3E, 0x7C, 0x00, 0x3E,
    0x7D, 0xFF, 0xBE, 0x7C, 0x00, 0x3E, 0x7C, 0x00,
    0x3E, 0x7D, 0xFF, 0xBE, 0x7C, 0x00, 0x3E, 0x7C,
    0x00, 0x3E, 0x3F, 0xFF, 0xFC, 0x00, 0x00, 0x00,
};

static const uint8_t IMG_0_5X[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x1E, 0x03, 0xFC, 0x1E, 0x03, 0xFC,
    0x61, 0x83, 0x00, 0x61, 0x83, 0x00, 0x61, 0x83,
    0xF0, 0x61, 0x83, 0xF0, 0x61, 0x80, 0x0C, 0x61,
    0x80, 0x0C, 0x1E, 0x33, 0xF0, 0x1E, 0x33, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t IMG_0_75X[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x3C, 0x7E, 0x3F, 0x3C, 0x7E, 0x3F,
    0xC3, 0x01, 0xB0, 0xC3, 0x01, 0xB0, 0xC3, 0x06,
    0x3F, 0xC3, 0x06, 0x3F, 0xC3, 0x06, 0x03, 0xC3,
    0x18, 0x03, 0x3C, 0xD8, 0x3F, 0x3C, 0xD8, 0x3F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t IMG_1X[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x0C, 0x30, 0x03, 0x0C, 0x30,
    0x0F, 0x0C, 0x30, 0x0F, 0x0C, 0x30, 0x03, 0x03,
    0xC0, 0x03, 0x03, 0xC0, 0x03, 0x0C, 0x30, 0x03,
    0x0C, 0x30, 0x0F, 0xCC, 0x30, 0x0F, 0xCC, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t IMG_1_25X[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x30, 0x1E, 0x3F, 0x30, 0x1E, 0x3F,
    0xF0, 0x61, 0xB0, 0xF0, 0x61, 0xB0, 0x30, 0x06,
    0x3F, 0x30, 0x06, 0x3F, 0x30, 0x18, 0x03, 0x30,
    0x18, 0x03, 0xFB, 0x7F, 0xBF, 0xFB, 0x7F, 0xBF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t IMG_1_5X[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0C, 0x03, 0xFC, 0x0C, 0x03, 0xFC,
    0x3C, 0x03, 0x00, 0x3C, 0x03, 0x00, 0x0C, 0x03,
    0xF0, 0x0C, 0x03, 0xF0, 0x0C, 0x00, 0x0C, 0x0C,
    0x00, 0x0C, 0x3F, 0x33, 0xF0, 0x3F, 0x33, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t IMG_2X[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x07, 0x86, 0x18, 0x07, 0x86, 0x18,
    0x18, 0x66, 0x18, 0x18, 0x66, 0x18, 0x01, 0x81,
    0xE0, 0x01, 0x81, 0xE0, 0x06, 0x06, 0x18, 0x06,
    0x06, 0x18, 0x1F, 0xE6, 0x18, 0x1F, 0xE6, 0x18,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t IMG_3X[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x07, 0x86, 0x18, 0x07, 0x86, 0x18,
    0x18, 0x66, 0x18, 0x18, 0x66, 0x18, 0x01, 0x81,
    0xE0, 0x01, 0x81, 0xE0, 0x18, 0x66, 0x18, 0x18,
    0x66, 0x18, 0x07, 0x86, 0x18, 0x07, 0x86, 0x18,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

__attribute__((optimize("unroll-loops")))
static void draw_img(pixel_t *fb, const uint8_t *img, uint16_t x, uint16_t y){
    uint16_t idx = 0;
    for(uint8_t i=0; i < IMG_H; i++) {
        for(uint8_t j=0; j < IMG_W; j++) {
            if(img[idx / 8] & (1 << (7 - idx % 8))){
                fb[x + j +  GW_LCD_WIDTH * (y + i)] = OVERLAY_COLOR_565;
            }
            idx++;
        }
    }
}

#define DARKEN_MASK_565 0x7BEF  // Mask off the MSb of each color
#define DARKEN_ADD_565 0x2104  // value of 4-red, 8-green, 4-blue to add back in a little gray, especially on black backgrounds
static inline void darken_pixel(pixel_t *p){
    // Quickly divide all colors by 2
    *p = ((*p >> 1) & DARKEN_MASK_565) + DARKEN_ADD_565;
}

__attribute__((optimize("unroll-loops")))
static void draw_rectangle(pixel_t *fb, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
    for(uint16_t i=y1; i < y2; i++){
        for(uint16_t j=x1; j < x2; j++){
            fb[j + GW_LCD_WIDTH * i] = OVERLAY_COLOR_565;
        }
    }
}

__attribute__((optimize("unroll-loops")))
static void draw_darken_rectangle(pixel_t *fb, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
    for(uint16_t i=y1; i < y2; i++){
        for(uint16_t j=x1; j < x2; j++){
            darken_pixel(&fb[j + GW_LCD_WIDTH * i]);
        }
    }
}

__attribute__((optimize("unroll-loops")))
static void draw_darken_rounded_rectangle(pixel_t *fb, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
    // *1 is inclusive, *2 is exclusive
    uint16_t h = y2 - y1;
    uint16_t w = x2 - x1;
    if (w < 16 || h < 16) {
        // Draw not rounded rectangle
        draw_darken_rectangle(fb, x1, y1, x2, y2);
        return;
    }

    // Draw upper left round
    for(uint8_t i=0; i < 8; i++) for(uint8_t j=0; j < 8; j++)
        if(ROUND[i] & (1 << (7 - j))) darken_pixel(&fb[x1 + j + GW_LCD_WIDTH * (y1 + i)]);

    // Draw upper right round
    for(uint8_t i=0; i < 8; i++) for(uint8_t j=0; j < 8; j++)
        if(ROUND[i] & (1 << (7 - j))) darken_pixel(&fb[x2 - j - 1 + GW_LCD_WIDTH * (y1 + i)]);

    // Draw lower left round
    for(uint8_t i=0; i < 8; i++) for(uint8_t j=0; j < 8; j++)
        if(ROUND[i] & (1 << (7 - j))) darken_pixel(&fb[x1 + j + GW_LCD_WIDTH * (y2 - i - 1)]);

    // Draw lower right round
    for(uint8_t i=0; i < 8; i++) for(uint8_t j=0; j < 8; j++)
        if(ROUND[i] & (1 <<  (7 - j))) darken_pixel(&fb[x2 - j - 1 + GW_LCD_WIDTH * (y2 - i - 1)]);

    // Draw upper rectangle
    for(uint16_t i=x1+8; i < x2 - 8; i++) for(uint8_t j=0; j < 8; j++)
        darken_pixel(&fb[ i + GW_LCD_WIDTH * (y1 + j)]);

    // Draw central rectangle
    for(uint16_t i=x1; i < x2; i++) for(uint16_t j=y1+8; j < y2-8; j++)
        darken_pixel(&fb[i+GW_LCD_WIDTH * j]);

    // Draw lower rectangle
    for(uint16_t i=x1+8; i < x2 - 8; i++) for(uint8_t j=0; j < 8; j++)
        darken_pixel(&fb[ i + GW_LCD_WIDTH * (y2 - j - 1)]);
}

#define INGAME_OVERLAY_X 265
#define INGAME_OVERLAY_Y 10
#define INGAME_OVERLAY_BARS_H 128
#define INGAME_OVERLAY_W 39
#define INGAME_OVERLAY_BORDER 4
#define INGAME_OVERLAY_BOX_GAP 2

#define INGAME_OVERLAY_BARS_W INGAME_OVERLAY_W
#define INGAME_OVERLAY_IMG_H  (IMG_H + 2 * INGAME_OVERLAY_BORDER)  // For when only an image is showing

#define INGAME_OVERLAY_BOX_W (INGAME_OVERLAY_BARS_W - (2 * INGAME_OVERLAY_BORDER) - 6)
#define INGAME_OVERLAY_BOX_X (INGAME_OVERLAY_X + ((INGAME_OVERLAY_BARS_W - INGAME_OVERLAY_BOX_W) / 2))
#define INGAME_OVERLAY_BOX_Y (INGAME_OVERLAY_Y + INGAME_OVERLAY_BORDER + 1)

// For when only an image is shown
#define INGAME_OVERLAY_IMG_X (INGAME_OVERLAY_X + ((INGAME_OVERLAY_BARS_W - IMG_W) / 2))
#define INGAME_OVERLAY_IMG_Y (INGAME_OVERLAY_Y + INGAME_OVERLAY_BORDER)

// Places the image at the bottom for bars-related overlay (volume, brightness)
#define INGAME_OVERLAY_BARS_IMG_X INGAME_OVERLAY_IMG_X
#define INGAME_OVERLAY_BARS_IMG_Y (INGAME_OVERLAY_Y + INGAME_OVERLAY_BARS_H - IMG_H - INGAME_OVERLAY_BORDER)

#define DARKEN_IMG_ONLY() draw_darken_rounded_rectangle(fb, \
                    INGAME_OVERLAY_X, \
                    INGAME_OVERLAY_Y, \
                    INGAME_OVERLAY_X + INGAME_OVERLAY_BARS_W, \
                    INGAME_OVERLAY_Y + INGAME_OVERLAY_IMG_H)


static uint8_t box_height(uint8_t n) {
    return ((INGAME_OVERLAY_BARS_IMG_Y - INGAME_OVERLAY_BOX_Y) / n) - INGAME_OVERLAY_BOX_GAP;
}

void common_ingame_overlay(void) {
    rg_app_desc_t *app = odroid_system_get_app();
    pixel_t *fb = lcd_get_active_buffer();
    int8_t level;
    uint8_t bh;
    uint16_t by = INGAME_OVERLAY_BOX_Y;

    switch(common_emu_state.overlay)
    {
        case INGAME_OVERLAY_NONE:
            break;
        case INGAME_OVERLAY_VOLUME:
            level = odroid_audio_volume_get();
            bh = box_height(ODROID_AUDIO_VOLUME_MAX);

            draw_darken_rounded_rectangle(fb,
                    INGAME_OVERLAY_X,
                    INGAME_OVERLAY_Y,
                    INGAME_OVERLAY_X + INGAME_OVERLAY_BARS_W,
                    INGAME_OVERLAY_Y + INGAME_OVERLAY_BARS_H);
            draw_img(fb, IMG_SPEAKER, INGAME_OVERLAY_BARS_IMG_X, INGAME_OVERLAY_BARS_IMG_Y);

            for(int8_t i=ODROID_AUDIO_VOLUME_MAX; i > 0; i--){
                if(i <= level)
                    draw_rectangle(fb,
                            INGAME_OVERLAY_BOX_X,
                            by,
                            INGAME_OVERLAY_BOX_X + INGAME_OVERLAY_BOX_W,
                            by + bh);
                else
                    draw_darken_rectangle(fb,
                            INGAME_OVERLAY_BOX_X,
                            by,
                            INGAME_OVERLAY_BOX_X + INGAME_OVERLAY_BOX_W,
                            by + bh);

                by += bh + INGAME_OVERLAY_BOX_GAP;
            }
            break;
        case INGAME_OVERLAY_BRIGHTNESS:
            level = odroid_display_get_backlight();
            bh = box_height(ODROID_BACKLIGHT_LEVEL_COUNT - 1);

            draw_darken_rounded_rectangle(fb,
                    INGAME_OVERLAY_X,
                    INGAME_OVERLAY_Y,
                    INGAME_OVERLAY_X + INGAME_OVERLAY_BARS_W,
                    INGAME_OVERLAY_Y + INGAME_OVERLAY_BARS_H);
            draw_img(fb, IMG_SUN, INGAME_OVERLAY_BARS_IMG_X, INGAME_OVERLAY_BARS_IMG_Y);

            for(int8_t i=ODROID_BACKLIGHT_LEVEL_COUNT-1; i > 0; i--){
                if(i <= level)
                    draw_rectangle(fb,
                            INGAME_OVERLAY_BOX_X,
                            by,
                            INGAME_OVERLAY_BOX_X + INGAME_OVERLAY_BOX_W,
                            by + bh);
                else
                    draw_darken_rectangle(fb,
                            INGAME_OVERLAY_BOX_X,
                            by,
                            INGAME_OVERLAY_BOX_X + INGAME_OVERLAY_BOX_W,
                            by + bh);

                by += bh + INGAME_OVERLAY_BOX_GAP;
            }
            break;
        case INGAME_OVERLAY_LOAD:
            DARKEN_IMG_ONLY();
            draw_img(fb, IMG_FOLDER, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
            break;
        case INGAME_OVERLAY_SAVE:
            DARKEN_IMG_ONLY();
            draw_img(fb, IMG_DISKETTE, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
            break;
        case INGAME_OVERLAY_SPEEDUP:
            DARKEN_IMG_ONLY();
            switch(app->speedupEnabled){
                case SPEEDUP_0_5x:
                    draw_img(fb, IMG_0_5X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
                case SPEEDUP_0_75x:
                    draw_img(fb, IMG_0_75X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
                case SPEEDUP_1x:
                    draw_img(fb, IMG_1X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
                case SPEEDUP_1_25x:
                    draw_img(fb, IMG_1_25X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
                case SPEEDUP_1_5x:
                    draw_img(fb, IMG_1_5X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
                case SPEEDUP_2x:
                    draw_img(fb, IMG_2X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
                case SPEEDUP_3x:
                    draw_img(fb, IMG_3X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
            }
            break;

    }
}

static void set_ingame_overlay(ingame_overlay_t type){
    common_emu_state.overlay = type;
    common_emu_state.last_overlay_time = get_elapsed_time();
}
