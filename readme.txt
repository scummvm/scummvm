ScummVM README
Last updated:    2002-04-05
Release version: 0.2.0
------------------------------------------------------------------------

For more information, the latest release and progress reports, please visit
the ScummVM home page at: http://scummvm.sourceforge.net/

About:
------

ScummVM is an implementation of the SCUMM (Script Creation Utility for
Maniac Mansion) engine used in various Lucas Arts games such as Monkey
Island and Day of the Tentacle. At this time ScummVM should be considered
ALPHA software, as it's still under heavy development. Be aware that while
many games will work with few major bugs, crashes can happen. Also note
that saved games can, and probably will, be incompatible between releases.

Supported Games:
----------------

At the moment the following games have been reported to work, and should
be playable to the end:

        Loom (256 color CD version)
	Monkey Island 1 (CD version)
	Monkey Island 2
	Indiana Jones And The Fate Of Atlantis
	Day Of The Tentacle
        Sam & Max

The following games should load, but are not yet fully playable. If you want
the latest updates on game compatability, visit our website and view the
compatability chart.

	Zak McKraken (256 color FM Towns version)
	Indiana Jones and the Last Crusade (256 color version)
        Full Throttle
        The Dig

The following games are SCUMM engine, but NOT supported by ScummVM (yet).

	Maniac Mansion
	Zak McKraken (16 color floppy version)
	Monkey Island (16 color floppy version)
	Loom (16 color floppy version)
	Curse of Monkey Island

Please be aware that the engine may contains bugs and non-implemented-
features that sometimes make it impossible to finish the game. Save often,
and please file a bug report (follow the link on the homepage) if you discover
such a bug. Attach a saved game if possible.

Supported Platforms:
-------------------
ScummVM has been ported to run on many platforms and operating systems.
Links to these ports can be found either on the ScummVM webpage or by a
google search. Many thanks to the effort of porters. If you have a port of
ScummVM and wish to commit it into the main CVS, feel free to contact us!

	Windows 	- SDL/GDI
	Windows CE 	- SDL		(iPaq and other handheld devices)
	Linux		- SDL/X11 	(includes iPaq devices running Linux)
	Macintosh	- SDL/CUSTOM	(Including Classic and Mac OS X)
	AmigaOS		- SDL/AGA
	MorphOS		- SDL
	Acorn (RiscOS)	- ???
	BeOS		- ???
	Dreamcast	- ???

Known Bugs:
-----------
This release has the following known bugs. There is no need to report them,
although patches to fix them are welcome.

        Sam and Max:
                - Subgames are not fully functional.
                - Game may freeze if you look at the Dragon Heart, or trip
                  an alarm, at Bumpusville
                - Some overlap may occur in graphics, expecially the intro
                - Music does not work with USE_ADLIB

       Day of the Tentacle:
                - German talkie version may crash when talking to Thomas
                  Jefferson.
       Loom (256 Talkie):
                - CD music and voices are not correctly syncronised
                - Some voices may be skipped altogether
                - Staff notes draw incorrectly

Reporting Bugs:
---------------
To report a bug, please create a SourceForge account and follow the bugs
link from our homepage. Please make sure the bug is reproducable, and 
still exists in the latest daily build/current CVS version.

Please include the following information:
		- Game version (PLEASE test the latest CVS/Daily build)
		- Bug details, including instructions on reproducing
		- Language of game (English, German, etc)
		- Version of game (Talkie, Floppy...)
		- Platform and Compiler (Win32, Linux, etc)
		- Attach a save game if possible

Compiling:
----------

You need SDL-1.2.2 (maybe older versions will work), and a supported
compiler. Several compilers, including GCC, mingw and Microsoft Visual C++
are supported. If you wish to use MP3-compressed CD tracks or .SOU files,
you will need to install the MAD library and define COMPRESSED_SOUND_FILE.
Tools for compressing .SOU files to .SO3 files can be found in the 'tools'
CVS module, or in the 'scummvm-tools' package.

	GCC:
	* Type make (or gmake if that's what GNU make is called on your
	  system) and hopefully ScummVM will compile for you.

	Microsoft Visual C++:
	* Open the workspace, scummwm.dsw
	* If you want to compile the GDI version, remove sdl.cpp from the
	  project and add windows.cpp. for the SDL version, remove
	  windows.cpp from the project and add sdl.cpp.
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
for *nix systems. To run Monkey Island from this install as a Windows user
you would make a shortcut with this command:

  C:\Games\LucasArts\scummvm.exe -f -pC:\Games\LucasArts\monkey\ monkey

