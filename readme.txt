ScummVM README
last updated: 2002-03-07
------------------------------------------------------------------------

For more info, please visit the ScummVM home page at:
http://scummvm.sourceforge.net

ScummVM is an implementation of the SCUMM engine used in various Lucas
Arts games such as Monkey Island and Day of the Tentacle. At this time
ScummVM should be considered ALPHA software. 

At the moment the following games have been reported to work, and should
be playable to the end:

	Monkey Island 1 (CD version)
	Monkey Island 2
	Indiana Jones And The Fate Of Atlantis
	Day Of The Tentacle
        Sam & Max

The following games should load, but are not yet in a playable form.

	Loom (256 color CD version)
	Zak McKraken (256 color FM towns version)
	Indiana Jones and the Last Crusade (256 color version)
        Full Throttle
        The Dig

Please be aware that the engine may contains bugs and non-implemented-
features that sometimes make it impossible to finish the game. Please file a
bug report (follow the link on the homepage) if you discover such a bug. Attach
a saved game if possible.

Known Bugs:
----------
This release has the following known bugs. There is no need to report them,
although patches to fix them are welcome.

        Sam and Max:
                - Subgames are not fully functional.
                - Game may freeze if you look at the Dragon Heart, or trip
                  an alarm, at Bumpusville
                - Some overlap may occur in graphics, expecially the intro
                - Music does not work with USE_ADLIB

       Monkey 1 (CD):
                - CD Music is not yet working.

Compiling:
----------

You need SDL-1.2.2 (maybe older versions will work), and a supported
compiler. At the moment only GCC and Microsoft Visual C++ are
supported.

GCC:
----

* Type make (or gmake if that's what GNU make is called on your system)
and hopefully ScummVM will compile for you.

Microsoft Visual C++:
---------------------

* Open the workspace, scummwm.dsw
* If you want to compile the GDI version, remove sdl.cpp from the
  project and add windows.cpp. for the SDL version, remove windows.cpp
  from the project and add sdl.cpp.
* Enter the path to the SDL include files in Tools|Options|Directories
* Now it should compile successfully.

Running:
--------

Before you run the engine, you need to put the game's datafiles in a
directory. The filenames must be in lowercase on *nix systems
(monkey2.000 and monkey2.001). If you use a game with speech, the file
monster.sou must reside in the same directory as scummvm.

You can either place the scummvm executable in directory in your path,
or place it one dir up from the games install dir.
Here is a good example installation directory structure.

<root>/Games/LucasArts/
|
+-- scummvm
|
+-- momkey/
|
+-- samnmax/

In this installation, <root> is either C: for Windows user, or /usr/
for *nix systems. To run samnmax from this install as a Windows user
you would make a shortcut with this command:

  C:\Games\LucasArts\scummvm.exe -f -p C:\Games\LucasArts\monkey\ monkey

The short game name you see at the end of the command line is very
important. You can get the current list of games and game names at:
http://scummvm.sourceforge.net/compatibility.php

Command Line Options:

        scummvm [-f] [-n] [-p<path>] [-s<n>] [-m<n>] [-t<n>] [-b<n>] game

        -p<path>  - path to where the game is installed
	-b<num>   - start in that room
        -s<num>   - set the scale factor, 1-3    default is '2'
        -m<num>   - set the music volume, 0-100  default is '60'
        -t<num>   - set music tempo              default is '2031616'
	-f        - fullscreen mode
        -n        - disable subtitles

In game Keys:

	Ctrl 0-9 and Shift 0-9 to load and save states.
	Ctrl-z quits
	Ctrl-f runs in fast mode.
	Ctrl-d starts the debugger.
	Ctrl-s shows memory consumption.
	F5 displays a save/load box.
	Space pauses.
	Alt-Enter toggles full screen (on *nix only)

Savegames:
----------

Savegames are by default put in the current directory. You can use the
environment variable SCUMMVM_SAVEPATH to specify where to put save games.
Don't forget the trailing directory separator.

Bash Example:
        export SCUMMVM_SAVEPATH=/tmp/scummvm_savegames/

Windows example:
        set SCUMMVM_SAVEPATH=C:\saved_games\

Playing sound with Timidity (*nix only):
----------------------------------------

Start Timidity with the following command line :

$ timidity -irv 7777

Then just start ScummVM and you should have sound.
In order to use timidity, you need to compile ScummVM with USE_TIMIDITY.

If you compile ScummVM with the USE_ADLIB flag, an Adlib card will be
emulated and ScummVM will output the music as sampled waves.
(doesn't work with Sam&Max)

------------------------------------------------------------------------
Good Luck,
The ScummVM team.
