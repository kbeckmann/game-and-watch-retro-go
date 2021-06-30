#include <odroid_system.h>
#include <string.h>

#include "main.h"
#include "bilinear.h"
#include "gw_lcd.h"
#include "gw_linker.h"
#include "gw_buttons.h"
#include "gnuboy/loader.h"
#include "gnuboy/hw.h"
#include "gnuboy/lcd.h"
#include "gnuboy/cpu.h"
#include "gnuboy/mem.h"
#include "gnuboy/sound.h"
#include "gnuboy/regs.h"
#include "gnuboy/rtc.h"
#include "gnuboy/defs.h"
#include "common.h"
#include "rom_manager.h"

#define ODROID_APPID_GB 1

#define NVS_KEY_SAVE_SRAM "sram"

// Use 60Hz for GB
#define AUDIO_BUFFER_LENGTH_GB (AUDIO_SAMPLE_RATE / 60)
#define AUDIO_BUFFER_LENGTH_DMA_GB ((2 * AUDIO_SAMPLE_RATE) / 60)

// #define blit screen_blit
// #define blit screen_blit_bilinear
// #define blit screen_blit_jth
// #define blit screen_blit_v3to5

#ifndef blit
#define blit screen_blit_v3to5
#endif


static uint32_t pause_pressed;
static uint32_t power_pressed;


static odroid_video_frame_t update1 = {GB_WIDTH, GB_HEIGHT, GB_WIDTH * 2, 2, 0xFF, -1, NULL, NULL, 0, {}};
static odroid_video_frame_t update2 = {GB_WIDTH, GB_HEIGHT, GB_WIDTH * 2, 2, 0xFF, -1, NULL, NULL, 0, {}};
static odroid_video_frame_t *currentUpdate = &update1;

static bool fullFrame = false;
static uint skipFrames = 0;

static bool saveSRAM = false;
static int  saveSRAM_Timer = 0;

// --- MAIN


static void netplay_callback(netplay_event_t event, void *arg)
{
    // Where we're going we don't need netplay!
}

#define WIDTH 320

static uint32_t skippedFrames = 0;


__attribute__((optimize("unroll-loops")))
static inline void screen_blit(void) {
    static uint32_t lastFPSTime = 0;
    static uint32_t frames = 0;
    uint32_t currentTime = HAL_GetTick();
    uint32_t delta = currentTime - lastFPSTime;

    frames++;

    if (delta >= 1000) {
        int fps = (10000 * frames) / delta;
        printf("FPS: %d.%d, frames %ld, delta %ld ms, skipped %ld\n", fps / 10, fps % 10, delta, frames, skippedFrames);
        frames = 0;
        skippedFrames = 0;
        lastFPSTime = currentTime;
    }

    int w1 = currentUpdate->width;
    int h1 = currentUpdate->height;
    int w2 = 266;
    int h2 = 240;

    int x_ratio = (int)((w1<<16)/w2) +1;
    int y_ratio = (int)((h1<<16)/h2) +1;
    int hpad = 27;
    //int x_ratio = (int)((w1<<16)/w2) ;
    //int y_ratio = (int)((h1<<16)/h2) ;
    int x2, y2 ;
    uint16_t* screen_buf = (uint16_t*)currentUpdate->buffer;
    uint16_t *dest = lcd_get_active_buffer();

    PROFILING_INIT(t_blit);
    PROFILING_START(t_blit);

    for (int i=0;i<h2;i++) {
        for (int j=0;j<w2;j++) {
            x2 = ((j*x_ratio)>>16) ;
            y2 = ((i*y_ratio)>>16) ;
            uint16_t b2 = screen_buf[(y2*w1)+x2];
            dest[(i*WIDTH)+j+hpad] = b2;
        }
    }

    PROFILING_END(t_blit);

#ifdef PROFILING_ENABLED
    printf("Blit: %d us\n", (1000000 * PROFILING_DIFF(t_blit)) / t_blit_t0.SecondFraction);
#endif

    lcd_swap();
}

