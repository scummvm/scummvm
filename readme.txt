2001-11-6

ScummVM is an implementation of the SCUMM engine used in various Lucas Arts games
such as Monkey Island and Day of the Tentacle.

At the moment the following games have been reported to work:
Monkey Island 1 (CD version)
Monkey Island 2
Indiana Jones And The Fate Of Atlantis
Day Of The Tentacle
Sam & Max (partially)

Please be aware that the engine contains bugs and non-implemented-features that
make it impossible to finish the game. Other stuff that's missing is music.

Compiling:
----------
You need SDL-1.2.2 (maybe older versions will work), and a supported compiler. At
the moment only GCC and Microsoft Visual C++ are supported.

GCC:
----
* Type make (or gmake if that's what GNU make is called on your system) and
hopefully ScummVM will compile for you.

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

You can use Ctrl 0-9 and Shift 0-9 to load and save states.
Ctrl-z quits, and Ctrl-f runs in fast mode.
Ctrl-d starts the debugger.
Ctrl-s shows memory consumption.


Playing sound with Timidity:
----------------------------
Start Timidity with the following command line :

$ timidity -irv 7777

Then just start ScummVM and you should have sound.


Good Luck,
Ludvig Strigeus











