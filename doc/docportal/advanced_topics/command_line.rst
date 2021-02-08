
=======================
Command line interface
=======================

Launching ScummVM from the command line lets you pass a number of options, some of which override the applicable global or game-specific settings. It also lets you launch straight into a game without going to the Launcher first. 

Some options, such as ``--help`` or ``--list-audio-devices``, allow you to view information about ScummVM directly on the command line without starting the Launcher or starting a game. 

Usage
=========

.. code-block::

    scummvm [option] [game]

To launch ScummVM, enter the path to ScummVM. Optionally, follow this with any of the options documented below, and/or the path to the game you wish to start. If a game has already been added to the games list in the Launcher, you can pass the game id instead of the path to the game. 


.. tabbed:: Windows
    
    .. panels::
        :column: col-lg-12

        The following examples assume that the path to ScummVM is ``C:\Program Files\ScummVM\scummvm.exe``. 

        ^^^^

        To run Monkey Island, fullscreen (``-f``):

        .. code-block:: 

            C:\Program Files\ScummVM\scummvm.exe -f monkey
        
    

        To run Full Throttle, fullscreen (``-f``) and with subtitles enabled (``-n``), specifying the path to the game on a CD (``-p``): 

        .. code-block::

            C:\Program Files\ScummVM\scummvm.exe -f -n -pD:``resource``ft

.. tabbed:: macOS
    
    .. panels::
        :column: col-lg-12

        The following examples assume the path to the ScummVM app is ``Applications/ScummVM.app``. 
        
        If you are unsure of the path to the app, drag the ScummVM app icon into the Terminal window. This prints the path to the ScummVM app. 
        
        To run ScummVM from the command line, add ``/Contents/MacOS/scummvm`` to that path. 

        ^^^^

        To run Monkey Island, fullscreen (``-f``):

        .. code::

            /Applications/ScummVM.app/Contents/MacOS/scummvm -f monkey 


        To run Full Throttle, fullscreen (``-f``) and with subtitles enabled (``-n``), specifying the path to the game on a CD (``-p``): 

        .. code::

            /Applications/ScummVM.app/Contents/MacOS/scummvm -f -n -p/cdrom/resource/ft

.. tabbed:: Linux
    
    .. panels::
        :column: col-lg-12

        The following examples assume the path to ScummVM is ``/usr/games/scummvm``.
        
        ^^^^^^

        To run Monkey Island, fullscreen (``-f``):
        
        .. code::

            /usr/games/scummvm -f monkey 


        To run Full Throttle, fullscreen (``-f``) and with subtitles enabled (``-n``), specifying the path to the game on a CD (``-p``):

        .. code::

            /usr/games/scummvm -f -n -p/cdrom/resource/ft

        .. tip:: 

            If ScummVM is on PATH, there is no need to enter the full path to ScummVM. Similarly, if a game is already added to the Launcher, there is no need to specify the path to the game.

        
Options
========

The meaning of most options can be inverted by prefixing them with ``no-``. For example, ``--no-aspect-ratio`` will turn aspect ratio correction off. This does not apply to short options. 

Option parameters are shown below in capitals, and need to be replaced with real values.

Short options are listed where they are available. 

