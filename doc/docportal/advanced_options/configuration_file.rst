
=======================
The configuration file
=======================

The configuration file provides a way to edit both global and game-specific settings by using a text file containing configuration keywords and parameters. 

Settings are also accessible directly from :doc:`the Launcher <../using_scummvm/the_launcher>` . To edit global settings, click **Options**. To edit game-specific settings, highlight a game and then click **Edit Game**.

Location
==========
The configuration file saves to different default locations depending on the platform ScummVM is running on.  

**Mac**:

**Windows**:

**Linux**:

**iOS**:

**Android**:

Using the configuration file
==================================

Global settings are listed under the ``[scummvm]`` heading. Game settings are listed under the heading for that game, for example ``[queen]`` for Flight of the Amazon Queen. Use the configuration keywords_ to change settings. 


Example of a configuration file
=================================

.. code-block::

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

There are many recognized configuration keywords. In the table below, each keyword is linked to its description on either the :doc:`../using_scummvm/global_settings` or :doc:`../using_scummvm/game_settings` page. 

.. csv-table:: 
  	:widths: 25 10 25 40
  	:header-rows: 1

		Keyword,Type,Default,Options
		":ref:`alt_intro <altintro>`",Boolean,FALSE,
		":ref:`altamigapalette <altamiga>`",Boolean,FALSE,
		":ref:`apple2gs_speedmenu <2gs>`",Boolean,FALSE,
		":ref:`aspect_ratio <ratio>`",Boolean,FALSE,
		auto_savenames,Boolean,FALSE,
		":ref:`bilinear_filtering <bilinear>`",Boolean,FALSE,
		":ref:`bright_palette <bright>`",Boolean,TRUE,
		":ref:`color <color>`",Boolean,,
		":ref:`commandpromptwindow <cmd>`",Boolean,FALSE,
		description,String,,
		":ref:`disable_dithering <dither>`",Boolean,FALSE,
		":ref:`disable_stamina_drain <stamina>`",Boolean,FALSE,
		":ref:`DurableArmor <durable>`",Boolean,FALSE,
		":ref:`EasyMouse <easy>`",Boolean,TRUE,
		":ref:`enable_black_lined_video <blackline>`",,FALSE,
		":ref:`enable_censoring <censor>`",Boolean,FALSE,
		":ref:`enable_color_blind <blind>`",Boolean,FALSE,
		":ref:`enable_gore <gore>`",Boolean,,
		":ref:`enable_gs <gs>`",Boolean,,
		":ref:`enable_high_resolution_graphics <hires>`",Boolean,TRUE,
		":ref:`enable_hq_video <hq>`",Boolean,TRUE,
		":ref:`enable_larryscale <larry>`",Boolean,TRUE,
		":ref:`enable_video_upscale <upscale>`",Boolean,TRUE,
		":ref:`extrapath <extra>`",String,None,Path
		":ref:`fade_style <fade>`",Boolean,TRUE,
		":ref:`filtering <filtering>`",Boolean,FALSE,
		":ref:`floating_cursors <floating>`",Boolean,FALSE,
		":ref:`fluidsynth_chorus_activate <chact>`",Boolean,TRUE,
		":ref:`fluidsynth_chorus_depth <chdepth>`",Number,80,0 - 210
		":ref:`fluidsynth_chorus_level <chlevel>`",Number,100,0 - 100
		":ref:`fluidsynth_chorus_nr <chnr>`",Integer,3,0 - 99
		":ref:`fluidsynth_chorus_speed <chspeed>`",Number,30,10 - 500
		":ref:`fluidsynth_chorus_waveform <chwave>`",String,Sine,"
		- sine 
		- triangle"
		":ref:`fluidsynth_misc_interpolation <interp>`",String,4th,"
		- none 
		- 4th 
		- 7th 
		- linear"
		":ref:`fluidsynth_reverb_activate <revact>`",Boolean,TRUE,
		":ref:`fluidsynth_reverb_damping <revdamp>`",Number,0,0 - 1
		":ref:`fluidsynth_reverb_level <revlevel>`",Number,90,0 - 100
		":ref:`fluidsynth_reverb_roomsize <revroom>`",Number,20,0 - 100
		":ref:`fluidsynth_reverb_width <revwidth>`",Number,1,0 - 100
		":ref:`frames_per_secondfl <fpsfl>`",Boolean,FALSE,
		":ref:`fullscreen <fullscreen>`",Boolean,FALSE,
		":ref:`gameid <gameid>`",String,,
		":ref:`gamepath <gamepath>`",String,,
		":ref:`gfx_mode <gfxmode>` ",String,2x,"
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
		- opengl  "
		":ref:`gm_device <gm>`",String,null," 
		- auto 
		- alsa_midi Through 
		- seq 
		- sndio
		- fluidsynth 
		- timidity"
		":ref:`GraphicsDithering <gdither>`",,TRUE,
		":ref:`helium_mode <helium>`",,FALSE,
		":ref:`help_style <help>`",,FALSE,
		":ref:`herculesfont <herc>`",,FALSE,
		":ref:`hpbargraphs <hp>`",,TRUE,
		":ref:`hypercheat <hyper>`",,FALSE,
		":ref:`improved <improved>`",,TRUE,
		":ref:`InvObjectsAnimated <objanimated>`",,TRUE,
		":ref:`language <lang>`",String,,
		":ref:`local_server_port <serverport>`",integer,12345,
		":ref:`midi_gain <gain>`",Number,,0 - 1000
		":ref:`mm_nes_classic_palette <classic>`",Boolean,FALSE,
		":ref:`monotext <mono>`",Boolean,TRUE,
		":ref:`mousebtswap <btswap>`",Boolean,FALSE,
		":ref:`mousesupport <support>`",Boolean,TRUE,
		":ref:`mt32_device <mt32>`",String,,"
		- Auto
		- alsa_midi Through
		- seq
		- sndio
		- fluidsynth
		- mt32
		- timidity "
		":ref:`multi_midi <multi>`",Boolean,,
		":ref:`music_driver <device>`",String,auto,"
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
		":ref:`music_volume <music>`",Number,,"
		- 0-256 "
		":ref:`mute <mute>`",Boolean,FALSE,
		":ref:`native_fb01 <fb01>`",,FALSE,
		":ref:`native_mt32 <nativemt32>`",Boolean,FALSE,
		":ref:`NaughtyMode <naughty>`",,TRUE,
		":ref:`nodelaymillisfl <nodelay>`",,FALSE,
		":ref:`ntsc <ntsc>`",,?,
		":ref:`object_labels <labels>`",,TRUE,
		":ref:`opl_driver <opl>`",String,,"
		- auto
		- mame
		- db
		- nuked
		- alsa
		- op2lpt
		- op3lpt "
		":ref:`originalsaveload <osl>`",Boolean,FALSE,
		":ref:`platform <platform>`",String,,
		":ref:`portaits_on, <portraits>`",Boolean,TRUE,
		":ref:`prefer_digitalsfx <dsfx>`",Boolean,TRUE,
		":ref:`render_mode <render>`",String,,"
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
		":ref:`savepath <savepath>`",String,,
		":ref:`scalemakingofvideos <scale>`",Boolean,FALSE,
		":ref:`scanlines <scan>`",Boolean,FALSE,
		":ref:`shorty <shorty>`",Boolean,FALSE,
		":ref:`show_fps <fps>`",Boolean,FALSE,
		":ref:`ShowItemCosts <cost>`",Boolean,FALSE,
		":ref:`silver_cursors <silver>`",Boolean,FALSE,
		":ref:`sitcom <sitcom>`",Boolean,FALSE,
		":ref:`skip_support <skip>`",Boolean,TRUE,
		":ref:`skiphallofrecordsscenes <skiphall>`",Boolean,FALSE,
		":ref:`smooth_scrolling <smooth>`",Boolean,TRUE,
		":ref:`speech_mute <speechmute>`",Boolean,FALSE,
		":ref:`stretch_mode <stretchmode>`",String,,"
		- center
		- pixel-perfect
		- fit
		- stretch
		- fit_force_aspect "
		":ref:`studio_audience <studio>`",Boolean,TRUE,
		":ref:`subtitles <speechmute>`",Boolean,FALSE,
		":ref:`talkspeed <talkspeed>`",Number,60,
		" - 0 - 255 "
		":ref:`TextWindowAnimated <textanimated>`",Boolean,TRUE,
		":ref:`themepath <themepath>`",String,none,
		":ref:`transparent_windows <transparentwindows>`",Boolean,TRUE,
		":ref:`transparentdialogboxes <transparentdialog>`",Boolean,FALSE,
		":ref:`tts_enabled <ttsenabled>`",Boolean,FALSE,
		":ref:`tts_narrator <ttsnarrator>`",Boolean,FALSE,
		":ref:`use_cdaudio <cd>`",Boolean,TRUE,
		":ref:`window_style <style>`",Boolean,TRUE,
		":ref:`windows_cursors <wincursors>`",Boolean,FALSE,
		save_slot,Number,,
		Savepath,String,,
		screenshotpath,String,,
		iconpath,String,,
		versioninfo,String,,
		gameid,String,,
		description,String,,
		confirm_exit,Boolean,,
		console,Boolean,TRUE,Windows only
		cdrom,Number,,
		joystick_num,Number,,
		controller_map_db,String,,
		audio_buffer_size,Number,"Calculated to keep
		below 45ms","
		- 256 
		- 512 
		- 1024 
		- 2048 
		- 4096                             
		- 8192 
		- 16384 
		- 32768" 
		output_rate,Number,,"
		- The output sample rate to use, in Hz. 
		- Sensible values are 11025, 22050 or 44100."
		alsa_port,Number,,"Port to use for output when 
		
	  using the ALSA music driver."
		tempo,Number,100,"
		- 50-200"
		copy_protection,boolean,FALSE,
		demo_mode,,FALSE,Start demo mode in MM
		boot_param,Number,none,Pass this number to boot script. 

		
