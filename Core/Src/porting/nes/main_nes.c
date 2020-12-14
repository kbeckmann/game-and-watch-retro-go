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

#ifndef GW_LCD_MODE_LUT8
#error "Only supports LCD LUT8 mode."
#endif

#ifdef BLIT_NEAREST
#define blit blit_nearest
#else
#define blit blit_normal
#endif

static uint32_t audio_mute;

extern unsigned char cart_rom[];
extern unsigned int cart_rom_len;

#ifdef NES_CACHE_ROM_IN_RAM
unsigned char ram_cart_rom[ROM_LENGTH] __attribute__((section (".emulator_data")));;
unsigned int  ram_cart_rom_len = ROM_LENGTH;

unsigned int crc32_le(unsigned int crc, unsigned char const * buf, unsigned int len);
#endif

static uint romCRC32;

static int16_t audiobuffer_emulator[AUDIO_BUFFER_LENGTH] __attribute__((section (".audio")));
static int16_t audiobuffer_dma[AUDIO_BUFFER_LENGTH * 2] __attribute__((section (".audio")));
static uint32_t dma_counter;
static dma_transfer_state_t dma_state;

extern SAI_HandleTypeDef hsai_BlockA1;
extern DMA_HandleTypeDef hdma_sai1_a;

static uint32_t pause_pressed;
static uint32_t power_pressed;

static bool fullFrame = 0;
static uint frameTime = 0;
static uint32_t vsync_wait_ms = 0;

static bool autoload = false;
static uint32_t active_framebuffer = 0;
// TODO
extern void store_save(uint8_t *data, size_t size);



// if i counted correctly this should max be 23077
uint8_t nes_save_buffer[24000];



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
    memset(framebuffer2, 13, sizeof(framebuffer2));

    odroid_display_force_refresh();
}

static uint32_t skippedFrames = 0;

void osd_wait_for_vsync()
{
    static uint32_t skipFrames = 0;
    static uint32_t lastSyncTime = 0;
    uint32_t t0;

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
    static uint32_t last_dma_counter = 0;
    t0 = get_elapsed_time();
    while (dma_counter == last_dma_counter) {
        __WFI();
    }
    vsync_wait_ms += get_elapsed_time_since(t0);

    last_dma_counter = dma_counter;

    lastSyncTime = get_elapsed_time();
}

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

void osd_audioframe(int audioSamples)
{
    if (odroid_system_get_app()->speedupEnabled)
        return;

    apu_process(audiobuffer_emulator, audioSamples); //get audio data

    size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : audioSamples;

    if (audio_mute) { 
        for (int i = 0; i < audioSamples; i++) {
            audiobuffer_dma[i + offset] = 0;
        }
        return;
    }

    // Write to DMA buffer and lower the volume to 1/4
    for (int i = 0; i < audioSamples; i++) {
        audiobuffer_dma[i + offset] = audiobuffer_emulator[i] >> 1;
    }
}

static inline void blit_normal(bitmap_t *bmp, uint8_t *framebuffer) {
        // LCD is 320 wide, framebuffer is only 256
    const int hpad = (WIDTH - NES_SCREEN_WIDTH) / 2;

    for (int y = 0; y < bmp->height; y++) {
        uint8_t *row = bmp->line[y];
        uint8_t *dest = NULL;
        if(active_framebuffer == 0) {
            dest = &framebuffer[WIDTH * y + hpad];
        } else {
            dest = &framebuffer[WIDTH * y + hpad];
        }
        memcpy(dest, row, bmp->width);
    }
}

static inline void blit_nearest(bitmap_t *bmp, uint8_t *framebuffer) {
    int w1 = bmp->width;
    int h1 = bmp->height;
    int w2 = WIDTH;
    int h2 = h1;

    // Blit: 5581 us
    // This can still be improved quite a bit by using aligned accesses.

    PROFILING_INIT(t_blit);
    PROFILING_START(t_blit);

    int ctr = 0;
    for (int y = 0; y < h2; y++) {
        uint8_t  *src_row  = bmp->line[y];
        uint8_t *dest_row = &framebuffer[y * w2];
        int x2 = 0;
        for (int x = 0; x < w1; x++) {
            uint8_t b2 = src_row[x];
            dest_row[x2++] = b2;
            if (ctr++ == 4) {
                ctr = 0;
                dest_row[x2++] = b2;
            }
        }
    }

    PROFILING_END(t_blit);

#ifdef PROFILING_ENABLED
    printf("Blit: %d us\n", (1000000 * PROFILING_DIFF(t_blit)) / t_blit_t0.SecondFraction);
#endif
}