Or, if you have, for example, Full Throttle on CD, and you CD drive is D:,
and you wish to disable subtitles and run in fullscreen:

  C:\Games\LucasArts\scummvm.exe -f -n -pD:\resource\ ft

The short game name you see at the end of the command line is very
important. You can get the current list of games and game names at:
http://scummvm.sourceforge.net/compatibility.php

Command Line Options:
---------------------

        scummvm [-f] [-n] [-p<path>] [-s<n>] [-m<n>] [-t<n>] [-b<n>] game

        -p<path>  - path to where the game is installed
	-b<num>   - start in that room
        -s<num>   - set the scale factor, 1-3    default is '2'
        -m<num>   - set the music volume, 0-100  default is '60'
        -t<num>   - set music tempo              default is '2031616'
        -e<num>   - select sound engine. see below.
        -g<num>   - select graphics scaler. see below.
	-f        - fullscreen mode
        -n        - disable subtitles
        -r        - enable roland conversion. Use this option if music doesn't sound correct.

In game Hot Keys:
-----------------

	Ctrl 0-9 and Shift 0-9 to load and save states.
	Ctrl-z quits
	Ctrl-f runs in fast mode.
	Ctrl-d starts the debugger.
	Ctrl-s shows memory consumption.
	F5 displays a save/load box.
	Space pauses.
	Alt-Enter toggles full screen (on *nix only)


Graphics filters:
-----------------
ScummVM offers several antialiasing filters to attempt to improve visual
quality. These are the same filters used in many other emulators, such as
MAME. They are:
        -g0 - No filtering. Fastest, obviously.
        -g1 - 2xSal
        -g2 - SuperSal. Less blurrier than 2x, but slightly slower
        -g3 - SuperEagle.

Note that at the moment filters are very slow, and slightly buggy. Use at 
your own risk! .... but feel free to submit a bugfix or optimisation patch :)


Music and Sound: 
----------------
By default, on most operating systems, ScummVM will automatically use ADLIB
emulation. However, some games do not include adlib music - such as Sam and
Max. To hear music in such games, you will need to remove the 'USE_ADLIB'
define and recompile using Midi support.. (TODO: Make ADLIB and MIDI dynamic!)

If you ARE using MIDI, you have several different choices of output, 
depending on your operating system and configuration.

        -e0 - Null output. Don't play any music. This is default on non-windows operating systems
        -e1 - Windows output. This uses the builtin Windows sequencer. Naturally, only on Windows
        -e4 - Quicktime sound, for Macintosh users.


Playing sound with Adlib emulation:
-----------------------------------
If you compile ScummVM with the USE_ADLIB flag, an Adlib card will be
emulated and ScummVM will output the music as sampled waves. This offers
far superior preformance to Timidity, however does not work with some games
such as Sam and Max.


Playing sound with Sequencer:
-----------------------------
If your soundcard driver supports a sequencer, you may set the environment
variable "SCUMMVM_MIDI" to your sequencer device - eg, /dev/sequencer

Then start ScummVM with the parameter '-e3'. This should work on several
cards, and may offer better preformance and quality than Adlib emulation.


Playing music with Timidity:
----------------------------
Start Timidity with the following command line :
	$ timidity -irv 7777
Then start ScummVM with the parameter '-e2'. However, Timidity is not designed
to cope with the rapid changes most iMUSE equipped games use - so Adlib emulation
or sequencer support are the recommended methods for non-windows/non-macintosh 
users.


Savegames:
----------

Savegames are by default put in the current directory. You can use the
environment variable SCUMMVM_SAVEPATH to specify where to put save games.
Don't forget the trailing directory separator. Also be aware that saved games
can, and probably WILL, break between ScummVM releases.

Bash Example:
        export SCUMMVM_SAVEPATH=/tmp/scummvm_savegames/

Windows example:
        set SCUMMVM_SAVEPATH=C:\saved_games\


------------------------------------------------------------------------
Good Luck and Happy Adventuring!
The ScummVM team.
http://scummvm.sourceforge.net/
------------------------------------------------------------------------
