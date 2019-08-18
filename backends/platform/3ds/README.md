ScummVM 3DS README
------------------------------------------------------------------------

Table of Contents:
------------------
1.0) Installation
 * 1.1 3DSX installation
 * 1.2 CIA installation
 * 1.3 Additional files

2.0) Controls
 * 2.1 Default key mappings
 * 2.2 Hover mode
 * 2.3 Drag mode

3.0) Supported Games

4.0) Compiling
 * 4.1 Prerequisites
 * * 4.1.1 Compiling third-party libraries
 * * 4.1.2 Manually setting up the environment
 * * 4.1.3 Note on FLAC
 * 4.2 Compiling ScummVM
 * 4.3 Warning for build sizes

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
the `scummvm.cia` file. Then, just like what is done with the 3DSX installation, you
need to extract all ScummVM 3DS files (`scummvm.cia` excluded) to the root of your SD
card so that all files reside in the `/3ds/scummvm/` directory.

1.3) Additional files
---------------------
In order to use the Virtual Keyboard, you need to get the:
`backends/vkeybd/packs/vkeybd_small.zip` file from ScummVM's repository, and
place it on your SD card, in the `/3ds/scummvm/kb` folder.

In case you want a translated GUI, you need to get the:
`scummvm/gui/themes/translations.dat` file from ScummVM's repository, and place
it on your SD card, in the `/3ds/scummvm/themes` folder.

2.0) Controls
-------------

2.1) Default key mappings
-------------------------
The D-Pad and A/B/X/Y buttons have mirrored usage. So they do the same things
depending on if you're right or left-handed.

|  Buttons   |   Function                     |
|------------|--------------------------------|
| A / D-left | Left-click                     |
| X / D-up   | Right-click                    |
| B / D-down | ESC (skips cutscenes and such) |
| L          | Use virtual keyboard           |
| R          | Toggle hover/drag modes        |
| Start      | Open game menu                 |
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

At the moment of writing, `faad` and `flac` are not in the devkitPro 3DS pacman
repository. They can be compiled by following the instructions in the section below,
in case they cannot be found through pacman.

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

4.1.3) Note on FLAC:
--------------------
At the moment of writing, the `libflac` library cannot be compiled for the 3DS out of the box.
However, the following patch (for the Nintendo Switch) can be adapted and applied to it:
https://github.com/devkitPro/pacman-packages/tree/master/switch/flac

Afterwards, the library can be built with:
```
 $  CFLAGS="${CFLAGS} -D__3DS__ " \
    ./configure --prefix="${PORTLIBS_PREFIX}" --host=arm-none-eabi \
    --disable-shared --enable-static \
    --disable-xmms-plugin --disable-cpplibs \
    --disable-sse --without-ogg
```

4.2) Compiling ScummVM
----------------------
Do the following in a fresh terminal.

In case you get a "compiler not found" message, add the toolchain's executables to your PATH:
```$ export PATH=$DEVKITARM/bin:$PATH```

Note: In more recent codebases of ScummVM, you may or may not need to set the following beforehand:
```$ export PKG_CONFIG_LIBDIR=$PORTLIBS/lib/pkgconfig```
See above for $PORTLIBS.

From the root of the scummvm repository:
```
 $ ./configure --host=3ds
 $ make
```
Additionally compile to specific formats to be used on the 3DS:
```
 $ make scummvm.3dsx
 $ make scummvm.cia
```
**_Read the warning about build sizes below._**

Assuming everything was successful, you'll be able to find the binary
files in the root of your scummvm folder.

Note: for the CIA format, you will need the 'makerom' and 'bannertool' tools which are
not supplied with devkitPro.

4.3) Warning for build sizes
---------------------------
The above configuration command will include all game engines by default and will
likely be too massive to be stable using either the 3DSX or the CIA format.
Until dynamic modules are figured out, you should configure engines like this:
```
 $ ./configure --host=3ds --disable-all-engines --enable-engine=scumm-7-8,myst,riven, \
     sword1,sword2,sword25,sci,lure,sky,agi,agos
```
Choose whatever engines you want, but if the ELF's .text section exceeds ~10MB-12MB,
you may experience crashes in memory-intensive games such as COMI, Broken Sword and Discworld 2.
