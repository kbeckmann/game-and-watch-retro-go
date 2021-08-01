LibMikMod Web Audio
===================

This is a port of the MikMod Sound Library to the web using [WebAssembly](https://developer.mozilla.org/en-US/docs/WebAssembly), [Web Audio](https://developer.mozilla.org/en-US/docs/Web/API/Web_Audio_API), [AudioWorklet](https://developer.mozilla.org/en-US/docs/Web/API/AudioWorklet) and [AudioWorkletNode](https://developer.mozilla.org/en-US/docs/Web/API/AudioWorkletNode).

The files [libmikmod/webaudio/exampleSimple.html](https://github.com/carlosrafaelgn/mikmod/blob/master/libmikmod/webaudio/exampleSimple.html) and [libmikmod/webaudio/exampleOptions.html](https://github.com/carlosrafaelgn/mikmod/blob/master/libmikmod/webaudio/exampleOptions.html) are examples on how to use the library in the web environment. Both files can be live previewed here:

- [exampleSimple.html](https://fplay.com.br/examples/exampleSimple.html)
- [exampleOptions.html](https://fplay.com.br/examples/exampleOptions.html)

Downloading the library
-----------------------

The web port of the library consists of three files that can be built following the instructions below. After being built, the files will be located in the `libmikmod/webaudio/dist` folder.

For convenience, you can download the prebuilt files, the same ones used in the live examples, from the following address (be sure to check the library version before using the files, because they could be outdated compared to the actual library):

https://fplay.com.br/examples/libmikmod.zip

Using the library
-----------------

To use the library, all you need to do is to reference the file `libmikmod/webaudio/dist/libmikmod.min.js` from your HTML page. That file adds the `LibMikMod` class to the global scope, from which you can call all the necessary methods and access all properties.

``` html
<script type="text/javascript" src="dist/libmikmod.min.js"></script>
```

*Important notice! Due to both LibMikMod and AudioWorklet's nature we can have only one module loaded at a time.*

Public properties (they are all static members):

- `WEB_VERSION: number`: Current version of the web library.
- `LIB_VERSION: string | null`: Current version of the MikMod library (set after `init()` is called and succeeds).
- `ERROR_FILE_READ / ERROR_MODULE_LOAD / ERROR_PLAYBACK: number`: Constants passed as the `errorCode` to the `onerror` callback.
- `initialized: boolean`: Boolean flag that indicates if the library has been properly initialized (if the method `init()` has already been called and has finished successfully).
- `initializing: boolean`: Boolean flag that indicates if the library is still being loaded.
- `initializationError: boolean`: Boolean flag that indicates if any errors happened during the load process of the library itself (an error here could indicate the wasm file is missing, the browser does not support at least one of the required API's or that you are trying to instantiate the library from a non-HTTPS domain, which is forbidden by most browsers, with the exception of the `localhost` domain).
- `infoSongName: string | null`: String that contains the name of the song (after a module has been successfully loaded).
- `infoModType: string | null`: String that contains the module type (after a module has been successfully loaded).
- `infoComment: string | null`: String that contains the comments (after a module has been successfully loaded).

Public methods (also, all static members):

- `isSupported(): boolean`: Returns whether or not the required API's are supported by the browser.
- `init(audioContext: AudioContext, libPath?: string | null): Promise<void>`: Initializes the library. This method must called only once and before any module is loaded. The `libPath` parameter indicates the prefix of the path where the library files are located (can be a path relative to the current page's address). The promise returned by the method resolves if the library could be loaded, or rejects if an error occurs during the loading process.
- `loadModule(options: LibMikModLoadOptions): void`: Loads a module using the given options. If another module was loaded / still loading, it is stopped and unloaded before this new module is loaded. Be sure to discard all previous `audioNode`'s either before or after this method returns, because the process of loading a new module renders all previously loaded modules useless. After successfully loading the new module, `onload` callback is called. If an error happens during the load process, `onerror` callback is called instead. All optional settings in `options` will take the previously given value when they are not provided.
- `changeGeneralOptions(options?: LibMikModGeneralOptions): void`: Changes the general options for the currently loaded module, if any.
- `stopModule(): void`: Stops and unloads the currently loaded module, if any.

`LibMikModGeneralOptions` interface:

``` ts
interface LibMikModGeneralOptions {
  // Amount of sound reverberation. Allowed values range from 0 (no reverberation)
  // to 15 (a rough estimate for chaos...). The default value is 0.
  reverb?: number;

  // This flag, if set, enables the interpolated mixers. Interpolated mixing gives
  // better sound but takes a bit more time than standard mixing. If the library is
  // built with the high quality mixer, interpolated mixing is always enabled,
  // regardless of this flag. The default value is true.
  interpolation?: boolean;

  // This flag, if set, enables low pass filtering on the output. The default value
  // is true.
  noiseReduction?: boolean;
}
```

`LibMikModLoadOptions` interface:

``` ts
interface LibMikModLoadOptions extends LibMikModGeneralOptions {
  // The audio context that will be used to create the AudioNode.
  audioContext: AudioContext;

  // Source from where to load the module.
  source: File | ArrayBuffer | Uint8Array;

  // Callback that is called when the module is successfully loaded. You must
  // connect the given audio node to another node, such as the audio context's
  // destination, in order to play the audio. Due to AudioWorkletNode's current
  // behavior, playback can only be paused/resumed by suspending/resuming the
  // audio context.
  onload: (audioNode: AudioWorkletNode) => void;

  // Callback that is called when an error happens either during the loading
  // process or during the playback.
  onerror: (errorCode: number, reason?: any) => void;

  // Callback that indicates when the module has reached its end. This could
  // never actually be called depending on the module and on the state of the
  // wrap and loop flags.
  onended: () => void;

  // This flag, if set, selects the high-quality software mixer. This mode yields
  // better sound quality, but needs more mixing time. The default value is true.
  hqMixer?: boolean;

  // This flag, if set, will make the module restart when it's finished. The
  // default value is false.
  wrap?: boolean;

  // This flag, if set, allows a module to loop (jump backwards). The default
  // value is false.
  loop?: boolean;

  // This flag, if set, applies a volume fade out during the module's last
  // pattern. The default value is true.
  fadeout?: boolean;
}
```

Again, check out the files [libmikmod/webaudio/exampleSimple.html](https://github.com/carlosrafaelgn/mikmod/blob/master/libmikmod/webaudio/exampleSimple.html) and [libmikmod/webaudio/exampleOptions.html](https://github.com/carlosrafaelgn/mikmod/blob/master/libmikmod/webaudio/exampleOptions.html) for examples on how to use the library in the web environment. 😊

Building the library
--------------------

In order to build the WebAssembly version of all native C files you must install and properly configure [emscripten](https://emscripten.org), making the command `emcc` globally accessible from any command prompt.

All JS files were originally written in TypeScript. Therefore, if you want to recreate them, the [TypeScript compiler](https://www.npmjs.com/package/typescript) must also be installed, and the command `tsc` must also be globally accessible from any command prompt.

Optionally, if you wish to generate the minified version of the JS files, you will also need to install any Java environment (JRE or JDK) along with the [Closure Compiler](https://developers.google.com/closure/compiler) tool.

There are two different sets of build scripts, each one located in its own folder, depending on the host OS: [libmikmod/webaudio/build/linux](https://github.com/carlosrafaelgn/mikmod/tree/master/libmikmod/webaudio/build/linux) and [libmikmod/webaudio/build/windows](https://github.com/carlosrafaelgn/mikmod/tree/master/libmikmod/webaudio/build/windows).

The first time you build the library, whether or not you have changed the C files, you must run `buildwasm`, generating all WebAssembly-related files (both wasm and JS). The JS file is generated in a temporary folder (libmikmod/webaudio/src/temp) and is reused everytime the next scripts are run.

After that you can run the scripts `tscbdbg` and `tscfdbg` at any time to convert the TypeScript files into JS.

- `tscbdbg` builds the "back-end" script file ([libmikmod/webaudio/dist/libmikmodprocessor.min.js](https://github.com/carlosrafaelgn/mikmod/blob/master/libmikmod/webaudio/dist/libmikmodprocessor.min.js)), responsive for generating the audio on a separate thread.
- `tscfdbg` builds the "front-end" script file ([libmikmod/webaudio/dist/libmikmod.min.js](https://github.com/carlosrafaelgn/mikmod/blob/master/libmikmod/webaudio/dist/libmikmod.min.js)), which is the only file you must actually reference from your HTML page.

Despite the extension `.min.js`, both scripts `tscbdbg` and `tscfdbg` do not generate minified files. For that, you must install the optional dependencies listed above and run the scripts `tscbmin` and `tscfmin`.

Don't forget to run either `tscbdbg` or `tscbmin` after rebuilding the native C files with the `buildwasm` script!

That's it! I hope you enjoy this port! 😊🙏
