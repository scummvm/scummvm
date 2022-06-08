
# Building ScummVM for Webassembly
The [Emscripten](https://emscripten.org/) target provides a script to build ScummVM as a single page browser app.

## Goals
This port of ScummVM has two primary use cases as its goals:

- **Demo App**: The goal of this use case is to provide an easy way for people to discover ScummVM and old adventure games. Game preservation is not just about archival but also accessibility. The primary goal is to make it as easy as possible to play any game which can legally be made available, and there's probably nothing easier than opening a webpage to do so.

- **ScummVM as a PWA** (progressive web app): There are platforms where native ScummVM is not readily available (primarily iOS/iPadOS). A PWA can work around these limitations. To really make this work, a few more features beyond what's in a Demo App would be required: 
  * Offline Support: PWAs can run offline. This means we have to find a way to cache some data which is downloaded on demand (engine plugins, game data etc.) 
  * Cloud Storage Integration: Users will have to have a way to bring their own games and export savegame data. This is best possible through cloud storage integration. This already exists in ScummVM, but a few adjustments will be necessary to make this work in a PWA.
  
See [chkuendig/scummvm-demo](http://github.com/chkuendig/scummvm-demo/) on how a ScummVM demo app can be built (incl. playable demo).
  
## About Webassembly and Emscripten
Emscripten is an LLVM/Clang-based compiler that compiles C and C++ source code to WebAssembly for execution in web browsers. 

**Note:** In general most code can be crosscompiled to webassembly just fine. There's a few minor things which are different, but the mayor difference comnes down to how instructions are processed: Javascript and webassembly do support asynchronous/non-blocking code, but in general everything is running in the same [event loop](https://developer.mozilla.org/en-US/docs/Web/JavaScript/EventLoop). This means also that webassembly code has to pause for the browser to do it's operations - render the page, process inputs, run I/O and so on. One consequence of this is that the page is not re-drawn until the webassembly code "yields" to the browser. Emscripten provides as much tooling as possible for this, but there's sometimes still a need to manually add a call to sleep into some engines.

## How to build for Webassembly
This folder contains a script to help build scummvm with Emscripten, it automatically downloads the correct emsdk version and also takes care of bundling the data and setting up a few demo games.

### Running build.sh

`build.sh` needs to be run from the root of the project. 
```Shell
./dists/emscripten/build.sh [Tasks] [Options]
```

**Tasks:** space separated list of tasks to run. These can be:  
* `build`: Run all tasks to build the complete app. These tasks are:
  *  `setup`: Download + install EMSDK and emscripten
  *  `libs`: Download and compile the required 3rd-party libraries required to build certain engines (libmad, a52dec etc)
  *   `configure`: Run the configure script with emconfigure with the recommended settings for a simple demo page 
  *   `make`: Run the make scripts with emmake
  *   `games`: Download some demos and set up all data require for the demo page. See `--bundle-games=` below.
  *   `dist`: Copy all files into a single build-emscripten folder to bring it all together
  *   `add-games`: Runs ScummVM once to add all bundled games to the default `scummvm.ini`
* `clean`: Cleanup build artifacts (keeps libs + emsdk in place)
* `run`: Start webserver and launch ScummVM in Chrome  
  
**Options:**
*  `-h`, `--help`: print a short help text
*  `--bundle-games=<games>`: comma-separated list of demos and freeware games to bundle. Either specify a target (e.g. `comi` or a target and a specific file after a `/` , e.g. `comi/comi-win-large-demo-en.zip`)
*  `-v`, `--verbose`: print all commands run by the script
*  `--*`: all other options are passed on to the scummvm configure script

Independent of the command executed, the script sets up a pre-defined emsdk environment in the subfolder `./dists/emscripten/build.sh`

**Example:**

See e.g. [chkuendig/scummvm-demo/.github/workflows/main.yml](https://github.com/chkuendig/scummvm-demo/blob/main/.github/workflows/main.yml) for an example:
```
./dists/emscripten/build.sh build --verbose --disable-all-engines --enable-plugins --default-dynamic  --enable-engine=adl,testbed,scumm,scumm_7_8,grim,monkey4,mohawk,myst,riven,sci32,agos2,sword2,drascula,sky,lure,queen,testbed,director,stark --bundle-games=testbed,comi/comi-win-large-demo-en.zip,warlock,sky/BASS-Floppy-1.3.zip,drascula/drascula-audio-mp3-2.0.zip,monkey4,feeble,queen/FOTAQ_Floppy.zip,ft,grim/grim-win-demo2-en.zip,lsl7,lure,myst,phantasmagoria,riven,hires1,tlj,sword2
```

## Current Status of Port
In general, ScummVM runs in the browser sufficiently to run all demos and freeware games.

* All engines compile (though I didn't test all of them), including ResidualVM with WebGL acceleration and shaders work as plugins (which means the initial page load is somewhat limited)
* Audio works and 3rd-party libraries for sound and video decoding are integrated.
* All data can be downloaded on demand (or in the case of the testbed generated as part of the build script)

## Known Issues + Possible Improvements

### Emscripten Optimizations
*   Optimize asyncify behaviour (we only have SDL functions calling wait currently), e.g with [SDL_HINT_EMSCRIPTEN_ASYNCIFY](https://wiki.libsdl.org/SDL_HINT_EMSCRIPTEN_ASYNCIFY)
*   Specify a `ASYNCIFY_ONLY` list in `configure` to  make asyncify only instrument functions in the call path as described in [emscripten.org: Asyncify](https://emscripten.org/docs/porting/asyncify.html)
*   Limit asyncify overhead by having a more specific setting for `ASYNCIFY_IMPORTS` in `configure`.
*   Don't use asyncify but rewrite main loop to improve performance

### Storage Integration
*   BrowserFS seems abandoned and never did a stable 2.0.0 release. It's worth replacing it.  
    * `scummvm_fs.js` is an early prototype for a custom FS which can be adopted for ScummVM specific needs, i.e.
      * Download all game assets in background once the game has started
      * Presist last game and last plugin for offline use
      * Implement support for range requests (currently not supported with `emrun` so another development server would have to be included as well)
      * Pre-load assets asynchronously (not blocking) - i.e. rest of the data of a game which has been launched
      * Loading indicators

*   Add support for save games (and game data?) on personal cloud storage (Dropbox, Google Drive).

### UI Integration
*   Build a nice webpage around the canvas.
    *   Allow showing/hiding of console, replace buttons/checkboxes from default emscripten template.
    *   Bonus: Adapt page padding/background color to theme (black when in game)

*   Automatically show console in case of exceptions

* Aspect Ratio is broken when starting a game until the window is resized once. Good starting points might be  https://github.com/emscripten-ports/SDL2/issues/47 or https://github.com/emscripten-core/emscripten/issues/10285
    * doesn't seem to affect 3d engines in opengl mode
    * definitely affects testbed in opengl or other modes