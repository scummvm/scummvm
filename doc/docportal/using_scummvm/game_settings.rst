=============
Game settings
=============

Each game can be configured to run using settings other than the :doc:`global_settings`

To open the game settings window, highlight a game in the main Launcher
window, and click on **Edit Game**. This window also opens any time a
game is added to the Launcher.


There are many settings, and they are separated into tabs:

`Game tab`_ | `Engine tab`_ | `Graphics tab`_ | `Audio tab`_ | `Volume tab`_ | `Midi tab`_ | `Paths tab`_ |

Settings may also be edited directly in the configuration file. These configuration keywords are listed below in italics. For more information, see :doc:`../advanced_options/configuration_file` .

_`Game tab`
------------

Use the game tab to display and change information about how the game is
identified by ScummVM.

,,,,,,,

**ID**
	This field shows the short name of the game. This ID can be used for launching the game from the command line.

	*gameid* (string)

**Name**
	This field shows the full title of the game, as well as some basic information such as original platform and language.
	 
	*description* (string)

**Language**
	For multi-language games, this option changes the language of the game. However, for games that have the language hardcoded, this option only changes the font of the subtitles. For instance, selecting German will add an umlaut to German subtitles, and selecting Spanish will add the tilde in Spanish subtitles. 

	*language* (string)


**Platform**
	Specify the original platform of the game.

	*platform* (string)
,,,,,,,



_`Graphics tab`
-----------------

Use the graphics tab to change how the game looks when it is played. For more information including comprehensive explanation of the graphics settings, see the :doc:`../advanced_options/graphics` page. 

,,,,,,,

**Override global graphic settings**

	Check this box to make the game use custom settings instead of global settings.


**Graphics mode**
	Changes the resolution of the game. For example, a 2x graphics mode will take a 320x200 resolution game and scale it up to 640x400. 

	*gfx_mode* (string)

		

**Render mode**
	Changes how the game is rendered, which affects how the game looks. 

	*render_mode* (string)
			

**Stretch mode**
	Changes the way the game is displayed in relation to the window size.

	*stretch_mode* (string)

**Aspect ratio correction**
	If ticked, corrects the aspect ratio so that games appear the same as they would on original 320x200 resolution displays. 

	*aspect_ratio* (boolean)

**Fullscreen mode**
	Switches between playing games in a window, or playing them in fullscreen mode. Switch between the two by using :kbd:`Alt + F5` while in a game.

	*fullscreen* (boolean)

**Filter graphics**
	If ticked, uses bilinear interpolation instead of nearest neighbor resampling for the aspect ratio correction and stretch mode. It does not affect the graphics mode. 

	*filtering* (boolean)
,,,,,


_`Audio tab`
--------------------

Use the audio tab to change the sound output in the game.

For more information including comprehensive explanation of the audio settings, see the :doc:`../advanced_options/audio` page. 


,,,,,,,

**Override global audio settings**
	Check this box to make the game use custom settings instead of global settings.

**Music device**
	Specifies the device ScummVM uses to output audio. When set to <default>, ScummVM will automatically choose the most appropriate option for the played game. If set to an MT-32 or a MIDI device, or if ScummVM chooses one of these automatically, the global settings on the MT-32 or MIDI tabs also apply. See the :doc:`global_settings` page for more information. 

	*music_driver* (string)

**AdLib Emulator**
	Chooses which emulator is used by ScummVM when the AdLib emulator is chosen as the preferred device.

	*opl_driver* (string)

**Text and Speech**
	For games with digitized speech, this setting allows the user to decide whether to play the game with speech only, subtitles only, or both.

	*speech_mute* (boolean)

	*subtitles* (boolean)
	
**Subtitle speed**
	Adjusts the length of time that the subtitles are displayed on screen. The lower the speed is set, the longer the subtitles are displayed.

	*talkspeed* (number)

,,,,,,

_`Volume tab`
-----------------
Use the volume tab to set the relative volumes for various sounds in the game.

,,,,,,,

