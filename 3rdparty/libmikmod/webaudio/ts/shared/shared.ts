/*	MikMod Web Audio library
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

interface LibMikModGeneralOptions {
	reverb?: number;
	interpolation?: boolean;
	noiseReduction?: boolean;
}

interface LibMikModInitialOptions extends LibMikModGeneralOptions {
	hqMixer?: boolean;
	wrap?: boolean;
	loop?: boolean;
	fadeout?: boolean;
}

interface LibMikModLoadOptions extends LibMikModInitialOptions {
	audioContext: AudioContext;
	source: File | ArrayBuffer | Uint8Array;
	onload: (audioNode: AudioWorkletNode) => void;
	onerror: (errorCode: number, reason?: any) => void;
	onended: () => void;
}

enum LibMikModMessageId {
	INIT = 1,
	LOAD_MODULE_BUFFER = 2,
	CHANGE_GENERAL_OPTIONS = 3,
	STOP_MODULE = 4,
	PLAYBACK_ERROR = 5,
	PLAYBACK_ENDED = 6
}

interface LibMikModMessage {
	id: number;
	messageId: LibMikModMessageId;
	options?: LibMikModGeneralOptions | LibMikModInitialOptions | null;
	buffer?: ArrayBuffer;
}

interface LibMikModResponse {
	id: number;
	messageId: LibMikModMessageId;
	errorCode?: number;
	errorStr?: string | null;
	infoSongName?: string | null;
	infoModType?: string | null;
	infoComment?: string | null;
}
