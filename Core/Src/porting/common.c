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
    .frame_time_10us = 10000 / 60,  // Most (all?) emus are 60fps
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
bool common_emu_frame_loop(){
    rg_app_desc_t *app = odroid_system_get_app();
    static int32_t frame_integrator = 0;
    int16_t frame_time_10us = common_emu_state.frame_time_10us;
    int16_t elapsed_10us = 100 * get_elapsed_time_since(common_emu_state.last_sync_time);
    bool draw_frame = !common_emu_state.skip_frames;

    odroid_system_tick(!draw_frame, 0, cpumon_stats.busy_ms);
    cpumon_reset();

    common_emu_state.pause_frames = 0;
    if(!draw_frame) common_emu_state.skip_frames = 0;

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
    if(frame_integrator > frame_time_10us) common_emu_state.skip_frames = 1;
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
    static uint8_t pause_pressed = 0;
    static uint8_t power_pressed = 0;
    static int8_t pause_pressed_count = 0;

    if(pause_pressed && pause_pressed_count >= 0){
        // Long-pressing PAUSE/SET to quickly toggle emulator speed.
        if(pause_pressed_count < 127) {
            pause_pressed_count++;
        }
        if(pause_pressed_count > 30) {  // 30 frames = half a second
            if(app->speedupEnabled == SPEEDUP_1x) {
                app->speedupEnabled = last_speedup;
            }
            else {
                last_speedup = app->speedupEnabled;
                app->speedupEnabled = SPEEDUP_1x;
            }

            pause_pressed_count = -1;
        }
    }

    if (pause_pressed != joystick->values[ODROID_INPUT_VOLUME]) {
        if (pause_pressed && pause_pressed_count >= 0) {
            // PAUSE/SET has been released
            odroid_overlay_game_menu(game_options);
            memset(framebuffer1, 0x0, sizeof(framebuffer1));
            memset(framebuffer2, 0x0, sizeof(framebuffer2));
            common_emu_state.startup_frames = 0;
            cpumon_stats.last_busy = 0;
        }
        pause_pressed = joystick->values[ODROID_INPUT_VOLUME];
        pause_pressed_count = 0;
    }

    if (power_pressed != joystick->values[ODROID_INPUT_POWER]) {
        printf("Power toggle %ld=>%d\n", power_pressed, !power_pressed);
        power_pressed = joystick->values[ODROID_INPUT_POWER];
        if (power_pressed) {
            printf("Power PRESSED %ld\n", power_pressed);
            HAL_SAI_DMAStop(&hsai_BlockA1);
            if(!joystick->values[ODROID_INPUT_VOLUME]) {
                app->saveState("");
            }
            odroid_system_sleep();
        }
    }

    if (common_emu_state.pause_after_frames > 0) {
        (common_emu_state.pause_after_frames)--;
        if (common_emu_state.pause_after_frames == 0) {
            pause_pressed = 1;
        }
    }
}

void cpumon_sleep(){
    uint t0 = get_elapsed_time();
    if(cpumon_stats.last_busy){
        cpumon_stats.busy_ms += t0 - cpumon_stats.last_busy;
    }
    else{
        cpumon_stats.busy_ms = 0;
    }
    __WFI();
    uint t1 = get_elapsed_time();
    cpumon_stats.last_busy = t1;
    cpumon_stats.sleep_ms += t1 - t0;
}

void cpumon_reset(){
    cpumon_stats.busy_ms = 0;
    cpumon_stats.sleep_ms = 0;
}
