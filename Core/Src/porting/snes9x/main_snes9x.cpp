#pragma GCC optimize("O0")

extern "C" {

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gw_linker.h"
#include "gw_lcd.h"

#include "odroid_system.h"

#include "rom_manager.h"

#include "heap.hpp"

extern void __libc_init_array(void);

}

#include <cstdio>
#include <cstddef>

#include "snes9x.h"

#include "memmap.h"
#include "dma.h"
#include "apu/apu.h"

#include "port.h"
#include "display.h"
#include "controls.h"

void S9xPutImage (int, int) {

  static uint32_t lastFPSTime = 0;
  static uint32_t frames = 0;

  uint32_t currentTime = HAL_GetTick();
  uint32_t delta = currentTime - lastFPSTime;
  uint16_t* curr_framebuffer = NULL;

  frames++;

  if (delta >= 1000) {
      int fps = (10000 * frames) / delta;
      printf("FPS: %d.%d, frames %ld, delta %ld ms\n", fps / 10, fps % 10, frames, delta);
      frames = 0;
      lastFPSTime = currentTime;
  }

}


const char *S9xChooseFilename(bool8 read_only)
{
	// Return a saved state
	return NULL;
}

void S9xSetPalette(void)
{
	return;
}

void S9xInitDisplay (int, char **)
{
}

void S9xDeinitDisplay (void) { }
void S9xTextMode (void) { }
void S9xGraphicsMode (void) { }
void S9xToggleSoundChannel (int) { }
bool8 S9xOpenSnapshotFile (const char *, bool8, STREAM *) { return 0; }
void S9xCloseSnapshotFile (STREAM) { }
const char * S9xStringInput (const char *) { return ""; }
const char * S9xGetDirectory (enum s9x_getdirtype) { return ""; }
const char * S9xGetFilename (const char *, enum s9x_getdirtype) { return ""; }
const char * S9xGetFilenameInc (const char *, enum s9x_getdirtype) { return ""; }
const char * S9xBasename (const char *) { return ""; }

// Routines the port has to implement if it uses command-line

void S9xExtraUsage (void) { }
void S9xParseArg (char **, int &, int) { }

// Routines the port may implement as needed

void S9xExtraDisplayUsage (void) { }
void S9xParseDisplayArg (char **, int &, int) { }
void S9xSetTitle (const char *) { }
void S9xInitInputDevices (void) { }

void S9xProcessEvents (bool8)
{
	odroid_gamepad_state_t joystick1;
	odroid_input_read_gamepad(&joystick1);

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

void S9xExit (void)
{
	exit(0);
}

static int frame_count;

void S9xSyncSpeed (void)
{
	static uint32_t lastSyncTime = 0;
	static uint32_t sync_count = 0;

	uint32_t currentTime = HAL_GetTick();
	uint32_t delta = currentTime - lastSyncTime;
	uint16_t* curr_framebuffer = NULL;

	sync_count++;

	if (delta >= 1000) {
		int syncsPerSecond = (10000 * sync_count) / delta;
		printf("sync/s: %d.%d, count %ld, delta %ld ms\n", syncsPerSecond / 10, syncsPerSecond % 10, sync_count, delta);
		sync_count = 0;
		lastSyncTime = currentTime;
	}

	IPPU.RenderThisFrame = sync_count % Settings.SkipFrames == 0;
}

void app_main_snes_cpp(uint8_t load_state, uint8_t start_paused)
{
	printf("Snes9x Load %s", ACTIVE_FILE->name);

	memset(framebuffer1, 0, sizeof(framebuffer1));
	// sets framebuffer1 as active buffer
	lcd_reset_active_buffer();

	S9xInitSettings();

	Settings.SixteenBitSound = FALSE;
	Settings.Stereo = FALSE;
	Settings.SoundPlaybackRate = 48000;
	Settings.SoundInputRate = 20000;
	Settings.SoundSync = FALSE;
	Settings.Mute = TRUE;
	Settings.AutoDisplayMessages = TRUE;
	Settings.Transparency = TRUE;
	Settings.SkipFrames = 2;

	if (!Memory.Init((uint8_t *) ROM_DATA) || !S9xInitAPU())
	{
		fprintf(stderr, "Snes9x: Memory allocation failure - not enough RAM/virtual memory available.\nExiting...\n");
		Memory.Deinit();
		S9xDeinitAPU();
		exit(1);
	}

	GFX.ScreenSize = SNES_WIDTH * SNES_HEIGHT_EXTENDED;
	GFX.PPL = SNES_WIDTH * 2;
	// GFX.Pitch = GFX.PPL * 2;
	GFX.Pitch = 320 * 2;
	GFX.Screen = (uint16*) framebuffer1;

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
}

extern "C" void app_main_snes(uint8_t load_state, uint8_t start_paused)
{
	// Initializes the heap used by new and new[]
	cpp_heap_init();

	// Call static c++ constructors now, *after* OSPI and other memory is copied
	__libc_init_array();

	app_main_snes_cpp(load_state, start_paused);
}