static void screen_blit_bilinear(void) {
    static uint32_t lastFPSTime = 0;
    static uint32_t frames = 0;
    uint32_t currentTime = HAL_GetTick();
    uint32_t delta = currentTime - lastFPSTime;

    frames++;

    if (delta >= 1000) {
        int fps = (10000 * frames) / delta;
        printf("FPS: %d.%d, frames %ld, delta %ld ms, skipped %ld\n", fps / 10, fps % 10, delta, frames, skippedFrames);
        frames = 0;
        skippedFrames = 0;
        lastFPSTime = currentTime;
    }

    int w1 = currentUpdate->width;
    int h1 = currentUpdate->height;

    int w2 = 320;
    int h2 = 216;

    int hpad = 0;
    uint16_t *dest = lcd_get_active_buffer();


    image_t dst_img;
    dst_img.w = 320;
    dst_img.h = 240;
    dst_img.bpp = 2;
    dst_img.pixels = (uint8_t *) dest;


    image_t src_img;
    src_img.w = currentUpdate->width;
    src_img.h = currentUpdate->height;
    src_img.bpp = 2;
    src_img.pixels = currentUpdate->buffer;

    float x_scale = ((float) w2) / ((float) w1);
    float y_scale = ((float) h2) / ((float) h1);




    PROFILING_INIT(t_blit);
    PROFILING_START(t_blit);

    imlib_draw_image(&dst_img, &src_img, hpad, 0, x_scale, y_scale, NULL, -1, 255, NULL,
                     NULL, IMAGE_HINT_BILINEAR, NULL, NULL);

    PROFILING_END(t_blit);

#ifdef PROFILING_ENABLED
    printf("Blit: %d us\n", (1000000 * PROFILING_DIFF(t_blit)) / t_blit_t0.SecondFraction);
#endif

    lcd_swap();
}

__attribute__((optimize("unroll-loops")))
__attribute__((section (".itcram_hot_text")))
static inline void screen_blit_v3to5(void) {
    static uint32_t lastFPSTime = 0;
    static uint32_t frames = 0;
    uint32_t currentTime = HAL_GetTick();
    uint32_t delta = currentTime - lastFPSTime;

    frames++;

    if (delta >= 1000) {
        int fps = (10000 * frames) / delta;
        printf("FPS: %d.%d, frames %ld, delta %ld ms, skipped %ld\n", fps / 10, fps % 10, delta, frames, skippedFrames);
        frames = 0;
        skippedFrames = 0;
        lastFPSTime = currentTime;
    }

    uint16_t *dest = lcd_get_active_buffer();

    PROFILING_INIT(t_blit);
    PROFILING_START(t_blit);

#define CONV(_b0)    (((0b11111000000000000000000000&_b0)>>10) | ((0b000001111110000000000&_b0)>>5) | ((0b0000000000011111&_b0)))
#define EXPAND(_b0)  (((0b1111100000000000 & _b0) << 10) | ((0b0000011111100000 & _b0) << 5) | ((0b0000000000011111 & _b0)))

    int y_src = 0;
    int y_dst = 0;
    int w = currentUpdate->width;
    int h = currentUpdate->height;
    for (; y_src < h; y_src += 3, y_dst += 5) {
        int x_src = 0;
        int x_dst = 0;
        for (; x_src < w; x_src += 1, x_dst += 2) {
            uint16_t *src_col = &((uint16_t *)currentUpdate->buffer)[(y_src * w) + x_src];
            uint32_t b0 = EXPAND(src_col[w * 0]);
            uint32_t b1 = EXPAND(src_col[w * 1]);
            uint32_t b2 = EXPAND(src_col[w * 2]);

            dest[((y_dst + 0) * WIDTH) + x_dst] = CONV(b0);
            dest[((y_dst + 1) * WIDTH) + x_dst] = CONV((b0+b1)>>1);
            dest[((y_dst + 2) * WIDTH) + x_dst] = CONV(b1);
            dest[((y_dst + 3) * WIDTH) + x_dst] = CONV((b1+b2)>>1);
            dest[((y_dst + 4) * WIDTH) + x_dst] = CONV(b2);

            dest[((y_dst + 0) * WIDTH) + x_dst + 1] = CONV(b0);
            dest[((y_dst + 1) * WIDTH) + x_dst + 1] = CONV((b0+b1)>>1);
            dest[((y_dst + 2) * WIDTH) + x_dst + 1] = CONV(b1);
            dest[((y_dst + 3) * WIDTH) + x_dst + 1] = CONV((b1+b2)>>1);
            dest[((y_dst + 4) * WIDTH) + x_dst + 1] = CONV(b2);
        }
    }

    PROFILING_END(t_blit);

#ifdef PROFILING_ENABLED
    printf("Blit: %d us\n", (1000000 * PROFILING_DIFF(t_blit)) / t_blit_t0.SecondFraction);
#endif

    lcd_swap();
}