.. csv-table:: 
    :widths: 35 10 55
    :header-rows: 1
    
		Option,Short,Description
        ``--add``,``-a``,"Adds all games from current or specified directory. 
        
    If ``--game=NAME`` is passed, only the game with id NAME is added. See also ``--detect``. Use ``--path=PATH`` before ``-a`` or ``--add`` to specify a directory."
        ``--alt-intro``, ,":ref:`Uses alternative intro for CD versions <altintro>`"
        ``--aspect-ratio``,,":ref:`Enables aspect ratio correction <ratio>`"
        ``--auto-detect``,,"Displays a list of games from the current or specified directory and starts the first game. Use ``--path=PATH`` before ``--auto-detect`` to specify a directory."
        ``--boot-param=NUM``,``-b``,"Pass number to the boot script (`boot param <https://wiki.scummvm.org/index.php/Boot_Params>`_)."
        ``--cdrom=DRIVE``,,"Sets the CD drive to play CD audio from. This can be a drive, path, or numeric index (default: 0)"
        ``--config=FILE``,``-c``,"Uses alternate configuration file"
        ``--console``,,"Enables the console window (default: enabled). Win32 and Symbian32 only." 
        ``--copy-protection``,,"Enables copy protection"
        ``--debug-channels-only``,,"Shows only the specified debug channels"
        ``--debugflags=FLAGS``,,"Enables engine specific debug flags"
        ``--debuglevel=NUM``,``-d``,"Sets debug verbosity level"
        ``--demo-mode``,,"Starts demo mode of Maniac Mansion or The 7th Guest"
        ``--detect``,,"Displays a list of games with their game id from the current or specified directory. This does not add the game to the games list. Use ``--path=PATH`` before ``--detect`` to specify a directory."
        ``--dimuse-tempo=NUM``,,"Sets internal Digital iMuse tempo (10 - 100) per second (default: 10)"
        ``--engine-speed=NUM``,,"Sets frames per second limit (0 - 100) for Grim Fandango or Escape from Monkey Island (default: 60)."  
        ``--dump-scripts``,``-u``,"Enables script dumping if a directory called 'dumps' exists in the current directory"
        ``--enable-gs``,,":ref:`Enables Roland GS mode for MIDI playback <gs>`"
        ``--extrapath=PATH``,,":ref:`Extra path to additional game data <extra>`"
        ``--filtering``,,":ref:`Forces filtered graphics mode <filtering>`"
        ``--fullscreen``,``-f``,":ref:`Forces full-screen mode <fullscreen>`"
        ``--game=NAME``,,"In combination with ``--add`` or ``--detect`` only adds or attempts to detect the :ref:`game with id NAME <gameid>`."
        ``--gfx-mode``,``-g``,":ref:`Selects graphics mode <gfxmode>`"
        ``--gui-theme=THEME``,,":ref:`Selects GUI theme <theme>`"
        ``--help``,``-h``,"Displays a brief help text and exit"
        ``--joystick=NUM``,,"Enables joystick input (default: 0 = first joystick)"
        ``--language``,``-q``,":ref:`Selects language <lang>` (en, de, fr, it, pt, es, jp, zh, kr, se, gb, hb, ru, cz)"
        ``--list-audio-devices``,,"Lists all available audio devices"
        ``--list-saves --game=TARGET``,,"Displays a list of saved games for the game (TARGET) specified"
        ``--list-targets``,``-t``,"Displays list of configured targets and exits"
        ``--list-themes``,,"Displays list of all usable GUI themes"
        ``--logfile=PATH``,-l,"Uses alternate path for log file (SDL backend only)." 
        ``--midi-gain=NUM``,,":ref:`Sets the gain for MIDI playback <gain>`. 0-1000 (default: 100). Only supported by some MIDI drivers"
        ``--multi-midi``,,":ref:`Enables combination AdLib and native MIDI <multi>`"
        ``--music-driver=MODE``,``-e``,":ref:`Selects preferred music device <device>`"
        ``--music-volume=NUM``,``-m``,":ref:`Sets the music volume <music>`, 0-255 (default: 192)"
        ``--native-mt32``,,":ref:`True Roland MT-32 (disables GM emulation) <mt32>`"
        ``--no-filtering``,,"Forces unfiltered graphics mode"
        ``--no-fullscreen``,``-F``,"Forces windowed mode"
        ``--opl-driver=DRIVER``,,":ref:`Selects AdLib (OPL) emulator <opl>`" 
        ``--output-rate=RATE``,,"Selects output sample rate in Hz" 
        ``--path=PATH``,``-p``,"Sets path to where the game is installed"
        ``--platform=STRING``,,":ref:`Specifes platform of game <platform>`. Allowed values: 2gs, 3do, acorn, amiga, atari, c64, fmtowns, nes, mac, pc pc98, pce, segacd, wii, windows."
        ``--recursive``,,"In combination with ``--add or ``--detect`` recurses down all subdirectories"
        ``--render-mode=MODE``,,":ref:`Enables additional render modes <render>`"
        ``--save-slot=NUM``,``-x``,"Specifies the saved game slot to load (default: autosave)"
        ``--savepath=PATH``,,":ref:`Specifies path to where saved games are stored <savepath>`"
        ``--sfx-volume=NUM``,``-s``,":ref:`Sets the sfx volume <sfx>`, 0-255 (default: 192)"
        ``--soundfont=FILE``,,":ref:`Selects the SoundFont for MIDI playback. <soundfont>`. Only supported by some MIDI drivers."
        ``--speech-volume=NUM``,``-r``,":ref:`Sets the speech volume <speechvol>`, 0-255 (default: 192)"
        ``--subtitles``,``-n``,":ref:`Enables subtitles  <speechmute>`"
        ``--talkspeed=NUM``,,":ref:`Sets talk speed for games <talkspeed>` (default: 60)"
        ``--tempo=NUM``,,"Sets music tempo (in percent, 50-200) for SCUMM games (default: 100)"
        ``--themepath=PATH``,,":ref:`Specifies path to where GUI themes are stored <themepath>`"
        ``--version``,``-v``,"Displays ScummVM version information and exits"




