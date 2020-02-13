ScummVM 3DS README
------------------------------------------------------------------------

Table of Contents:
------------------
[1.0) Installation](#10-installation)
 * [1.1 3DSX installation](#11-3dsx-installation)
 * [1.2 CIA installation](#12-cia-installation)

[2.0) Controls](#20-controls)
 * [2.1 Default key mappings](#21-default-key-mappings)
 * [2.2 Hover mode](#22-hover-mode)
 * [2.3 Drag mode](#23-drag-mode)
 * [2.4 Magnify mode](#24-magnify-mode)

[3.0) Supported Games](#30-supported-games)

[4.0) Compiling](#40-compiling)
 * [4.1 Prerequisites](#41-prerequisites)
 * * [4.1.1 Compiling third-party libraries](#411-compiling-third-party-libraries)
 * * [4.1.2 Manually setting up the environment](#412-manually-setting-up-the-environment)
 * [4.2 Compiling ScummVM](#42-compiling-scummvm)

1.0) Installation
-----------------
There are two possible formats to be used: 3DSX and CIA.
The 3DSX format is considered more ethical because it does not make use of 
invalid title IDs, which get logged. It is also compatible with homebrew loading
methods that do not involve CFW.
The 3DSX format is exclusively used by the Homebrew Launcher and its derivatives.
The CIA format can be installed directly to the 3DS home menu and can be launched
using any CFW (Custom Firmware) of your choice.

Installing the Homebrew Launcher or any CFW is beyond the scope of this README.

1.1) 3DSX installation
----------------
You need to merely extract the ScummVM 3DSX files to your SD card so that all 
files reside in the `/3ds/scummvm/` directory. It can then be launched by Homebrew Launcher
or a similar implementation.

1.2) CIA installation
---------------------
The CIA format requires a DSP binary dump saved on your SD card as `/3ds/dspfirm.cdc`
for proper audio support. You can search online to find software to dump this.
Not having this file will cause many problems with games that need audio, sometimes
even crashing, so this is NOT considered optional.

Using any CIA installation software (search elsewhere for that), you need to install
the `scummvm.cia` file.

2.0) Controls
-------------

2.1) Default key mappings
-------------------------

The key mappings can be customized in the options dialog for the global mappings,
and in the edit game dialog for per-game mappings. Per-game mappings overlay the
global mappings, so if a button is bound to an action twice, the per-game mapping
wins.

The default keymap is:

|  Buttons   |   Function                     |
|------------|--------------------------------|
| A          | Left-click                     |
| B          | Right-click                    |
| Y          | ESC (skips cutscenes and such) |
| X          | Use virtual keyboard           |
| L          | Toggle magnify mode on/off     |
| R          | Toggle hover/drag modes        |
| Start      | Open global main menu          |
| Select     | Open 3DS config menu           |
| Circle Pad | Move the cursor                |

2.2) Hover mode
---------------
When you use the touchscreen, you are simulating the mere moving of the mouse. You
can click only with taps, meaning it is impossible to drag stuff or hold down a
mouse button without using buttons mapped to right/left-click.

2.3) Drag mode
--------------
Every time you touch and release the touchscreen, you are simulating the click and
release of the mouse buttons. At the moment, this is only a left-click.

2.4) Magnify mode
-----------------
Due to the low resolutions of the 3DS's two screens (400x240 for the top, and 320x240
for the bottom), games that run at a higher resolution will inevitably lose some visual
detail from being scaled down. This can result in situations where essential information
is undiscernable, such as text. Magnify mode increases the scale factor of the top screen
back to 1; the bottom screen remains unchanged. The touchscreen can then be used to change
which part of the game display is being magnified. This can all be done even in situations
where the cursor is disabled, such as during full-motion video (FMV) segments.

When activating magnify mode, touchscreen controls are automatically switched to hover
mode; this is to reduce the risk of the user accidentally inputting a click when changing
the magnified area via dragging the stylus. Clicking can still be done at will as in normal
hover mode. Turning off magnify mode will revert controls back to what was being used
previously (ex: if drag mode was in use prior to activating magnify mode, drag mode will
be reactivated upon exiting magnify mode), as well as restore the top screen's previous
scale factor.

Currently magnify mode can only be used when the following conditions are met:
 - In the 3DS config menu, "Use Screen" is set to "Both"
 - A game is currently being played
 - The horizontal and/or vertical resolution in-game is greater than that of the top screen

Magnify mode cannot be used in the Launcher menu.

3.0) Supported Games
--------------------
The full game engine compatibility list can be found here:
https://scummvm.org/compatibility/

While all the above games should run on the 3DS (report if they do not), there are
many games which are unplayable due to the lack of CPU speed on the 3DS. So if
you play any games that run really slow, this is not considered a bug, but rather
a hardware limitation. Though possible GPU optimizations are always in the works.
The New 3DS console has much better performance, but there are still many newer and
high-resolution games that cannot be played. A list of these unplayable games and
game engines will eventually be listed here.

4.0) Compiling
--------------
4.1) Prerequisites
------------------
 - Latest version of devkitPro, which comes with devkitARM and `libctru`
 - `citro3d` thorugh devkitPro's pacman
 - Optional: You should compile third-party libraries for the 3ds (commonly referred
   to as portlibs in the devkitPRO community). Some games requires these to operate
   properly.


