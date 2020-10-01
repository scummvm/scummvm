=============
Game settings
=============

Each game can be configured to run using settings other than the :doc:`global_settings`

To open the game settings window, highlight a game in the main Launcher
window, and click on **Edit Game**. This window also opens any time a
game is added to the Launcher.


There are many settings, and they are separated into tabs:

`Game tab`_ | `Engine tab`_ | `Graphics tab`_ | `Audio tab`_ | `Volume tab`_ | `Midi tab`_ | `MT-32 tab`_ | `Paths tab`_ |

Settings may also be edited directly in the configuration file. These configuration keywords are listed below in italics. For more information, including a complete list of configuration keywords, see the :doc:`../advanced_options/configuration_file` .

_`Game tab`
------------

Use the game tab to display and change information about how the game is
identified by ScummVM.

,,,,,,,

.. _gameid:

**ID**
	This field shows the short name of the game. This ID can be used for launching the game from the command line.

	*gameid* 

.. _description:

**Name**
	This field shows the full title of the game, as well as some basic information such as original platform and language.
	 
	*description* 

.. _lang:

**Language**
	For multi-language games, this option changes the language of the game. However, for games that have the language hardcoded, this option only changes the font of the subtitles. For instance, selecting German will add an umlaut to German subtitles, and selecting Spanish will add the tilde in Spanish subtitles. 

	*language* 

.. _platform:

**Platform**
	Specifies the original platform of the game.

	*platform* 

,,,,,,,



_`Graphics tab`
-----------------

Use the graphics tab to change how the game looks when it is played. For more information including comprehensive explanation of the graphics settings, see the :doc:`../advanced_options/graphics` page. 

,,,,,,,

**Override global graphic settings**

	Check this box to make the game use custom settings instead of global settings.


**Graphics mode**
	Changes the resolution of the game. For example, a 2x graphics mode will take a 320x200 resolution game and scale it up to 640x400. 

	*gfx_mode* 

		

**Render mode**
	Changes how the game is rendered, which affects how the game looks. 

	*render_mode* 
			

**Stretch mode**
	Changes the way the game is displayed in relation to the window size.

	*stretch_mode* 

**Aspect ratio correction**
	If ticked, corrects the aspect ratio so that games appear the same as they would on original 320x200 resolution displays. 

	*aspect_ratio* 

**Fullscreen mode**
	Switches between playing games in a window, or playing them in fullscreen mode. Switch between the two by using :kbd:`Alt + F5` while in a game.

	*fullscreen* 

**Filter graphics**
	If ticked, uses bilinear interpolation instead of nearest neighbor resampling for the aspect ratio correction and stretch mode. It does not affect the graphics mode. 

	*filtering* 
,,,,,


_`Audio tab`
--------------------

Use the audio tab to change the sound output in the game.

For more information including comprehensive explanation of the audio settings, see the :doc:`../advanced_options/audio` page. 


,,,,,,,

**Override global audio settings**
	Check this box to make the game use custom settings instead of global settings.

**Music device**
	Specifies the device ScummVM uses to output audio. When set to <default>, ScummVM will automatically choose the most appropriate option for the played game. If set to an MT-32 or a General MIDI device, or if ScummVM chooses one of these automatically, the settings on the MT-32 or MIDI tabs also apply. 

	*music_driver* 

**AdLib Emulator**
	Chooses which emulator is used by ScummVM when the AdLib emulator is chosen as the preferred device.

	*opl_driver* 

**Text and Speech**
	For games with digitized speech, this setting allows the user to decide whether to play the game with speech only, subtitles only, or both.

	*speech_mute* 

	*subtitles* 
	
**Subtitle speed**
	Adjusts the length of time that the subtitles are displayed on screen. The lower the speed is set, the longer the subtitles are displayed.

	*talkspeed* 

,,,,,,

_`Volume tab`
-----------------
Use the volume tab to set the relative volumes for various sounds in the game.

,,,,,,,

**Override global volume settings**
	Check this box to make the game use custom settings instead of the global ones.



**Music volume**
	Adjusts the volume of the music played back in the game. 

	*music_volume* 

**SFX volume**
	Adjusts the volume of the sound effects within the game.

	*sfx_volume* 


**Speech volume**
	Adjusts the volume of the digitized speech in the game, if it has any.

	*speech_volume* 

