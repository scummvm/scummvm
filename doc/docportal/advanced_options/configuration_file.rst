
=======================
The configuration file
=======================

The configuration file provides a way to edit both global and game-specific settings by using a text file containing configuration keywords and parameters. 

Settings are also accessible directly from :doc:`the Launcher <../using_scummvm/the_launcher>` . To edit :doc:`global settings <../using_scummvm/global_settings>`, click **Options**. To edit :doc:`game-specific settings <../using_scummvm/game_settings>`, highlight a game and then click **Edit Game**.

Location
==========

The configuration file saves to different default locations, depending on the platform. 



.. tabs::

	.. tab:: Mac OSX

		``~/Library/Preferences/ScummVM Preferences``, where ``~`` is your Home directory. 

		Note that if an earlier version of ScummVM was installed on your system, the previous default location of ``~/.scummvmrc`` will be kept.
	
	.. tab:: Windows

		**95/98/ME**: 
		``<windir>\scummvm.ini``

		**Windows NT4**: ``<windir>\Profiles\username\Application Data\ScummVM\scummvm.ini``

		**Windows 2000/XP**: ``\Documents and Settings\username\Application Data\ScummVM\scummvm.ini``

		**Window 7/Vista**: ``\Users\username\AppData\Roaming\ScummVM\scummvm.ini``

		Note that if an earlier version of ScummVM was installed under Windows, the previous default location of <windir>\scummvm.ini will be kept.

	.. tab:: Linux/Unix

		We follow the XDG Base Directory Specification. This means our configuration can be found in ``$XDG_CONFIG_HOME/scummvm/scummvm.ini``

		If XDG_CONFIG_HOME is not defined or empty, ``~/.config`` will be used, where ``~`` is your Home directory. 
		
		Note that ``.config`` is a hidden directory; to view it use ``ls -a`` on the command line. If you are using a GUI file manager, go to **View > Show Hidden Files**, or use the keyboard shortcut :kbd:`Ctrl + H`.

	.. tab:: iOS

		``/var/mobile/Library/ScummVM/Preferences`` or ``/Preferences`` for a sandboxed version.

	.. tab:: Other

		``scummvm.ini`` in the ScummVM directory. 

Using the configuration file
==================================

Global settings are listed under the ``[scummvm]`` heading. Game settings are listed under the heading for that game, for example ``[queen]`` for Flight of the Amazon Queen. Use the configuration keywords_ to change settings. 


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

There are many recognized configuration keywords. In the table below, each keyword is linked to an explanatory description on either the :doc:`../using_scummvm/global_settings` or :doc:`../using_scummvm/game_settings` page. 

.. csv-table:: 
  	:widths: 25, 25, 15, 35
  	:header-rows: 1

		Keyword,Type,Default,Options
		alsa_port,number,,
		":ref:`alt_intro <altintro>`",boolean,false,
		":ref:`altamigapalette <altamiga>`",boolean,false,
		":ref:`apple2gs_speedmenu <2gs>`",boolean,false,
		":ref:`aspect_ratio <ratio>`",boolean,false,
		audio_buffer_size,number,"Calculated","
	- 256 
	- 512 
	- 1024 
	- 2048 
	- 4096                             
	- 8192 
	- 16384 
	- 32768" 
		auto_savenames,boolean,false,
		
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
		demo_mode,,false,
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
		":ref:`fullscreen <fullscreen>`",boolean,false,
		":ref:`gameid <gameid>`",string,,
		":ref:`gamepath <gamepath>`",string,,
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
	- alsa_midi Through
	- seq 
	- sndio
	- fluidsynth 
	- timidity"
		":ref:`GraphicsDithering <gdither>`",,true,
		":ref:`helium_mode <helium>`",,false,
		":ref:`help_style <help>`",,false,
		":ref:`herculesfont <herc>`",,false,
		":ref:`hpbargraphs <hp>`",,true,
		":ref:`hypercheat <hyper>`",,false,
		iconpath,string,,
		":ref:`improved <improved>`",,true,
		":ref:`InvObjectsAnimated <objanimated>`",,true,
		joystick_num,number,,
		":ref:`language <lang>`",string,,
		":ref:`local_server_port <serverport>`",integer,12345,
		":ref:`midi_gain <gain>`",number,,"- 0 - 1000"
		":ref:`mm_nes_classic_palette <classic>`",boolean,false,
		":ref:`monotext <mono>`",boolean,true,
		":ref:`mousebtswap <btswap>`",boolean,false,
		":ref:`mousesupport <support>`",boolean,true,
		":ref:`mt32_device <mt32>`",string,,"
	- auto
	- alsa_midi Through
	- seq 
	- sndio
	- fluidsynth
	- mt32
	- timidity "
		":ref:`multi_midi <multi>`",boolean,,
		":ref:`music_driver <device>`",string,auto,"	
	- null
	- auto
	- alsa_Midi Through 
	- seq
	- sndio
	- fluidsynth 
	- mt32
	- timidity
	- pcspk 
	- pcjr
	- cms
	- adlib "
		":ref:`music_volume <music>`",number,,"- 0-256 "
		":ref:`mute <mute>`",boolean,false,
		":ref:`native_fb01 <fb01>`",,false,
		":ref:`native_mt32 <nativemt32>`",boolean,false,
		":ref:`NaughtyMode <naughty>`",,true,
		":ref:`nodelaymillisfl <nodelay>`",,false,
		":ref:`ntsc <ntsc>`",,?,
		":ref:`object_labels <labels>`",,true,
		":ref:`opl_driver <opl>`",string,,"
	- auto
	- mame
	- db
	- nuked
	- alsa
	- op2lpt
	- op3lpt "
		":ref:`originalsaveload <osl>`",boolean,false,
		output_rate,number,,"
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
		":ref:`TextWindowAnimated <textanimated>`",boolean,true,
		":ref:`themepath <themepath>`",string,none,
		":ref:`transparent_windows <transparentwindows>`",boolean,true,
		":ref:`transparentdialogboxes <transparentdialog>`",boolean,false,
		":ref:`tts_enabled <ttsenabled>`",boolean,false,
		":ref:`tts_narrator <ttsnarrator>`",boolean,false,
		":ref:`use_cdaudio <cd>`",boolean,true,
		versioninfo,string,,
		":ref:`window_style <style>`",boolean,true,
		":ref:`windows_cursors <wincursors>`",boolean,false,
		
		
		
		
	
		
		
		
		
		
		
	
		
		
		
		

		