4.1.1) Compiling third-party libraries
--------------------------------------
It is strongly recommended that you use devkitPro's pacman in order to get the most recent
portlibs for your build.

The following libraries can be downloaded with pacman:

|  Library      |  Package              |
----------------|------------------------
|  zlib         |  3ds-zlib             |
|  libpng       |  3ds-libpng           |
|  libjpeg      |  3ds-libjpeg-turbo    |
|  freetype2    |  3ds-freetype         |
|  libmad       |  3ds-libmad           |
|  libogg       |  3ds-libogg           |
|  tremor       |  3ds-libvorbisidec    |
|  flac         |  3ds-flac             |
|  curl         |  3ds-curl             |

At the moment of writing, the version of `freetype2` packaged by devkitPro has an issue
where it allocates too much data on the stack when ScummVM loads GUI themes.
As a workaround, an older version can be used. Version 2.6.5 is known to work well. The
instructions below can be used to compile it.

At the moment of writing, `faad` is not in the devkitPro 3DS pacman repository. It
can be compiled by following the instructions in the section below, in case it cannot
be found through pacman.

The following pacman packages are also recommended:
 - `3ds-dev`
 - `devkitpro-pkgbuild-helpers`

Once you have the `devkitpro-pkgbuild-helpers` package, you should be able to find
the following scripts in your `/opt/devkitpro` folder:
 - `devkitarm.sh`
 - `3dsvars.sh`

Run them one after the other with `source` in order to setup your environment variables
for cross-compiling:
```
 $ source /opt/devkitpro/devkitarm.sh
 $ source /opt/devkitpro/3dsvars.sh
```

After that, you can download the libraries you want to cross compile, run any autoconf
scripts that they may have, and then they can usually be built with the following steps
from their source directory:
```
 $ mkdir -p $PORTLIBS
 $ ./configure --prefix=$PORTLIBS --host=arm-none-eabi --disable-shared \
     --enable-static
 $ make
 $ make install
```
Most libraries used can be compiled with same commands and configuration flags.

4.1.2) Manually setting up the environment
------------------------------------------
In case you don't have the helpers package downloaded, you can use the following to set-up
your environment variables.

It is assumed that you have these variables already set up. If not, then do so:
 - DEVKITPRO    Your root devkitPro directory
 - DEVKITARM    Your root devkitARM directory (probably same as $DEVKITPRO/devkitARM)
 - CTRULIB      Your root libctru directory (probably same as $DEVKITPRO/libctru)

In the source directory of the library:
```
 $ export PORTLIBS=$DEVKITPRO/portlibs/3ds
 $ export PATH=$DEVKITARM/bin:$PATH
 $ export PKG_CONFIG_PATH=$PORTLIBS/lib/pkgconfig
 $ export PKG_CONFIG_LIBDIR=$PORTLIBS/lib/pkgconfig
 $ export CFLAGS="-g -march=armv6k -mtune=mpcore -mfloat-abi=hard -O2
                    -mword-relocations -ffunction-sections -fdata-sections"
 $ export CPPFLAGS="-I$PORTLIBS/include -I$CTRULIB/include"
 $ export LDFLAGS="-L$PORTLIBS/lib"
 ```

4.2) Compiling ScummVM
----------------------
Do the following in a fresh terminal.

In case you get a "compiler not found" message, add the toolchain's executables to your PATH:
```$ export PATH=$DEVKITARM/bin:$PATH```

Note: In more recent codebases of ScummVM, you may or may not need to set the following beforehand:
```$ export PKG_CONFIG_LIBDIR=$PORTLIBS/lib/pkgconfig```
See above for $PORTLIBS.

ScummVM doesn't provide the CA certificates bundle required by the cloud synchronization features.
You need to download it from the curl website: https://curl.haxx.se/ca/cacert.pem, and instruct
the build system to package it in the binary:
```$ export DIST_3DS_EXTRA_FILES=/path/to/cacert.pem```
The name of the file must be `cacert.pem`.

From the root of the scummvm repository:
```
 $ ./configure --host=3ds --enable-plugins --default-dynamic
 $ make
```
Additionally compile to specific formats to be used on the 3DS:
```
 $ make scummvm.3dsx
 $ make scummvm.cia
```

Assuming everything was successful, you'll be able to find the binary
files in the root of your scummvm folder.

Note: for the CIA format, you will need the 'makerom' and 'bannertool' tools which are
not supplied with devkitPro.

Note: using dynamic plugins as suggested is required when building with most or all of the
game engines enabled in order to keep the memory usage low and avoid stability issues.
