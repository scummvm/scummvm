ScummVM README
Last updated:    2002-05-01
Release version: 0.2.0 [CVS Build]
------------------------------------------------------------------------

For more information, compatibility lists, details on donating, the latest
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

If you enjoy ScummVM feel free to donate using the PayPal button on the
ScummVM homepage. This will help us buy utilities needed to develop ScummVM
easier and quicker. If you cannot donate, help and contribute a patch!

Contacting:
-----------
The easiest way to contact the ScummVM team is by subitting bug reports or
commenting in our forums. You can also join and e-mail the scummvm-devel
mailing list, or chat with us on irc (#scummvm, irc.openprojects.net)

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


The following games should load, but are not yet fully playable. Play these at
your own risk, and please do not file bug reports about them. If you want
the latest updates on game compatibility, visit our web site and view the
compatibility chart.

        Monkey Island 1 (VGA floppy)                   [Game: monkeyvga]
        Zak McKraken (256 color FM Towns version)      [Game: zak256]
        Indiana Jones and the Last Crusade (256 color) [Game: indy3]
        Full Throttle                                  [Game: ft]
        The Dig                                        [Game: dig]

The following games are SCUMM engine, but NOT supported by ScummVM (yet).

        Maniac Mansion
        Zak McKraken (16 color floppy version)
        Monkey Island 1 (EGA, 16 color floppy version)
        Loom (16 color floppy version)
        Curse of Monkey Island

Please be aware that the engine may contains bugs and non-implemented-
features that sometimes make it impossible to finish the game. Save often,
and please file a bug report (details on submitted bug reports are below)
if you discover such a bug.


Supported Platforms:
-------------------

ScummVM has been ported to run on many platforms and operating systems.
Links to these ports can be found either on the ScummVM web page or by a
Google search. Many thanks to the effort of porters. If you have a port of
ScummVM and wish to commit it into the main CVS, feel free to contact us!

        Windows         - SDL
        Windows CE      - SDL           (iPaq and other handheld devices)
        Linux           - SDL/X11       (includes iPaq devices running Linux)
        Macintosh       - SDL/CUSTOM    (Including Classic and Mac OS X)
        AmigaOS         - SDL/AGA
        MorphOS         - SDL
        BeOS            - SDL
        Acorn (RiscOS)  - ???
        Dreamcast       - ???


Known Bugs:
-----------

This release has the following known bugs. There is no need to report them,
although patches to fix them are welcome. If you discover a bug that is not
listed here, nor in the compatibility table on the web site, please see below.

       Indy 4 (Fate Of Atlantis):
                - Keyboard fighting does not work. ScummVM now defaults to
                  using mouse fighting.



       Sam and Max:
                - Subgames are not fully functional.

                - The Wak-A-Rat game will appear to freeze at the end.
                  However, it hasn't really. Just hit Escape and wait.

                - The intro credits will not play correctly if you let the
                  previous cutscene run (the Mad Doctor scene).

                - The Mystery Vortex may hang the game if you hang around in
                  the main room for too long. This is caused by a script race



       Loom (256 Talkie):
                - If you are having random crashes, this is a Windows bug.
                  Try copying the data files from CD to your hard disk.

                - Bobbin may rarely appear to walk backwards.


Reporting Bugs:
---------------
To report a bug, please create a SourceForge account and follow the bugs
link from our homepage. Please make sure the bug is reproducible, and 
still exists in the latest daily build/current CVS version. Also check the
compatibility listing for that game to ensure the issue is not already known.

Please include the following information:
        - Game version (PLEASE test the latest CVS/Daily build)
        - Bug details, including instructions on reproducing
        - Language of game (English, German, etc)
        - Version of game (Talkie, Floppy...)
        - Platform and Compiler (Win32, Linux, etc)
        - Attach a save game if possible
        - If this bug only occurred recently, please note the last
          version without the bug, and the first version including
          the bug. That way we can fix it quicker by looking at the
          changes made.


Compiling:
----------

You need SDL-1.2.2 or newer (older versions may work, but are unsupported), and
a supported compiler. Several compilers, including GCC, mingw and Microsoft
Visual C++ are supported. If you wish to use MP3-compressed CD tracks or
.SOU files, you will need to install the MAD library and define
COMPRESSED_SOUND_FILE. Tools for compressing .SOU files to .SO3 files can be
found in the 'tools' CVS module, or in the 'scummvm-tools' package.

On Win9x/NT/XP you can define WIN_DBG and attach WinDbg to browse the debug 
messages (see http://www.sysinternals.com/ntw2k/freeware/debugview.shtml).

        GCC:
        * Type make (or gmake if that's what GNU make is called on your
          system) and hopefully ScummVM will compile for you.

        Microsoft Visual C++:
        * Open the workspace, scummwm.dsw
        * Enter the path to the SDL include files in Tools|Options|Directories
        * Now it should compile successfully.

        BeOS:
        * Open the 'scummvm.proj' file in BeIDE. Compile as normal.

        PocketPC - Windows CE:
        * Download the SDLAudio library:
          http://arisme.free.fr/PocketScumm/sources/SDLAudio-1.2.3-src.zip
        * Open and compile the SDLAudio WCEBuild/WCEBuild workspace in EVC++
        * Open the ScummVM wince/PocketScumm workspace
        * Enter the SDLAudio directory to your includes path
        * Enter the compiled SDLAudio.lib to your link libraries list
        * Now it should compile successfully


Running:
--------

Before you run the engine, you need to put the game's datafiles in a
directory. The filenames must be in lowercase on *nix systems
(monkey2.000 and monkey2.001). If you use a game with speech, the file
monster.sou must reside in the same directory as the datafiles.

For example, to run a copy of Monkey Island installed in C:\Games\LucasArts
under Windows, you would make a shortcut calling this command:

  C:\Games\LucasArts\scummvm.exe -f -pC:\Games\LucasArts\monkey\ monkey

Or, if you have, for example, Full Throttle on CD, and your CD drive is D:,
and you wish to disable subtitles and run in fullscreen:

  C:\Games\LucasArts\scummvm.exe -f -n -pD:\resource\ ft

Note that if you run the game once this way, and specify the -w commandline
parameter (or edit scummvm.ini manually), ScummVM will remember the path,
and other settings for this game.

The short game name you see at the end of the command line is very
important. A short list is contained at the top of this file. You can also
get the current list of games and game names at:
        http://scummvm.sourceforge.net/compatibility.php

For the adventurous, who like to live on the edge... you can download daily
builds of Win32, Redhat, Debian and other packages here:
        Daily builds - http://scummvm.sourceforge.net/daily/


Command Line Options:
---------------------

        scummvm [OPTIONS] [GAME]

        [GAME]     - Short name of game to load. E.g. monkey for Monkey Island.
        
        -p<path>   - Path to where the game is installed. Default is Cwd.
        -b<num>    - Start in room <num>.
        -c<num>    - Drive to play cd audio from.  E.g., 0 is first drive.
        -s<num>    - Set the sfx volume, 0-255.    Default is '100'
        -m<num>    - Set the music volume, 0-100.  Default is '60'
        -t<num>    - Set music tempo. Defaults: Adlib: 0x1F0000, Midi: 0x460000
        -e<mode>   - Select sound engine. See below.
        -g<mode>   - Select graphics scaler. See below.
        -f         - Full-screen mode.
        -n         - Disable subtitles. Use with games that have voice.
        -r         - Enable Roland conversion. Try if music sounds incorrect.
        -a         - Enable amiga pal conversion, for playing Amiga versions
        -d[<num>]  - Set debug verbosity to <num>
        -w[<file>] - Write configuration file
        -l<file>   - Load alternate configration file (default: scummvm.ini)

In game Hot Keys:
-----------------
        Ctrl 0-9 and Shift 0-9   - load and save states.
        Ctrl-Alt 1-7             - cycles between graphics filters
        Ctrl-z                   - quits
        Ctrl-f                   - runs in fast mode.
        Ctrl-d                   - starts the debugger.
        Ctrl-s                   - shows memory consumption.
        F5                       - displays a save/load box.
        Space                    - pauses.
        Alt-Enter                - toggles full screen/windowed


Graphics filters:
-----------------
ScummVM offers several anti-aliasing filters to attempt to improve visual
quality. These are the same filters used in many other emulators, such as
MAME.

They are:
        -gnormal     - No filtering, original 320x200 resolution. Fastest.
        -g2x         - No filtering, double screen/window size to 640x480 (default)
        -g3x         - No filtering, triple screen/window size to 800x600
        -g2xsai      - 2xsai filtering, double screen/window size to 640x480
        -gsuper2xsai - Enhanced 2xsai filtering. 640x480 screen/window size
        -gsupereagle - Less blurry than 2xsai, but slower. Also 640x480
        -gadvmame2x  - 640x480 scaling. Doesn't rely on blurring like 2xSAI.

Note that filters are very slow when ScummVM is compiled in a debug
configuration without optimizations. And there is always a speed impact when
using any form of anti-aliasing/linear filtering.


Autosaves:
----------

Because ScummVM is still a beta product, it -can- crash and/or hang
occasionally. As such, every five minutes it will save a game in Slot 0.


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


Music and Sound: 
----------------
By default, on most operating systems, ScummVM will automatically use ADLIB
emulation. However, some games do not include Adlib music - such as Sam and
Max. Note: MIDI may not be available on all operating systems or may need
manual configuration.

If you ARE using MIDI, you have several different choices of output, 
depending on your operating system and configuration.
        
        -eadlib     - Uses internal Adlib Emulation (default) 
        -ewindows   - Windows MIDI. Uses built-in sequencer, for Windows users
        -eseq       - Uses /dev/sequencer for MIDI, *nix users. See below.
        -eqt        - Quicktime sound, for Macintosh users.
        -ecore      - CoreAudio sound, for MacOS X users.
        -eamidi     - Uses the MorphOS MIDI system, for MorphOS users
        -enull      - Null output. Don't play any music.
        

Playing sound with Adlib emulation:
-----------------------------------
By default an Adlib card will be emulated and ScummVM will output the music
as sampled waves. This offers far superior performance to Timidity, however
does not work with some games such as Sam and Max.


Playing sound with MIDI:
------------------------
Use the appropriate -e<mode> command line option from the list above to
select your preferred MIDI device. For example, if you wish to use Windows
MIDI over the default Adlib Emulation, use the -ewindows option.


Playing sound with Sequencer:
-----------------------------
If your soundcard driver supports a sequencer, you may set the environment
variable "SCUMMVM_MIDI" to your sequencer device - eg, /dev/sequencer

Then start ScummVM with the parameter '-eseq'. This should work on several
cards, and may offer better performance and quality than Adlib emulation.

Using MP3 files for CD audio:
-----------------------------
Use LAME or some other mp3 encoder to rip the cd audio tracks to files. Name
the files track1.mp3 track2.mp3 etc. ScummVM must be compiled with MAD support
to use this option. You'll need to rip the file from the CD as a WAV file,
then encode the MP3 files in Constant Bit Rate sampled at 22 kHz. This can
be done with the following LAME command line:

lame -t -q 0 -b 96 --resample 22.05 track1.wav track1.mp3


Compressing MONSTER.SOU with MP3:.
---------------------------------
You need LAME, and our extract util from the tools dir to perform this task,
and ScummVM must be compiled with MAD support.

Make a backup file of your MONSTER.SOU before attempting this. Copy your
MONSTER.SOU file to a temporary folder. Then run:
  extract monster.sou
In about 30 minutes or so, you will have a much smaller monster.so3 file, copy
this file to your game dir. You can safely remove the monter.sou file.


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


Credits:
--------
  The core ScummVM team:
        Ludvig Strigeus    - Original developer, lead developer 'simon' module
        Vincent Hamm       - Current lead developer, ScummVM
        James Brown        - Co-lead developer, ScummVM
        Jeremy Newman      - Webmaster

 Porters:
        Lionel Ulmer       - X11/Linux port
        Nicolas Bacca      - PocketPC/WinCE port
        Mutwin Kraus       - Macintosh port
        Max Horn           - Macintosh port
        Marcus Comstedt    - DreamCast port
        Ruediger Hanke     - MorphOS port

 Contributors:
        Claudio Matsuoka   - Daily builds (http://scummvm.sf.net/daily/)
        Janne Huttunen     - Zak256/Indy256/LoomCD actor mask support
        Jeroen Janssen     - Numerous readability and bugfix patches
        Gregory Montoir    - AdvanceMAME Scale-2X implementation
        Mikesch Nepomuk    - MI1 VGA Floppy patches.
        Edward Rudd        - Fixes for playing MP3 versions of MI1/Loom Audio
        Daniel Schepler    - Final MI1 CD music support
        Tim 'realmz'       - Initial MI1 CD music support
        Jonathan 'khalek'  - Expert weaver in the Loom
        Nicolas Noble      - Config file support
        Pawel Kolodziejski - Added missing Dig SMUSH codecs

        And to all the contributors, users, and beta testers we've missed.
        Thanks!

 Special thanks to:
        Jimmi Thogersen - For ScummRev, and much obscure code/documentation
        Kevin Carnes    - For Scumm16, the basis of ScummVM older gfx codecs

        Aric Wilmunder, Ron Gilbert, David Fox, Vince Lee, and all those at
        LucasFilm/LucasArts who made SCUMM the insane mess to reimplement
        that it is today. Feel free to drop us a line and tell us what you
        think, guys!




------------------------------------------------------------------------
Good Luck and Happy Adventuring!
The ScummVM team.
http://scummvm.sourceforge.net/
------------------------------------------------------------------------
