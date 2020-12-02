#include <odroid_system.h>

#include <string.h>
#include <nofrendo.h>
#include <bitmap.h>
#include <nes.h>
#include <nes_input.h>
#include <nes_state.h>
#include <nes_input.h>
#include <osd.h>
#include "buttons.h"
#include "gw_lcd.h"
#include "rom_info.h"

#define WIDTH  320
#define HEIGHT 240
#define BPP      4

#define APP_ID 30

#define AUDIO_SAMPLE_RATE   (48000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60)

typedef enum {
    DMA_TRANSFER_STATE_HF = 0x00,
    DMA_TRANSFER_STATE_TC = 0x01,
} dma_transfer_state_t;

static uint32_t audioBuffer[AUDIO_BUFFER_LENGTH];

extern unsigned char cart_rom[];
extern unsigned int cart_rom_len;
unsigned char ram_cart_rom[ROM_LENGTH] __attribute__((section (".emulator_data")));;
unsigned int  ram_cart_rom_len = ROM_LENGTH;

static uint romCRC32;

static int16_t pendingSamples = 0;
static int16_t audiobuffer_emulator[AUDIO_BUFFER_LENGTH] __attribute__((section (".audio")));
static int16_t audiobuffer_dma[AUDIO_BUFFER_LENGTH * 2] __attribute__((section (".audio")));
static dma_transfer_state_t dma_state;

extern SAI_HandleTypeDef hsai_BlockA1;
extern DMA_HandleTypeDef hdma_sai1_a;

static odroid_gamepad_state_t joystick1;
static odroid_gamepad_state_t joystick2;
static odroid_gamepad_state_t *localJoystick = &joystick1;
static odroid_gamepad_state_t *remoteJoystick = &joystick2;

static bool overscan = true;
static uint autocrop = false;
static bool netplay  = false;

static bool fullFrame = 0;
static uint frameTime = 0;


void odroid_display_force_refresh(void)
{
    // forceVideoRefresh = true;
}

int osd_init()
{
   return 0;
}

// TODO: Move to lcd.c/h
extern LTDC_HandleTypeDef hltdc;

void osd_setpalette(rgb_t *pal)
{
    uint32_t clut[256];

    for (int i = 0; i < 64; i++)
    {
        uint16_t c = (pal[i].b>>3) | ((pal[i].g>>2)<<5) | ((pal[i].r>>3)<<11);

        // The upper bits are used to indicate background and transparency.
        // They need to be indexed as well.
        clut[i]        = (pal[i].b) | (pal[i].g << 8) | (pal[i].r << 16);
        clut[i | 0x40] = (pal[i].b) | (pal[i].g << 8) | (pal[i].r << 16);
        clut[i | 0x80] = (pal[i].b) | (pal[i].g << 8) | (pal[i].r << 16);
    }

    // Update the color-LUT in the LTDC peripheral
    HAL_LTDC_ConfigCLUT(&hltdc, clut, 256, 0);
    HAL_LTDC_EnableCLUT(&hltdc, 0);

    // color 13 is "black". Makes for a nice border.
    memset(framebuffer1, 13, sizeof(framebuffer1));

    odroid_display_force_refresh();
}

static uint32_t skippedFrames = 0;

void osd_wait_for_vsync()
{
    static uint32_t skipFrames = 0;
    static uint32_t lastSyncTime = 0;

    uint32_t elapsed = get_elapsed_time_since(lastSyncTime);

    if (skipFrames == 0) {
        rg_app_desc_t *app = odroid_system_get_app();
        if (elapsed > frameTime) skipFrames = 1;
        if (app->speedupEnabled) skipFrames += app->speedupEnabled * 2;
    } else if (skipFrames > 0) {
        skipFrames--;
        skippedFrames++;
    }

    // Tick before submitting audio/syncing
    odroid_system_tick(!nes_getptr()->drawframe, fullFrame, elapsed);

    nes_getptr()->drawframe = (skipFrames == 0);

    // Wait until the audio buffer has been transmitted
    static dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;
    while (dma_state != last_dma_state) {
        __NOP();
    }

    lastSyncTime = get_elapsed_time();
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    dma_state = DMA_TRANSFER_STATE_HF;
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    dma_state = DMA_TRANSFER_STATE_TC;
}