void osd_blitscreen(bitmap_t *bmp)
{
    static uint32_t lastFPSTime = 0;
    static uint32_t frames = 0;
    uint32_t currentTime = HAL_GetTick();
    uint32_t delta = currentTime - lastFPSTime;

    frames++;

    if (delta >= 1000) {
        int fps = (10000 * frames) / delta;
        printf("FPS: %d.%d, frames %ld, delta %ld ms, skipped %ld\n", fps / 10, fps % 10, frames, delta, skippedFrames);
        frames = 0;
        skippedFrames = 0;
        vsync_wait_ms = 0;
        lastFPSTime = currentTime;
    }

    // This takes less than 1ms
    if(active_framebuffer == 0) {
        blit(bmp, framebuffer1);
        active_framebuffer = 1;
    } else {
        blit(bmp, framebuffer2);
        active_framebuffer = 0;
    }

    HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
}

void HAL_LTDC_ReloadEventCallback (LTDC_HandleTypeDef *hltdc) {
    if(active_framebuffer == 0) {
        HAL_LTDC_SetAddress(hltdc, (uint32_t)framebuffer2, 0);
    } else {
        HAL_LTDC_SetAddress(hltdc, (uint32_t)framebuffer1, 0);
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

    if (pause_pressed != (buttons & B_PAUSE)) {
        if (pause_pressed) {
            printf("Pause pressed %ld=>%d\n", audio_mute, !audio_mute);
            audio_mute = !audio_mute;
        }
        pause_pressed = buttons & B_PAUSE;
    }

    if (power_pressed != (buttons & B_POWER)) {
        printf("Power toggle %ld=>%d\n", power_pressed, !power_pressed);
        power_pressed = buttons & B_POWER;
        if (buttons & B_POWER) {
            printf("Power PRESSED %ld\n", power_pressed);
            HAL_SAI_DMAStop(&hsai_BlockA1);

            if(!(buttons & B_PAUSE)) {
                // Always save as long as PAUSE is not pressed
                state_save(nes_save_buffer, 24000);
                store_save(nes_save_buffer, 24000);
            }

            GW_EnterDeepSleep();
        }
    }

    odroid_overlay_game_menu();

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
#ifdef NES_CACHE_ROM_IN_RAM
    *data = (unsigned char*)ram_cart_rom;
    return ram_cart_rom_len;
#else
    *data = (unsigned char*)cart_rom;
    return cart_rom_len;
#endif
}

uint osd_getromcrc()
{
   return romCRC32;
}

void osd_loadstate()
{
    frameTime = get_frame_time(nes_getptr()->refresh_rate);
    if(autoload) {
        autoload = false;
        uint32_t save_size = &__SAVE_END__ - &__SAVE_START__;
        if(save_size < 64 * 1024) {
            // no save support
            return;
        }

        uint8_t *ptr = (uint8_t*)&__SAVE_START__;
        state_load(ptr, 24000);
    }
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

    uint32_t buttons = GW_GetBootButtons();
    pause_pressed = (buttons & B_PAUSE);
    power_pressed = (buttons & B_POWER);

    // Always load the previous game unless pause is pressed
    autoload = !pause_pressed;


    printf("app_main ROM: cart_rom_len=%d\n", cart_rom_len);

#ifdef NES_CACHE_ROM_IN_RAM
    memcpy(ram_cart_rom, cart_rom, cart_rom_len);
    romCRC32 = crc32_le(0, (const uint8_t*)(ram_cart_rom + 16), ram_cart_rom_len - 16);
#else
    romCRC32 = crc32_le(0, (const uint8_t*)(cart_rom + 16), cart_rom_len - 16);
#endif

    printf("Nofrendo start!\n");

    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));

    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, sizeof(audiobuffer_dma) / sizeof(audiobuffer_dma[0]));

    // nofrendo_start("Rom name (E).nes", NES_PAL, AUDIO_SAMPLE_RATE);
    nofrendo_start("Rom name (USA).nes", NES_NTSC, AUDIO_SAMPLE_RATE);

    return 0;
}
