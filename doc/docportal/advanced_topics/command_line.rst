
=======================
Command line interface
=======================

Launching ScummVM from the command line lets you pass a number of options, some of which override the applicable global or game-specific settings. It also lets you launch straight into a game without going to the Launcher first.


Usage
=========

.. code-block::

    scummvm [option] [game]

- By default, executing ``scummvm`` on the command line starts the Launcher.
- Specify a game or use ``--auto-detect`` to start a game directly.
- Some options, such as ``--help`` or ``--list-audio-devices``, allow you to view information about ScummVM directly on the command line without starting the Launcher or starting a game.


Starting a game
===============

To start a game on the command line, three options are possible.

For games that have previously been added to ScummVM:

.. code::

    scummvm <target>

For any supported games, even if they have not been added to ScummVM:

.. code::

    scummvm -p <path to game files> <game id>

or

.. code::

    scummvm -p <path to game files> --auto-detect

In all cases additional options can be specified on the command line, for example to specify the graphics mode or which saved game to load.

The ``target`` is a name assigned to a game added to ScummVM. It is also known as the ID in the GUI, and can be changed in the :ref:`Game Options <id>`. In the configuration file, this is the game name in square brackets. If the same game is added to ScummVM twice, it will have two different ``target`` names, and they can be configured differently. To see the list of configured targets:

.. code::

    scummvm --list-targets

The ``game id`` is a unique identifier for any game supported by ScummVM. To see a list of all supported games for your ScummVM release:

.. code::

    scummvm --list-games

Specify a ``target`` on the command line to use settings already configured for that target, unless different options are specified on the command line. There is no need to specify the path for the game files since this is already part of the target configuration.

Use the ``game id`` or ``--auto-detect`` to start games with default settings for any settings not specified on the command line. The path to the game file needs to be specified.

The full ``game id`` contains both an engine name and a game name. For example, the game id for Full Throttle is ``scumm:ft``, where ``scumm`` is the engine name. In many cases where there is no ambiguity, you can omit the engine name when specifying a ``game id`` on the command line. For example, both ``scumm:ft`` and ``ft`` can be used. However, if there is an ambiguity with the game name, the full ``game id`` must be used. For example, for Indiana Jones and the Fate of Atlantis only ``scumm:atlantis`` can be used as there would otherwise be an ambiguity with ``cryomni3d:atlantis``.

.. tab-set::

    .. tab-item:: Windows

        .. card::

            The following examples assume that the path to ScummVM is ``C:\Program Files\ScummVM\scummvm.exe``.

        To run Monkey Island, fullscreen (``-f``), if the game has already been added to ScummVM under the `target` name `monkey1`:

        .. code-block::

            C:\Program Files\ScummVM\scummvm.exe -f monkey1



        To run Full Throttle, fullscreen (``-f``) and with subtitles enabled (``-n``), specifying the path to the game on a CD (``-p``):

        .. code-block::

            C:\Program Files\ScummVM\scummvm.exe -f -n -p "D:\resource" scumm:ft

    .. tab-item:: macOS

        .. card::

            The following examples assume the path to the ScummVM app is ``Applications/ScummVM.app``.

        If you are unsure of the path to the app, drag the ScummVM app icon into the Terminal window. This prints the path to the ScummVM app.

        To run ScummVM from the command line, add ``/Contents/MacOS/scummvm`` to that path.

        To run Monkey Island, fullscreen (``-f``), if the game has already been added to ScummVM under the `target` name `monkey1`:

        .. code::

            /Applications/ScummVM.app/Contents/MacOS/scummvm -f monkey1


        To run Full Throttle, fullscreen (``-f``) and with subtitles enabled (``-n``), specifying the path to the game on a CD (``-p``):

        .. code::

            /Applications/ScummVM.app/Contents/MacOS/scummvm -f -n -p "/Volumes/Full Throttle/resource" scumm:ft

    .. tab-item:: Linux

        .. card::

            The following examples assume the path to ScummVM is ``/usr/games/scummvm``.

        To run Monkey Island, fullscreen (``-f``), if the game has already been added to ScummVM under the `target` name `monkey1`:

        .. code::

            /usr/games/scummvm -f monkey1


        To run Full Throttle, fullscreen (``-f``) and with subtitles enabled (``-n``), specifying the path to the game on a CD (``-p``):

        .. code::

            /usr/games/scummvm -f -n -p /cdrom/resource scumm:ft

        .. tip::

            If ScummVM is on PATH, there is no need to enter the full path to ScummVM. Similarly, if a game is already added to the Launcher, there is no need to specify the path to the game.


