# Command line interface

## Launching ScummVM

### Usage

````bash
scummvm [OPTIONS] [GAME]
````

### [GAME]

[GAME] is the game id of the game you wish to launch. For example, 'monkey' for Monkey Island.

The game id may be the default game id, or a user configured id set either directly in the game-specific section of the configuration file or in the Launcher [game settings window](using/game_settings).

### [OPTIONS]

[OPTIONS] refers to any of the following options:

````bash
-v, --version            Display ScummVM version information and exit
-h, --help               Display a brief help text and exit
-z, --list-games         Display list of supported games and exit
-t, --list-targets       Display list of configured targets and exit
--list-saves=TARGET      Display a list of saved games for the game (TARGET) specified
-a, --add                Add all games from current or specified directory.
                           If --game=ID is passed only the game with id ID is added. See also --detect
                           Use --path=PATH before -a, --add to specify a directory.
--detect                 Display a list of games with their ID from current or
                           specified directory without adding it to the config.
                           Use --path=PATH before --detect to specify a directory.
--game=ID                In combination with --add or --detect only adds or attempts to
                           detect the game with id ID.
--auto-detect            Display a list of games from current or specified directory
                           and start the first one. Use --path=PATH before --auto-detect
                           to specify a directory.
--recursive              In combination with --add or --detect recurse down all subdirectories

-c, --config=CONFIG      Use alternate configuration file
-p, --path=PATH          Path to where the game is installed
-x, --save-slot[=NUM]    Save game slot to load (default: autosave)
-f, --fullscreen         Force full-screen mode
-F, --no-fullscreen      Force windowed mode
-g, --gfx-mode=MODE      Select graphics scaler (1x,2x,3x,2xsai,super2xsai,
                           supereagle,advmame2x,advmame3x,hq2x,hq3x,tv2x,
                           dotmatrix)
--filtering              Force filtered graphics mode
--no-filtering           Force unfiltered graphics mode
--gui-theme=THEME        Select GUI theme
--themepath=PATH         Path to where GUI themes are stored
--list-themes            Display list of all usable GUI themes
-e, --music-driver=MODE  Select music driver (see README for details)
--list-audio-devices     List all available audio devices
-q, --language=LANG      Select language (en,de,fr,it,pt,es,jp,zh,kr,se,gb,
                           hb,ru,cz)
-m, --music-volume=NUM   Set the music volume, 0-255 (default: 192)
-s, --sfx-volume=NUM     Set the sfx volume, 0-255 (default: 192)
-r, --speech-volume=NUM  Set the speech volume, 0-255 (default: 192)
--midi-gain=NUM          Set the gain for MIDI playback, 0-1000 (default:
                           100) (only supported by some MIDI drivers)
-n, --subtitles          Enable subtitles (use with games that have voice)
-b, --boot-param=NUM     Pass number to the boot script (boot param)
-d, --debuglevel=NUM     Set debug verbosity level
--debugflags=FLAGS       Enable engine specific debug flags
                           (separated by commas)
--debug-channels-only    Show only the specified debug channels
-u, --dump-scripts       Enable script dumping if a directory called 'dumps'
                           exists in the current directory

--cdrom=DRIVE            CD drive to play CD audio from; can either be a
                           drive, path, or numeric index (default: 0 = best
                           choice drive)
--joystick[=NUM]         Enable joystick input (default: 0 = first joystick)
--platform=WORD          Specify platform of game (allowed values: 2gs, 3do,
                           acorn, amiga, atari, c64, fmtowns, nes, mac, pc, pc98,
                           pce, segacd, wii, windows)
--savepath=PATH          Path to where saved games are stored
--extrapath=PATH         Extra path to additional game data
--soundfont=FILE         Select the SoundFont for MIDI playback (only
                           supported by some MIDI drivers)
--multi-midi             Enable combination AdLib and native MIDI
--native-mt32            True Roland MT-32 (disable GM emulation)
--enable-gs              Enable Roland GS mode for MIDI playback
--output-rate=RATE       Select output sample rate in Hz (e.g. 22050)
--opl-driver=DRIVER      Select AdLib (OPL) emulator (db, mame)
--aspect-ratio           Enable aspect ratio correction
--render-mode=MODE       Enable additional render modes (hercGreen, hercAmber,
                           cga, ega, vga, amiga, fmtowns, pc9821, pc9801, 2gs,
                           atari, macintosh)

--alt-intro              Use alternative intro for CD versions of Beneath a
                           Steel Sky and Flight of the Amazon Queen
--copy-protection        Enable copy protection in SCUMM games, when
                           ScummVM disables it by default.
--talkspeed=NUM          Set talk speed for games (default: 60)
--demo-mode              Start demo mode of Maniac Mansion or The 7th Guest
--tempo=NUM              Set music tempo (in percent, 50-200) for SCUMM games
                           (default: 100)
--dimuse-tempo=NUM       Set internal Digital iMuse tempo (10 - 100) per second
                           (default: 10)
````

The meaning of most long options, which are those options starting with a double-dash, can be inverted by prefixing them with ```no-```. For example, ```--no-aspect-ratio``` will turn aspect ratio correction off. This is useful if you want to override a setting in the configuration file.

## Launching saved games

### --list-saves

This option is used to display a list of the current saved games of the specified target game and the corresponding save slots. 

#### Usage

````bash
scummvm --list-saves=TARGET
````

Engines which currently support ```--list-saves``` are:

- AGI
- AGOS
- BLADERUNNER
- CGE
- CINE
- CRUISE
- CRYOMNI3D
- DRACI
- GROOVIE
- HUGO
- KYRA
- LURE
- MOHAWK
- PARALLACTION
- QUEEN
- SAGA
- SCI
- SCUMM
- SKY
- SWORD1
- SWORD2
- TEENAGENT
- TINSEL
- TITANIC
- TOON
- TOUCHE
- TSAGE
- TUCKER
- ZVISION

### --save-slot <a id="save_slot"></a>

This option is used to load a saved game directly from the command line.

#### Usage 

```--save-slot[SLOT]``` or ```-x[SLOT]```, where [SLOT] is the save slot number.

Engines which currently support ```--save-slot``` are:

- AGI
- BLADERUNNER
- CGE
- CINE
- CRUISE
- CRYOMNI3D
- DRACI
- GROOVIE
- HUGO
- KYRA
- LURE
- MOHAWK
- QUEEN
- SAGA
- SCI
- SCUMM
- SKY
- SWORD1
- SWORD2
- TEENAGENT
- TINSEL
- TITANIC
- TOON
- TOUCHE
- TSAGE
- TUCKER
- ZVISION

## Examples
<!-- tabs:start -->

### ** Windows **

Running Monkey Island, fullscreen, from a hard disk:

````bash
C:\Games\LucasArts\scummvm.exe -f -pC:\Games\LucasArts\monkey\monkey
````

Running Full Throttle from CD, fullscreen and with subtitles enabled: 

````bash
C:\Games\LucasArts\scummvm.exe -f -n -pD:\resource\ft
````

### ** Unix **
Running Monkey Island, fullscreen, from a hard disk:

````bash
/path/to/scummvm -f -p/games/LucasArts/monkey/monkey 
````

Running Full Throttle from CD, fullscreen and with subtitles enabled: 

````bash
/path/to/scummvm -f -n -p/cdrom/resource/ft
````
<!-- tabs:end -->