**Mute All**
	Mutes all game audio. 

	*mute* 

,,,,,,


_`MIDI tab`
---------------

Use the MIDI tab to change the settings of General MIDI devices. This tab will only be available for games that use MIDI. 

,,,,,,,

**Override global MIDI settings**
	Check this box to make the game use custom settings instead of the global settings.

**GM Device**
	Specifies the preferred General MIDI (GM) device for the game. 

	*gm_device* 

**Soundfont**
	Specifies the path to a soundfont file, if this is required by the GM device. 

	*soundfont* 


**Mixed AdLib/MIDI mode**
	Combines MIDI music with AdLib sound effects. 

	*multi_midi* 

**MIDI gain**
	Adjusts the relative volume of the MIDI audio. This is only supported by some music devices.
	 
	*midi_gain* 



_`MT-32 tab`
---------------

Use the MT-32 tab to change the settings of MT-32 devices. This tab will only be available for games that use MIDI. 

,,,,,,,,,,,,,

MT-32 Device
	Specifies the preferred MT-32 device for the game. 

**True Roland MT-32 (disable GM emulation)**
	Enable this option only if you are using an actual Roland MT-32, LAPC-I, CM-64, CM-32L, CM-500 or other MT-32 compatible device. Note that this cannot be used in conjuntion with the Roland GS device option. 


**Roland GS device (enable MT-32 mappings)**
	 Enable this option if you are using a GS device that has an MT-32 map, such as an SC-55, SC-88 or SC-8820. Note that this cannot be used in conjunction with the True Roland MT-32 option. 

,,,,,


_`Paths tab`
--------------

Use the paths tab to tell ScummVM where to look for particular game files. 

,,,,,,,

**Save Path**
	Chooses the folder in which ScummVM will store the saved games. If this is not set, the saved games will be stored in the default directory.

	*savepath* 

**Extra Path**
	Chooses the folder that ScummVM will in look for various extra files. These could include one or more of:

	* Additional datafiles required for certain games 
	* Soundfonts 
	* MT-32 ROMs 

	*extrapath* 

.. _gamepath:

**Game Path**
	Chooses folder in which the game’s data files are stored.

	*gamepath* 

,,,,,,,

_`Engine tab`
--------------

Some settings are unique to a particular game or game engine. Conversely, some settings may be greyed out if they are not applicable.

To find out which engine powers your game, have a look at the Supported Games `Wiki page
<https://wiki.scummvm.org/index.php?title=Category:Supported_Games>`_

Engines: ADL_ | AGI_ | BLADERUNNER_ | CGE_ | CINE_ | DRASCULA_ | DREAMWEB_ | HDB_ | HOPKINS_ | KYRA_ | LURE_ | MADS_ | NEVERHOOD_ | SCI_ | SCUMM_ | SHERLOCK_ | SKY_ | SUPERNOVA_ | TOLTECS_ | WINTERMUTE_ | XEEN_ |

,,,,,,,

.. _ADL:

ADL
******
.. _ntsc:

TV emulation
	Emulate composite output to an NTSC TV.

	*ntsc* 

.. _color:

Color Graphics
	Use color graphics instead of monochrome.

	*color* 

.. _scan:

Show scanlines
	Darken every other scanline to mimic the look of CRT.

	*scanlines* 

.. _mono:

Always use sharp monochrome text
	Do not emulate NTSC artifacts for text

	*monotext* 

,,,,,,,,,

.. _AGI:

**AGI**
=================

.. _osl:

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

	*originalsaveload* 

.. _altamiga:

Use an alternative palette
	Use an alternative palette, common for all Amiga games. This was the old behavior

	*altamigapalette* 

.. _support:

Mouse support
	Enables mouse support. Allows to use mouse for movement and in game menus.

	*mousesupport* 

.. _herc:

Use Hercules hires font
	Uses Hercules hi-res font, when font file is available.

	*herculesfont*

.. _cmd:

Pause when entering commands
	Shows a command prompt window and pauses the game (like in SCI) instead of a real-time prompt.

	*commandpromptwindow* 

.. _2gs:

Add speed menu
	Add game speed menu (similar to PC version)

	*apple2gs_speedmenu* 
,,,,,,	

.. _BLADERUNNER:

**BLADERUNNER**
=================

.. _sitcom:

