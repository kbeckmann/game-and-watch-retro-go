#include <odroid_system.h>
#include <string.h>

#include "gw_lcd.h"
#include "gw_linker.h"
#include "gnuboy/loader.h"
#include "gnuboy/hw.h"
#include "gnuboy/lcd.h"
#include "gnuboy/cpu.h"
#include "gnuboy/mem.h"
#include "gnuboy/sound.h"
#include "gnuboy/regs.h"
#include "gnuboy/rtc.h"
#include "gnuboy/defs.h"

#define APP_ID 20

#define AUDIO_SAMPLE_RATE   (48000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60)

typedef enum {
    DMA_TRANSFER_STATE_HF = 0x00,
    DMA_TRANSFER_STATE_TC = 0x01,
} dma_transfer_state_t;

#define NVS_KEY_SAVE_SRAM "sram"


static uint32_t pause_pressed;
static uint32_t power_pressed;

static dma_transfer_state_t dma_state;
static uint32_t audio_mute;

static int16_t pendingSamples = 0;
static int16_t audiobuffer_emulator[AUDIO_BUFFER_LENGTH * 2 * 2 * 2] __attribute__((section (".audio")));
static int16_t audiobuffer_dma[AUDIO_BUFFER_LENGTH * 2] __attribute__((section (".audio")));

extern SAI_HandleTypeDef hsai_BlockA1;
extern DMA_HandleTypeDef hdma_sai1_a;


static odroid_video_frame_t update1 = {GB_WIDTH, GB_HEIGHT, GB_WIDTH * 2, 2, 0xFF, -1, NULL, NULL, 0, {}};
static odroid_video_frame_t update2 = {GB_WIDTH, GB_HEIGHT, GB_WIDTH * 2, 2, 0xFF, -1, NULL, NULL, 0, {}};
static odroid_video_frame_t *currentUpdate = &update1;

static bool fullFrame = false;
static uint skipFrames = 0;

static bool netplay = false;

static bool saveSRAM = false;
static int  saveSRAM_Timer = 0;
// --- MAIN


static void netplay_callback(netplay_event_t event, void *arg)
{
    // Where we're going we don't need netplay!
}

#define WIDTH 320

// int[] resizePixels(int[] pixels,int w1,int h1,int w2,int h2) {
//     int[] temp = new int[w2*h2] ;
//     // EDIT: added +1 to account for an early rounding problem
//     int x_ratio = (int)((w1<<16)/w2) +1;
//     int y_ratio = (int)((h1<<16)/h2) +1;
//     //int x_ratio = (int)((w1<<16)/w2) ;
//     //int y_ratio = (int)((h1<<16)/h2) ;
//     int x2, y2 ;
//     for (int i=0;i<h2;i++) {
//         for (int j=0;j<w2;j++) {
//             x2 = ((j*x_ratio)>>16) ;
//             y2 = ((i*y_ratio)>>16) ;
//             temp[(i*w2)+j] = pixels[(y2*w1)+x2] ;
//         }                
//     }                
//     return temp ;
// }

static uint32_t skippedFrames = 0;

__attribute__((optimize("unroll-loops")))
static inline void screen_blit(void) {
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
    for (int i=0;i<h2;i++) {
        for (int j=0;j<w2;j++) {
            x2 = ((j*x_ratio)>>16) ;
            y2 = ((i*y_ratio)>>16) ;
            uint16_t b2 = screen_buf[(y2*w1)+x2];
            framebuffer1[(i*WIDTH)+j+hpad] = b2;
            // temp[(i*w2)+j] = pixels[(y2*w1)+x2] ;
        }
    }

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


static bool SaveState(char *pathName)
{
    // For convenience we also write the sram to its own file
    // So that it can be imported in other emulators
    // No save ;_;
    return 0;
    // sram_save();

    // return state_save(pathName) == 0;
}

static bool LoadState(char *pathName)
{
    return true;
    /*if (state_load(pathName) != 0)
    {
        emu_reset();

        if (saveSRAM) sram_load();

        return false;
    }
    return true;*/
}


static bool palette_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    /* what?! */
    /*int pal = pal_get_dmg();
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
        emu_run(true);
    }

    if (pal == 0) strcpy(option->value, "GBC");
    else sprintf(option->value, "%d/%d", pal, max);

    return event == ODROID_DIALOG_ENTER;*/
    return false;
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

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    dma_state = DMA_TRANSFER_STATE_HF;
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    dma_state = DMA_TRANSFER_STATE_TC;
}

// Hacky but it works: Locate the framebuffer in ITCRAM
uint8_t gb_buffer1[GB_WIDTH * GB_HEIGHT * 2]  __attribute__((section (".itcram_data")));

