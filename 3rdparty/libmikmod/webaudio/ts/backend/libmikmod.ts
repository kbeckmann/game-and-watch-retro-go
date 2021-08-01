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

interface LibMikModCLib {
	HEAP8: Uint8Array;
	HEAPF32: Float32Array;

	_getVersion(): number;
	_init(): number;
	_freeModule(): void;
	_terminate(): void;
	_preLoadModule(length: number): number;
	_loadModule(mixfreq: number, reverb: number, hqMixer: number, interpolation: number, noiseReduction: number, wrap: number, loop: number, fadeout: number): number;
	_changeGeneralOptions(reverb: number, interpolation: number, noiseReduction: number): void;
	_update(): number;
	_getErrno(): number;
	_getStrerr(code: number): number;
	_getSongName(): number;
	_getModType(): number;
	_getComment(): number;
	_getAudioBuffer(): number;
	_getAudioBufferMaxLength(): number;
	_getAudioBufferUsedLength(): number;
}

class LibMikMod {
	private static cLib: LibMikModCLib | null = null;
	private static tmpBuffer: Float32Array | null = null;

	private static audioBufferPtr = 0;
	private static audioBufferUsedLength = 0;

	private static lastReverb = 0;
	private static lastHqMixer = 1;
	private static lastInterpolation = 1;
	private static lastNoiseReduction = 1;
	private static lastWrap = 0;
	private static lastLoop = 0;
	private static lastFadeout = 1;

	public static loading = false;
	public static loaded = false;
	public static loadErrorStr: string | null = null;

	public static init(wasmBinary: ArrayBuffer): Promise<void> {
		return (LibMikMod.loaded ? Promise.resolve() : new Promise((resolve, reject) => {
			if (LibMikMod.loading) {
				reject(LibMikMod.loadErrorStr = "The library was still loading");
				return;
			}

			if (LibMikMod.loadErrorStr) {
				reject(LibMikMod.loadErrorStr);
				return;
			}

			LibMikMod.loading = true;

			LibMikModCLib({ wasmBinary }).then((value) => {
				LibMikMod.cLib = value;

				const r = value._init();

				if (!r) {
					LibMikMod.loading = false;
					LibMikMod.loaded = true;
					LibMikMod.loadErrorStr = null;
					resolve();
				} else {
					LibMikMod.loading = false;
					reject(LibMikMod.loadErrorStr = LibMikMod.getStrerr(r));
				}
			}, (reason) => {
				LibMikMod.loading = false;
				reject(LibMikMod.loadErrorStr = ((reason ? (reason.message || reason.toString()) : null) || "Unknown error while loading the library"));
			});
		}));
	}

	public static terminate(): void {
		if (LibMikMod.cLib) {
			LibMikMod.cLib._terminate();
			LibMikMod.cLib = null;

			LibMikMod.audioBufferPtr = 0;
			LibMikMod.audioBufferUsedLength = 0;
		}
	}

	public static getVersion(): number {
		return (LibMikMod.cLib ? LibMikMod.cLib._getVersion() : 0);
	}

