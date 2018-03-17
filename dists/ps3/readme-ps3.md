Prerequisites
=============
- A homebrew enabled PlayStation 3 console. As of now that mostly means having a custom firmware installed. Obtaining and installing such a software is out of the scope of this document. Sorry, but you're on your own for that one.
- At least one ResidualVM supported game. The list of compatible games can be seen here: http://www.residualvm.org/compatibility/
- An USB drive.

Installing
==========
From a computer, download the installable package of the PS3 port from ResidualVM's main site. It should be a .pkg file. Copy it to an USB drive.
After having plugged the USB drive to you PS3, the installation package should appear in the XMB under the "Games > Install Package" menu. Installing it copies ResidualVM and its dependencies to your PS3's hard drive. It also adds the "Games > PlayStation 3 > ResidualVM" XMB entry which is to be used to launch ResidualVM.

Configuring and playing games
=============================
The user manual describes how to add games to ResidualVM and launch them : http://wiki.residualvm.org/index.php/Running_ResidualVM

PlayStation 3 Specifics
=======================
Games can be launched either from an USB drive or from the internal hard drive. The internal hard drive has better performance though.
Savegames are wrote in the /hdd0/game/RESI12000/saves folder.

Joypad button mapping
=====================
- Left stick      => Mouse
- R1 + Left stick => Slow Mouse
- Cross           => Left mouse button
- Circle          => Right mouse button
- DPad            => Cursor Keys (useful for character motion)
- R1 + DPad       => Diagonal Cursor Keys
- L1              => Game menu (F5)
- R1              => Shift (used to enable Mass Add in menu)
- Square          => Period '.' (used to skip dialog lines)
- R1 + Square     => Space ' '
- Triangle        => Escape (used to skip cutscenes)
- R1 + Triangle   => Return
- Start           => ScummVM's global in-game menu
- Select          => Toggle virtual keyboard
- R1 + Select     => AGI predictive input dialog

Disclaimer
==========
Unauthorized distribution of an installable package with non freeware games included is a violation of the copyright law and is as such forbidden.

Building from source
====================
This port of ResidualVM to the PS3 is based on SDL2. It uses the open source SDK PSL1GHT.

The dependencies needed to build it are :

- The toolchain from https://github.com/ps3dev/ps3toolchain
- SDL from https://bitbucket.org/bgK/sdl_psl1ght
- ResidualVM from https://github.com/residualvm/residualvm

Once all the dependencies are correctly setup, an installable package can be obtained from source by issuing the following command :

./configure --host=ps3 && make ps3pkg