Options
========

The meaning of most options can be inverted by prefixing them with ``no-``. For example, ``--no-aspect-ratio`` will turn aspect ratio correction off. This does not apply to short options.

Option parameters are shown below in capitals, and need to be replaced with real values.

Short options are listed where they are available.

.. csv-table::
    :widths: 25 10 45 15
    :class: command
    :header-rows: 1

		Option,Short,Description,Default
        ``--add``,``-a``,"Adds all games from current or specified directory. If ``--game=ID`` is passed, only the game with specified ID is added. See also ``--detect``. Use ``--path=PATH`` before ``-a`` or ``--add`` to specify a directory.",
        ``--alt-intro``, ,":ref:`Uses alternative intro for CD versions <altintro>`, Sky and Queen engines only",false
        ``--aspect-ratio``,,":ref:`Enables aspect ratio correction <ratio>`",false
        ``--auto-detect``,,"Displays a list of games from the current or specified directory and starts the first game. Use ``--path=PATH`` before ``--auto-detect`` to specify a directory",
        ``--boot-param=NUM``,``-b``,"Pass number to the boot script (`boot param <https://wiki.scummvm.org/index.php/Boot_Params>`_).",0
        ``--cdrom=DRIVE``,,"Sets the CD drive to play CD audio from. This can be a drive, path, or numeric index",0
        ``--config=FILE``,``-c``,"Uses alternate configuration file",
        ``--console``,,"Enables the console window. Win32 and Symbian32 only.",true
        ``--copy-protection``,,"Enables copy protection",false
        ``--debug-channels-only``,,"Shows only the specified debug channels",
        ``--debugflags=FLAGS``,,"Enables engine specific debug flags",
        ``--debuglevel=NUM``,``-d``,"Sets debug verbosity level",0
        ``--demo-mode``,,"Starts demo mode of Maniac Mansion or The 7th Guest",false
        ``--detect``,,"Displays a list of games with their game id from the current or specified directory. This does not add the game to the games list. Use ``--path=PATH`` before ``--detect`` to specify a directory.",
        ``--dirtyrects``,, Enables dirty rectangles optimisation in software renderer,true
    	``--disable-display``,,Disables any graphics output. Use for headless events playback by `Event Recorder <https://wiki.scummvm.org/index.php/Event_Recorder>`_ ,false
        ``--dump-midi``,, "Dumps MIDI events to 'dump.mid' while game is running. Overwrites file if it already exists.",false
        ``--dump-scripts``,``-u``,"Enables script dumping if a directory called 'dumps' exists in the current directory",false
        ``--enable-gs``,,":ref:`Enables Roland GS mode for MIDI playback <gs>`",false
        ``--engine=ID``,,"In combination with ``--list-games`` or ``--list-all-games`` only lists games for this engine",
        ``--engine-speed=NUM``,,"Sets frame-per-second limit for Grim Fandango or Escape from Monkey Island. 0 is no limit. Allowed values 0 - 100", 60
        ``--extrapath=PATH``,,":ref:`Extra path to additional game data <extra>`",
        ``--filtering``,,":ref:`Forces filtered graphics mode <filtering>`",false
        ``--fullscreen``,``-f``,":ref:`Forces full-screen mode <fullscreen>`",false
        ``--game=ID``,,"In combination with ``--add`` or ``--detect`` only adds or attempts to detect the game with specified ID.",
        ``--gfx-mode``,``-g``,":ref:`Selects graphics mode <gfxmode>`",normal
        ``--gui-theme=THEME``,,":ref:`Selects GUI theme <theme>`",
        ``--help``,``-h``,"Displays a brief help text and exit",
        ``--iconspath=PATH``,,":ref:`Path to additional icons for the launcher grid view <iconspath>`",
        ``--initial-cfg=FILE``,``-i``,"Loads an initial configuration file if no configuration file has been saved yet.",
        ``--joystick=NUM``,,"Enables joystick input.",0
        ``--language``,``-q``,":ref:`Selects language <lang>`. Allowed values: en, de, fr, it, pt, es, jp, zh, kr, se, gb, hb, ru, cz",en
        ``--list-all-debugflags``,,"Lists all debug flags",
        ``--list-all-engines``,,"Lists all detection engines, then exits",
        ``--list-audio-devices``,,"Lists all available audio devices",
        ``--list-debugflags=engine``,,"Lists engine-specific debug flags. If ``engine=global`` or engine is not specified, then it lists global debug flags.",
        ``--list-engines``,,"Lists supported engines, then exits",
        ``--list-games``,``-z``, "Lists supported games, then exits.",
        ``--list-records=TARGET``,, "Lists recordings for the specified target (`Event Recorder <https://wiki.scummvm.org/index.php/Event_Recorder>`_)",
        ``--list-saves --game=TARGET``,,"Displays a list of saved games for the game specified, or for all targets if no game specified.",
        ``--list-targets``,``-t``,"Lists configured targets, then exits",
        ``--list-themes``,,"Lists all usable GUI themes",
        ``--logfile=PATH``,``-l``,"Uses alternate path for log file (SDL backend only).",
        ``--md5``,,"Shows MD5 hash of the file given by ``--md5-path=PATH``. If ``--md5-length=NUM`` is passed then it shows the MD5 hash of the first or last ``NUM`` bytes of the file given by ``PATH``. If ``--md5-engine=ENGINE_ID`` option is passed then it auto-calculates the required bytes and its hash, overriding ``--md5-length``",
        ``--md5mac``,,"Shows MD5 hash for both the resource fork and data fork of the file given by ``--md5-path=PATH``. If ``--md5-length=NUM`` is passed then it shows the MD5 hash of the first or last``NUM`` bytes of each fork.",
        ``--md5-engine=ENGINE_ID``,,"Used with ``--md5`` to specify the engine for which number of bytes to be hashed must be calculated. This option overrides ``--md5-length`` if used along with it. Use ``--list-engines`` to find all engine IDs.",
        ``--md5-length=NUM``,,"Used with ``--md5`` or ``--md5mac`` to specify the number of bytes to be hashed.If ``NUM`` is 0, MD5 hash of the whole file is calculated. If ``NUM`` is negative, the MD5 hash is calculated from the tail. Is overriden if passed with ``--md5-engine`` option",0
        ``--md5-path=PATH``,,"Used with ``--md5`` or ``--md5mac`` to specify path of file to calculate MD5 hash of", ./scummvm
        ``--midi-gain=NUM``,,":ref:`Sets the gain for MIDI playback <gain>` Only supported by some MIDI drivers. 0-1000",100
        ``--multi-midi``,,":ref:`Enables combination AdLib and native MIDI <multi>`",false
        ``--music-driver=MODE``,``-e``,":ref:`Selects preferred music device <device>`",auto
        ``--music-volume=NUM``,``-m``,":ref:`Sets the music volume <music>`, 0-255",192
    	``--native-mt32``,,":ref:`True Roland MT-32 (disables GM emulation) <mt32>`",false
        ``--no-fullscreen``,``-F``,"Forces windowed mode",
        ``--opl-driver=DRIVER``,,":ref:`Selects AdLib (OPL) emulator <opl>`",
        ``--output-channels=CHANNELS``,,"Select output channel count, for example, 2 for stereo.",
        ``--output-rate=RATE``,,"Selects output sample rate in Hz, for example 22050Hz.",
        ``--path=PATH``,``-p``,"Sets path to where the game is installed",
        ``--platform=STRING``,,":ref: `Specifies platform of game <platform>`
        Allowed values:

        - 2gs
        - 3do
        - acorn
        - amiga
        - atari
        - c64
        - fmtowns
        - nes
        - mac
        - pc
        - pc98
        - pce
        - segacd
        - wii
        - windows",
        ``--random-seed=SEED``,,":ref:`Sets the random seed used to initialize entropy <seed>`",
        ``--record-file-name=FILE``,,"Specifies recorded file name (`Event Recorder <https://wiki.scummvm.org/index.php/Event_Recorder>`_)",record.bin
        ``--record-mode=MODE``,,"Specifies record mode for `Event Recorder <https://wiki.scummvm.org/index.php/Event_Recorder>`_. Allowed values: record, playback, info, update, passthrough.", none
        ``--recursive``,,"In combination with ``--add or ``--detect`` recurses down all subdirectories",
        ``--renderer=RENDERER``,,"Selects 3D renderer. Allowed values: software, opengl, opengl_shaders",
        ``--render-mode=MODE``,,":ref:`Enables additional render modes <render>`.
        Allowed values:

        - hercGreen
        - hercAmber
        - cga
        - ega
        - vga
        - amiga
        - fmtowns
        - pc9821
        - pc9801
        - 2gs
        - atari
        - macintosh
        - macintoshbwdefault", default
        ``--save-slot=NUM``,``-x``,"Specifies the saved game slot to load", 0 (autosave)
        ``--savepath=PATH``,,":ref:`Specifies path to where saved games are stored <savepath>`",
        ``--scale-factor=FACTOR``,,"Specifies the factor to scale the graphics by",
        ``--scaler=MODE``,, "Selects graphics scaler.
        Allowed values:

        - normal
        - hq
        - edge
        - advmame
        - sai
        - supersai
        - supereagle
        - pm
        - dotmatrix
        - tv",default
        ``--screenshotpath=PATH``,,"Specify path where screenshot files are created. SDL backend only.",
        ``--screenshot-period=NUM``,,"When recording, triggers a screenshot every NUM milliseconds.(`Event Recorder <https://wiki.scummvm.org/index.php/Event_Recorder>`_)",60000
        ``--sfx-volume=NUM``,``-s``,":ref:`Sets the sfx volume <sfx>`, 0-255",192
    	``--show-fps``,,Turns on frames-per-second information in 3D games,false
        ``--soundfont=FILE``,,":ref:`Selects the SoundFont for MIDI playback. <soundfont>`. Only supported bysome MIDI drivers.",
        ``--speech-volume=NUM``,``-r``,":ref:`Sets the speech volume <speechvol>`, 0-255",192
        ``--start-movie=NAME@NUM``,,"Starts Director movie at specified frame. Either can be specified without the other.",
        ``--stretch-mode=MODE``,, "Selects stretch mode.
        Allowed values:

        - center
        - pixel-perfect
        - even-pixels
        - fit
        - stretch
        - fit_force_aspect",
        ``--subtitles``,``-n``,":ref:`Enables subtitles  <speechmute>`",
        ``--talkspeed=NUM``,,":ref:`Sets talk speed for games <talkspeed>`",60
        ``--tempo=NUM``,,"Sets music tempo (in percent, 50-200) for SCUMM games.",100
        ``--themepath=PATH``,,":ref:`Specifies path to where GUI themes are stored <themepath>`",
        ``--version``,``-v``,"Displays ScummVM version information, then exits.",
        "``--window-size=W,H``",,"Sets the ScummVM window size to the specified dimensions. OpenGL only.",
