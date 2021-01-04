#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include <odroid_system.h>

#include "porting.h"
#include "crc32.h"

#include <string.h>
#include <nofrendo.h>
#include <bitmap.h>
#include <nes.h>
#include <nes_input.h>
#include <nes_state.h>
#include <nes_input.h>
#include <osd.h>


#define WIDTH  320
#define HEIGHT 240
#define BPP      4
#define SCALE    4


#define APP_ID 30

#define AUDIO_SAMPLE_RATE   (48000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60)

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *fb_texture;
uint16_t fb_data[WIDTH * HEIGHT * BPP];

SDL_AudioSpec wanted;
void fill_audio(void *udata, Uint8 *stream, int len);

extern unsigned char cart_rom[];
extern unsigned int cart_rom_len;
static uint romCRC32;

static uint16_t myPalette[64];

static int16_t audioBufferA[AUDIO_BUFFER_LENGTH * 2];
static int16_t audioBufferB[AUDIO_BUFFER_LENGTH * 2];
static int16_t pendingSamples = 0;

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

void osd_setpalette(rgb_t *pal)
{
   for (int i = 0; i < 64; i++)
   {
      uint16_t c = (pal[i].b>>3) | ((pal[i].g>>2)<<5) | ((pal[i].r>>3)<<11);
    //   myPalette[i] = (c>>8) | ((c&0xff)<<8);
      myPalette[i] = c;
   }
   odroid_display_force_refresh();
}


void osd_wait_for_vsync()
{
}


void osd_audioframe(int audioSamples)
{
    // printf("%d\n", audioSamples);
    if (odroid_system_get_app()->speedupEnabled)
        return;

    apu_process(audioBufferA, audioSamples); //get audio data

    //16 bit mono -> 32-bit (16 bit r+l)
    // for (int i = audioSamples - 1; i >= 0; --i)
    // {
    //     int16_t sample = audioBufferA[i];
    //     audioBufferA[i*2] = sample;
    //     audioBufferA[i*2+1] = sample;
    // }

    pendingSamples = audioSamples;
}