	public static loadModule(destinationSampleRate: number, srcBuffer?: ArrayBuffer | Uint8Array, options?: LibMikModInitialOptions | null): number {
		if (!LibMikMod.cLib)
			return 3; // MMERR_DYNAMIC_LINKING

		if (!srcBuffer)
			return 2; // MMERR_OUT_OF_MEMORY

		const ptr = LibMikMod.cLib._preLoadModule(srcBuffer.byteLength);
		if (!ptr)
			return 2; // MMERR_OUT_OF_MEMORY

		const dstBuffer = new Uint8Array(LibMikMod.cLib.HEAP8.buffer, ptr, srcBuffer.byteLength);

		if ("set" in srcBuffer)
			dstBuffer.set(srcBuffer, 0);
		else
			dstBuffer.set(new Uint8Array(srcBuffer, 0, srcBuffer.byteLength), 0);

		LibMikMod.audioBufferPtr = 0;
		LibMikMod.audioBufferUsedLength = 0;

		if (options) {
			if (options.reverb !== undefined && options.reverb >= 0 && options.reverb <= 15)
				LibMikMod.lastReverb = options.reverb;

			if (options.hqMixer !== undefined)
				LibMikMod.lastHqMixer = (options.hqMixer ? 1 : 0);

			if (options.interpolation !== undefined)
				LibMikMod.lastInterpolation = (options.interpolation ? 1 : 0);

			if (options.noiseReduction !== undefined)
				LibMikMod.lastNoiseReduction = (options.noiseReduction ? 1 : 0);

			if (options.wrap !== undefined)
				LibMikMod.lastWrap = (options.wrap ? 1 : 0);

			if (options.loop !== undefined)
				LibMikMod.lastLoop = (options.loop ? 1 : 0);

			if (options.fadeout !== undefined)
				LibMikMod.lastFadeout = (options.fadeout ? 1 : 0);
		}

		const r = LibMikMod.cLib._loadModule(destinationSampleRate, LibMikMod.lastReverb, LibMikMod.lastHqMixer, LibMikMod.lastInterpolation, LibMikMod.lastNoiseReduction, LibMikMod.lastWrap, LibMikMod.lastLoop, LibMikMod.lastFadeout);
		if (!r) {
			const audioBufferPtr = LibMikMod.cLib._getAudioBuffer();
			if (!audioBufferPtr)
				return 2; // MMERR_OUT_OF_MEMORY
		}

		return r;
	}

	public static changeGeneralOptions(options?: LibMikModGeneralOptions | null): void {
		if (options) {
			if (options.reverb !== undefined && options.reverb >= 0 && options.reverb <= 15)
				LibMikMod.lastReverb = options.reverb;

			if (options.interpolation !== undefined)
				LibMikMod.lastInterpolation = (options.interpolation ? 1 : 0);

			if (options.noiseReduction !== undefined)
				LibMikMod.lastNoiseReduction = (options.noiseReduction ? 1 : 0);

			if (LibMikMod.cLib)
				LibMikMod.cLib._changeGeneralOptions(LibMikMod.lastReverb, LibMikMod.lastInterpolation, LibMikMod.lastNoiseReduction);
		}
	}

	public static stopModule(): void {
		if (LibMikMod.cLib) {
			LibMikMod.cLib._freeModule();

			LibMikMod.audioBufferPtr = 0;
			LibMikMod.audioBufferUsedLength = 0;
		}
	}

	private static getString(ptr: number): string | null {
		if (LibMikMod.cLib && ptr) {
			const heap = LibMikMod.cLib.HEAP8;

			let len = 0;

			for (let i = ptr; heap[i] && len < 1024; i++)
				len++;

			if (!len)
				return "";

			const arr: number[] = new Array(len);

			while (len-- > 0)
				arr[len] = heap[ptr + len];

			return String.fromCharCode.apply(String, arr);
		}

		return null;
	}

	public static getSongName(): string | null {
		return (LibMikMod.cLib ? LibMikMod.getString(LibMikMod.cLib._getSongName()) : null);
	}

	public static getModType(): string | null {
		return (LibMikMod.cLib ? LibMikMod.getString(LibMikMod.cLib._getModType()) : null);
	}

	public static getComment(): string | null {
		return (LibMikMod.cLib ? LibMikMod.getString(LibMikMod.cLib._getComment()) : null);
	}

	public static getErrno(): number {
		return (LibMikMod.cLib ? LibMikMod.cLib._getErrno() : 0);
	}

	public static getStrerr(code: number): string | null {
		return (LibMikMod.cLib ? LibMikMod.getString(LibMikMod.cLib._getStrerr(code)) : null);
	}