Sitcom mode
	Game will add laughter after actor's line or narration

	*sitcom* 

.. _shorty:

Shorty mode
	Game will shrink the actors and make their voices high pitched

	*shorty* 

.. _nodelay:

Frame limiter high performance mode
	This mode may result in high CPU usage! It avoids use of delayMillis() function.

	*nodelaymillisfl* 

.. _fpsfl:

Max frames per second limit
	This mode targets a maximum of 120 fps. When disabled, the game targets 60 fps

	*frames_per_secondfl* 

.. _stamina:

Disable McCoy's quick stamina drain
	When running, McCoy won't start slowing down as soon as the player stops clicking the mouse

	*disable_stamina_drain* 
,,,,,,,,

.. _CGE:

**CGE**
=================

.. _blind:

Color Blind Mode
	Enable Color Blind Mode by default

	*enable_color_blind* 

,,,,,,

.. _CINE:

**CINE**
=================

Use original save/load screen
	Use the original save/load screens instead of the ScummVM one

	*originalsaveload* 

.. _transparentdialog:

Use transparent dialog boxes in 16 color scenes
	Use transparent dialog boxes in 16 color scenes even if the original game version did not support them

	*transparentdialogboxes* 

,,,,,,,,,

.. _DRASCULA:

**DRASCULA**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

	*originalsaveload* 
,,,,,,,,,,

.. _DREAMWEB:

**DREAMWEB**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

	*originalsaveload* 

.. _bright:

Use bright palette mode
	Display graphics using the game's bright palette

	*bright_palette* 
,,,,,,,,,,

.. _HDB:

**HDB**
=================

.. _hyper:

Enable cheat mode
	Debug info and level selection becomes available

	*hypercheat* 

,,,,,,,,,,

.. _HOPKINS:

**HOPKINS**
=================

.. _gore:

Gore Mode
	Enable Gore Mode when available

	*enable_gore* 

,,,,,,,,,,

.. _KYRA:

**KYRA**
=================

.. _studio:

Enable studio audience
	Studio audience adds an applause and cheering sounds whenever Malcolm makes a joke

	*studio_audience* 

.. _skip:

Skip support
	This option allows the user to skip text and cutscenes.

	*skip_support* 

.. _helium:

Enable helium mode
	Helium mode makes people sound like they've inhaled Helium.

	*helium_mode* 

.. _smooth:

Enable smooth scrolling when walking
	When enabled, this option makes scrolling smoother when changing from one screen to another.

	*smooth_scrolling* 

.. _floating:

Enable floating cursors
	When enabled, this option changes the cursor when it floats to the  edge of the screen to a directional arrow. The player can then click to walk towards that direction.

	*floating_cursors* 
Suggest save names
	When enabled, this option will fill in an autogenerated savegame escription into the input prompt.

	*auto_savenames* 
.. _hp:

HP bar graphs
	Enable hit point bar graphs

	*hpbargraphs* 

.. _btswap:

Fight Button L/R Swap
	Left button to attack, right button to pick up items

	*mousebtswap* 

,,,,,,,,,,

.. _LURE:

**LURE**
=================

.. _ttsnarrator:

TTS Narrator
	Use text-to-speech to read the descriptions, if test-to-speech is available)

	*tts_narrator* 

,,,,,,,,,,

.. _MADS:

**MADS**
=================

.. _easy:

Easy mouse interface
	Shows object names when hovering the mouse over them

	*EasyMouse* 

.. _objanimated:

Animated inventory items
	Animated inventory items

	*InvObjectsAnimated* 

.. _windowanimated:

Animated game interface
	Animated game interface

	*TextWindowAnimated* 

.. _naughty:

Naughty game mode
	Naughty game mode

	*NaughtyMode* 

.. _gdither:

Graphics dithering
	Graphics dithering

	*GraphicsDithering* 

,,,,,,,,,,

.. _NEVERHOOD:

**NEVERHOOD**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

	*originalsaveload* 

.. _skiphall:

Skip the Hall of Records storyboard scenes
	Allows the player to skip past the Hall of Records storyboard scenes

	*skiphallofrecordsscenes* 

.. _scale:

Scale the making of videos to full screen
	Scale the making of videos, so that they use the whole screen

	*scalemakingofvideos* 

,,,,,,,,,,

.. _QUEEN:
**QUEEN**
==========