// 3 pages
uint8_t state_save_buffer[192 * 1024] __attribute__((section (".emulator_data")));

void app_main(void)
{
    odroid_gamepad_state_t joystick;

    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, &netplay_callback);

    // Hack: Use the same buffer twice
    update1.buffer = gb_buffer1;
    update2.buffer = gb_buffer1;

    //saveSRAM = odroid_settings_app_int32_get(NVS_KEY_SAVE_SRAM, 0);
    saveSRAM = false;

    // Load ROM
    loader_init(NULL);

    // RTC
    memset(&rtc, 0, sizeof(rtc));

    // Video
    memset(framebuffer1, 0, sizeof(framebuffer1));
    memset(&fb, 0, sizeof(fb));
    fb.w = GB_WIDTH;
  	fb.h = GB_HEIGHT;
  	fb.pixelsize = 2;
  	fb.pitch = fb.w * fb.pixelsize;
  	fb.ptr = currentUpdate->buffer;
  	fb.enabled = 1;
    fb.byteorder = 1;
    fb.blit_func = &screen_blit;

    // Audio
    memset(audiobuffer_emulator, 0, sizeof(audiobuffer_emulator));
    memset(&pcm, 0, sizeof(pcm));
    pcm.hz = AUDIO_SAMPLE_RATE;
  	pcm.stereo = 0;
  	pcm.len = AUDIO_BUFFER_LENGTH * 2;
  	pcm.buf = (n16*)&audiobuffer_emulator;
  	pcm.pos = 0;

    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, audiobuffer_dma, sizeof(audiobuffer_dma) / sizeof(audiobuffer_dma[0]));

    rg_app_desc_t *app = odroid_system_get_app();

    emu_init();

    //pal_set_dmg(odroid_settings_Palette_get());
    pal_set_dmg(2);

    /*if (app->startAction == ODROID_START_ACTION_RESUME)
    {
        odroid_system_emu_load_state(0);
    }
    else if (saveSRAM)
    {
        sram_load();
    }*/

    // Don't load state if the pause button is held while booting
    odroid_input_read_gamepad(&joystick);
    if (!joystick.values[ODROID_INPUT_VOLUME]) {
        state_load(&__SAVE_START__, &__SAVE_END__ - &__SAVE_START__);
    }

    const int frameTime = get_frame_time(60);

    while (true)
    {
        odroid_input_read_gamepad(&joystick);

        /*if (joystick.values[ODROID_INPUT_MENU]) {
            odroid_overlay_game_menu();
        }
        else if (joystick.values[ODROID_INPUT_VOLUME]) {
            odroid_dialog_choice_t options[] = {
                {100, "Palette", "7/7", !hw.cgb, &palette_update_cb},
                {101, "More...", "", 1, &advanced_settings_cb},
                ODROID_DIALOG_CHOICE_LAST
            };
            odroid_overlay_game_settings_menu(options);
        }*/

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

        if (pause_pressed != joystick.values[ODROID_INPUT_VOLUME]) {
            pause_pressed = joystick.values[ODROID_INPUT_VOLUME];
            if (pause_pressed) {
                printf("Pause pressed %d=>%d\n", audio_mute, !audio_mute);
                audio_mute = !audio_mute;
            }
        }

        if (power_pressed != joystick.values[ODROID_INPUT_POWER]) {
            printf("Power toggle %d=>%d\n", power_pressed, !power_pressed);
            power_pressed = joystick.values[ODROID_INPUT_POWER];
            if (power_pressed) {
                printf("Power PRESSED %d\n", power_pressed);
                HAL_SAI_DMAStop(&hsai_BlockA1);
                lcd_backlight_off();

                if(!joystick.values[ODROID_INPUT_VOLUME]) {
                    // Always save as long as PAUSE is not pressed
                    memset(state_save_buffer, '\x00', sizeof(state_save_buffer));
                    state_save(state_save_buffer, sizeof(state_save_buffer));
                    store_save(state_save_buffer, sizeof(state_save_buffer));
                }

                GW_EnterDeepSleep();
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

            size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : AUDIO_BUFFER_LENGTH;

            // Write to DMA buffer and lower the volume to 1/4
            // printf("pcm.pos pos=%d\n", pcm.pos);
            if (audio_mute) { 
                for (int i = 0; i < AUDIO_BUFFER_LENGTH; i++) {
                    audiobuffer_dma[i + offset] = 0;
                }
            } else {
                for (int i = 0; i < AUDIO_BUFFER_LENGTH; i++) {
                    audiobuffer_dma[i + offset] = pcm.buf[i] >> 1;
                }
            }

            // Wait until the audio buffer has been transmitted
            static dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;
            while (dma_state == last_dma_state) {
                __NOP();
            }
            last_dma_state = dma_state;

        }
    }
}
