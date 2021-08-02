#include <cstdio>
#include "snes9x.h"

#include "memmap.h"
#include "dma.h"
#include "apu/apu.h"

#include "port.h"
#include "display.h"
#include "controls.h"


////////////

#include <odroid_system.h>

#include <SDL2/SDL.h>

#define WIDTH  SNES_WIDTH
#define HEIGHT SNES_HEIGHT_EXTENDED
#define BPP      4
#define SCALE    4

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *fb_texture;
uint16_t fb_data[WIDTH * HEIGHT * BPP];

uint16_t framebuffer1[320 * 240 * 2];
uint16_t framebuffer2[320 * 240 * 2];

static odroid_gamepad_state_t joystick1;


///////////



// Routines the port has to implement even if it doesn't use them

void S9xPutImage (int, int) {
	SDL_UpdateTexture(fb_texture, NULL, fb_data, WIDTH * BPP);
	SDL_RenderCopy(renderer, fb_texture, NULL, NULL);
	SDL_RenderPresent(renderer);
 }

void S9xInitDisplay (int, char **)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		return;

	window = SDL_CreateWindow("emulator",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		WIDTH * SCALE, HEIGHT * SCALE,
		0);
	if (!window)
		return;

	renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_PRESENTVSYNC);
	if (!renderer)
		return;

	fb_texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING,
		WIDTH, HEIGHT);
	if (!fb_texture)
		return;

	S9xGraphicsInit();
}

void S9xDeinitDisplay (void) { }
void S9xTextMode (void) { }
void S9xGraphicsMode (void) { }
void S9xToggleSoundChannel (int) { }
bool8 S9xOpenSnapshotFile (const char *, bool8, STREAM *) { return 0; }
void S9xCloseSnapshotFile (STREAM) { }
const char * S9xStringInput (const char *) { return "STUB"; }
const char * S9xGetDirectory (enum s9x_getdirtype) { return "STUB"; }
const char * S9xGetFilename (const char *, enum s9x_getdirtype) { return "STUB"; }
const char * S9xGetFilenameInc (const char *, enum s9x_getdirtype) { return "STUB"; }
const char * S9xBasename (const char *) { return "STUB"; }

// Routines the port has to implement if it uses command-line

void S9xExtraUsage (void) { }
void S9xParseArg (char **, int &, int) { }

// Routines the port may implement as needed

void S9xExtraDisplayUsage (void) { }
void S9xParseDisplayArg (char **, int &, int) { }
void S9xSetTitle (const char *) { }
void S9xInitInputDevices (void) { }
void S9xProcessEvents (bool8) {

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
                exit(0);
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

	S9xReportButton(ODROID_INPUT_A, joystick1.values[ODROID_INPUT_A]);
	S9xReportButton(ODROID_INPUT_B, joystick1.values[ODROID_INPUT_B]);
	S9xReportButton(ODROID_INPUT_START, joystick1.values[ODROID_INPUT_START]);
	S9xReportButton(ODROID_INPUT_SELECT, joystick1.values[ODROID_INPUT_SELECT]);
	S9xReportButton(ODROID_INPUT_UP, joystick1.values[ODROID_INPUT_UP]);
	S9xReportButton(ODROID_INPUT_DOWN, joystick1.values[ODROID_INPUT_DOWN]);
	S9xReportButton(ODROID_INPUT_LEFT, joystick1.values[ODROID_INPUT_LEFT]);
	S9xReportButton(ODROID_INPUT_RIGHT, joystick1.values[ODROID_INPUT_RIGHT]);
}

const char * S9xSelectFilename (const char *, const char *, const char *, const char *) { return "STUB"; }

bool8 S9xOpenSoundDevice (void)
{
}

void S9xSyncSpeed (void)
{
}

void S9xHandlePortCommand (s9xcommand_t cmd, int16 data1, int16 data2)
{
}

