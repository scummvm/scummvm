# ScummVM libretro port
## Building ScummVM core
To build the core with the default configuration, type in a shell the following:
```
git clone https://github.com/scummvm/scummvm
cd scummvm/backends/platform/libretro
make
```
Use `make all` to build the core along with datafiles/themes (`scummvm.zip`) and core info file (which can be built separately with `make datafiles`/`make coreinfo`).

"Work in progress" engines are not built by default, to include them pass `NO_WIP=0` to make.

To crossbuild for specific platforms, pass the relevant `platform` variable to make (refer to Makefile for supported ones).

### Build for Android
To build for Android:
* install android-sdk
* install android-ndk (e.g. through sdkmanager included in android-sdk/cmdline-tools)
* make sure all needed sdk/ndk paths are included in PATH
* type in a shell the following:
```
git clone https://github.com/scummvm/scummvm
cd scummvm/backends/platform/libretro/jni
ndk-build
```
Core will be built for all available android targets by default

### Data files and themes
[Data files](https://wiki.scummvm.org/index.php/Datafiles) required for certain games and core functions (e.g. virtual keyboard) and default [ScummVM GUI themes](https://wiki.scummvm.org/index.php/GUI_Themes) are bundled during the build process in `scummvm.zip` file. Libretro core info file is created during build as well.
Extract `scummvm.zip` and select relevant paths in ScummVM GUI (or modify `scummvm.ini` accordingly) to load properly needed data files/themes in the core.

Note that both datafiles and themes included in `scummvm.zip` need to be consistent with ScummVM version in use, hence need to be generally rebuilt and replaced for each new version.

## Libretro playlists for ScummVM core
Playlists used in Libretro frontends (e.g. Retroarch) are plain text lists used to directly launch a game with a specific core from the user interface. Those lists are structured to pass to the core the path of a specific content file to be loaded (e.g. a ROM zip fiile). Detailed info can be found in [Libretro documentation](https://docs.libretro.com/guides/roms-playlists-thumbnails/).

> WARNING: ScummVM core playlist generation with Retroarch Scanner is **NOT recommended** as it is based on a third party database instead of ScummVM game detection system, hence it is not guaranteed to work properly.

ScummVM core can accept as content the playlist-provided path to any of the files inside a valid game folder, the ScummVM internal detection system will try to autodetect the game from the content file parent folder and run the game with default ScummVM options.

The core also supports dedicated per game **hook** plain text files with `.scummvm` extension, which can be used as target path in the playlist. Content of this file shall be a string corresponding to one of the following ScummVM identifiers:

  - **target**: this is the game identifier of each entry in the internal Launcher list, hence corresponding to entries in ScummVM configuration file (e.g. 'scummvm.ini'). In this case the game must be added from ScummVM GUI first, and the hook files can be placed anywhere, as the path for the game files is already part of the target configuration. The game will be launched with the options set in ScummVM

  - **game ID**: this is a unique identifier for any game supported by ScummVM. This identifier is hard coded in each engine source and can be subject to change, hence it is **not a recommended choice**. A list of current game IDs is available [here](https://scummvm.org/compatibility). In this case the game will be launched even if not added in ScummVM Launcher, the hook file must be placed in the game folder and the game will be launched with default ScummVM options

### Creating ScummVM core playlist
The recommended way to generate a ScummVM core playlist is from within the ScummVM Launcher (i.e. the interal core GUI) with the Playlist Generator tool. Both needed hook files and the playlist are created automatically, based on ScummVM Launcher games list.
  - Load the core from RetroArch and start it to reach the ScummVM GUI (i.e. the Launcher)
  - Add games to the list as required using the GUI buttons ('Mass Add' available).
  - Select **Global Options** and then the **Backend** tab.
  - Check or select the path of frontend playlists. A `ScummVM.lpl` file will be created or overwritten in there.
  - Check the 'Hooks location' setting, to have one `.scummvm` in each game folder or all of them in a `scummvm_hooks` folder in the `save` path.
  - Check the 'Playlist version' setting. JSON format should be selected, 6-lines format is deprecated and provided for backwards compatibility only.
  - Select the 'Clear existing hooks' checkbox to remove any existing `.scummvm` file in the working folders.
  - Press the 'Generate playlist' button.

Operation status will be shown in the same dialog, while details will be given in the frontend logs.

## Core options
### Cursor Movement
  - **Exclusive cursor control with RetroPad**: allows the use of RetroPad only to control mouse cursor, excluding the other inputs (e.g. physical mouse, touch screen).
  - **Gamepad Cursor Speed**: sets the mouse cursor speed multiplier when moving the cursor with the RetroPad left analog stick or D-Pad. The default value of '1.0' is optimised for games that have a native resolution of '320x200' or '320x240'. When running 'high definition' games with a resolution of '640x400' or '640x480', a Gamepad Cursor Speed of '2.0' is recommended.
  - **Gamepad Cursor Acceleration**: the amount of time (In seconds) it takes for the cursor to reach full speed
  - **Analog Cursor Response**: determines how the speed of the cursor varies when tilting the RetroPad left analog stick. 'Linear': Speed is directly proportional to analog stick displacement. This is standard behaviour with which most users will be familiar. 'Quadratic': Speed increases quadratically with analog stick displacement. This allows for greater precision when making small movements without sacrificing maximum speed at full analog range. This mode may require practice for effective use.
  - **Analog Deadzone**: sets the deadzone in percentage of the RetroPad analog sticks. Used to eliminate cursor drift/unwanted input.
  - **Mouse Speed**: sets the mouse cursor speed multiplier when moving the cursor with the RetroMouse.
  - **Mouse Fine Control Speed Reduction**: sets the mouse cursor speed reduction as percentage of normal speed when fine control is activated.
### Video settings
  - **Hardware acceleration**: request video hardware acceleration (OpenGL or OpenGLES2) to the frontend if supported. It is needed to reload the core to apply this setting.
  - **ScummVM Launcher aspect ratio**: set ScummVM Launcher aspect ratio.
  - **ScummVM Launcher resolution**: set ScummVM Launcher aspect ratio.
### Timing
  - **Frame rate cap**: set core frame rate upper limit. Reducing the limit will improve the performance on lower end devices. Changing this setting will reset the core.
  - **Sample rate**: set core sample rate. Reducing the rate will slightly improve the performance on lower end devices. Changing this setting will reset the core.
### RetroPad mapping
Settings to map each RetroPad key to ScummVM controls.
