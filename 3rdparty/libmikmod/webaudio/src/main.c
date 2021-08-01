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

#include <string.h>
#include "mikmod.h"

#define MAX_MODULE_LENGTH (10 * 1024 * 1024)

extern struct MDRIVER drv_webaudio;
extern int audioBufferUsedLength;

MODULE* libMikModModule = NULL;
char* libMikModTempData = NULL;
int libMikModTempDataLength = 0;
int libMikModInitialized = 0;

int getVersion() {
	return LIBMIKMOD_VERSION;
}

int init() {
	if (libMikModInitialized)
		return 0;

	MikMod_errno = 0;

	MikMod_InitThreads();

	MikMod_RegisterDriver(&drv_webaudio);

	MikMod_RegisterAllLoaders();

	md_mode =
		// These ones take effect only after MikMod_Init or MikMod_Reset
		//DMODE_16BITS | // enable 16 bit output
		//DMODE_STEREO | // enable stereo output
		DMODE_SOFT_SNDFX | // Process sound effects via software mixer
		DMODE_SOFT_MUSIC | // Process music via software mixer
		DMODE_HQMIXER | // Use high-quality (slower) software mixer
		DMODE_FLOAT | // enable float output

		// These take effect immediately.
		//DMODE_SURROUND | // enable surround sound
		DMODE_INTERP | // enable interpolation
		//DMODE_REVERSE | // reverse stereo
		//DMODE_SIMDMIXER | // enable SIMD mixing
		DMODE_NOISEREDUCTION // Low pass filtering
	;

	if (MikMod_Init(NULL))
		return MikMod_errno;

	libMikModInitialized = 1;

	return 0;
}

void freeModule() {
	if (libMikModModule) {
		Player_Stop();
		Player_Free(libMikModModule);
		libMikModModule = NULL;
	}
}

void terminate() {
	if (libMikModTempData) {
		MikMod_free(libMikModTempData);
		libMikModTempData = NULL;
		libMikModTempDataLength = 0;
	}

	freeModule();

	if (libMikModInitialized) {
		MikMod_Exit();
		libMikModInitialized = 0;
	}
}

char* preLoadModule(int length) {
	freeModule();

	if (libMikModTempData) {
		MikMod_free(libMikModTempData);
		libMikModTempData = NULL;
		libMikModTempDataLength = 0;
	}

	if (length > MAX_MODULE_LENGTH)
		return 0;

	libMikModTempData = MikMod_malloc(length);
	if (libMikModTempData)
		libMikModTempDataLength = length;

	return libMikModTempData;
}

int loadModule(int mixfreq, int reverb, BOOL hqMixer, BOOL interpolation, BOOL noiseReduction, BOOL wrap, BOOL loop, BOOL fadeout) {
	freeModule();

	if (!libMikModTempData || !libMikModTempDataLength)
		return 0;

	md_mixfreq = mixfreq;

	md_mode = (md_mode & ~(DMODE_HQMIXER | DMODE_INTERP | DMODE_NOISEREDUCTION)) |
		// These ones take effect only after MikMod_Init or MikMod_Reset
		(hqMixer ? DMODE_HQMIXER : 0) | // Use high-quality (slower) software mixer
		// These take effect immediately.
		(interpolation ? DMODE_INTERP : 0) | // enable interpolation
		(noiseReduction ? DMODE_NOISEREDUCTION : 0) // Low pass filtering
	;

	md_volume = 128; // global sound volume (0-128)
	md_musicvolume = 128; // volume of song
	md_sndfxvolume = 128; // volume of sound effects
	md_reverb = (reverb <= 0 ? 0 : (reverb >= 15 ? 15 : reverb)); // 0 = none;  15 = chaos
	md_pansep = 0; // 0 = mono;  128 == 100% (full left/right)

	MikMod_errno = 0;

	if (MikMod_Reset(NULL))
		return MikMod_errno;

	libMikModModule = Player_LoadMem(libMikModTempData, libMikModTempDataLength, 64, 0);
	MikMod_free(libMikModTempData);
	libMikModTempData = NULL;
	libMikModTempDataLength = 0;

	if (!libMikModModule)
		return MikMod_errno;

	libMikModModule->wrap = wrap;
	libMikModModule->loop = loop;
	libMikModModule->fadeout = fadeout;
	Player_Start(libMikModModule);

	return 0;
}

void changeGeneralOptions(int reverb, BOOL interpolation, BOOL noiseReduction) {
	md_mode = (md_mode & ~(DMODE_INTERP | DMODE_NOISEREDUCTION)) |
		// These take effect immediately.
		(interpolation ? DMODE_INTERP : 0) | // enable interpolation
		(noiseReduction ? DMODE_NOISEREDUCTION : 0) // Low pass filtering
	;

	md_reverb = (reverb <= 0 ? 0 : (reverb >= 15 ? 15 : reverb)); // 0 = none;  15 = chaos
}

int update() {
	if (Player_Active()) {
		MikMod_Update();
		return (MikMod_errno ? -1 : audioBufferUsedLength);
	}
	return -1;
}

int getErrno() {
	return MikMod_errno;
}

const CHAR* getStrerr(int code) {
	return MikMod_strerror(code);
}

const CHAR* getSongName() {
	return (libMikModModule ? libMikModModule->songname : NULL);
}

const CHAR* getModType() {
	return (libMikModModule ? libMikModModule->modtype : NULL);
}

const CHAR* getComment() {
	return (libMikModModule ? libMikModModule->comment : NULL);
}
