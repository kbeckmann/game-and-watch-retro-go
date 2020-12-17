#include <odroid_system.h>
#include <string.h>

#include "main.h"
#include "bilinear.h"
#include "gw_lcd.h"
#include "gw_linker.h"
#include "gw_buttons.h"
#include "shared.h"
#include "rom_manager.h"
#include "common.h"

#define APP_ID 30

#define SMS_WIDTH 256
#define SMS_HEIGHT 192

#define GG_WIDTH 160
#define GG_HEIGHT 144

#define PIXEL_MASK 0x1F
#define PAL_SHIFT_MASK 0x80

static uint16_t palette[32];

static uint skipFrames = 0;

static bool consoleIsGG = false;
static bool consoleIsSMS = false;

// TODO: Move to lcd.c/h
extern LTDC_HandleTypeDef hltdc;

void set_config();
unsigned int crc32_le(unsigned int crc, unsigned char const * buf,unsigned int len);

// --- MAIN

static int load_rom_from_flash(void)
{
    static uint8 sram[0x8000];
    cart.rom = (uint8 *)ROM_DATA;
    cart.size = ROM_DATA_LENGTH;
    //cart.sram = cart.sram ?: rg_alloc(0x8000, MEM_SLOW);
    cart.sram = sram;
    cart.pages = cart.size / 0x4000;
    cart.crc = crc32_le(0, cart.rom, cart.size);
    cart.loaded = 1;

    set_config();

    printf("%s: OK. cart.size=%d, cart.crc=%#010lx\n", __func__, (int)cart.size, cart.crc);

    return 1;
}

static void netplay_callback(netplay_event_t event, void *arg)
{
    // Where we're going we don't need netplay!
}

static bool SaveState(char *pathName)
{
    return false;
}

static bool LoadState(char *pathName)
{
    return true;
}

uint8_t *fb_buffer =emulator_framebuffer;

static inline void blit_normal(bitmap_t *bmp, uint16_t *framebuffer) {
    const int hpad = (WIDTH - bmp->viewport.w) / 2;
    const int vpad = (HEIGHT - bmp->viewport.h) / 2;

    for (int y = 0; y < bmp->viewport.h; y++) {
        uint8_t *row = &bmp->data[(y + bmp->viewport.y) * bmp->pitch];
        uint16_t *dest = NULL;
        dest = &framebuffer[WIDTH * (y + vpad) + hpad];
        for (int x = 0; x < bmp->viewport.w; x++) {
            uint16_t pixel = palette[row[x + bmp->viewport.x] & 0x1f];
            dest[x] = pixel << 8 | pixel >> 8;
        }
    }
}

void sms_pcm_submit() {
    size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : AUDIO_BUFFER_LENGTH;
    if (audio_mute) {
        for (int i = 0; i < AUDIO_BUFFER_LENGTH; i++) {
            audiobuffer_dma[i + offset] = 0;
        }
    } else {
        for (int i = 0; i < AUDIO_BUFFER_LENGTH; i++) {
            /* mix left & right */
            audiobuffer_dma[i + offset] = (sms_snd.output[0][i] + sms_snd.output[1][i]) >> 1;
        }
    }
}

void app_main_smsplusgx(void)
{
    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, &netplay_callback);

    // Load ROM
    rg_app_desc_t *app = odroid_system_get_app();

    load_rom_from_flash();

    system_reset_config();

    sms.use_fm = 0;

    // sms.dummy = framebuffer[0]; //A normal cart shouldn't access this memory ever. Point it to vram just in case.
    // sms.sram = malloc(SRAM_SIZE);

    bitmap.width = SMS_WIDTH;
    bitmap.height = SMS_HEIGHT;
    bitmap.pitch = bitmap.width;
    bitmap.data = fb_buffer;

    option.sndrate = AUDIO_SAMPLE_RATE;
    option.overscan = 0;
    option.extra_gg = 0;

    system_init2();
    system_reset();

    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, sizeof(audiobuffer_dma) / sizeof(audiobuffer_dma[0]));

    consoleIsSMS = sms.console == CONSOLE_SMS || sms.console == CONSOLE_SMS2;
    consoleIsGG  = sms.console == CONSOLE_GG || sms.console == CONSOLE_GGMS;

    // if (consoleIsSMS) odroid_system_set_app_id(APP_ID + 1);
    // if (consoleIsGG)  odroid_system_set_app_id(APP_ID + 2);

