2001-10-08

This is the first pre-alpha release of ScummVM. It is an implementation of the SCUMM engine used in various Lucas Arts games such as Monkey Island and Day of the Tentacle.

The engine is far from complete, and thus only Monkey Island 2 is supported at the moment.

Please be aware that the engine contains bugs and non-implemented-features that make it impossible to finish the game. Other stuff that's missing is sound and savegames.

Compiling:
--------
You need SDL-1.2.2 (maybe older versions will work), and a supported compiler. At the moment only GCC and Microsoft Visual C++ are supported.

GCC:
----
* Type make (or gmake if that's what GNU make is called on your system) and hopefully ScummVM will compile for you.

Please note that big endian support is preliminary and contains bugs.


Microsoft Visual C++:
---------------------
* Open the workspace, scummwm.dsw
* If you want to compile the GDI version, remove sdl.cpp from the project and add windows.cpp.
  for the SDL version, remove windows.cpp from the project and add sdl.cpp.
* Enter the path to the SDL include files in Tools|Options|Directories
* Now it should compile successfully.


Running:
--------
Before you run the engine, you need to put the game's datafiles in the same directory as the scummvm executable. The filenames must be in lowercase (monkey2.000 and monkey2.001).

Good Luck,
Ludvig Strigeus