	public static process(outputs: Float32Array[][]): boolean {
		if (!LibMikMod.cLib)
			return false;

		let audioBufferUsedLength = LibMikMod.audioBufferUsedLength;

		if (!audioBufferUsedLength) {
			for (let attempts = 0; attempts < 3; attempts++) {
				audioBufferUsedLength = LibMikMod.cLib._update();

				if (audioBufferUsedLength < 0)
					return false;

				if (audioBufferUsedLength) {
					LibMikMod.audioBufferPtr = LibMikMod.cLib._getAudioBuffer();
					LibMikMod.audioBufferUsedLength = audioBufferUsedLength;
					break;
				}
			}

			// Output silence if we cannot fill the buffer
			if (!audioBufferUsedLength)
				return true;
		}

		let tmpBuffer: Float32Array | null = null;

		for (let o = outputs.length - 1; o >= 0; o--) {
			const channels = outputs[o];

			for (let c = channels.length - 1; c >= 0; c--) {
				const channel = channels[c];

				if (!tmpBuffer) {
					// Convert mono 32-bit float samples into bytes
					const maxBytes = channel.length << 2;

					if (audioBufferUsedLength >= maxBytes) {
						audioBufferUsedLength = maxBytes;

						// Convert bytes into mono 32-bit float samples
						tmpBuffer = new Float32Array(LibMikMod.cLib.HEAP8.buffer, LibMikMod.audioBufferPtr, audioBufferUsedLength >> 2);
						LibMikMod.audioBufferPtr += audioBufferUsedLength;
						LibMikMod.audioBufferUsedLength -= audioBufferUsedLength;
					} else {
						// We had data, but not enough to fill the buffer... This should not
						// happen as long as BUFFERSIZE in drv_webaudio.c is a multiple of maxBytes,
						// which is currently true, since BUFFERSIZE is 32768, and maxBytes is (128 * 4) = 512

						if (!LibMikMod.tmpBuffer || LibMikMod.tmpBuffer.byteLength !== maxBytes)
							LibMikMod.tmpBuffer = new Float32Array(channel.length);

						// Convert bytes into mono 32-bit float samples
						let samplesRead = audioBufferUsedLength >> 2;

						tmpBuffer = new Float32Array(LibMikMod.cLib.HEAP8.buffer, LibMikMod.audioBufferPtr, samplesRead);
						LibMikMod.audioBufferPtr = 0;
						LibMikMod.audioBufferUsedLength = 0;
						audioBufferUsedLength = 0;

						LibMikMod.tmpBuffer.set(tmpBuffer);

						let remainingSamples = channel.length - samplesRead;

						while (remainingSamples > 0) {
							if (!audioBufferUsedLength) {
								for (let attempts = 0; attempts < 3; attempts++) {
									audioBufferUsedLength = LibMikMod.cLib._update();

									if (audioBufferUsedLength < 0) {
										if (LibMikMod.getErrno())
											return false;

										audioBufferUsedLength = 0;
										break;
									}

									if (audioBufferUsedLength) {
										LibMikMod.audioBufferPtr = LibMikMod.cLib._getAudioBuffer();
										LibMikMod.audioBufferUsedLength = audioBufferUsedLength;
										break;
									}
								}

								// Output silence if we cannot fill the buffer
								if (!audioBufferUsedLength) {
									LibMikMod.tmpBuffer.fill(0, samplesRead);
									break;
								}
							}

							// Convert mono 32-bit float samples into bytes
							let remainingBytes = remainingSamples << 2;
							if (remainingBytes > audioBufferUsedLength)
								remainingBytes = audioBufferUsedLength;

							// Convert bytes into mono 32-bit float samples
							const samplesToReadNow = remainingBytes >> 2;

							tmpBuffer = new Float32Array(LibMikMod.cLib.HEAP8.buffer, LibMikMod.audioBufferPtr, samplesToReadNow);
							LibMikMod.audioBufferPtr += remainingBytes;
							LibMikMod.audioBufferUsedLength -= remainingBytes;

							LibMikMod.tmpBuffer.set(tmpBuffer, samplesRead);

							samplesRead += samplesToReadNow;
							remainingSamples -= samplesToReadNow;
						}

						tmpBuffer = LibMikMod.tmpBuffer;
					}
				}

				channel.set(tmpBuffer);
			}
		}

		return true;
	}
}