#if 0
    if (app->startAction == ODROID_START_ACTION_RESUME)
    {
        odroid_system_emu_load_state(0);
    }
#endif

    const int refresh_rate = (sms.display == DISPLAY_NTSC) ? FPS_NTSC : FPS_PAL;
    const int frameTime = get_frame_time(refresh_rate);
    bool fullFrame = false;

    // Video
    memset(framebuffer1, 0, sizeof(framebuffer1));
    memset(framebuffer2, 0, sizeof(framebuffer2));

    while (true)
    {
        uint32_t buttons = buttons_get();

        uint startTime = get_elapsed_time();
        bool drawFrame = !skipFrames;
        bool pause_pressed = false;

        input.pad[0] = 0x00;
        input.pad[1] = 0x00;
        input.system = 0x00;

        if (buttons & B_Up)     input.pad[0] |= INPUT_UP;
        if (buttons & B_Down)   input.pad[0] |= INPUT_DOWN;
        if (buttons & B_Left)   input.pad[0] |= INPUT_LEFT;
        if (buttons & B_Right)  input.pad[0] |= INPUT_RIGHT;
        if (buttons & B_A)      input.pad[0] |= INPUT_BUTTON2;
        if (buttons & B_B)      input.pad[0] |= INPUT_BUTTON1;

        if (consoleIsSMS)
        {
            if (buttons & B_GAME) input.system |= INPUT_START;
            if (buttons & B_TIME) input.system |= INPUT_PAUSE;
        }
        else if (consoleIsGG)
        {
            if (buttons & B_GAME) input.system |= INPUT_START;
            if (buttons & B_TIME) input.system |= INPUT_PAUSE;
        }

        if (buttons & B_POWER) {
            HAL_SAI_DMAStop(&hsai_BlockA1);
            lcd_backlight_off();
            GW_EnterDeepSleep();
        }

        if (buttons & B_PAUSE) {
            if (!pause_pressed) {
                pause_pressed = true;
                audio_mute = !audio_mute;
            }
        }
        else
            pause_pressed = false;

        system_frame(!drawFrame);

        if (drawFrame)
        {
            static uint32_t lastFPSTime = 0;
            static uint32_t frames = 0;
            uint32_t currentTime = HAL_GetTick();
            uint32_t delta = currentTime - lastFPSTime;

            frames++;

            if (delta >= 1000) {
                int fps = (10000 * frames) / delta;
                printf("FPS: %d.%d, frames %ld, delta %ld ms\n", fps / 10, fps % 10, frames, delta);
                printf("SND: %d %d\n", sms_snd.sample_count, AUDIO_BUFFER_LENGTH);
                frames = 0;
                lastFPSTime = currentTime;
            }

            render_copy_palette((uint16_t *)&palette);
            // This takes less than 1ms
            if(active_framebuffer == 0) {
                blit_normal(&bitmap, framebuffer1);
                active_framebuffer = 1;
            } else {
                blit_normal(&bitmap, framebuffer2);
                active_framebuffer = 0;
            }

            HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
        }

        // See if we need to skip a frame to keep up
        if (skipFrames == 0)
        {
            if (get_elapsed_time_since(startTime) > frameTime) skipFrames = 1;
            if (app->speedupEnabled) skipFrames += app->speedupEnabled * 2.5;
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }

        // Tick before submitting audio/syncing
        odroid_system_tick(!drawFrame, fullFrame, get_elapsed_time_since(startTime));

        if (!app->speedupEnabled)
        {
            sms_pcm_submit();
            static dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;
            while (dma_state == last_dma_state) {
                __NOP();
            }
            last_dma_state = dma_state;
        }
    }
}
