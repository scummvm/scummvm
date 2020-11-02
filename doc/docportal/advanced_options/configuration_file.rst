
=======================
The configuration file
=======================

The configuration file provides a way to edit both global and game-specific settings by using a text file containing configuration keywords and parameters. 

Settings are also accessible directly from :doc:`the Launcher <../using_scummvm/the_launcher>` . To edit global settings, click **Options**. To edit game-specific settings, highlight a game and then click **Edit Game**.

Location
==========

The configuration file saves to different default locations, depending on the platform. 



.. tabbed:: macOS

	.. panels::
		:column: col-lg-12 mb-2

		``~/Library/Preferences/ScummVM Preferences``, where ``~`` is your Home directory. 

		.. note::

			If an earlier version of ScummVM was installed on your system, the configuration file remains in the previous default location of ``~/.scummvmrc``.
		
		.. tip::
			
			To see the Libraries folder you will need to view hidden files by using the keyboard shortcut :kbd:`Cmd + Shift + . ` 
		
	
.. tabbed:: Windows

	.. panels::
		:column: col-lg-12 mb-2

		95/98/ME
		^^^^^^^^^^ 
	
		``<windir>\scummvm.ini``
	
		---
		:column: col-lg-12 mb-2

		Windows NT4 
		^^^^^^^^^^^^

		``<windir>\Profiles\username\Application Data\ScummVM\scummvm.ini``

		---
		:column: col-lg-12 mb-2

		Windows 2000/XP
		^^^^^^^^^^^^^^^^^
		``\Documents and Settings\username\Application Data\ScummVM\scummvm.ini``

		---
		:column: col-lg-12 mb-2

		Window 7/Vista
		^^^^^^^^^^^^^^^^
		``\Users\username\AppData\Roaming\ScummVM\scummvm.ini``

	.. tip::

		``<windir>`` refers to the Windows directory. Most commonly, this is ``C:\WINDOWS``.

	.. note::
	
		If an earlier version of ScummVM was installed under Windows, the configuration file remains in the previous default location of ``<windir>\scummvm.ini`` 

.. tabbed:: Linux/Unix

	.. panels::
		:column: col-lg-12 mb-2

		We follow the XDG Base Directory Specification. This means our configuration can be found in ``$XDG_CONFIG_HOME/scummvm/scummvm.ini``

		If XDG_CONFIG_HOME is not defined or empty, ``~/.config`` is used, where ``~`` is your Home directory. 
		
		If ScummVM was installed using Snap, the configuration file can be found at ``~/snap/scummvm/current/.config/scummvm/scummvm.ini``

		.. note::
			
			``.config`` is a hidden directory. To view it use ``ls -a`` on the command line. If you are using a GUI file manager, go to **View > Show Hidden Files**, or use the keyboard shortcut :kbd:`Ctrl + H`.

	

.. tabbed:: Other

	.. panels::
		:column: col-lg-12 mb-2

		See the relevant Platform page for details. 


Using the configuration file
==================================

Global settings are listed under the ``[scummvm]`` heading. Global :doc:`Keymaps settings <../settings/keymaps>` are listed under the ``[keymapper]`` heading. Game-specific settings, including keymaps, are listed under the heading for that game, for example ``[queen]`` for Flight of the Amazon Queen. Use the configuration keywords_ to change settings. 


Example of a configuration file
=================================

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


Keywords
===========

There are many recognized configuration keywords. In the table below, each keyword is linked to an explanatory description in the Settings pages. 