__attribute__((optimize("unroll-loops")))
__attribute__((section (".itcram_hot_text")))
static inline void screen_blit_jth(void) {
    static uint32_t lastFPSTime = 0;
    static uint32_t frames = 0;
    uint32_t currentTime = HAL_GetTick();
    uint32_t delta = currentTime - lastFPSTime;

    frames++;

    if (delta >= 1000) {
        int fps = (10000 * frames) / delta;
        printf("FPS: %d.%d, frames %ld, delta %ld ms, skipped %ld\n", fps / 10, fps % 10, delta, frames, skippedFrames);
        frames = 0;
        skippedFrames = 0;
        lastFPSTime = currentTime;
    }


    uint16_t* screen_buf = (uint16_t*)currentUpdate->buffer;
    uint16_t *dest = lcd_get_active_buffer();

    PROFILING_INIT(t_blit);
    PROFILING_START(t_blit);


    int w1 = currentUpdate->width;
    int h1 = currentUpdate->height;
    int w2 = 320;
    int h2 = 240;

    const int border = 24;

    // Iterate on dest buf rows
    for(int y = 0; y < border; ++y) {
        uint16_t *src_row  = &screen_buf[y * w1];
        uint16_t *dest_row = &dest[y * w2];
        for (int x = 0, xsrc=0; x < w2; x+=2,xsrc++) {
            dest_row[x]     = src_row[xsrc];
            dest_row[x + 1] = src_row[xsrc];
        }
    }

    for (int y = border, src_y = border; y < h2-border; y+=2, src_y++) {
        uint16_t *src_row  = &screen_buf[src_y * w1];
        uint32_t *dest_row0 = (uint32_t *) &dest[y * w2];
        for (int x = 0, xsrc=0; x < w2; x++,xsrc++) {
            uint32_t col = src_row[xsrc];
            dest_row0[x] = (col | (col << 16));
        }
    }

    for (int y = border, src_y = border; y < h2-border; y+=2, src_y++) {
        uint16_t *src_row  = &screen_buf[src_y * w1];
        uint32_t *dest_row1 = (uint32_t *)&dest[(y + 1) * w2];
        for (int x = 0, xsrc=0; x < w2; x++,xsrc++) {
            uint32_t col = src_row[xsrc];
            dest_row1[x] = (col | (col << 16));
        }
    }

    for(int y = 0; y < border; ++y) {
        uint16_t *src_row  = &screen_buf[(h1-border+y) * w1];
        uint16_t *dest_row = &dest[(h2-border+y) * w2];
        for (int x = 0, xsrc=0; x < w2; x+=2,xsrc++) {
            dest_row[x]     = src_row[xsrc];
            dest_row[x + 1] = src_row[xsrc];
        }
    }


    PROFILING_END(t_blit);

#ifdef PROFILING_ENABLED
    printf("Blit: %d us\n", (1000000 * PROFILING_DIFF(t_blit)) / t_blit_t0.SecondFraction);
#endif

    lcd_swap();
}

