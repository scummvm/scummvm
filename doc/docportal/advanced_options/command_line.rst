
=======================
Command line interface
=======================

Launching ScummVM from the command line allows you to pass a number of options, some of which override the applicable global or game-specific settings.

Some other options, such as ``--help`` or ``--list-audio-devices``, allow you to view information about ScummVM directly on the command line without opening the Launcher or starting a game. 

Short options are listed where they are available. 

.. csv-table:: 
  	:header-rows: 1
    :width: 100% 
    
		Option,Short,Description
        \--add,-a,"Add all games from current or specified directory.

        If \--game=ID is passed only the game with id ID is added. 

        See also \--detect. 

        Use \--path=PATH before -a, \--add to specify a directory."
        \--alt-intro, ,":ref:`Use alternative intro for CD versions <altintro>`"
        \--aspect-ratio,,":ref:`Enable aspect ratio correction <ratio>`"
        \--auto-detect,,"Display a list of games from current or specified directory 
        
        and start the first one. 

        Use \--path=PATH before \--auto-detect to specify a directory."
        \--boot-param=NUM,-b,"Pass number to the boot script (boot param)"
        \--cdrom=DRIVE,,"CD drive to play CD audio from; 

        can either be a drive, path, or numeric index 

        (default: 0 = best choice drive)"
        \--config=FILE,-c,"Use alternate configuration file"
        \--copy-protection,,"Enable copy protection"
        \--debug-channels-only,,"Show only the specified debug channels"
        \--debugflags=FLAGS,,"Enable engine specific debug flags"
        \--debuglevel=NUM,-d,"Set debug verbosity level"
        \--demo-mode,,"Start demo mode of Maniac Mansion or The 7th Guest"
        \--detect,,"Display a list of games with their ID from current or specified directory 

        without adding it to the config. 

        Use \--path=PATH before \--detect to specify a directory."
        \--dimuse-tempo=NUM,,"Set internal Digital iMuse tempo (10 - 100) per second (default: 10)"
        \--dump-scripts,-u,"Enable script dumping if a directory called 
        
        'dumps' exists in the current directory"
        \--enable-gs,,"Enable Roland GS mode for MIDI playback"
        \--extrapath=PATH,,":ref:`Extra path to additional game data <extra>`"
        \--filtering,,":ref:`Force filtered graphics mode <filtering>`"
        \--fullscreen,-f,":ref:`Force full-screen mode <fullscreen>`"
        \--game=ID,,"In combination with \--add or \--detect 

        only adds or attempts to detect the game with id ID."
        \--gfx-mode,-g,":ref:`Select graphics scaler <gfxmode>`"
        \--gui-theme=THEME,,":ref:`Select GUI theme <theme>`"
        \--help,-h,"Display a brief help text and exit"
        \--joystick[=NUM],,"Enable joystick input (default: 0 = first joystick)"
        \--language,-q,":ref:`Select language <lang>` (en,de,fr,it,

        pt,es,jp,zh,kr,se,gb,hb,ru,cz)"
        \--list-audio-devices,,"List all available audio devices"
        \--list-saves --game=TARGET,,"Display a list of saved games for the game (TARGET) specified"
        \--list-targets,-t,"Display list of configured targets and exit"
        \--list-themes,,"Display list of all usable GUI themes"
        --midi-gain=NUM,,":ref:`Set the gain for MIDI playback <gain>`

        0-1000 (default: 100)

        Only supported by some MIDI drivers"
        \--multi-midi,,":ref:`Enable combination AdLib and native MIDI <multi>`"
        \--music-driver=MODE,-e,":ref:`Select preferred music device <device>`"
        \--music-volume=NUM,-m,":ref:`Set the music volume <music>`, 0-255 (default: 192)"
        \--native-mt32,,":ref:`True Roland MT-32 (disable GM emulation) <mt32>`"
        \--no-filtering,,"Force unfiltered graphics mode"
        \--no-fullscreen,-F,"Force windowed mode"
        \--opl-driver=DRIVER,,":ref:`Select AdLib (OPL) emulator <opl>`" 
        \--output-rate=RATE,,"Select output sample rate in Hz" 
        \--path=PATH,-p,"Path to where the game is installed"
        \--platform=STRING,,":ref:`Specify platform of game <platform>`
        (allowed values: 2gs, 3do, acorn, 

        amiga, atari, c64, fmtowns, nes, mac, pc pc98, 

        pce, segacd, wii, windows)"
        \--recursive,,"In combination with \--add or \--detect recurse down all subdirectories"
        \--render-mode=MODE,,":ref:`Enable additional render modes <render>`"
        \--save-slot=NUM,-x,"Saved game slot to load (default: autosave)"
        \--savepath=PATH,,":ref:`Path to where saved games are stored <savepath>`"
        \--sfx-volume=NUM,-s,":ref:`Set the sfx volume <sfx>`, 0-255 (default: 192)"
        \--soundfont=FILE,,":ref:`Select the SoundFont for MIDI playback. <soundfont>` 
        
        Only supported by some MIDI drivers."
        \--speech-volume=NUM,-r,":ref:`Set the speech volume <speechvol>`, 0-255 (default: 192)"
        \--subtitles,-n,":ref:`Enable subtitles  <speechmute>`"
        \--talkspeed=NUM,,":ref:`Set talk speed for games <talkspeed>` (default: 60)"
        \--tempo=NUM,,"Set music tempo (in percent, 50-200) for SCUMM games (default: 100)"
        \--themepath=PATH,,"Path to where GUI themes are stored"
        \--version,-v,"Display ScummVM version information and exit"

The meaning of most long options can be inverted by prefixing them with ``no-``. For example, ``--no-aspect-ratio`` will turn aspect ratio correction off. 


Example
------------

.. tabs::

    .. tab::

        **Windows**

        Running Monkey Island, fullscreen, from a hard disk:

        .. code:: 

            C:\Games\LucasArts\scummvm.exe -f -pC:\Games\LucasArts\monkey\monkey
       

        Running Full Throttle from CD, fullscreen and with subtitles enabled: 

        .. code::

            C:\Games\LucasArts\scummvm.exe -f -n -pD:\resource\ft

    .. tab::

        **Linux/Unix**
            
        Running Monkey Island, fullscreen, from a hard disk:

        .. code::

            /path/to/scummvm -f -p/games/LucasArts/monkey/monkey 


        Running Full Throttle from CD, fullscreen and with subtitles enabled: 

        .. code::

            /path/to/scummvm -f -n -p/cdrom/resource/ft

