ScummVM README
Last updated:    2002-04-15
Release version: 0.2.0  [CVS build]
------------------------------------------------------------------------

For more information, compatability lists, details on donating, the latest
release, progress reports and more, please visit the ScummVM home page
at: http://scummvm.sourceforge.net/

About:
------

ScummVM is an implementation of the SCUMM (Script Creation Utility for
Maniac Mansion) engine used in various Lucas Arts games such as Monkey
Island and Day of the Tentacle. At this time ScummVM should be considered
ALPHA software, as it's still under heavy development. Be aware that while
many games will work with few major bugs, crashes can happen. Also note
that saved games can, and probably will, be incompatible between releases.

Also ScummVM is capable of playing several non-SCUMM games, at the moment
this includes Simon The Sorcerer.

If you enjoy ScummVM feel free to donates using the PayPal button on the
ScummVM homepage.


Supported Games:
----------------

At the moment the following games have been reported to work, and should
be playable to the end:

        Loom (256 color CD version)            [Game: loomcd]
        Monkey Island 1 (CD version)           [Game: monkey1]
        Monkey Island 2                        [Game: monkey2]
        Indiana Jones And The Fate Of Atlantis [Game: atlantis]
        Day Of The Tentacle                    [Game: tentacle]
        Sam & Max                              [Game: samnmax]
        Simon The Sorcerer                     [Game: simon1dos or simon1win]


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
and please file a bug report (details on submitted bug reports are below)
if you discover such a bug.

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
although patches to fix them are welcome. If you discover a bug that is not
listed here, nor in the compatability table on the website, please see below.

        Sam and Max:
                - Subgames are not fully functional.
                - Game may freeze if you look at the Dragon Heart, or trip
                  an alarm, at Bumpusville
                - Some overlap may occur in graphics, expecially the intro
                - Music does not work in adlib mode, use midi

       Loom (256 Talkie):
                - CD music and voices are not always syncronised
                - Actor masks are not yet used - actors will always appear
                  'on top' of everything.
                - Some voices may be skipped altogether
                - Staff notes draw incorrectly
                - Copy data files from CD to harddisk. Windows (XP at least)
                  cannot correctly access data from a volume playing music.


Reporting Bugs:
---------------

To report a bug, please create a SourceForge account and follow the bugs
link from our homepage. Please make sure the bug is reproducable, and 
still exists in the latest daily build/current CVS version. Also check the
compatability listing for that game to ensure the issue is not already known.

Please include the following information:
		- Game version (PLEASE test the latest CVS/Daily build)
		- Bug details, including instructions on reproducing
		- Language of game (English, German, etc)
		- Version of game (Talkie, Floppy...)
		- Platform and Compiler (Win32, Linux, etc)
		- Attach a save game if possible
                - If this bug only occured recently, please note the last
                  version without the bug, and the first version including
                  the bug. That way we can fix it quicker by looking at the
                  changes made.


Compiling:
----------

You need SDL-1.2.2 (older versions may work, but are unsupported), and a
supported compiler. Several compilers, including GCC, mingw and Microsoft
Visual C++ are supported. If you wish to use MP3-compressed CD tracks or
.SOU files, you will need to install the MAD library and define
COMPRESSED_SOUND_FILE. Tools for compressing .SOU files to .SO3 files can be
found in the 'tools' CVS module, or in the 'scummvm-tools' package.

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

For the adventurous, who like to live on the edge... you can download daily
builds of Win32, Redhat, Debian and other packages here:
        Daily builds - http://scummvm.sourceforge.net/daily/


Command Line Options:
---------------------

        scummvm [-f] [-n] [-p<path>] [-s<n>] [-m<n>] [-t<n>] [-b<n>] game

        -p<path>  - path to where the game is installed
	-b<num>   - start in that room
        -c<num>   - drive to play cd audio from. eg, 0 is first drive
        -s<num>   - set the scale factor, 1-3    default is '2'
        -m<num>   - set the music volume, 0-100  default is '60'
        -t<num>   - set music tempo              default is '2031616'
        -e<mode>   - select sound engine. see below.
        -g<mode>  - select graphics scaler. see below.
	-f        - fullscreen mode
        -n        - disable subtitles
        -r        - enable roland conversion. try if music sounds incorrect.


In game Hot Keys:
-----------------
	Ctrl 0-9 and Shift 0-9 to load and save states.
        Ctrl-Alt 0-4 cycles between filter modes in the order listed below
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
        -gnormal     - No filtering, original 320x200 resolution. Fastest.
        -g2x         - No filtering, double screen/window size to 640x480
        -g3x         - No filtering, triple screen/window size to 800x600
        -g2xsai      - 2xsai filtering, double screen/window size to 640x480
        -gsuper2xsai - Enhanced 2xsai filtering. 640x480 screen/window size
        -gsupereagle - Less blurry than 2xsai, but slower. Also 640x480

Note that filters are very slow when ScummVM is compiled in a debug
configuration without optimisations. And there is always a speed impact when
using any form of antialiasing/linear filtering.


Music and Sound: 
----------------
By default, on most operating systems, ScummVM will automatically use ADLIB
emulation. However, some games do not include adlib music - such as Sam and
Max. ScummVM will automatically switch to MIDI for these games - however note
that MIDI may not be available on all operating systems or may need manual
configuration.

If you ARE using MIDI, you have several different choices of output, 
depending on your operating system and configuration.
        
	-eadlib     - Uses internal Adlib Emulation (default) 
        -ewindows   - Windows MIDI. Uses builtin sequencer. Only on Windows
	-etimidity  - Uses Timidity for music. Requires Timidity.
	-eseq       - Uses /dev/sequencer for MIDI. *nix only.
        -eqt        - Quicktime sound, for Macintosh users.
	-eamidi     - Uses the MorphOS MIDI system. MorphOS only.
	-enull      - Null output. Don't play any music.
	

Playing sound with Adlib emulation:
-----------------------------------
By default an Adlib card will be emulated and ScummVM will output the music
as sampled waves. This offers far superior preformance to Timidity, however
does not work with some games such as Sam and Max.

Playing sound with MIDI:
------------------------
On Windows, MorphOS and Macintosh, MIDI will automatically be configured.
However under UNIX, you may need to configure MIDI manually. Please see the
below sections for information on using the various MIDI methods.

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
Then start ScummVM with the parameter '-e2'. However, Timidity is not
designed to cope with the rapid changes most iMUSE equipped games use - so
its use over Adlib emulation or sequencer support is not recommended.



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


Coding style:
------------
For ScummVM coding, we use hugging braces, and two-space tab indents.
We occasionally run the following 'indent' parameters to ensure everything
is kept standard:

-br -bap -nbc -lp -ce -cdw -brs -nbad -nbc -npsl -nip -ts2 -ncs -nbs
-npcs -nbap -Tbyte -Tvoid -Tuint32 -Tuint8 -Tuint16 -Tint -Tint8
-Tint16 -Tint32 -TArrayHeader -TMemBlkHeader -TVerbSlot -TObjectData
-TImageHeader -TRoomHeader -TCodeHeader -TResHdr -TBompHeader
-TMidiChannelAdl -TGui -TScumm -TSoundEngine -TPart -TPlayer


------------------------------------------------------------------------
Good Luck and Happy Adventuring!
The ScummVM team.
http://scummvm.sourceforge.net/
------------------------------------------------------------------------