// __attribute__((optimize("unroll-loops")))
// static inline void screen_blit(void)
// {
//     // printf("%d x %d\n", bmp->width, bmp->height);
//     for (int y = 0; y < currentUpdate->height; y++) {
//         // uint8_t *row = bmp->line[line];
//         for (int x = 0; x < currentUpdate->width; x++) {
//             uint16_t* screen_buf = (uint16_t*)currentUpdate->buffer;
//             uint16_t b1 = screen_buf[(currentUpdate->width * y + x)];
//             // uint16_t b2 = screen_buf[(WIDTH * y + x)*2 + 1];
//             // for (int line = 0; line < bmp->height; line++) {
//             //     uint8_t *row = bmp->line[line];
//             //     for (int x = 0; x < bmp->width; x++) {
//             //         framebuffer1[WIDTH * line + x + hpad] = myPalette[row[x] & 0b111111];
//             //     }
//             // }
//             // framebuffer1[WIDTH * y + x] = currentUpdate->palette[b1];
//             framebuffer1[WIDTH * (y+48) + x + 80] = b1;
//         }
//     }
//     /*
//     odroid_video_frame_t *previousUpdate = (currentUpdate == &update1) ? &update2 : &update1;

//     fullFrame = odroid_display_queue_update(currentUpdate, previousUpdate) == SCREEN_UPDATE_FULL;

//     // swap buffers
//     currentUpdate = previousUpdate;
//     fb.ptr = currentUpdate->buffer;
//     */
// }

#define STATE_SAVE_BUFFER_LENGTH 1024 * 192

static bool SaveState(char *pathName)
{
    printf("Saving state...\n");

    // Use GB_ROM_SRAM_CACHE (which points to _GB_ROM_UNPACK_BUFFER)
    // as a temporary save buffer.
    memset(GB_ROM_SRAM_CACHE,  '\x00', STATE_SAVE_BUFFER_LENGTH);
    size_t size = gb_state_save(GB_ROM_SRAM_CACHE, STATE_SAVE_BUFFER_LENGTH);
    store_save(ACTIVE_FILE->save_address, GB_ROM_SRAM_CACHE, size);

    // Restore the cache that was overwritten above.
    gb_loader_restore_cache();

    return 0;
}

static bool LoadState(char *pathName)
{
    gb_state_load(ACTIVE_FILE->save_address, ACTIVE_FILE->save_size);
    return true;
}


static bool palette_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int pal = pal_get_dmg();
    int max = pal_count_dmg();

    if (event == ODROID_DIALOG_PREV) {
        pal = pal > 0 ? pal - 1 : max;
    }

    if (event == ODROID_DIALOG_NEXT) {
        pal = pal < max ? pal + 1 : 0;
    }

    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        odroid_settings_Palette_set(pal);
        pal_set_dmg(pal);
        lcd_reset_active_buffer();
        emu_run(true);
        lcd_swap();
        lcd_sync();
    }

    if (pal == 0) strcpy(option->value, "GBC");
    else sprintf(option->value, "%d/%d", pal, max);

    return event == ODROID_DIALOG_ENTER;
}

