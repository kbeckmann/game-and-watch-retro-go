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
static uint32_t palette_spaced[32];


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

uint8_t *fb_buffer = emulator_framebuffer;

#define CONV(_b0) ((0b11111000000000000000000000&_b0)>>10) | ((0b000001111110000000000&_b0)>>5) | ((0b0000000000011111&_b0));

__attribute__((optimize("unroll-loops")))
void blit_gg(bitmap_t *bmp, uint16_t *framebuffer) {	/* 160 x 144 -> 320 x 240 */
    int y_src = 0;
    int y_dst = 0;
    for (; y_src < bmp->viewport.h; y_src += 3, y_dst += 5) {
        int x_src = 0;
        int x_dst = 0;
        for (; x_src < bmp->viewport.w; x_src += 1, x_dst += 2) {
            uint8_t *src_col = &bmp->data[(y_src + bmp->viewport.y) * bmp->pitch + x_src + bmp->viewport.x];
            uint32_t b0 = palette_spaced[src_col[bmp->pitch * 0] & 0x1f];
            uint32_t b1 = palette_spaced[src_col[bmp->pitch * 1] & 0x1f];
            uint32_t b2 = palette_spaced[src_col[bmp->pitch * 2] & 0x1f];

            framebuffer[((y_dst + 0) * WIDTH) + x_dst] = CONV(b0);
            framebuffer[((y_dst + 1) * WIDTH) + x_dst] = CONV((b0+b1)>>1);
            framebuffer[((y_dst + 2) * WIDTH) + x_dst] = CONV(b1);
            framebuffer[((y_dst + 3) * WIDTH) + x_dst] = CONV((b1+b2)>>1);
            framebuffer[((y_dst + 4) * WIDTH) + x_dst] = CONV(b2);

            framebuffer[((y_dst + 0) * WIDTH) + x_dst + 1] = CONV(b0);
            framebuffer[((y_dst + 1) * WIDTH) + x_dst + 1] = CONV((b0+b1)>>1);
            framebuffer[((y_dst + 2) * WIDTH) + x_dst + 1] = CONV(b1);
            framebuffer[((y_dst + 3) * WIDTH) + x_dst + 1] = CONV((b1+b2)>>1);
            framebuffer[((y_dst + 4) * WIDTH) + x_dst + 1] = CONV(b2);
        }
    }
}