void osd_audioframe(int audioSamples)
{
    if (odroid_system_get_app()->speedupEnabled)
        return;

    apu_process(audiobuffer_emulator, audioSamples); //get audio data

    size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : audioSamples;

    // Write to DMA buffer and lower the volume to 1/4
    for (int i = 0; i < audioSamples; i++) {
        audiobuffer_dma[i + offset] = audiobuffer_emulator[i] >> 1;
    }
}

void osd_blitscreen(bitmap_t *bmp)
{
    static uint32_t lastFPSTime = 0;
    static uint32_t lastTime = 0;
    static uint32_t frames = 0;
    uint32_t currentTime = HAL_GetTick();
    uint32_t delta = currentTime - lastFPSTime;

    frames++;

    if (delta >= 1000) {
        int fps = (10000 * frames) / delta;
        printf("FPS: %d.%d, frames %d, delta %d ms, skipped %d\n", fps / 10, fps % 10, delta, frames, skippedFrames);
        frames = 0;
        skippedFrames = 0;
        lastFPSTime = currentTime;
    }

    lastTime = currentTime;

    // LCD is 320 wide, framebuffer is only 256
    const int hpad = (WIDTH - NES_SCREEN_WIDTH) / 2;

    // This takes less than 1ms
    for (int y = 0; y < bmp->height; y++) {
        uint8_t *row = bmp->line[y];
        uint32_t *dest = &framebuffer1[WIDTH * y + hpad];
        memcpy(dest, row, bmp->width);
    }
}

void osd_getinput(void)
{
    uint16 pad0 = 0;

    uint32_t buttons = buttons_get();
    if(buttons & B_GAME) pad0 |= INP_PAD_START;
    if(buttons & B_TIME) pad0 |= INP_PAD_SELECT;
    if(buttons & B_Up)   pad0 |= INP_PAD_UP;
    if(buttons & B_Down)   pad0 |= INP_PAD_DOWN;
    if(buttons & B_Left)   pad0 |= INP_PAD_LEFT;
    if(buttons & B_Right)   pad0 |= INP_PAD_RIGHT;
    if(buttons & B_A)   pad0 |= INP_PAD_A;
    if(buttons & B_B)   pad0 |= INP_PAD_B;

    // Enable to log button presses
#if 0
    static old_pad0;
    if (pad0 != old_pad0) {
        printf("pad0=%02x\n", pad0);
        old_pad0 = pad0;
    }
#endif

    input_update(INP_JOYPAD0, pad0);
}

size_t osd_getromdata(unsigned char **data)
{
    *data = (unsigned char*)ram_cart_rom;
   return ram_cart_rom_len;
}

uint osd_getromcrc()
{
   return romCRC32;
}

void osd_loadstate()
{
    frameTime = get_frame_time(nes_getptr()->refresh_rate);
}

static bool SaveState(char *pathName)
{
    return true;
}

static bool LoadState(char *pathName)
{
    return true;
}

int app_main(void)
{
    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, NULL);

    printf("app_main ROM: cart_rom_len=%ld\n", cart_rom_len);

    memcpy(ram_cart_rom, cart_rom, cart_rom_len);
    romCRC32 = crc32_le(0, (const uint8_t*)(ram_cart_rom + 16), ram_cart_rom_len - 16);

    printf("Nofrendo start!\n");

    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));

    HAL_SAI_Transmit_DMA(&hsai_BlockA1, audiobuffer_dma, sizeof(audiobuffer_dma) / sizeof(audiobuffer_dma[0]));

    // nofrendo_start("Rom name (E).nes", NES_PAL, AUDIO_SAMPLE_RATE);
    nofrendo_start("Rom name (USA).nes", NES_NTSC, AUDIO_SAMPLE_RATE);

    return 0;
}