bool S9xPollButton (uint32 id, bool *pressed)
{
	return 0;
}

bool S9xPollAxis (uint32 id, int16 *value)
{
	return 0;
}

bool S9xPollPointer (uint32 id, int16 *x, int16 *y)
{
	return 0;
}
void S9xMessage (int type, int number, const char *message)
{
	printf("%s\n", message);
}

void S9xAutoSaveSRAM (void)
{
}

bool8 S9xInitUpdate (void)
{
	return (TRUE);
}

bool8 S9xDeinitUpdate (int width, int height)
{
	S9xPutImage(width, height);
	return (TRUE);
}

bool8 S9xContinueUpdate (int width, int height)
{
	return (TRUE);
}

void S9xSetPalette(void)
{
	return;
}

void S9xExit (void)
{
	exit(0);
}



int main(int argc, char *argv[])
{
	// Read rom
	uint8_t *ROM_DATA;
	uint32_t ROM_DATA_LENGTH;

	FILE *fp = fopen(argv[1], "rb");

	if (!fp)
		return -1;

	fseek(fp, 0, SEEK_END);
	ROM_DATA_LENGTH = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	ROM_DATA = (uint8_t *) malloc(ROM_DATA_LENGTH);
	fread(ROM_DATA, 1, ROM_DATA_LENGTH, fp);
	fclose(fp);
	////////////

	printf("\n\nSnes9x " VERSION " for linux\n");

	S9xInitSettings();

	Settings.SixteenBitSound = FALSE;
	Settings.Stereo = FALSE;
	Settings.SoundPlaybackRate = 48000;
	Settings.SoundInputRate = 20000;
	Settings.SoundSync = FALSE;
	Settings.Mute = TRUE;
	Settings.AutoDisplayMessages = TRUE;
	Settings.Transparency = TRUE;
	Settings.SkipFrames = 0;

	if (!Memory.Init(ROM_DATA) || !S9xInitAPU())
	{
		fprintf(stderr, "Snes9x: Memory allocation failure - not enough RAM/virtual memory available.\nExiting...\n");
		Memory.Deinit();
		S9xDeinitAPU();
		exit(1);
	}

	GFX.ScreenSize = SNES_WIDTH * SNES_HEIGHT_EXTENDED;
	GFX.PPL = SNES_WIDTH * 2;
	GFX.Pitch = GFX.PPL * 2;
	GFX.Screen = (uint16*) fb_data;

	S9xGraphicsInit();

	S9xInitDisplay(0, NULL);

	S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
	S9xSetController(1, CTL_NONE, 1, 0, 0, 0);

	#define MAP_BUTTON(id, name) S9xMapButton((id), S9xGetCommandT((name)), false)
	MAP_BUTTON(ODROID_INPUT_A, "Joypad1 A");
	MAP_BUTTON(ODROID_INPUT_B, "Joypad1 Y");
	MAP_BUTTON(ODROID_INPUT_START, "Joypad1 Start");
	MAP_BUTTON(ODROID_INPUT_SELECT, "Joypad1 X"); // Select
	MAP_BUTTON(ODROID_INPUT_LEFT, "Joypad1 Left");
	MAP_BUTTON(ODROID_INPUT_RIGHT, "Joypad1 Right");
	MAP_BUTTON(ODROID_INPUT_UP, "Joypad1 Up");
	MAP_BUTTON(ODROID_INPUT_DOWN, "Joypad1 Down");

	S9xInitSound(0);
	S9xSetSoundMute(FALSE);

	uint32 saved_flags = CPU.Flags;

	if (!Memory.LoadROMMem(ROM_DATA, ROM_DATA_LENGTH)) {
		fprintf(stderr, "Error opening the ROM file.\n");
		exit(1);
	}

	CPU.Flags = saved_flags;
	Settings.StopEmulation = FALSE;

	while (1)
	{
		S9xMainLoop();
		S9xProcessEvents(FALSE);
	}
	return 0;
}