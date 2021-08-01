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

class LibMikMod {
	// Due to both LibMikMod and AudioWorklet's nature we can
	// have only one module loaded at a time...

	public static readonly WEB_VERSION = 1;
	public static LIB_VERSION: string | null = null;

	public static readonly ERROR_FILE_READ = 1;
	public static readonly ERROR_MODULE_LOAD = 2;
	public static readonly ERROR_PLAYBACK = 3;

	public static initialized = false;
	public static initializing = false;
	public static initializationError = false;

	public static infoSongName: string | null = null;
	public static infoModType: string | null = null;
	public static infoComment: string | null = null;

	private static currentId = 0;
	private static audioNode: AudioWorkletNode | null = null;
	
	private static onload: ((audioNode: AudioWorkletNode) => void) | null = null;
	private static onerror: ((errorCode: number, reason?: any) => void) | null = null;
	private static onended: (() => void) | null = null;

	public static isSupported(): boolean {
		// Should we also check for HTTPS? Because, apparently, the browser already undefines
		// AudioWorklet when not serving the files from a secure origin...
		return (("AudioWorklet" in window) && ("AudioWorkletNode" in window) && ("WebAssembly" in window));
	}

	public static async init(audioContext: AudioContext, libPath?: string | null): Promise<void> {
		if (LibMikMod.initialized || LibMikMod.initializing || LibMikMod.initializationError)
			return;

		if (!libPath)
			libPath = "";
		else if (!libPath.endsWith("/"))
			libPath += "/";

		LibMikMod.initializing = true;

		LibMikMod.currentId = 0;

		try {
			const response = await fetch(libPath + "libmikmodclib.wasm?" + LibMikMod.WEB_VERSION);

			const wasmBuffer = await response.arrayBuffer();

			await audioContext.audioWorklet.addModule(libPath + "libmikmodprocessor.min.js?" + LibMikMod.WEB_VERSION);

			await new Promise<void>(function (resolve, reject) {
				const audioNode = new AudioWorkletNode(audioContext, "libmikmodprocessor");

				audioNode.port.onmessage = function (ev) {
					const message = ev.data as LibMikModResponse;

					if (!message || message.messageId !== LibMikModMessageId.INIT || !LibMikMod.initializing || LibMikMod.currentId)
						return;

					if (message.errorStr) {
						reject(message.errorStr);
					} else {
						LibMikMod.LIB_VERSION = ((message.id >>> 16) & 0xFF) + "." + ((message.id >>> 8) & 0xFF) + "." + (message.id & 0xFF);
						resolve();
					}
				};

				audioNode.onprocessorerror = function (ev) {
					reject(ev);
				};

				LibMikMod.audioNode = audioNode;

				LibMikMod.postMessage({
					id: LibMikMod.currentId,
					messageId: LibMikModMessageId.INIT,
					buffer: wasmBuffer
				});
			});

			LibMikMod.initialized = true;
		} finally {
			if (!LibMikMod.initialized)
				LibMikMod.initializationError = true;

			LibMikMod.initializing = false;

			LibMikMod.stopModule();
		}
	}

	private static postMessage(message: LibMikModMessage): void {
		if (!LibMikMod.audioNode)
			return;

		if (message.buffer)
			LibMikMod.audioNode.port.postMessage(message, [message.buffer]);
		else
			LibMikMod.audioNode.port.postMessage(message);
	}

