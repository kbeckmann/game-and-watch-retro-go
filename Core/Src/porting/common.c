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


/**
 * Call this each time a frame is drawn.
 *
 * Currently assumes that framerate is 60 fps.
 *
 * Emu responsibilities:
 *    * increment `common_emu_state.skip_frames` if a frame came in too slow.
 * @returns bool Whether or not to draw the frame.
 */
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
            }
            else if(joystick->values[ODROID_INPUT_LEFT]){
                // Volume Up
                last_key = ODROID_INPUT_LEFT;
                int8_t level = odroid_audio_volume_get();
                if (level > ODROID_AUDIO_VOLUME_MIN) odroid_audio_volume_set(--level);
            }
            else if(joystick->values[ODROID_INPUT_RIGHT]){
                // Volume Down
                last_key = ODROID_INPUT_RIGHT;
                int8_t level = odroid_audio_volume_get();
                if (level < ODROID_AUDIO_VOLUME_MAX) odroid_audio_volume_set(++level);
            }
            else if(joystick->values[ODROID_INPUT_UP]){
                // Brightness Up
                last_key = ODROID_INPUT_UP;
                int8_t level = odroid_display_get_backlight();
                if (level < ODROID_BACKLIGHT_LEVEL_COUNT - 1) odroid_display_set_backlight(++level);
            }
            else if(joystick->values[ODROID_INPUT_DOWN]){
                // Brightness Down
                last_key = ODROID_INPUT_DOWN;
                int8_t level = odroid_display_get_backlight();
                if (level > 0) odroid_display_set_backlight(--level);
            }
            else if(joystick->values[ODROID_INPUT_A]){
                // Save State
                last_key = ODROID_INPUT_A;
                odroid_audio_mute(true);
                odroid_system_emu_save_state(0);
                odroid_audio_mute(false);
                common_emu_state.startup_frames = 0;
            }
            else if(joystick->values[ODROID_INPUT_B]){
                // Load State
                last_key = ODROID_INPUT_B;
                odroid_system_emu_load_state(0);
                common_emu_state.startup_frames = 0;
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
#define IMG_H 32
#define IMG_W 32

static const uint8_t IMG_SPEAKER[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x00, 0x00, 0x00, 0x40, 0x00, 0x03, 0x04, 0x20,
    0x00, 0x07, 0x02, 0x10, 0x00, 0x0F, 0x01, 0x10,
    0x07, 0xFF, 0x10, 0x90, 0x0F, 0xFF, 0x08, 0x90,
    0x0F, 0xFF, 0x04, 0x90, 0x0F, 0xFF, 0x04, 0x90,
    0x0F, 0xFF, 0x04, 0x90, 0x0F, 0xFF, 0x04, 0x90,
    0x0F, 0xFF, 0x04, 0x90, 0x0F, 0xFF, 0x04, 0x90,
    0x07, 0xFF, 0x08, 0x90, 0x00, 0x0F, 0x10, 0x90,
    0x00, 0x07, 0x01, 0x10, 0x00, 0x03, 0x02, 0x10,
    0x00, 0x00, 0x04, 0x20, 0x00, 0x00, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00,
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

#define DARKEN_MASK_565 0x7BEF
static inline void darken_pixel(pixel_t *p){
    // Quickly divide all colors by 2
    *p = (*p >> 1) & DARKEN_MASK_565;
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
static void draw_darken_rounded_rectangle(pixel_t *fb, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
    // *1 is inclusive, *2 is exclusive
    uint16_t h = y2 - y1;
    uint16_t w = x2 - x1;
    if (w < 16 || h < 16) {
        // Draw not rounded rectangle
        for(uint16_t i=y1; i < y2; i++){
            for(uint16_t j=x1; j < x2; j++){
                darken_pixel(&fb[j + GW_LCD_WIDTH * i]);
            }
        }
    }
    else {
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
}

#define INGAME_OVERLAY_X 265
#define INGAME_OVERLAY_Y 10
#define INGAME_OVERLAY_H 128
#define INGAME_OVERLAY_W 45

#define INGAME_OVERLAY_BOX_W (INGAME_OVERLAY_W - 10)

#define IMG_X (INGAME_OVERLAY_X + ((INGAME_OVERLAY_W - IMG_W) / 2))
#define IMG_Y (INGAME_OVERLAY_Y + INGAME_OVERLAY_H - IMG_H - 4)

void common_ingame_overlay(void) {
    pixel_t *fb = lcd_get_active_buffer();
    draw_darken_rounded_rectangle(fb, INGAME_OVERLAY_X, INGAME_OVERLAY_Y, INGAME_OVERLAY_X + INGAME_OVERLAY_W, INGAME_OVERLAY_Y + INGAME_OVERLAY_H);
    draw_img(fb, IMG_SPEAKER, IMG_X, IMG_Y);
}