__attribute__((optimize("unroll-loops")))
void blit_sms(bitmap_t *bmp, uint16_t *framebuffer) {	/* 256 x 192 -> 320 x 230 */
    const int hpad = (WIDTH - 307) / 2;
    const int vpad = (HEIGHT - 230) / 2;

    uint32_t block[6 * 5]; /* workspace: 5 rows, 6 pixels wide */

    int y_src = 1;         /* 1st and last row of 192 will not be scaled */
    int y_dst = 1 + vpad;  /* the remaining 190 are scaled */
    for (; y_src < bmp->viewport.h - 1; y_src += 5, y_dst += 6) {
        int x_src = 0;
        int x_dst = hpad;
        for (; x_src < bmp->viewport.w - 1; x_src += 5, x_dst += 6) {
            for (int y = 0; y < 5; y++) {
                uint8_t *src_row = &bmp->data[(y_src + y + bmp->viewport.y) * bmp->pitch];
                uint32_t b0 = palette_spaced[src_row[x_src + 0] & 0x1f];
                uint32_t b1 = palette_spaced[src_row[x_src + 1] & 0x1f];
                uint32_t b2 = palette_spaced[src_row[x_src + 2] & 0x1f];
                uint32_t b3 = palette_spaced[src_row[x_src + 3] & 0x1f];
                uint32_t b4 = palette_spaced[src_row[x_src + 4] & 0x1f];

                block[(y * 6) + 0] = b0;
                block[(y * 6) + 1] = (b0+b1+b1+b1)>>2;
                block[(y * 6) + 2] = (b1+b2)>>1;
                block[(y * 6) + 3] = (b2+b3)>>1;
                block[(y * 6) + 4] = (b3+b3+b3+b4)>>2;
                block[(y * 6) + 5] = b4;
            }

            for (int x = 0; x < 6; x++) {
                uint32_t b0 = block[(0 * 6) + x];
                uint32_t b1 = block[(1 * 6) + x];
                uint32_t b2 = block[(2 * 6) + x];
                uint32_t b3 = block[(3 * 6) + x];
                uint32_t b4 = block[(4 * 6) + x];

                framebuffer[((y_dst + 0) * WIDTH) + x + x_dst] = CONV(b0);
                framebuffer[((y_dst + 1) * WIDTH) + x + x_dst] = CONV((b0+b1+b1+b1)>>2);
                framebuffer[((y_dst + 2) * WIDTH) + x + x_dst] = CONV((b1+b2)>>1);
                framebuffer[((y_dst + 3) * WIDTH) + x + x_dst] = CONV((b2+b3)>>1);
                framebuffer[((y_dst + 4) * WIDTH) + x + x_dst] = CONV((b3+b3+b3+b4)>>2);
                framebuffer[((y_dst + 5) * WIDTH) + x + x_dst] = CONV(b4);
            }
        }

        /* Last column, x_src = 255 */
	uint8_t *src_col = &bmp->data[(y_src + bmp->viewport.y) * bmp->pitch + x_src];
	uint32_t b0 = palette_spaced[src_col[bmp->pitch * 0] & 0x1f];
	uint32_t b1 = palette_spaced[src_col[bmp->pitch * 1] & 0x1f];
	uint32_t b2 = palette_spaced[src_col[bmp->pitch * 2] & 0x1f];
	uint32_t b3 = palette_spaced[src_col[bmp->pitch * 3] & 0x1f];
	uint32_t b4 = palette_spaced[src_col[bmp->pitch * 4] & 0x1f];

	framebuffer[((y_dst + 0) * WIDTH) + x_dst] = CONV(b0);
	framebuffer[((y_dst + 1) * WIDTH) + x_dst] = CONV((b0+b1+b1+b1)>>2);
	framebuffer[((y_dst + 2) * WIDTH) + x_dst] = CONV((b1+b2)>>1);
	framebuffer[((y_dst + 3) * WIDTH) + x_dst] = CONV((b2+b3)>>1);
	framebuffer[((y_dst + 4) * WIDTH) + x_dst] = CONV((b3+b3+b3+b4)>>2);
	framebuffer[((y_dst + 5) * WIDTH) + x_dst] = CONV(b4);
    }

    y_src = 0;		   /* First & last row */
    y_dst = 0 + vpad;
    for (; y_src < bmp->viewport.h; y_src += 191, y_dst += 228) {
        uint8_t *src_row = &bmp->data[(y_src + bmp->viewport.y) * bmp->pitch];
        uint16_t *dest_row = &framebuffer[WIDTH * y_dst];
        int x_src = 0;
        int x_dst = hpad;
        for (; x_src < bmp->viewport.w - 1; x_src += 5, x_dst += 6) {
            uint32_t b0 = palette_spaced[src_row[x_src + 0] & 0x1f];
            uint32_t b1 = palette_spaced[src_row[x_src + 1] & 0x1f];
            uint32_t b2 = palette_spaced[src_row[x_src + 2] & 0x1f];
            uint32_t b3 = palette_spaced[src_row[x_src + 3] & 0x1f];
            uint32_t b4 = palette_spaced[src_row[x_src + 4] & 0x1f];

            dest_row[x_dst + 0]   = CONV(b0);
            dest_row[x_dst + 1] = CONV((b0+b1+b1+b1)>>2);
            dest_row[x_dst + 2] = CONV((b1+b2)>>1);
            dest_row[x_dst + 3] = CONV((b2+b3)>>1);
            dest_row[x_dst + 4] = CONV((b3+b3+b3+b4)>>2);
            dest_row[x_dst + 5] = CONV(b4);
        }
        /* Last column, x_src = 255 */
        dest_row[x_dst] = CONV(palette_spaced[src_row[x_src] & 0x1f]);
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
            for (int i = 0; i < 32; i++) {
                uint16_t p = (palette[i] << 8) | (palette[i] >> 8);
                palette_spaced[i] = ((0b1111100000000000 & p) << 10) |
                                    ((0b0000011111100000 & p) << 5) |
                                    ((0b0000000000011111 & p));
            }

            if(active_framebuffer == 0) {
                if (consoleIsSMS)
                    blit_sms(&bitmap, framebuffer1);
                if (consoleIsGG)
                    blit_gg(&bitmap, framebuffer1);
                active_framebuffer = 1;
            } else {
                if (consoleIsSMS)
                    blit_sms(&bitmap, framebuffer2);
                if (consoleIsGG)
                    blit_gg(&bitmap, framebuffer2);
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