	public static loadModule(options: LibMikModLoadOptions): void {
		if (!LibMikMod.initialized)
			throw new Error("Library not initialized");

		if (!options)
			throw new Error("Null options");

		if (!options.audioContext)
			throw new Error("Null audioContext");

		const source = options.source;

		if (!source)
			throw new Error("Null source");

		LibMikMod.stopModule();

		const audioNode = new AudioWorkletNode(options.audioContext, "libmikmodprocessor");

		LibMikMod.currentId++;

		audioNode.port.onmessage = LibMikMod.handleResponse;
		audioNode.onprocessorerror = LibMikMod.notifyProcessorError;

		LibMikMod.audioNode = audioNode;

		LibMikMod.infoSongName = null;
		LibMikMod.infoModType = null;
		LibMikMod.infoComment = null;

		LibMikMod.onload = options.onload;
		LibMikMod.onerror = options.onerror;
		LibMikMod.onended = options.onended;

		const id = LibMikMod.currentId,
			initialOptions: LibMikModInitialOptions = {
				hqMixer: options.hqMixer,
				wrap: options.wrap,
				loop: options.loop,
				fadeout: options.fadeout,
				reverb: options.reverb,
				interpolation: options.interpolation,
				noiseReduction: options.noiseReduction
			};

		if ("lastModified" in source) {
			if ("arrayBuffer" in source) {
				source.arrayBuffer().then(function (arrayBuffer) {
					if (id !== LibMikMod.currentId)
						return;

					LibMikMod.postMessage({
						id: LibMikMod.currentId,
						messageId: LibMikModMessageId.LOAD_MODULE_BUFFER,
						buffer: arrayBuffer,
						options: initialOptions
					});
				}, function (reason) {
					if (id !== LibMikMod.currentId)
						return;

					LibMikMod.notifyReaderError(reason);
				});
			} else {
				const reader = new FileReader();
				reader.onerror = function (ev) {
					if (id !== LibMikMod.currentId)
						return;

					LibMikMod.notifyReaderError(ev);
				};
				reader.onload = function () {
					if (id !== LibMikMod.currentId)
						return;

					if (!reader.result)
						LibMikMod.notifyReaderError("Empty reader result");
					else
						LibMikMod.postMessage({
							id: LibMikMod.currentId,
							messageId: LibMikModMessageId.LOAD_MODULE_BUFFER,
							buffer: reader.result as ArrayBuffer,
							options: initialOptions
						});
				};
				reader.readAsArrayBuffer(source);
			}
		} else {
			LibMikMod.postMessage({
				id: LibMikMod.currentId,
				messageId: LibMikModMessageId.LOAD_MODULE_BUFFER,
				buffer: source,
				options: initialOptions
			});
		}
	}

	public static changeGeneralOptions(options?: LibMikModGeneralOptions): void {
		if (!LibMikMod.initialized)
			throw new Error("Library not initialized");

		LibMikMod.postMessage({
			id: LibMikMod.currentId,
			messageId: LibMikModMessageId.CHANGE_GENERAL_OPTIONS,
			options: options
		});
	}

	private static cleanUp(): void {
		LibMikMod.currentId++;

		LibMikMod.infoSongName = null;
		LibMikMod.infoModType = null;
		LibMikMod.infoComment = null;

		LibMikMod.audioNode = null;

		LibMikMod.onload = null;
		LibMikMod.onerror = null;
		LibMikMod.onended = null;
	}

	public static stopModule(): void {
		if (!LibMikMod.audioNode)
			return;

		LibMikMod.postMessage({
			id: LibMikMod.currentId,
			messageId: LibMikModMessageId.STOP_MODULE
		});

		LibMikMod.cleanUp();
	}

	private static notifyReaderError(reason?: any): void {
		LibMikMod.notifyError(LibMikMod.ERROR_FILE_READ, reason);
	}

	private static notifyProcessorError(reason?: any): void {
		LibMikMod.notifyError(LibMikMod.ERROR_PLAYBACK, reason);
	}

	private static notifyError(errorCode: number, reason?: any): void {
		if (!LibMikMod.audioNode)
			return;

		const onerror = LibMikMod.onerror;

		LibMikMod.cleanUp();

		if (onerror)
			onerror(errorCode, reason);
	}

	private static notifyEnded(): void {
		if (!LibMikMod.audioNode)
			return;

		const onended = LibMikMod.onended;

		LibMikMod.cleanUp();

		if (onended)
			onended();
	}

	private static handleResponse(ev: MessageEvent): void {
		const message = ev.data as LibMikModResponse;

		if (!message)
			return;

		switch (message.messageId) {
			case LibMikModMessageId.LOAD_MODULE_BUFFER:
				if (message.id !== LibMikMod.currentId || !LibMikMod.audioNode)
					break;

				if (message.errorCode) {
					LibMikMod.notifyError(LibMikMod.ERROR_MODULE_LOAD, message.errorStr || message.errorCode.toString());
				} else {
					LibMikMod.infoSongName = (message.infoSongName || null);
					LibMikMod.infoModType = (message.infoModType || null);
					LibMikMod.infoComment = (message.infoComment || null);

					if (LibMikMod.onload)
						LibMikMod.onload(LibMikMod.audioNode);
				}
				break;

			case LibMikModMessageId.PLAYBACK_ERROR:
				if (message.id !== LibMikMod.currentId)
					break;

				LibMikMod.notifyProcessorError(message.errorStr || message.errorCode?.toString());
				break;

			case LibMikModMessageId.PLAYBACK_ENDED:
				if (message.id !== LibMikMod.currentId)
					break;

				LibMikMod.notifyEnded();
				break;
		}
	}
}