Alternate intro
	Plays the alternate intro for Flight of the Amazon Queen.
	
	*alt_intro* 

,,,,,,,,,,,,,,,

.. _SCI:

**SCI**
=================

.. _dither:

Skip EGA dithering pass (full color backgrounds)
	Skip dithering pass in EGA games, graphics are shown with full colors

	*disable_dithering* 

.. _hires:

Enable high resolution graphics
	Enable high resolution graphics/content

	*enable_high_resolution_graphics* 

.. _blackline:

Enable black-lined video
	Draw black lines over videos to increase their apparent sharpness

	*enable_black_lined_video* 

.. _hq:

Use high-quality video scaling
	Use linear interpolation when upscaling videos, where possible

	*enable_hq_video* 

.. _larry:

Use high-quality "LarryScale" cel scaling
	Use special cartoon scaler for drawing character sprites

	*enable_larryscale*

.. _dsfx:

Prefer digital sound effects
	Prefer digital sound effects instead of synthesized ones

	*prefer_digitalsfx*

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

	*originalsaveload* 

.. _fb01:

Use IMF/Yamaha FB-01 for MIDI output
	Use an IBM Music Feature card or a Yamaha FB-01 FM synth module for MIDI output

	*native_fb01* 

.. _cd:

Use CD audio
	Use CD audio instead of in-game audio, if available

	*use_cdaudio* 

.. _wincursors:

Use Windows cursors
	Use the Windows cursors (smaller and monochrome) instead of the DOS ones

	*windows_cursors* 

.. _silver:

Use silver cursors
	Use the alternate set of silver cursors instead of the normal golden ones

	*silver_cursors* 

.. _upscale:

Upscale videos
	Upscale videos to double their size

	*enable_video_upscale* 

.. _censor:

Enable content censoring	
	Enable the game's built-in optional content censoring"),
		
	*enable_censoring* 

,,,,,,,,,,

.. _SCUMM:

**SCUMM**
=================

.. _labels:

Show Object Line
	Show the names of objects at the bottom of the screen

	*object_labels* 

.. _classic:

Use NES Classic Palette
	Use a more neutral color palette that closely emulates the NES Classic

	*mm_nes_classic_palette* 

,,,,,,,,,,

.. _SHERLOCK:

**SHERLOCK**
=================
Use original load/save screens
	Use the original save/load screens instead of the ScummVM ones"),
	
	*originalsaveload* 

.. _fade:

Pixellated scene transitions
	When changing scenes, a randomized pixel transition is done

	*fade_style* 

.. _help:

Don't show hotspots when moving mouse
	Only show hotspot names after you actually click on a hotspot or action button

	*help_style* 

.. _portraits:

Show character portraits
	Show portraits for the characters when conversing

	*portraits_on* 

.. _style:

Slide dialogs into view
	Slide UI dialogs into view, rather than simply showing them immediately

	*window_style* 

.. _transparentwindows:

Transparent windows
	Show windows with a partially transparent background

	*transparent_windows* 

TTS Narrator
	Use text-to-speech to read the descriptions, if text-to-speech is available.

	*tts_narrator*
,,,,,,,,,,

.. _SKY:

**SKY**
=================
.. _altintro:

Floppy intro
	Use the floppy version's intro (CD version only)

	*alt_intro*
,,,,,,,,,,

.. _SUPERNOVA:

**SUPERNOVA**
=================
.. _improved:

Improved mode
	Removes some repetitive actions, adds possibility to change verbs by keyboard.

	*improved* 

,,,,,,,,,,

.. _TOLTECS:

**TOLTECS**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

	*originalsaveload* 
	
,,,,,,,,,,

.. _WINTERMUTE:

**WINTERMUTE**
=================

.. _fps:

Show FPS-counter
	Show the current number of frames per second in the upper left corner

	*show_fps* 

.. _bilinear:

Sprite bilinear filtering (SLOW)
	Apply bilinear filtering to individual sprites

	*bilinear_filtering* 

,,,,,,,,,,

.. _XEEN:

**XEEN**
=================

.. _cost:

Show item costs in standard inventory mode
	Shows item costs in standard inventory mode, allowing the value of items to be compared

	*ShowItemCosts* 

.. _durable:

More durable armor
	Armor won't break until character is at -80HP, rather than merely -10HP

	*DurableArmor* 

