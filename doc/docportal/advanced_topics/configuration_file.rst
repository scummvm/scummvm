
=======================
Configuration file
=======================

The configuration file provides a way to edit both global and game-specific settings. It is a text file containing configuration keys and parameters.

Settings are also accessible directly from the Launcher. See :doc:`../use_scummvm/how_to_settings`.

Location
==========

The configuration file saves to different default locations, depending on the platform. The configuration file path is also displayed on the :doc:`../settings/paths` tab.


.. tabbed:: Windows

	.. panels::
		:column: col-lg-12 mb-2

		``%APPDATA%\ScummVM\scummvm.ini``

		For Windows 95/98/ME, the file is at ``C:\WINDOWS\scummvm.ini``


.. tabbed:: macOS

	.. panels::
		:column: col-lg-12 mb-2

		``~/Library/Preferences/ScummVM Preferences``

		.. note::

			If an earlier version of ScummVM was installed on your system, the configuration file remains in the previous default location of ``~/.scummvmrc``.

		.. tip::

			To see the Library folder, press :kbd:`Option` when clicking **Go** in the Finder menu.


.. tabbed:: Linux

	.. panels::
		:column: col-lg-12 mb-2

		ScummVM follows the XDG Base Directory Specification, so by default the configuration file is found at ``~/.config/scummvm/scummvm.ini``, but its location might vary depending on the value of the ``XDG_CONFIG_HOME`` environment variable.

		If ScummVM was installed using Snap, the configuration file is found at ``~/snap/scummvm/current/.config/scummvm/scummvm.ini``

		.. note::

			``.config`` is a hidden directory. To view it use ``ls -a`` on the command line.




Using the configuration file
==================================

Global settings are listed under the ``[scummvm]`` heading. Global :doc:`keymaps settings <../settings/keymaps>` are listed under the ``[keymapper]`` heading. Game-specific settings, including keymaps, are listed under the heading for that game, for example ``[queen]`` for Flight of the Amazon Queen. Use the configuration keys to change settings.


Example of a configuration file
************************************

.. code::

    [scummvm]
    gfx_mode=supereagle
    fullscreen=true
    savepath=C:\saves\

    [sky]
    path=C:\games\SteelSky\

    [germansky]
    gameid=sky
    language=de
    path=C:\games\SteelSky\
    description=Beneath a Steel Sky w/ German subtitles

    [germandott]
    gameid=tentacle
    path=C:\german\tentacle\
    description=German version of DOTT

    [tentacle]
    path=C:\tentacle\
    subtitles=true
    music_volume=40
    sfx_volume=255

    [loomcd]
    cdrom=1
    path=C:\loom\
    talkspeed=5
    savepath=C:\loom\saves\

    [monkey2]
    path=C:\amiga_mi2\
    music_driver=windows

.. _configuration_keys:

Configuration keys
=====================

There are many recognized configuration keys. In the table below, each key is either linked to an explanatory description in the Settings pages, or has further information in the **Decription/Options** column.