/*static bool save_sram_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        saveSRAM = !saveSRAM;
        odroid_settings_app_int32_set(NVS_KEY_SAVE_SRAM, saveSRAM);
    }

    strcpy(option->value, saveSRAM ? "Yes" : "No");

    return event == ODROID_DIALOG_ENTER;
    return -
}

static bool rtc_t_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (option->id == 'd') {
        if (event == ODROID_DIALOG_PREV && --rtc.d < 0) rtc.d = 364;
        if (event == ODROID_DIALOG_NEXT && ++rtc.d > 364) rtc.d = 0;
        sprintf(option->value, "%03d", rtc.d);
    }
    if (option->id == 'h') {
        if (event == ODROID_DIALOG_PREV && --rtc.h < 0) rtc.h = 23;
        if (event == ODROID_DIALOG_NEXT && ++rtc.h > 23) rtc.h = 0;
        sprintf(option->value, "%02d", rtc.h);
    }
    if (option->id == 'm') {
        if (event == ODROID_DIALOG_PREV && --rtc.m < 0) rtc.m = 59;
        if (event == ODROID_DIALOG_NEXT && ++rtc.m > 59) rtc.m = 0;
        sprintf(option->value, "%02d", rtc.m);
    }
    if (option->id == 's') {
        if (event == ODROID_DIALOG_PREV && --rtc.s < 0) rtc.s = 59;
        if (event == ODROID_DIALOG_NEXT && ++rtc.s > 59) rtc.s = 0;
        sprintf(option->value, "%02d", rtc.s);
    }
    return event == ODROID_DIALOG_ENTER;
}

static bool rtc_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_ENTER) {
        static odroid_dialog_choice_t choices[] = {
            {'d', "Day", "000", 1, &rtc_t_update_cb},
            {'h', "Hour", "00", 1, &rtc_t_update_cb},
            {'m', "Min",  "00", 1, &rtc_t_update_cb},
            {'s', "Sec",  "00", 1, &rtc_t_update_cb},
            ODROID_DIALOG_CHOICE_LAST
        };
        odroid_overlay_dialog("Set Clock", choices, 0);
    }
    sprintf(option->value, "%02d:%02d", rtc.h, rtc.m);
    return false;
}

static bool advanced_settings_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
   if (event == ODROID_DIALOG_ENTER) {
      odroid_dialog_choice_t options[] = {
        {101, "Set clock", "00:00", 1, &rtc_update_cb},
        {102, "Auto save SRAM", "No", 1, &save_sram_update_cb},
        ODROID_DIALOG_CHOICE_LAST
      };
      odroid_overlay_dialog("Advanced", options, 0);
   }
   return false;
}*/

void pcm_submit() {
    uint8_t volume = odroid_audio_volume_get();
    int32_t factor = volume_tbl[volume];
    size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : AUDIO_BUFFER_LENGTH_GB;

    if (audio_mute || volume == ODROID_AUDIO_VOLUME_MIN) {
        for (int i = 0; i < AUDIO_BUFFER_LENGTH_GB; i++) {
            audiobuffer_dma[i + offset] = 0;
        }
    } else {
        for (int i = 0; i < AUDIO_BUFFER_LENGTH_GB; i++) {
            int32_t sample = pcm.buf[i];
            audiobuffer_dma[i + offset] = (sample * factor) >> 8;
        }
    }
}


rg_app_desc_t * init(uint8_t load_state)
{
    odroid_system_init(ODROID_APPID_GB, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, &netplay_callback);

    // bzhxx : fix LCD glitch at the start by cleaning up the buffer emulator
    memset(emulator_framebuffer, 0x0, sizeof(emulator_framebuffer));

    // Hack: Use the same buffer twice
    update1.buffer = emulator_framebuffer;
    update2.buffer = emulator_framebuffer;

    //saveSRAM = odroid_settings_app_int32_get(NVS_KEY_SAVE_SRAM, 0);
    saveSRAM = false;

    // Load ROM
    loader_init(NULL);

    // RTC
    memset(&rtc, 0, sizeof(rtc));

    // Video
    memset(framebuffer1, 0, sizeof(framebuffer1));
    memset(framebuffer2, 0, sizeof(framebuffer2));
    memset(&fb, 0, sizeof(fb));
    fb.w = GB_WIDTH;
    fb.h = GB_HEIGHT;
    fb.format = GB_PIXEL_565_LE;
    fb.pitch = update1.stride;
    fb.ptr = currentUpdate->buffer;
    fb.enabled = 1;
    fb.blit_func = &blit;

    // Audio
    memset(audiobuffer_emulator, 0, sizeof(audiobuffer_emulator));
    memset(&pcm, 0, sizeof(pcm));
    pcm.hz = AUDIO_SAMPLE_RATE;
    pcm.stereo = 0;
    pcm.len = AUDIO_BUFFER_LENGTH_GB;
    pcm.buf = (n16*)&audiobuffer_emulator;
    pcm.pos = 0;

    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *) audiobuffer_dma, AUDIO_BUFFER_LENGTH_DMA_GB);

    rg_app_desc_t *app = odroid_system_get_app();

    emu_init();

    pal_set_dmg(odroid_settings_Palette_get());

    // Don't load state if the pause button is held while booting
    uint32_t boot_buttons = GW_GetBootButtons();
    pause_pressed = (boot_buttons & B_PAUSE);
    power_pressed = (boot_buttons & B_POWER);

    if (load_state) {
        LoadState("");
    }
    return app;
}