**Override global volume settings**
	Check this box to make the game use custom settings instead of the global ones.



**Music volume**
	Adjusts the volume of the music played back in the game. 

	*music_volume* (number)

**SFX volume**
	Adjusts the volume of the sound effects within the game.

	*sfx_volume* (number)


**Speech volume**
	Adjusts the volume of the digitized speech in the game, if it has any.

	*speech_volume* (number)

**Mute All**
	Mutes all game audio. 

	*mute* (boolean)

,,,,,,


_`MIDI tab`
---------------

Use the MIDI tab to change settings of the MIDI music in the game.

,,,,,,,

**Override global MIDI settings**
	Check this box to make the game use custom settings instead of the global settings.

**GM Device**
	Choose which software synthesizer (general midi device) to use to play midi music. 

	*gm_device* 

**Soundfont**
	Some midi devices require you to provide a soundfont, which contains samples of instruments for the device to play back. This setting allows you to choose a soundfont. 

	*soundfont* (string - path)


**Mixed AdLib/MIDI mode**
	Some games contain sound effects that are exclusive to the AdLib soundtrack. For these games, use this mode to combine MIDI music with AdLib sound effects.

	*multi_midi* (boolean)

**MIDI gain**
	Adjusts the relative volume of the general MIDI music. This is only supported by some music devices.
	 
	*midi_gain* (number)




_`MT-32 tab`
---------------

Override global MT-32 settings
	Check this box to make the game use custom settings instead of the global settings.

MT-32 Device
	++Find reference for this

**True Roland MT-32 (disable GM emulation)**
	Enable this option if you are using an actual Roland MT-32, LAPC-I, CM-64, CM-32L, CM-500, or a GS device with an MT-32 map.


**Roland GS device (enable MT-32 mappings)**
	 Enable this if you are using a GS device that has an MT-32 map, such as a SC-55, SC-88 or SC-8820. Roland GS mode may be disabled for games that use General MIDI natively. 

,,,,,


_`Paths tab`
--------------

Use the paths tab to tell ScummVM where to look for particular game files. 

,,,,,,,

**Save Path**
	The folder in which ScummVM will store the saved games. If this is not set, the saved games will be stored in the default directory.

	*savepath* (string)

**Extra Path**
	This is the folder that ScummVM will look for various extra files. These could include one or more of:

	* Additional datafiles required for certain games 
	* Soundfonts 
	* MT-32 ROMs 

	*extrapath* (string)

**Game Path**
	The folder in which the game’s data files are stored.

	*gamepath* (string)
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

TV emulation
	Emulate composite output to an NTSC TV.

	*ntsc* (boolean)
Color Graphics
	Use color graphics instead of monochrome.

	*color* (boolean)
Show scanlines
	Darken every other scanline to mimic the look of CRT.

	*scanlines* (boolean)
Always use sharp monochrome text
	Do not emulate NTSC artifacts for text

	*monotext* (boolean)
,,,,,,,,,

.. _AGI:

**AGI**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

	*originalsaveload* (boolean)

Use an alternative palette
	Use an alternative palette, common for all Amiga games. This was the old behavior

	*altamigapalette* (boolean)

Mouse support
	Enables mouse support. Allows to use mouse for movement and in game menus.

	*mousesupport* (boolean)

Use Hercules hires font
	Uses Hercules hi-res font, when font file is available.

	*herculesfont*

Pause when entering commands
	Shows a command prompt window and pauses the game (like in SCI) instead of a real-time prompt.

	*commandpromptwindow* (boolean)
Add speed menu
	Add game speed menu (similar to PC version)

	*apple2gs_speedmenu* (boolean)
,,,,,,	

.. _BLADERUNNER:

**BLADERUNNER**
=================

Sitcom mode
	Game will add laughter after actor's line or narration

	*sitcom* (boolean)

Shorty mode
	Game will shrink the actors and make their voices high pitched

	*shorty* (boolean)
Frame limiter high performance mode
	This mode may result in high CPU usage! It avoids use of delayMillis() function.

	*nodelaymillisfl* (boolean)

