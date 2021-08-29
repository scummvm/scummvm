# Building ScummVM for Webassembly
The [Emscripten](https://emscripten.org/) target provides a script to build ScummVM as a single page browser app.
> Emscripten is an LLVM/Clang-based compiler that compiles C and C++ source code to WebAssembly for execution in web browsers. 

## Current State
*   All engines compile (though I didn't test all of them), including ResidualVM with WebGL acceleration and shaders.
*   Audio works and 3rd-party libraries for sound and video decoding are integrated.
*   Proof of concept integration with [BrowserFS](https://github.com/jvilk/browserfs) to download game data lazily when required and to support local savegames.

## How to build for Webassembly
This folder contains a script to help build scummvm with Emscripten, it automatically downloads the correct emsdk version and also takes care of bundling the data and setting up a few demo games.

### Running `emscripten/build.sh`

`emscripten/build.sh` needs to be run from the root of the project. 
```Shell
./dists/emscripten/build.sh libs|configure|make|data|dist|all|clean
```
It accepts a single parameter with 7 valid commands:
*   `libs`: Download and compile the required 3rd-party libraries required to build certain engines (libmad, a52dec etc)
*   `configure`: Run the configure script with emconfigure with the recommended settings for a simple demo page 
*   `make`: Run the make scripts with emmake
*   `data`: Download some demos and set up all data require for the demo page 
*   `dist`: Copy all files into a single build-emscripten folder to bring it all together
*   `all`: Run all of the above commands
*   `clean`: Remove all object files, built libs, bundled data etc

Independent of the command executed, the script sets up a pre-defined emsdk environment in the subfolder `./dists/emscripten/build.sh`

## Known Issues + Possible Improvements
Some ideas for possible improvements:

### Emscripten Optimizations
*   Optimize asyncify behaviour (we only have SDL functions calling wait currently), e.g with [SDL_HINT_EMSCRIPTEN_ASYNCIFY](https://wiki.libsdl.org/SDL_HINT_EMSCRIPTEN_ASYNCIFY).
*   Specify a `ASYNCIFY_ONLY` list to to make asyncify only instrument functions in the call path as described in [emscripten.org: Asyncify](https://emscripten.org/docs/porting/asyncify.html)
*   Don't use asyncify but rewrite main loop to improve performance
*   Shrink code size or execution speed with `-Os` or `-Oz` [emcc arguments](https://emscripten.org/docs/tools_reference/emcc.html#emcc-compiler-optimization-options).

### Storage Integration
*   BrowserFS seems abandoned and never did a stable 2.0.0 release. Maybe there's a better way to handle storage?

*   File loading improvements:
    *   Load assets with HTTP Range request headers.
    *   Load assets asynchronously (not blocking) via a worker.

*   Add support for save games (and game data?) on personal cloud storage (Dropbox, Google Drive).

### UI Integration
*   Responsiveness: Adjust the canvas size when resizing the browser. 

*   Bug: Fullscreen mode doesn't work.

*   Build a nice webpage around the canvas.
    *   Allow hiding of console, replace buttons/checkboxes from default emscripten template.
    *   Bonus: Adapt page padding/background color to theme (black when in game)

*   ScummVM shouldn't be able to "close" (there's no concept for that:
    *   Remove "exit" buttons from all menus.
    *   Change any programmatic "exits" to cause a restart of Scummvm (or refresh of the page).

*   Pass CLI parameters for ScummVM via URL parameters to allow for "deep-linking" to a specific game.

### Other Bugs + Tasks
*   Bug: Vorbis support is broken - parts seems to have been patched out so  `-lvorbisfile` triggers an error during configure (and [emscripten-core/emscripten#9849](https://github.com/emscripten-core/emscripten/pull/9849) doesn't seem to fix this).
*   Bug: Going back to main menu from Grim (and other Residual Games?) messes up the render context and the UI is unusable.
*   Check all disabled features (e.g. TiMidity++) and see if they could be enabled (some might never make sense, e.g. anything requiring MIDI Hardware, Update Checking etc).