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

// https://developer.mozilla.org/en-US/docs/Web/API/AudioWorkletGlobalScope
// https://developer.mozilla.org/en-US/docs/Web/API/AudioWorkletProcessor/process
// https://developers.google.com/web/updates/2017/12/audio-worklet
// https://developers.google.com/web/updates/2018/06/audio-worklet-design-pattern
// https://github.com/GoogleChromeLabs/web-audio-samples/tree/main/audio-worklet

class LibMikModProcessor extends AudioWorkletProcessor {
	// Due to both LibMikMod and AudioWorklet's nature we can
	// have only one module loaded at a time...

	private id = 0;
	private ended = false;

	public constructor() {
		super();

		this.port.onmessage = this.handleMessage.bind(this);
	}

	private postResponse(response: LibMikModResponse): void {
		if (this.port)
			this.port.postMessage(response);
	}

	private handleMessage(ev: MessageEvent): any {
		const message = ev.data as LibMikModMessage;

		if (!message)
			return;

		switch (message.messageId) {
			case LibMikModMessageId.INIT:
				if (message.id || this.id || LibMikMod.loaded || LibMikMod.loading || LibMikMod.loadErrorStr || !message.buffer)
					break;

				LibMikMod.init(message.buffer).then(() => {
					this.ended = true;
					this.id = -1;
	
					this.postResponse({
						id: LibMikMod.getVersion(),
						messageId: LibMikModMessageId.INIT
					});
				}, () => {
					this.ended = true;
					this.id = -1;
	
					this.postResponse({
						id: 0,
						messageId: LibMikModMessageId.INIT,
						errorCode: -1,
						errorStr: LibMikMod.loadErrorStr
					});
				});
				break;

			case LibMikModMessageId.LOAD_MODULE_BUFFER:
				if (!message.id || this.id)
					break;

				this.id = message.id;

				const result = LibMikMod.loadModule(sampleRate, message.buffer, message.options);
				if (result) {
					this.ended = true;
					this.id = -1;
	
					LibMikMod.stopModule();

					this.postResponse({
						id: message.id,
						messageId: LibMikModMessageId.LOAD_MODULE_BUFFER,
						errorCode: result,
						errorStr: LibMikMod.getStrerr(result)
					});
				} else {
					this.postResponse({
						id: this.id,
						messageId: LibMikModMessageId.LOAD_MODULE_BUFFER,
						infoSongName: LibMikMod.getSongName(),
						infoModType: LibMikMod.getModType(),
						infoComment: LibMikMod.getComment()
					});
				}
				break;

			case LibMikModMessageId.CHANGE_GENERAL_OPTIONS:
				if (message.id !== this.id)
					break;

				LibMikMod.changeGeneralOptions(message.options);
				break;

			case LibMikModMessageId.STOP_MODULE:
				if (message.id !== this.id)
					break;

				this.ended = true;
				this.id = -1;

				LibMikMod.stopModule();
				break;
		}
	}

	public process(inputs: Float32Array[][], outputs: Float32Array[][], parameters: { [key: string]: Float32Array }): boolean {
		if (this.ended)
			return false;

		if (!LibMikMod.process(outputs)) {
			if (!this.ended) {
				const id = this.id,
					result = LibMikMod.getErrno();

				this.ended = true;
				this.id = -1;

				LibMikMod.stopModule();

				if (result)
					this.postResponse({
						id,
						messageId: LibMikModMessageId.PLAYBACK_ERROR,
						errorCode: result,
						errorStr: LibMikMod.getStrerr(result)
					});
				else
					this.postResponse({
						id,
						messageId: LibMikModMessageId.PLAYBACK_ENDED
					});
			}
			return false;
		}
		return true;
	}
}

registerProcessor("libmikmodprocessor", LibMikModProcessor);