Max frames per second limit
	This mode targets a maximum of 120 fps. When disabled, the game targets 60 fps

	*frames_per_secondfl* (boolean)

Disable McCoy's quick stamina drain
	When running, McCoy won't start slowing down as soon as the player stops clicking the mouse

	*disable_stamina_drain* (boolean)
,,,,,,,,

.. _CGE:

**CGE**
=================

Color Blind Mode
	Enable Color Blind Mode by default

	*enable_color_blind* (boolean)
,,,,,,

.. _CINE:

**CINE**
=================

Use original save/load screen
	Use the original save/load screens instead of the ScummVM one

	*originalsaveload* (boolean)
Use transparent dialog boxes in 16 color scenes
	Use transparent dialog boxes in 16 color scenes even if the original game version did not support them

	*transparentdialogboxes* (boolean)
,,,,,,,,,

.. _DRASCULA:

**DRASCULA**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

	*originalsaveload* (boolean)
,,,,,,,,,,

.. _DREAMWEB:

**DREAMWEB**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

	*originalsaveload* (boolean)

Use bright palette mode
	Display graphics using the game's bright palette

	*bright_palette* (boolean)
,,,,,,,,,,

.. _HDB:

**HDB**
=================

Enable cheat mode
	Debug info and level selection becomes available

	*hypercheat* (boolean)
,,,,,,,,,,

.. _HOPKINS:

**HOPKINS**
=================

Gore Mode
	Enable Gore Mode when available

	*enable_gore* (boolean)
,,,,,,,,,,

.. _KYRA:

**KYRA**
=================

Enable studio audience
	Studio audience adds an applause and cheering sounds whenever Malcolm makes a joke

	*studio_audience* (boolean)
Skip support
	This option allows the user to skip text and cutscenes.

	*skip_support* (boolean)
Enable helium mode
	Helium mode makes people sound like they've inhaled Helium.

	*helium_mode* (boolean)
Enable smooth scrolling when walking
	When enabled, this option makes scrolling smoother when changing from one screen to another.

	*smooth_scrolling* (boolean)

Enable floating cursors
	When enabled, this option changes the cursor when it floats to the  edge of the screen to a directional arrow. The player can then click to walk towards that direction.

	*floating_cursors* (boolean)
Suggest save names
	When enabled, this option will fill in an autogenerated savegame escription into the input prompt.

	*auto_savenames* (boolean)
HP bar graphs
	Enable hit point bar graphs

	*hpbargraphs* (boolean)
Fight Button L/R Swap
	Left button to attack, right button to pick up items

	*mousebtswap* (boolean)
,,,,,,,,,,

.. _LURE:

**LURE**
=================

TTS Narrator
	Use text-to-speech to read the descriptions, if test-to-speech is available)

	*tts_narrator* (boolean)

,,,,,,,,,,

.. _MADS:

**MADS**
=================

Easy mouse interface
	Shows object names when hovering the mouse over them

	*EasyMouse* (boolean)
Animated inventory items
	Animated inventory items

	*InvObjectsAnimated* (boolean)
Animated game interface
	Animated game interface

	*TextWindowAnimated* (boolean)

Naughty game mode
	Naughty game mode

	*NaughtyMode* (boolean)

Graphics dithering
	Graphics dithering

	*GraphicsDithering* (boolean)
,,,,,,,,,,

.. _NEVERHOOD:

**NEVERHOOD**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

	*originalsaveload* (boolean)

Skip the Hall of Records storyboard scenes
	Allows the player to skip past the Hall of Records storyboard scenes

	*skiphallofrecordsscenes* (boolean)
Scale the making of videos to full screen
	Scale the making of videos, so that they use the whole screen

	*scalemakingofvideos* (boolean)
,,,,,,,,,,

.. _QUEEN:
**QUEEN**
==========

Alternate intro
	Plays the alternate intro for Flight of the Amazon Queen.
	
	*alt_intro* (boolean)

,,,,,,,,,,,,,,,
.. _SCI:

