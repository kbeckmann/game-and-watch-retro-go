#include <odroid_system.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <SDL2/SDL.h>

#include <odroid_system.h>

#include "porting.h"
#include "crc32.h"

#include "gw_lcd.h"
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

#define NVS_KEY_SAVE_SRAM "sram"

#define WIDTH  GB_WIDTH
#define HEIGHT GB_HEIGHT
#define BPP      2
#define SCALE    4

#define AUDIO_SAMPLE_RATE   (48000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60)

// Use 60Hz for GB
#define AUDIO_BUFFER_LENGTH_GB (AUDIO_SAMPLE_RATE / 60)
#define AUDIO_BUFFER_LENGTH_DMA_GB ((2 * AUDIO_SAMPLE_RATE) / 60)

static odroid_video_frame_t update1 = {GB_WIDTH, GB_HEIGHT, GB_WIDTH * 2, 2, 0xFF, -1, NULL, NULL, 0, {}};
static odroid_video_frame_t update2 = {GB_WIDTH, GB_HEIGHT, GB_WIDTH * 2, 2, 0xFF, -1, NULL, NULL, 0, {}};
static odroid_video_frame_t *currentUpdate = &update1;

static bool fullFrame = false;
static uint skipFrames = 0;

static bool saveSRAM = false;

// 3 pages
uint8_t state_save_buffer[192 * 1024];

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *fb_texture;
uint16_t fb_data[WIDTH * HEIGHT * BPP];

SDL_AudioSpec wanted;
void fill_audio(void *udata, Uint8 *stream, int len);

extern unsigned char cart_rom[];
extern unsigned int cart_rom_len;

void odroid_display_force_refresh(void)
{
    // forceVideoRefresh = true;
}


static inline void blit(void) {
    // we want 60 Hz for NTSC
    int wantedTime = 1000 / 60;
    SDL_Delay(wantedTime); // rendering takes basically "0ms"

    memcpy(fb_data, currentUpdate->buffer, sizeof(fb_data));

    SDL_UpdateTexture(fb_texture, NULL, fb_data, WIDTH * BPP);
    SDL_RenderCopy(renderer, fb_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void fill_audio(void *udata, Uint8 *stream, int len)
{

}

int init_window(int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return 0;

    window = SDL_CreateWindow("emulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width * SCALE, height * SCALE,
        0);
    if (!window)
        return 0;

    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
        return 0;

    fb_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING,
        width, height);
    if (!fb_texture)
        return 0;


#if 0
    /* Set the audio format */
    wanted.freq = AUDIO_SAMPLE_RATE;
    wanted.format = AUDIO_S16;
    wanted.channels = 1;    /* 1 = mono, 2 = stereo */
    wanted.samples = AUDIO_BUFFER_LENGTH * 2;  /* Good low-latency value for callback */
    wanted.callback = fill_audio;
    wanted.userdata = NULL;

    /* Open the audio device, forcing the desired format */
    if (SDL_OpenAudio(&wanted, NULL) < 0) {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        return(-1);
    }

    SDL_PauseAudio(0);
#endif

    return 0;
}

static void netplay_callback(netplay_event_t event, void *arg)
{
    // Where we're going we don't need netplay!
}

static bool SaveState(char *pathName)
{
    return 0;
}

static bool LoadState(char *pathName)
{
    return true;
}

void pcm_submit(void)
{

}

void init(void)
{
    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, &netplay_callback);

    // Hack: Use the same buffer twice
    update1.buffer = fb_data;
    update2.buffer = fb_data;

    //saveSRAM = odroid_settings_app_int32_get(NVS_KEY_SAVE_SRAM, 0);
    saveSRAM = false;

    // Load ROM
    loader_init(NULL);

    // RTC
    memset(&rtc, 0, sizeof(rtc));

    // Video
    memset(fb_data, 0, sizeof(fb_data));
    memset(&fb, 0, sizeof(fb));
    fb.w = GB_WIDTH;
    fb.h = GB_HEIGHT;
    fb.format = GB_PIXEL_565_LE;
    fb.pitch = update1.stride;
    fb.ptr = currentUpdate->buffer;
    fb.enabled = 1;
    fb.blit_func = &blit;

    emu_init();

    //pal_set_dmg(odroid_settings_Palette_get());
    pal_set_dmg(2);
}


int main(int argc, char *argv[])
{
    init_window(WIDTH, HEIGHT);

    init();
    odroid_gamepad_state_t joystick;

    while (true)
    {
        odroid_input_read_gamepad(&joystick);

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

        emu_run(drawFrame);

        // Tick before submitting audio/syncing
        odroid_system_tick(!drawFrame, fullFrame, get_elapsed_time_since(startTime));
    }

    SDL_Quit();

    return 0;
}