void app_main_gb(uint8_t load_state)
{
    rg_app_desc_t *app = init(load_state);
    odroid_gamepad_state_t joystick;


    const int frameTime = get_frame_time(60);

    while (true)
    {
        wdog_refresh();

        odroid_input_read_gamepad(&joystick);

        if (joystick.values[ODROID_INPUT_VOLUME]) {

            // TODO: Sync framebuffers in a nicer way
            lcd_sync();

            odroid_dialog_choice_t options[] = {
                {300, "Palette", "7/7", !hw.cgb, &palette_update_cb},
                // {301, "More...", "", 1, &advanced_settings_cb},
                ODROID_DIALOG_CHOICE_LAST
            };
            odroid_overlay_game_menu(options);

        }
        // else if (joystick.values[ODROID_INPUT_VOLUME]) {
        //     odroid_dialog_choice_t options[] = {
        //         {100, "Palette", "7/7", !hw.cgb, &palette_update_cb},
        //         // {101, "More...", "", 1, &advanced_settings_cb},
        //         ODROID_DIALOG_CHOICE_LAST
        //     };
        //     odroid_overlay_game_settings_menu(options);
        // }

        uint startTime = get_elapsed_time();
        bool drawFrame = !skipFrames;

        pad_set(PAD_UP, joystick.values[ODROID_INPUT_UP]);
        pad_set(PAD_RIGHT, joystick.values[ODROID_INPUT_RIGHT]);
        pad_set(PAD_DOWN, joystick.values[ODROID_INPUT_DOWN]);
        pad_set(PAD_LEFT, joystick.values[ODROID_INPUT_LEFT]);
        pad_set(PAD_SELECT, joystick.values[ODROID_INPUT_SELECT]);
        pad_set(PAD_START, joystick.values[ODROID_INPUT_START]);
        pad_set(PAD_A, joystick.values[ODROID_INPUT_A]);
        pad_set(PAD_B, joystick.values[ODROID_INPUT_B]);

        if (power_pressed != joystick.values[ODROID_INPUT_POWER]) {
            printf("Power toggle %ld=>%d\n", power_pressed, !power_pressed);
            power_pressed = joystick.values[ODROID_INPUT_POWER];
            if (power_pressed) {
                printf("Power PRESSED %ld\n", power_pressed);
                HAL_SAI_DMAStop(&hsai_BlockA1);
                if(!joystick.values[ODROID_INPUT_VOLUME]) {
                    // Always save as long as PAUSE is not pressed
                    SaveState("");
                }

                odroid_system_sleep();
            }
        }

        emu_run(drawFrame);

        if (saveSRAM)
        {
            if (ram.sram_dirty)
            {
                saveSRAM_Timer = 120; // wait 2 seconds
                ram.sram_dirty = 0;
            }

            if (saveSRAM_Timer > 0 && --saveSRAM_Timer == 0)
            {
                // TO DO: Try compressing the sram file, it might reduce stuttering
                sram_save();
            }
        }

        if (skipFrames == 0)
        {
            if (get_elapsed_time_since(startTime) > frameTime) skipFrames = 1;
            if (app->speedupEnabled) {
                skipFrames += app->speedupEnabled * 2;
                skippedFrames += app->speedupEnabled * 2;
            }
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }

        // Tick before submitting audio/syncing
        odroid_system_tick(!drawFrame, fullFrame, get_elapsed_time_since(startTime));

        if (!app->speedupEnabled)
        {
            // odroid_audio_submit(pcm.buf, pcm.pos >> 1);
            // handled in pcm_submit instead.
            static dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;
            while (dma_state == last_dma_state) {
                __NOP();
            }
            last_dma_state = dma_state;
        }
    }
}