**SCI**
=================

Skip EGA dithering pass (full color backgrounds)
	Skip dithering pass in EGA games, graphics are shown with full colors

	*disable_dithering* (boolean)

Enable high resolution graphics
	Enable high resolution graphics/content

	*enable_high_resolution_graphics* (boolean)

Enable black-lined video
	Draw black lines over videos to increase their apparent sharpness

	*enable_black_lined_video* (boolean)

Use high-quality video scaling
	Use linear interpolation when upscaling videos, where possible

	*enable_hq_video* (boolean)
Use high-quality "LarryScale" cel scaling
	Use special cartoon scaler for drawing character sprites

	*enable_larryscale*

Prefer digital sound effects
	Prefer digital sound effects instead of synthesized ones

	*prefer_digitalsfx*

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

	*originalsaveload* (boolean)

Use IMF/Yamaha FB-01 for MIDI output
	Use an IBM Music Feature card or a Yamaha FB-01 FM synth module for MIDI output

	*native_fb01* (boolean)

Use CD audio
	Use CD audio instead of in-game audio, if available

	*use_cdaudio* (boolean)

Use Windows cursors
	Use the Windows cursors (smaller and monochrome) instead of the DOS ones

	*windows_cursors* (boolean)

Use silver cursors
	Use the alternate set of silver cursors instead of the normal golden ones

	*silver_cursors* (boolean)

Upscale videos
	Upscale videos to double their size

	*enable_video_upscale* (boolean)

Enable content censoring	
	Enable the game's built-in optional content censoring"),
		
	*enable_censoring* (boolean)
,,,,,,,,,,

.. _SCUMM:

**SCUMM**
=================

Show Object Line
	Show the names of objects at the bottom of the screen

	*object_labels* (boolean)

Use NES Classic Palette
	Use a more neutral color palette that closely emulates the NES Classic

	*mm_nes_classic_palette* (boolean)
,,,,,,,,,,

.. _SHERLOCK:

**SHERLOCK**
=================
Use original load/save screens
	Use the original save/load screens instead of the ScummVM ones"),
	
	*originalsaveload* (boolean)

Pixellated scene transitions
	When changing scenes, a randomized pixel transition is done

	*fade_style* (boolean)

Don't show hotspots when moving mouse
	Only show hotspot names after you actually click on a hotspot or action button

	*help_style* (boolean)

Show character portraits
	Show portraits for the characters when conversing

	*portraits_on* (boolean)

Slide dialogs into view
	Slide UI dialogs into view, rather than simply showing them immediately

	*window_style* (boolean)

Transparent windows
	Show windows with a partially transparent background

	*transparent_windows* (boolean)

TTS Narrator
	Use text-to-speech to read the descriptions, if text-to-speech is available.

	*tts_narrator*
,,,,,,,,,,

.. _SKY:

**SKY**
=================

Floppy intro
	Use the floppy version's intro (CD version only)

	*alt_intro* (boolean)
,,,,,,,,,,

.. _SUPERNOVA:

**SUPERNOVA**
=================

Improved mode
	Removes some repetitive actions, adds possibility to change verbs by keyboard.

	*improved* (boolean)
,,,,,,,,,,

.. _TOLTECS:

**TOLTECS**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

	*originalsaveload* (boolean)
,,,,,,,,,,

.. _WINTERMUTE:

**WINTERMUTE**
=================

Show FPS-counter
	Show the current number of frames per second in the upper left corner

	*show_fps* (boolean)

Sprite bilinear filtering (SLOW)
	Apply bilinear filtering to individual sprites

	*bilinear_filtering* (boolean)

,,,,,,,,,,

.. _XEEN:

**XEEN**
=================

Show item costs in standard inventory mode
	Shows item costs in standard inventory mode, allowing the value of items to be compared

	*ShowItemCosts* (boolean)

More durable armor
	Armor won't break until character is at -80HP, rather than merely -10HP

	*DurableArmor* (boolean)
,,,,,,,,,,
