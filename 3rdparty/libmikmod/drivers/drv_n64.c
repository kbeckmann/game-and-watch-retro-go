/*	MikMod sound library
	(c) 1998, 1999, 2000 Miodrag Vallat and others - see file AUTHORS for
	complete list.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.
 
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Library General Public License for more details.
 
	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.
*/

/*==============================================================================

  Driver for output on n64 platform
  Originally from https://dragonminded.com/n64dev/ by DragonMinded

  Altered and updated to mikmod 3.3.11 by Victor Vieux <github@vrgl117.games>

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DRV_N64

#include <malloc.h>
#include <audio.h>
#include "mikmod_internals.h"

static SBYTE *N64_buffer = NULL;

static BOOL N64_IsThere(void)
{
	return 1;
}

static int N64_Init(void)
{
	/* Stereo buffer, interleaved */
	N64_buffer = (SBYTE *) MikMod_malloc(sizeof(short) * 2 * audio_get_buffer_length());
	if (!N64_buffer) return 1;
	return VC_Init();
}

static void N64_Exit(void)
{
	MikMod_free(N64_buffer);
	N64_buffer = NULL;
	VC_Exit();
}

static void N64_Update(void)
{
	int i;

	if (!audio_can_write()) return;

	i = audio_get_buffer_length();

	if (md_mode & DMODE_STEREO)
	{
		VC_WriteBytes(N64_buffer, i * 2 * sizeof(short));
	}
	else
	{
		VC_WriteBytes(N64_buffer, i * sizeof(short));
		for (--i; i >= 0; --i)
		{
			/* Convert mono to stereo / interleaved. */
			/* |a|b|c|d|.|.|.|.| -> |a|b|c|d|.|.|d|d|  */
			/* |a|b|c|d|.|.|d|d| -> |a|b|c|d|c|c|d|d|  */
			/* |a|b|c|d|c|c|d|d| -> |a|b|b|b|c|c|d|d|  */
			/* |a|b|b|b|c|c|d|d| -> |a|a|b|b|c|c|d|d|  */
			N64_buffer[(i * 2) + 1] = N64_buffer[i];
			N64_buffer[i * 2] = N64_buffer[i];
		}
	}

	audio_write((short *) N64_buffer);
}

MIKMODAPI MDRIVER drv_n64={
	NULL,
	"Nintendo 64",
	"N64 Driver v1.2",
	255,255,
	"n64",
	NULL,
	NULL,
	N64_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	N64_Init,
	N64_Exit,
	NULL,
	VC_SetNumVoices,
	VC_PlayStart,
	VC_PlayStop,
	N64_Update,
	NULL,
	VC_VoiceSetVolume,
	VC_VoiceGetVolume,
	VC_VoiceSetFrequency,
	VC_VoiceGetFrequency,
	VC_VoiceSetPanning,
	VC_VoiceGetPanning,
	VC_VoicePlay,
	VC_VoiceStop,
	VC_VoiceStopped,
	VC_VoiceGetPosition,
	VC_VoiceRealVolume
};

#else

#include "mikmod_internals.h"
MISSING(drv_n64);

#endif

/* ex:set ts=4: */