void osd_blitscreen(bitmap_t *bmp)
{
    static uint32_t lastFPSTime = 0;
    static uint32_t lastTime = 0;
    static uint32_t frames = 0;

    frames++;
    uint32_t currentTime = SDL_GetTicks();
    float delta = currentTime - lastFPSTime;
    if (delta >= 1000) {
        printf("FPS: %f\n", ((float)frames / (delta / 1000.0f)));
        frames = 0;
        lastFPSTime = currentTime;
    }

    // we want 60 Hz for NTSC
    int wantedTime = 1000 / 60;
    SDL_Delay(wantedTime); // rendering takes basically "0ms"
    lastTime = currentTime;

    // LCD is 320 wide, framebuffer is only 256
    const int hpad = (WIDTH - NES_SCREEN_WIDTH) / 2;

    // printf("%d x %d\n", bmp->width, bmp->height);
    for (int line = 0; line < bmp->height; line++) {
        uint8_t *row = bmp->line[line];
        for (int x = 0; x < bmp->width; x++) {

            // This doesn't look good, but why? There is a palette and the
            // data seems to be stored with LUT indexes
            
            // this will read oob
            // fb_data[(2*line    ) * WIDTH + x + hpad] = myPalette[row[x]];
            // fb_data[(2*line + 1) * WIDTH + x + hpad] = myPalette[row[x]];

            fb_data[(2*line    ) * WIDTH + x + hpad] = myPalette[row[x] & 0b111111];
            fb_data[(2*line + 1) * WIDTH + x + hpad] = myPalette[row[x] & 0b111111];

            // fb_data[(2*line) * WIDTH + x] = row[x];
            // fb_data[(2*line + 1) * WIDTH + x] = row[x];
        }
    }

    SDL_UpdateTexture(fb_texture, NULL, fb_data, WIDTH * BPP);
    SDL_RenderCopy(renderer, fb_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void osd_getinput(void)
{
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            // printf("Press %d\n", event.key.keysym.sym);
            switch (event.key.keysym.sym) {
            case SDLK_x:
                joystick1.values[ODROID_INPUT_A] = 1;
                break;
            case SDLK_z:
                joystick1.values[ODROID_INPUT_B] = 1;
                break;
            case SDLK_LSHIFT:
                joystick1.values[ODROID_INPUT_START] = 1;
                break;
            case SDLK_LCTRL:
                joystick1.values[ODROID_INPUT_SELECT] = 1;
                break;
            case SDLK_UP:
                joystick1.values[ODROID_INPUT_UP] = 1;
                break;
            case SDLK_DOWN:
                joystick1.values[ODROID_INPUT_DOWN] = 1;
                break;
            case SDLK_LEFT:
                joystick1.values[ODROID_INPUT_LEFT] = 1;
                break;
            case SDLK_RIGHT:
                joystick1.values[ODROID_INPUT_RIGHT] = 1;
                break;
            case SDLK_ESCAPE:
                nes_getptr()->poweroff = 1;
                break;
            default:
                break;
            }
        } else if (event.type == SDL_KEYUP) {
            // printf("Release %d\n", event.key.keysym.sym);
            switch (event.key.keysym.sym) {
            case SDLK_x:
                joystick1.values[ODROID_INPUT_A] = 0;
                break;
            case SDLK_z:
                joystick1.values[ODROID_INPUT_B] = 0;
                break;
            case SDLK_LSHIFT:
                joystick1.values[ODROID_INPUT_START] = 0;
                break;
            case SDLK_LCTRL:
                joystick1.values[ODROID_INPUT_SELECT] = 0;
                break;
            case SDLK_UP:
                joystick1.values[ODROID_INPUT_UP] = 0;
                break;
            case SDLK_DOWN:
                joystick1.values[ODROID_INPUT_DOWN] = 0;
                break;
            case SDLK_LEFT:
                joystick1.values[ODROID_INPUT_LEFT] = 0;
                break;
            case SDLK_RIGHT:
                joystick1.values[ODROID_INPUT_RIGHT] = 0;
                break;
            default:
                break;
            }
        }
    }

    uint16 pad0 = 0, pad1 = 0;

    if (joystick1.values[ODROID_INPUT_START])  pad0 |= INP_PAD_START;
    if (joystick1.values[ODROID_INPUT_SELECT]) pad0 |= INP_PAD_SELECT;
    if (joystick1.values[ODROID_INPUT_UP])     pad0 |= INP_PAD_UP;
    if (joystick1.values[ODROID_INPUT_RIGHT])  pad0 |= INP_PAD_RIGHT;
    if (joystick1.values[ODROID_INPUT_DOWN])   pad0 |= INP_PAD_DOWN;
    if (joystick1.values[ODROID_INPUT_LEFT])   pad0 |= INP_PAD_LEFT;
    if (joystick1.values[ODROID_INPUT_A])      pad0 |= INP_PAD_A;
    if (joystick1.values[ODROID_INPUT_B])      pad0 |= INP_PAD_B;

    static old_pad0;
    if (pad0 != old_pad0) {
        printf("pad0=%02x\n", pad0);
        old_pad0 = pad0;
    }
    input_update(INP_JOYPAD0, pad0);
}


size_t osd_getromdata(unsigned char **data)
{
    *data = (unsigned char*)cart_rom;
   return cart_rom_len;
}

uint osd_getromcrc()
{
   return romCRC32;
}

void osd_loadstate()
{
}

static bool SaveState(char *pathName)
{
    return true;
}

static bool LoadState(char *pathName)
{
    return true;
}

void fill_audio(void *udata, Uint8 *stream, int len)
{
    // TODO!
    // memcpy(audioBufferB, audioBufferA, len);
    // SDL_MixAudio(stream, (Uint8*)audioBufferB, len, 100);
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
}


int main(int argc, char *argv[])
{
    init_window(WIDTH, HEIGHT);

    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, NULL);

    printf("app_main ROM: cart_rom_len=%ld\n", cart_rom_len);

    romCRC32 = crc32_le(0, (const uint8_t*)(cart_rom + 16), cart_rom_len - 16);

    int region = NES_NTSC;
    // region = NES_PAL;

    printf("Nofrendo start!\n");

    // nofrendo_start("Rom name (E).nes", NES_PAL, AUDIO_SAMPLE_RATE);
    nofrendo_start("Rom name (USA).nes", NES_NTSC, AUDIO_SAMPLE_RATE);

    SDL_Quit();

    return 0;
}