.. csv-table:: 
  	:width: 100% 
	:widths: 20 20 10 50
  	:header-rows: 1 

		Keyword,Type,Default,Options
		alsa_port,number,,
		":ref:`alt_intro <altintro>`",boolean,false,
		":ref:`altamigapalette <altamiga>`",boolean,false,
		":ref:`apple2gs_speedmenu <2gs>`",boolean,false,
		":ref:`aspect_ratio <ratio>`",boolean,false,
		":ref:`audio_buffer_size <buffer>`",number,"Calculated","
	- 256 
	- 512 
	- 1024 
	- 2048 
	- 4096                             
	- 8192 
	- 16384 
	- 32768"
		":ref:`autosave_period <autosave>`", integer, 300s 
		auto_savenames,boolean,false
		":ref:`bilinear_filtering <bilinear>`",boolean,false,
		boot_param,number,none,
		":ref:`bright_palette <bright>`",boolean,true,
		cdrom,number,,
		":ref:`color <color>`",boolean,,
		":ref:`commandpromptwindow <cmd>`",boolean,false,
		confirm_exit,boolean,,
		console,boolean,true,
		controller_map_db,string,,
		copy_protection,boolean,false,
		demo_mode,boolean,false,
		":ref:`description <description>`",string,,
		":ref:`disable_dithering <dither>`",boolean,false,
		":ref:`disable_stamina_drain <stamina>`",boolean,false,
		":ref:`DurableArmor <durable>`",boolean,false,
		":ref:`EasyMouse <easy>`",boolean,true,
		":ref:`enable_black_lined_video <blackline>`",,false,
		":ref:`enable_censoring <censor>`",boolean,false,
		":ref:`enable_color_blind <blind>`",boolean,false,
		":ref:`enable_gore <gore>`",boolean,,
		":ref:`enable_gs <gs>`",boolean,,
		":ref:`enable_high_resolution_graphics <hires>`",boolean,true,
		":ref:`enable_hq_video <hq>`",boolean,true,
		":ref:`enable_larryscale <larry>`",boolean,true,
		":ref:`enable_video_upscale <upscale>`",boolean,true,
		":ref:`enable_tts <ttsenabled>`",boolean,false,
		":ref:`extrapath <extra>`",string,None,"		path"
		":ref:`fade_style <fade>`",boolean,true,
		":ref:`filtering <filtering>`",boolean,false,
		":ref:`floating_cursors <floating>`",boolean,false,
		":ref:`fluidsynth_chorus_activate <chact>`",boolean,true,
		":ref:`fluidsynth_chorus_depth <chdepth>`",number,80,"- 0 - 210"
		":ref:`fluidsynth_chorus_level <chlevel>`",number,100,"- 0 - 100"
		":ref:`fluidsynth_chorus_nr <chnr>`",integer,3,"- 0 - 99"
		":ref:`fluidsynth_chorus_speed <chspeed>`",number,30,"- 10 - 500"
		":ref:`fluidsynth_chorus_waveform <chwave>`",string,Sine," 
	- sine
	- triangle"
		":ref:`fluidsynth_misc_interpolation <interp>`",string,4th,"
	- none
	- 4th
	- 7th
	- linear."
		":ref:`fluidsynth_reverb_activate <revact>`",boolean,true,
		":ref:`fluidsynth_reverb_damping <revdamp>`",number,0,"- 0 - 1"
		":ref:`fluidsynth_reverb_level <revlevel>`",number,90,"- 0 - 100"
		":ref:`fluidsynth_reverb_roomsize <revroom>`",number,20,"- 0 - 100"
		":ref:`fluidsynth_reverb_width <revwidth>`",number,1,"- 0 - 100"
		":ref:`frames_per_secondfl <fpsfl>`",boolean,false,
		:ref:`frontpanel_touchpad_mode <frontpanel>`,boolean, false
		":ref:`fullscreen <fullscreen>`",boolean,false,
		":ref:`gameid <gameid>`",string,,
		gamepath,string,,
		":ref:`gfx_mode <gfxmode>`",string,2x,"
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
		":ref:`GraphicsDithering <gdither>`",boolean,true,
		":ref:`gui_browser_native <guibrowser>`", boolean, true
		":ref:`helium_mode <helium>`",boolean,false,
		":ref:`help_style <help>`",boolean,false,
		":ref:`herculesfont <herc>`",boolean,false,
		":ref:`hpbargraphs <hp>`",boolean,true,
		":ref:`hypercheat <hyper>`",boolean,false,
		iconpath,string,,
		":ref:`improved <improved>`",boolean,true,
		":ref:`InvObjectsAnimated <objanimated>`",boolean,true,
		":ref:`joystick_deadzone <deadzone>`",integer, 3
		joystick_num,number,,
		":ref:`kbdmouse_speed <mousespeed>`", integer, 10
		":doc:`keymap_engine-default_DOWN <../settings/keymaps>`",string,JOY_DOWN
		":doc:`keymap_engine-default_LCLK <../settings/keymaps>`",string,MOUSE_LEFT JOY_A
		":doc:`keymap_engine-default_LEFT <../settings/keymaps>`",string,JOY_LEFT
		":doc:`keymap_engine-default_MCLK <../settings/keymaps>`",string,MOUSE_MIDDLE
		":doc:`keymap_engine-default_MENU <../settings/keymaps>`",string,F5 JOY_LEFT_SHOULDER
		":doc:`keymap_engine-default_PAUSE <../settings/keymaps>`",string,SPACE
		":doc:`keymap_engine-default_PIND <../settings/keymaps>`",string,
		":doc:`keymap_engine-default_RCLK <../settings/keymaps>`",string,MOUSE_RIGHT JOY_B
		":doc:`keymap_engine-default_RETURN <../settings/keymaps>`",string,RETURN
		":doc:`keymap_engine-default_RIGHT <../settings/keymaps>`",string,JOY_RIGHT
		":doc:`keymap_engine-default_SKIP <../settings/keymaps>`",string,ESCAPE JOY
		":doc:`keymap_engine-default_SKLI <../settings/keymaps>`",string,PERIOD JOY_X
		":doc:`keymap_engine-default_UP <../settings/keymaps>`",string,JOY_UP 
		":doc:`keymap_global_DEBUGGER <../settings/keymaps>`",string,C+A+d 
		":doc:`keymap_global_MENU <../settings/keymaps>`",string,C+F5 JOY_START
		":doc:`keymap_global_MUTE <../settings/keymaps>`",string,C+u 
		":doc:`keymap_global_QUIT <../settings/keymaps>`",string,C+q 
		":doc:`keymap_global_VMOUSEDOWN <../settings/keymaps>`",string,JOY_LEFT_STICK_Y+
		":doc:`keymap_global_VMOUSELEFT <../settings/keymaps>`",string,JOY_LEFT_STICK_X-
		":doc:`keymap_global_VMOUSERIGHT <../settings/keymaps>`",string,JOY_LEFT_STICK_X+
		":doc:`keymap_global_VMOUSESLOW <../settings/keymaps>`",string,JOY_RIGHT_SHOULDER
		":doc:`keymap_global_VMOUSEUP <../settings/keymaps>`",string,JOY_LEFT_STICK_Y-
		":doc:`keymap_gui_CLOS <../settings/keymaps>`",string,ESCAPE JOY_Y
		":doc:`keymap_gui_DOWN <../settings/keymaps>`",string,JOY_DOWN
		":doc:`keymap_gui_INTRCT <../settings/keymaps>`",string,JOY_A
		":doc:`keymap_gui_LEFT <../settings/keymaps>`",string,
		":doc:`keymap_gui_RIGHT <../settings/keymaps>`",string,JOY_RIGHT
		":doc:`keymap_gui_UP <../settings/keymaps>`",string,JOY_UP
		":doc:`keymap_sdl-graphics_ASPT <../settings/keymaps>`",string,C+A+a
		":doc:`keymap_sdl-graphics_CAPT <../settings/keymaps>`",string,C+m
		":doc:`keymap_sdl-graphics_FILT <../settings/keymaps>`",string,C+A+f
		":doc:`keymap_sdl-graphics_FLT1 <../settings/keymaps>`",string,C+A+1 
		":doc:`keymap_sdl-graphics_FLT2 <../settings/keymaps>`",string,C+A+2
		":doc:`keymap_sdl-graphics_FLT3 <../settings/keymaps>`",string,C+A+3 
		":doc:`keymap_sdl-graphics_FLT4 <../settings/keymaps>`",string,C+A+4 
		":doc:`keymap_sdl-graphics_FLT5 <../settings/keymaps>`",string,C+A+5 
		":doc:`keymap_sdl-graphics_FLT6 <../settings/keymaps>`",string, C+A+6
		":doc:`keymap_sdl-graphics_FLT7 <../settings/keymaps>`",string,C+A+7
		":doc:`keymap_sdl-graphics_FLT8 <../settings/keymaps>`",string,C+A+8
		":doc:`keymap_sdl-graphics_FULS <../settings/keymaps>`",string,A+RETURN
		":doc:`keymap_sdl-graphics_SCL- <../settings/keymaps>`",string,C+A+MINUS 
		":doc:`keymap_sdl-graphics_SCL+ <../settings/keymaps>`",string,C+A+PLUS 
		":doc:`keymap_sdl-graphics_SCRS <../settings/keymaps>`",string,A+s 
		":doc:`keymap_sdl-graphics_STCH <../settings/keymaps>`",string,C+A+s 
		":ref:`language <lang>`",string,,
		":ref:`local_server_port <serverport>`",integer,12345,
		":ref:`midi_gain <gain>`",number,,"- 0 - 1000"
		":ref:`mm_nes_classic_palette <classic>`",boolean,false,
		":ref:`monotext <mono>`",boolean,true,
		":ref:`mousebtswap <btswap>`",boolean,false,
		":ref:`mousesupport <support>`",boolean,true,
		":ref:`mt32_device <mt32>`",string,,"
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
		":ref:`music_volume <music>`",number,,"- 0-256 "
		":ref:`mute <mute>`",boolean,false,
		":ref:`native_fb01 <fb01>`",boolean,false,
		":ref:`native_mt32 <nativemt32>`",boolean,false,
		":ref:`NaughtyMode <naughty>`",boolean,true,
		":ref:`nodelaymillisfl <nodelay>`",boolean,false,
		":ref:`ntsc <ntsc>`",boolean,,
		":ref:`object_labels <labels>`",boolean,true,
		":ref:`opl_driver <opl>`",string,,"
	- auto
	- mame
	- db
	- nuked
	- alsa
	- op2lpt
	- op3lpt "
		":ref:`originalsaveload <osl>`",boolean,false,
		":ref:`output_rate <outputrate>`",number,,"
	Sensible values are:

	- 11025 
	- 22050
	- 44100"
		":ref:`platform <platform>`",string,,
		":ref:`portaits_on, <portraits>`",boolean,true,
		":ref:`prefer_digitalsfx <dsfx>`",boolean,true,
		":ref:`render_mode <render>`",string,,"	
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
		":ref:`rootpath <rootpath>`",string,,
		":ref:`savepath <savepath>`",string,,
		save_slot,number,,
		":ref:`scalemakingofvideos <scale>`",boolean,false,
		":ref:`scanlines <scan>`",boolean,false,
		screenshotpath,string,,
		":ref:`shorty <shorty>`",boolean,false,
		":ref:`show_fps <fps>`",boolean,false,
		":ref:`ShowItemCosts <cost>`",boolean,false,
		":ref:`silver_cursors <silver>`",boolean,false,
		":ref:`sitcom <sitcom>`",boolean,false,
		":ref:`skip_support <skip>`",boolean,true,
		":ref:`skiphallofrecordsscenes <skiphall>`",boolean,false,
		":ref:`smooth_scrolling <smooth>`",boolean,true,
		":ref:`speech_mute <speechmute>`",boolean,false,
		":ref:`stretch_mode <stretchmode>`",string,,"
	- center 
	- pixel-perfect 
	- fit 
	- stretch 
	- fit_force_aspect "
		":ref:`studio_audience <studio>`",boolean,true,
		":ref:`subtitles <speechmute>`",boolean,false,
		":ref:`talkspeed <talkspeed>`",number,60,"- 0 - 255 "
		tempo,number,100,"- 50-200"
		":ref:`TextWindowAnimated <windowanimated>`",boolean,true,
		":ref:`themepath <themepath>`",string,none,
		":ref:`transparent_windows <transparentwindows>`",boolean,true,
		":ref:`transparentdialogboxes <transparentdialog>`",boolean,false,
		":ref:`tts_enabled <ttsenabled>`",boolean,false,
		":ref:`tts_narrator <ttsnarrator>`",boolean,false,
		":ref:`use_cdaudio <cd>`",boolean,true,
		versioninfo,string,,
		":ref:`window_style <style>`",boolean,true,
		":ref:`windows_cursors <wincursors>`",boolean,false,
		
		
		
		
	
		
		
		
		
		
		
	
		
		
		
		

		
