/*	LibMikMod Web Audio library
	(c) 2021 Carlos Rafael Gimenes das Neves.

	https://github.com/sezero/mikmod
	https://github.com/carlosrafaelgn/mikmod/tree/master/libmikmod/webaudio

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

#include "mikmod_internals.h"

#define BUFFERSIZE 32768

static SBYTE* audioBuffer = NULL;
int audioBufferUsedLength = 0;

SBYTE* getAudioBuffer() {
	return audioBuffer;
}

int getAudioBufferMaxLength() {
	return BUFFERSIZE;
}

int getAudioBufferUsedLength() {
	return audioBufferUsedLength;
}

static BOOL WebAudio_IsThere() {
	return 1;
}

static int WebAudio_Init() {
	md_mode |= DMODE_SOFT_MUSIC | DMODE_SOFT_SNDFX;

	if (!(audioBuffer = (SBYTE*)MikMod_malloc(BUFFERSIZE)))
		return 1;

	if (VC_Init()) {
		MikMod_free(audioBuffer);
		audioBuffer = 0;
		return 1;
	}

	return 0;
}

static void WebAudio_Exit() {
	VC_Exit();

	if (audioBuffer) {
		MikMod_free(audioBuffer);
		audioBuffer = 0;
	}
}

static void WebAudio_Update() {
	audioBufferUsedLength = (audioBuffer ? VC_WriteBytes(audioBuffer, BUFFERSIZE) : 0);
}

static int WebAudio_Reset() {
	audioBufferUsedLength = 0;
	return 0;
}

MDRIVER drv_webaudio = {
	NULL, // next
	"Web Audio",
	"Web Audio driver v0.1",
	0, 255,
	"webaudio",
	NULL, // CmdLineHelp
	NULL, // CommandLine
	WebAudio_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	WebAudio_Init,
	WebAudio_Exit,
	WebAudio_Reset,
	VC_SetNumVoices,
	VC_PlayStart,
	VC_PlayStop,
	WebAudio_Update,
	0, // Pause
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