.. csv-table::
  	:header-rows: 1
	:class: config

		Key,Type,Default,Description/Options
		alsa_port,integer,,Specifies which ALSA port ScummVM uses when using the ALSA music driver (Linux).
		":ref:`alt_intro <altintro>`",boolean,false,
		":ref:`altamigapalette <altamiga>`",boolean,false,
		":ref:`always_christmas <christmas>`",boolean,true,
		":ref:`antialiasing <antialiasing>`", integer,0,"0, 2, 4, 8"
		":ref:`apple2gs_speedmenu <2gs>`",boolean,false,
		":ref:`aspect_ratio <ratio>`",boolean,false,
		":ref:`audio_buffer_size <buffer>`",integer,"Calculated based on output sampling frequency to keep audio latency below 45ms.","Overrides the size of the audio buffer. Allowed values

	- 256
	- 512
	- 1024
	- 2048
	- 4096
	- 8192
	- 16384
	- 32768"
		":ref:`audio_override <aoverride>`",boolean,true,
		":ref:`auto_savenames <autoname>`",boolean,false,
		":ref:`autosave_period <autosave>`", integer, 300,
		auto_savenames,boolean,false, Automatically generates names for saved games
		":ref:`bilinear_filtering <bilinear>`",boolean,false,
		`boot_param <https://wiki.scummvm.org/index.php/Boot_Params>`_,integer,none,
		":ref:`bright_palette <bright>`",boolean,true,
		":ref:`camera_on_player <silencer>`",boolean,true,
		cdrom,integer,0, "Sets which CD drive to play CD audio from (as a numeric index). If a negative number is set, ScummVM does not access the CD drive."
		":ref:`cdromdelay <cdrom>`",boolean,,
		":ref:`cheat <cheat>`",boolean,false,
		":ref:`cheats <cheats>`",boolean,true,
		":ref:`color <color>`",boolean,,
		":ref:`commandpromptwindow <cmd>`",boolean,false,
		":ref:`confirm_exit <guiconfirm>`",boolean,false,
		console,boolean,true, Enables the console window for Win32/Symbian32.
		controller_map_db,string,"``gamecontrollerdb.txt`` is loaded from the specified extrapath", "Specifies the custom controller mapping file to load to complete the default database (SDL backend only)."
		copy_protection,boolean,false, Enables copy protection
		demo_mode,boolean,false, Starts demo mode of Maniac Mansion or the 7th Guest
		":ref:`correct_spanish_credits <spanishcredits>`",boolean,false,
		":ref:`credits_music <creditsmusic>`",boolean,false,
		":ref:`datausr_load <datausr>`",boolean,false,
		":ref:`debug <debugmode>`",boolean,false,
		":ref:`description <description>`",string,,
		desired_screen_aspect_ratio,string,auto,
		dimuse_tempo,integer,10,"Sets internal Digital iMuse tempo per second; 0 - 100"
		":ref:`disable_dithering <dither>`",boolean,false,
		":ref:`disable_stamina_drain <stamina>`",boolean,false,
		":ref:`displaytext <displaytext>`",boolean,true,
		":ref:`dos_music_tempo <dostempo>`",boolean,true,"If set to false, plays the Windows version music tempo."
		":ref:`DurableArmor <durable>`",boolean,false,
		":ref:`easier_ai <ai>`",boolean,false,
		":ref:`EasyMouse <easy>`",boolean,true,
		":ref:`enable_assets_mod <assets>`",boolean,true,
		":ref:`enable_bearded_musicians <beard>`",boolean,false,
		":ref:`enable_black_lined_video <blackline>`",,false,
		":ref:`enable_censoring <censor>`",boolean,false,
		":ref:`enable_color_blind <blind>`",boolean,false,
		":ref:`enable_enhancements <enhancements>`",boolean,true,
		":ref:`enable_font_antialiasing <fontantialias>`",boolean,true,
		":ref:`enable_gore <gore>`",boolean,,
		":ref:`enable_gs <gs>`",boolean,,
		":ref:`enable_high_resolution_graphics <hires>`",boolean,true,
		":ref:`enable_hq_video <hq>`",boolean,true,
		":ref:`enable_larryscale <larry>`",boolean,true,
		":ref:`enable_reporter <reporter>`",boolean,false,RISC OS only.
		":ref:`enable_video_upscale <upscale>`",boolean,true,
		":ref:`enable_tts <ttsenabled>`",boolean,false,
		enable_unsupported_game_warning,boolean,true, Shows a warning when adding a game that is unsupported.
		extra,string, ,"Shows additional information about a game, such as version"
		":ref:`english_speech <english>`",boolean,false,
		":ref:`extrapath <extra>`",string,None,
		":ref:`iconspath <iconspath>`",string,None,
		":ref:`infiniteAmmo <infA>`",boolean,false,
		":ref:`infiniteHealth <infH>`",boolean,false,
		":ref:`disable_fade_effects <fadeout>`",boolean,false,
		":ref:`doublefps <double>`",boolean,false,
		":ref:`fade_style <fade>`",boolean,true,
		":ref:`fast_movie_speed <fastmovie>`",boolean,false,
		":ref:`filtering <filtering>`",boolean,false,
		":ref:`floating_cursors <floating>`",boolean,false,
		":ref:`fluidsynth_chorus_activate <chact>`",boolean,true,
		":ref:`fluidsynth_chorus_depth <chdepth>`",integer,80,"- 0 - 210"
		":ref:`fluidsynth_chorus_level <chlevel>`",integer,100,"- 0 - 100"
		":ref:`fluidsynth_chorus_nr <chnr>`",integer,3,"- 0 - 99"
		":ref:`fluidsynth_chorus_speed <chspeed>`",integer,30,"- 10 - 500"
		":ref:`fluidsynth_chorus_waveform <chwave>`",string,Sine,"
	- sine
	- triangle"
		":ref:`fluidsynth_misc_interpolation <interp>`",string,4th,"
	- none
	- 4th
	- 7th
	- linear."
		":ref:`fluidsynth_reverb_activate <revact>`",boolean,true,
		":ref:`fluidsynth_reverb_damping <revdamp>`",integer,0,"- 0 - 1"
		":ref:`fluidsynth_reverb_level <revlevel>`",integer,90,"- 0 - 100"
		":ref:`fluidsynth_reverb_roomsize <revroom>`",integer,20,"- 0 - 100"
		":ref:`fluidsynth_reverb_width <revwidth>`",integer,1,"- 0 - 100"
		":ref:`font_antialiasing <fontantialias>`",boolean,false,
		":ref:`font_override <fontoverride>`",boolean,false,
		":ref:`footsteps <footsteps>`",boolean,true,
		":ref:`force_2d_renderer <2d>`",boolean,false,
		":ref:`frameLimit <framelimit>`",boolean,true,
		":ref:`frameSkip <frameskip>`",boolean,false,
		":ref:`frames_per_secondfl <fpsfl>`",boolean,false,
		":ref:`frontpanel_touchpad_mode <frontpanel>`",boolean, false
		":ref:`fullscreen <fullscreen>`",boolean,false,
		gameid,string,,"Short name of the game. For internal use only, do not edit."
		gamepath,string,,Specifies the path to the game
		":ref:`gfx_mode <gfxmode>`",string,normal (1x),"
	- 1x
	- 2x
	- 3x
	- 2xsai
	- super2xsai
	- supereagle
	- advmame2x
	- advmame3x
	- hq2x
	- hq3x
	- tv2x
	- dot-matrix
	- opengl"
		":ref:`gm_device <gm>`",string,null,"
	- auto
	- alsa
	- seq
	- sndio
	- fluidsynth
	- timidity"
		":ref:`gui_browser_native <guibrowser>`", boolean, true
		gui_browser_show_hidden,boolean,false, Shows hidden files/folders in the ScummVM file browser.
		gui_list_max_scan_entries,integer,-1, "Specifies the threshold for scanning directories in the Launcher. If the number of game entires exceeds the specified number, then scanning is skipped."
		":ref:`gui_return_to_launcher_at_exit <guireturn>`",boolean,false,
		gui_saveload_chooser,string,grid,"- list
	- grid"
		gui_saveload_last_pos,string,0,
		":ref:`gui_use_game_language <guilanguage>`",boolean, ,
		":ref:`helium_mode <helium>`",boolean,false,
		":ref:`help_style <help>`",boolean,false,
		":ref:`herculesfont <herc>`",boolean,false,
		":ref:`hpbargraphs <hp>`",boolean,true,
		":ref:`hypercheat <hyper>`",boolean,false,
		":ref:`iconspath <iconspath>`",string,,
		":ref:`improved <improved>`",boolean,true,
		":ref:`intro_music_digital <digitalmusic>`",boolean,true,
		":ref:`InvObjectsAnimated <objanimated>`",boolean,true,
		":ref:`joystick_deadzone <deadzone>`",integer, 3
		joystick_num,integer,0,Enables joystick input and selects which joystick to use. The default is the first joystick.
		":ref:`kbdmouse_speed <mousespeed>`", integer, 10
		":ref:`keymap_engine-default_DOWN <down>`",string,JOY_DOWN
		":ref:`keymap_engine-default_LCLK <LCLK>`",string,MOUSE_LEFT JOY_A
		":ref:`keymap_engine-default_LEFT <left>`",string,JOY_LEFT
		":ref:`keymap_engine-default_MCLK <MCLK>`",string,MOUSE_MIDDLE
		":ref:`keymap_engine-default_MENU <menu>`",string,F5 JOY_LEFT_SHOULDER
		":ref:`keymap_engine-default_PAUSE <pause>`",string,SPACE
		":ref:`keymap_engine-default_PIND <PIND>`",string,
		":ref:`keymap_engine-default_RCLK <RCLK>`",string,MOUSE_RIGHT JOY_B
		":ref:`keymap_engine-default_RETURN <RETURN>`",string,RETURN
		":ref:`keymap_engine-default_RIGHT <right>`",string,JOY_RIGHT
		":ref:`keymap_engine-default_SKIP <skip>`",string,ESCAPE JOY
		":ref:`keymap_engine-default_SKLI <SKLI>`",string,PERIOD JOY_X
		":ref:`keymap_engine-default_UP <up>`",string,JOY_UP
		":ref:`keymap_global_DEBUGGER <debug>`",string,C+A+d
		":ref:`keymap_global_MENU <gmm>`",string,C+F5 JOY_START,
		":ref:`keymap_global_MUTE <mute>`",string,C+u,
		":ref:`keymap_global_QUIT <globalquit>`",string,C+q,
		":ref:`keymap_global_VMOUSEDOWN <vmousedown>`",string,JOY_LEFT_STICK_Y+,
		":ref:`keymap_global_VMOUSELEFT <vmouseleft>`",string,JOY_LEFT_STICK_X-,
		":ref:`keymap_global_VMOUSERIGHT <vmouseright>`",string,JOY_LEFT_STICK_X+,
		":ref:`keymap_global_VMOUSESLOW <vmouseslow>`",string,JOY_RIGHT_SHOULDER,
		":ref:`keymap_global_VMOUSEUP <vmouseup>`",string,JOY_LEFT_STICK_Y-,
		":ref:`keymap_gui_CLOS <close>`",string,ESCAPE JOY_Y,
		":ref:`keymap_gui_DOWN <guidown>`",string,JOY_DOWN,
		":ref:`keymap_gui_INTRCT <interact>`",string,JOY_A,
		":ref:`keymap_gui_LEFT <guileft>`",string,
		":ref:`keymap_gui_RIGHT <guiright>`",string,JOY_RIGHT,
		":ref:`keymap_gui_UP <guiup>`",string,JOY_UP,
		":ref:`keymap_sdl-graphics_ASPT <ASPT>`",string,C+A+a,
		":ref:`keymap_sdl-graphics_CAPT <CAPT>`",string,C+m,
		":ref:`keymap_sdl-graphics_FILT <FILT>`",string,C+A+f
		":ref:`keymap_sdl-graphics_FLT1 <FLT1>`",string,C+A+1
		":ref:`keymap_sdl-graphics_FLT2 <FLT2>`",string,C+A+2
		":ref:`keymap_sdl-graphics_FLT3 <FLT3>`",string,C+A+3
		":ref:`keymap_sdl-graphics_FLT4 <FLT4>`",string,C+A+4
		":ref:`keymap_sdl-graphics_FLT5 <FLT5>`",string,C+A+5
		":ref:`keymap_sdl-graphics_FLT6 <FLT6>`",string, C+A+6
		":ref:`keymap_sdl-graphics_FLT7 <FLT7>`",string,C+A+7
		":ref:`keymap_sdl-graphics_FLT8 <FLT8>`",string,C+A+8
		":ref:`keymap_sdl-graphics_FULS <FULS>`",string,A+RETURN
		":ref:`keymap_sdl-graphics_SCL- <SCL>`",string,C+A+MINUS
		":ref:`keymap_sdl-graphics_SCL+ <SCL>`",string,C+A+PLUS
		":ref:`keymap_sdl-graphics_SCRS <SCRS>`",string,A+s
		":ref:`keymap_sdl-graphics_STCH <STCH>`",string,C+A+s
		":ref:`language <lang>`",string,,
		":ref:`local_server_port <serverport>`",integer,12345,
		":ref:`mac_v3_low_quality_music <macmusic>`",boolean,false,
		":ref:`midi_gain <gain>`",integer,,"- 0 - 1000"
		":ref:`midi_mode <midimode>`",string,,"- Standard
	- D110
	- FB01"
		":ref:`mm_nes_classic_palette <classic>`",boolean,false,
		":ref:`monotext <mono>`",boolean,true,
		":ref:`mouse <mouse>`",boolean,true,
		":ref:`mousebtswap <btswap>`",boolean,false,
		":ref:`mousesupport <support>`",boolean,true,
		":ref:`movie <movie>`",boolean,true,
		":ref:`mpegmovies <mpeg>`",boolean,true,
		":ref:`mt32_device <mt32>`",string,auto,"
	- auto
	- alsa
	- seq
	- fluidsynth
	- mt32
	- timidity "
		":ref:`multi_midi <multi>`",boolean,,
		":ref:`music_driver [scummvm] <device>`",string,auto,"
	- null
	- auto

	- seq (Unix)
	- sndio (Unix)
	- alsa (Unix)
	- CAMD (Amiga)
	- core (Mac)
	- coremidi (Mac - hardware)

	- windows (Windows)

	- fluidsynth
	- mt32
	- adlib
	- pcspk
	- pcjr
	- cms
	- timidity
	"
		"music_driver [game]",string, auto, "
	The same options as ``music_driver in [scummvm]`` plus:

	- towns
	- C64
	- pc98
	- segacd
	"
		music_mute,boolean,false, Mutes the game music.
		":ref:`music_volume <music>`",integer,192,"- 0-256 "
		":ref:`mute <mute>`",boolean,false,
		":ref:`native_mt32 <nativemt32>`",boolean,false,
		":ref:`NaughtyMode <naughty>`",boolean,true,
		":ref:`noanimwhileturning <noanim>`",boolean,false,
		":ref:`nodelaymillisfl <nodelay>`",boolean,false,
		":ref:`ntsc <ntsc>`",boolean,,
		":ref:`object_labels <labels>`",boolean,true,
		opl2lpt_parport,,null,
		":ref:`opl3_mode <opl3mode>`",boolean,false,
		":ref:`opl_driver <opl>`",string,,"
	- auto
	- mame
	- db
	- nuked
	- alsa
	- op2lpt
	- op3lpt
	- rwopl3 "
		":ref:`original_gui <originalgui>`",boolean,true,
		":ref:`original_menus <originalmenu>`",boolean,false,
		":ref:`originalsaveload <osl>`",boolean,false,
		":ref:`output_channels <outputchannels>`",integer,,"
	Supported values are:

	- 1
	- 2"
		":ref:`output_rate <outputrate>`",integer,,"
	Sensible values are:

	- 11025
	- 22050
	- 44100"
		":ref:`palette_mods <palette>`",boolean,false,
		":ref:`platform <platform>`",string,,
		":ref:`portaits_on <portraits>`",boolean,true,
		":ref:`prefer_digitalsfx <dsfx>`",boolean,true,
		":ref:`renderer <renderer>`",string,default,"
	- opengl
	- opengl_shaders
	- software"
		":ref:`render_mode <render>`",string,default,"
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
	- macintosh "
		":ref:`repeatwillihint <hint>`",boolean,,
		":ref:`restored <restored>`",boolean,true,
		":ref:`retrowaveopl3_bus <adlib>`",string,,"
	Specifies how the RetroWave OPL3 is connected:

	- serial (connected to a USB port using a PotatoPi)
	- spi (connected as a HAT using SPI) "
		":ref:`retrowaveopl3_disable_buffer <adlib>`",boolean,false,
		":ref:`retrowaveopl3_port <adlib>`",string,,"
	Specifies the serial port that the RetroWave OPL3 is connected to.
	For example:

	- COM3
	- ttyACM2 "
		":ref:`retrowaveopl3_spi_cs <adlib>`",string,,"Specifies the GPIO chip and line that the RetroWave OPL3 is connected to. Use the format <chip>,<line>."
		":ref:`rgb_rendering <rgb>`",boolean,false,
		":ref:`rootpath <rootpath>`",string,,
		":ref:`savepath <savepath>`",string,,
		save_slot,integer,autosave, Specifies the saved game slot to load
		":ref:`scalemakingofvideos <scale>`",boolean,false,
		":ref:`scanlines <scan>`",boolean,false,
		screenshotpath,string,See :ref:`screenshotpath <screenshotpath>`,Specifies where screenshots are saved
		":ref:`semi_smooth_scroll <semi>`",boolean,false,
		sfx_mute,boolean,false, Mutes the game sound effects.
		":ref:`sfx_volume <sfx>`",integer,192,
		":ref:`shorty <shorty>`",boolean,false,
		":ref:`show_fps <fps>`",boolean,false,
		":ref:`ShowItemCosts <cost>`",boolean,false,
		":ref:`silver_cursors <silver>`",boolean,false,
		":ref:`sitcom <sitcom>`",boolean,false,
		":ref:`skip_support <skipsupport>`",boolean,true,
		":ref:`skiphallofrecordsscenes <skiphall>`",boolean,false,
		":ref:`slim_hotspots <hotspots>`",boolean,true,
		":ref:`smooth_scrolling <smooth>`",boolean,true,
		":ref:`sound <sound>`",boolean,true,
		":ref:`speech_mute <speechmute>`",boolean,false,
		":ref:`speech_volume <speechvol>`",integer,192,
		":ref:`speedrun_mode <speedrun>`",boolean,false,
		":ref:`stretch_mode <stretchmode>`",string,,"
	- center
	- pixel-perfect
	- fit
	- stretch
	- fit_force_aspect "
		":ref:`studio_audience <studio>`",boolean,true,
		":ref:`subtitles <speechmute>`",boolean,false,
		":ref:`talkspeed <talkspeed>`",integer,60,"- 0 - 255 "
		tempo,integer,100,"Sets the music tempo, in percent, for SCUMM games.

	- 50-200"
		":ref:`targetedjump <jump>`",boolean,true,
		":ref:`TextWindowAnimated <windowanimated>`",boolean,true,
		":ref:`themepath <themepath>`",string,none,
		":ref:`transition_mode <tmode>`",boolean,false, "For Riven, this is a string with :ref:`4 options <tspeed>`
		- Disabled
		- Fastest
		- Normal
		- Best"
		":ref:`transparent_windows <transparentwindows>`",boolean,true,
		":ref:`transparentdialogboxes <transparentdialog>`",boolean,false,
		":ref:`trim_fmtowns_to_200_pixels <trim>`",boolean,false,
		":ref:`tts_enabled <ttsenabled>`",boolean,false,
		":ref:`tts_enabled_objects <tts_objects>`",boolean,false,
		":ref:`tts_enabled_speech <tts_speech>`",boolean,false,
		":ref:`tts_narrator <ttsnarrator>`",boolean,false,
		use_cdaudio,boolean,true, "If true, ScummVM uses audio from the game CD."
		versioninfo,string,,Shows the ScummVM version that created the configuration file.
		":ref:`unlockAlllevels <unlock>`",boolean,false,
		":ref:`usecd <usecd>`",boolean,false,
		":ref:`use_crawl_subs <crawlsubs>`",boolean,true,
		":ref:`usehighres <highres>`",boolean,false,
		":ref:`use_linear_filtering <linearfilter>`",boolean,true,
		":ref:`version <usa>`",boolean,false,
		":ref:`voice <voice>`",boolean,true,
		":ref:`venusenabled <venus>`",boolean,true,
		":ref:`vsync <vsync>`",boolean,true,
		":ref:`wallcollision <wall>`",boolean,false,
		":ref:`water_effects <water>`",boolean,,
		":ref:`window_style <style>`",boolean,true,
		":ref:`windows_cursors <wincursors>`",boolean,false,
		":ref:`zip_mode <zip>`",boolean,,



.. _screenshotpath:

Screenshot path
	The default location for the screenshotpath depends on your system.


	.. tabbed:: Windows

		.. panels::
			:column: col-lg-12 mb-2

			In ``Users\username\My Pictures\ScummVM Screenshots``

	.. tabbed:: macOS

		.. panels::
			:column: col-lg-12 mb-2

			On the Desktop.


	.. tabbed:: Linux

		.. panels::
			:column: col-lg-12 mb-2

			In the XDG Pictures user directory, for example ``~/Pictures/ScummVM Screenshots``

	.. tabbed:: Any other OS

		.. panels::
			:column: col-lg-12 mb-2

			In the current directory.


