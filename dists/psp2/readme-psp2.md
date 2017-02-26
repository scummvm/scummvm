Prerequisites
=============
- A homebrew enabled PlayStation Vita console.
- At least one ScummVM supported game. The list of compatible games can be seen here: http://www.scummvm.org/compatibility/
The page http://wiki.scummvm.org/index.php/Where_to_get_the_games references some places where those games can be bought. Demonstration versions for most of the supported games are downloadable on http://scummvm.org/demos/

Installing
==========
From a computer, download the installable package. Unzip and copy the .vpk file it to the PlayStation Vita and install it.

Configuring and playing games
=============================
The user manual describes how to add games to ScummVM and launch them : http://wiki.scummvm.org/index.php/User_Manual

PlayStation Vita Specifics
==========================
Saves are wrote in the ux0:/data/scummvm/saves folder.

Joypad button mapping
=====================
- Left stick     => Mouse
- R + Left stick => Slow Mouse
- Cross          => Left mouse button
- Circle         => Right mouse button
- DPad           => Cursor Keys (useful for character motion)
- R + DPad       => Diagonal Cursor Keys
- L Trigger      => Game menu (F5)
- Square         => Period '.' (used to skip dialog lines)
- R + Square     => Space ' '
- Triangle       => Escape (used to skip cutscenes)
- R + Triangle   => Return
- Start          => ScummVM's global in-game menu
- Select         => Toggle virtual keyboard
- R + Select     => AGI predictive input dialog

Disclaimer
==========
Unauthorized distribution of an installable package with non freeware games included is a violation of the copyright law and is as such forbidden.

Building from source
====================
This port of ScummVM to the PSP2 is based on SDL2. It uses the open source SDK VITASDK.

The dependencies needed to build it are :

- The toolchain from https://github.com/vitadev/vdpm
- zlib, libpng, libjpeg-turbo, libogg, libvorbis, flac, curl, openssl, freetype, libvita2d from https://github.com/vitadev/vdpm
- libmad from https://github.com/Cpasjuste/libmad-psp2
- SDL2 from https://github.com/Cpasjuste/SDL-Vita
- ScummVM from https://github.com/Cpasjuste/scummvm

Once all the dependencies are correctly setup, an installable package can be obtained from source by issuing the following command :

./configure --host=psp2 && make psp2vpk

Thanks
======
Xavier from consoleX for donating a ps vita device
xerpi for initial SDL2 port
VITASDK and henkaku developers
