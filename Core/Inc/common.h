#pragma once
#include "main.h"
extern SAI_HandleTypeDef hsai_BlockA1;
extern DMA_HandleTypeDef hdma_sai1_a;

#define WIDTH  320
#define HEIGHT 240
#define BPP      4

#define AUDIO_SAMPLE_RATE   (48000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60)



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
