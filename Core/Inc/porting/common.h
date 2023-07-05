#pragma once

#include <odroid_system.h>

#include "main.h"

extern SAI_HandleTypeDef hsai_BlockA1;
extern DMA_HandleTypeDef hdma_sai1_a;

#define WIDTH  320
#define HEIGHT 240
#define BPP      4

// Default to 50Hz as it results in more samples than at 60Hz
#define AUDIO_SAMPLE_RATE   (48000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50)



typedef enum {
    DMA_TRANSFER_STATE_HF = 0x00,
    DMA_TRANSFER_STATE_TC = 0x01,
} dma_transfer_state_t;
extern dma_transfer_state_t dma_state;
extern uint32_t dma_counter;

extern uint32_t audioBuffer[AUDIO_BUFFER_LENGTH];
extern uint32_t audio_mute;


extern int16_t pendingSamples;
extern int16_t audiobuffer_emulator[AUDIO_BUFFER_LENGTH] __attribute__((section (".audio")));
extern int16_t audiobuffer_dma[AUDIO_BUFFER_LENGTH * 2] __attribute__((section (".audio")));

extern const uint8_t volume_tbl[ODROID_AUDIO_VOLUME_MAX + 1];
extern const uint8_t volume_tbl_low[ODROID_AUDIO_VOLUME_MAX + 1];
extern const uint8_t volume_tbl_very_low[ODROID_AUDIO_VOLUME_MAX + 1];

bool common_emu_frame_loop(void);
void common_emu_input_loop(odroid_gamepad_state_t *joystick, odroid_dialog_choice_t *game_options);

typedef struct {
    uint last_busy;
    uint busy_ms;
    uint sleep_ms;
} cpumon_stats_t;
extern cpumon_stats_t cpumon_stats;

/**
 * Just calls `__WFI()` and measures time spent sleeping.
 */
void cpumon_sleep(void);
void cpumon_busy(void);
void cpumon_reset(void);


enum {
    INGAME_OVERLAY_NONE,
    INGAME_OVERLAY_VOLUME,
    INGAME_OVERLAY_BRIGHTNESS,
    INGAME_OVERLAY_SAVE,
    INGAME_OVERLAY_LOAD,
    INGAME_OVERLAY_SPEEDUP,
};
typedef uint8_t ingame_overlay_t;

/**
 * Holds common higher-level emu options that need to be used at not-neat
 * locations in each emulator.
 *
 * There should only be one of these objects instantiated.
 */
typedef struct {
    uint32_t last_sync_time;
    uint32_t last_overlay_time;
    uint16_t skipped_frames;
    int16_t frame_time_10us;
    uint8_t skip_frames:2;
    uint8_t pause_frames:1;
    uint8_t pause_after_frames:3;
    uint8_t startup_frames:2;
    uint8_t overlay:3;
} common_emu_state_t;

extern common_emu_state_t common_emu_state;


/**
 * Drawable stuff over current emulation.
 */
void common_ingame_overlay(void);
