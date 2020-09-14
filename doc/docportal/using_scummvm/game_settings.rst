=============
Game settings
=============

Each game can be configured to run using settings other than the :doc:`global_settings`

To open the game settings window, highlight a game in the main Launcher
window, and click on **Edit Game**. This window also opens any time a
game is added to the Launcher.

There are many settings, and they are separated into tabs:

`Game tab`_ | `Engine tab`_ | `Graphics tab`_ | `Audio tab`_ | `Volume tab`_ | `Midi tab`_ | `Paths tab`_ |


_`Game tab`
------------

Use the game tab to display and change information about how the game is
identified by ScummVM.

,,,,,,,

**ID**
	This field shows the short name of the game. This ID can be used for launching the game from Fullscreen modethe command line.

**Name**
	This field shows the full title of the game, as well as some basic information such as original platform and language.

**Language**
	This option is only applicable to Maniac Mansion, Zak McKracken, The Dig, The Curse of Monkey Island, Beneath a Steel Sky, Broken Sword 1, and Simon the Sorcerer 1 and 2. With the exception of Beneath a Steel Sky, Broken Sword 1, and multilanguage versions of Goblins games and Nippon Safes Inc., using this option does not change the language of the game, which is usually hardcoded, but rather is only used to select the appropriate font. For example, for a German version of a game, a font with umlauts. The Dig and The Curse of Monkey Island are another exception: the non-English versions of these games can be set to English, which only affect the language of subtitles. The game speech will remain the same.

**Platform**
	Specify the original platform of the game.

,,,,,,,



_`Graphics tab`
-----------------

Use the graphics tab to change how the game looks when it is played.

,,,,,,,

**Override global graphic settings**

	Check this box to make the game use custom settings instead of global settings.


**Graphics mode**
	Changes the graphic filter the game uses. This option has to be used to enable a graphic filter for a game with an original resolution higher than 320x200 or 320x240, because the corresponding global settings does not apply.



**Render mode**
	Changes the render mode the game uses.


**Aspect ratio correction**
	Most games supported by ScummVM were designed to be played at a screen resolution of 320x200 using rectangular pixels (higher than they were wide). Most modern systems use square pixels, which means that the image appears to be squeezed vertically and the characters look wider and shorter than they should. If this option is checked, ScummVM corrects for this by stretching the game window to a resolution of 320x240, which looks the same as 320x200 on old monitors. As with Graphic filters, this takes a little processing power to achieve. Aspect ratio correction can be combined with a Graphic filter.

**Fullscreen mode**
	Switches between playing games in a window, or playing them in fullscreen mode. Switch between the two by using :kbd:`Alt + F5` while in a game.

,,,,,


_`Audio tab`
--------------------

Use the audio tab to change the sound output in the game.

For more information about any of the audio settings, see the `Music and
sound <page>`__ page.

,,,,,,,

**Override global audio settings**
	Check this box to make the game use custom settings instead of global settings.



**Music driver**
	The method ScummVM uses to output MIDI music. For more details, see the section on music drivers.



**AdLib Emulator**
	The emulator used by ScummVM to generate the music when the AdLib music driver is selected. Two emulators are currently available: MAME OPL emulator was the emulator that was used up to version 0.13.1, and the DOSBox OPL emulator has been added but is still experimental.

**Output rate**
	The sample rate at which ScummVM plays sounds, including music if using an emulation music driver such as the AdLib music driver.

**Text and Speech**
	For games with digitized speech, setting allows the user to decide whether to play the game with speech and without any subtitles, or with subtitles displaying the words spoken in the digitized speech but not the speech, or with both.



**Subtitle speed**
	Adjusts the length of time that the subtitles are displayed on screen: the lower the speed is set, the longer the subtitles are displayed.

,,,,,,

_`Volume tab`
-----------------
Use the volume tab to set the relative volumes for various sounds in the game.

,,,,,,,

**Override global volume settings**
	Check this box to make the game use custom settings instead of the global ones.



**Music volume**
	Adjusts the volume of the music played back in the game. This is usually MIDI music played back with one of the music drivers, but some games use digitized music.


**SFX volume**
	Adjusts the volume of the sound effects within the game.


**Speech volume**
	Adjusts the volume of the digitized speech in the game, if it has any.

,,,,,,


_`MIDI tab`
---------------

Use the MIDI tab to change settings about the MIDI music in the game.

,,,,,,,

**Override global MIDI settings**
	Check this box to make the game use custom settings instead of the global settings.


**Soundfont**
	Some music drivers require you to provide them with a Soundfont, which contains samples of instruments for the device to play back. This setting allows you to choose one.


**Mixed AdLib/MIDI mode**
	Some games contain sound effects that are exclusive to the AdLib soundtrack. For these games, you may wish to use this mode in order to combine MIDI music with AdLib sound effects.


**MIDI gain**
	Adjusts the relative volume of the general MIDI music. This is only supported by some of the music drivers.


_`MT-32 tab`
---------------

Override global MT-32 settings
	Check this box to make the game use custom settings instead of the global settings.

MT-32 Device
	**Find out about this?**

**True Roland MT-32 (disable GM emulation)**
	ScummVM will treat your device as a real MT-32. Because the instrument mappings and system exclusive commands of the MT-32 vary from those of General MIDI devices, you should only enable this option if you are using an actual Roland MT-32, LAPC-I, CM-64, CM-32L, CM-500, or GS device with an MT-32 map.



**Roland GS device (enable MT-32 mappings)**
	ScummVM will initialize your GS-compatible device with settings that mimic the MT-32's reverb, (lack of) chorus, pitch bend sensitivity, etc. If it is specified in conjunction with True Roland MT-32 (above), ScummVM will select the MT-32-compatible map and drumset on your GS device. This setting works better than default GM or GS emulation with games that do not have custom instrument mappings (Loom and The Secret of Monkey Island). You should only specify both settings if you are using a GS device that has an MT-32 map, e.g. SC-55, SC-88, SC-8820, etc. Please note that Roland GS Mode is automatically disabled in both Day of the Tentacle and Sam & Max Hit the Road, since they use General MIDI natively. If neither of the above settings is enabled, ScummVM will initialize your device in General MIDI mode and use GM emulation in games with MT-32 soundtracks



,,,,,


_`Paths tab`
--------------

Use the paths tab to tell ScummVM where to look for particular files of
the game.

,,,,,,,

**Save Path**
	The folder in which ScummVM will store the game's saved games. If this is not set, the saved games will be stored in the default directory.



**Extra Path**
	This is the folder that ScummVM will look in for various extra files. These could include one or more of:

	* Additional datafiles required for certain games (for example, kyra.dat)
	* Soundfonts (see MIDI tab)
	* MT-32 ROMs (see MT-32 emulation)



**Game Path**
	The folder in which the game’s data files are stored.


,,,,,,,

_`Engine tab`
--------------

Some settings are unique to a particular game or game engine. Conversely, some settings may be greyed out if they are not applicable.

To find out which engine powers your game, have a look at the Supported Games `Wiki page
<https://wiki.scummvm.org/index.php?title=Category:Supported_Games>`_

Engines: ADL_ | AGI_ | BLADERUNNER_ | CGE_ | CINE_ | DRASCULA_ | DREAMWEB_ | HDB_ | HOPKINS_ | KYRA_ | LURE_ | MADS_ | NEVERHOOD_ | SCI_ | SCUMM_ | SHERLOCK_ | SKY_ | SUPERNOVA_ | TOLTECS_ | WINTERMUTE_ | XEEN_ |

,,,,,,,

.. _ADL:

**ADL**
=================

TV emulation
	Emulate composite output to an NTSC TV.

Color Graphics
	Use color graphics instead of monochrome.

Show scanlines
	Darken every other scanline to mimic the look of CRT.

Always use sharp monochrome text
	Do not emulate NTSC artifacts for text

,,,,,,,,,

.. _AGI:

**AGI**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

Use an alternative palette
	Use an alternative palette, common for all Amiga games. This was the old behavior

Mouse support
	Enables mouse support. Allows to use mouse for movement and in game menus.

Use Hercules hires font
	Uses Hercules hires font, when font file is available.

Pause when entering commands
	Shows a command prompt window and pauses the game (like in SCI) instead of a real-time prompt.

Add speed menu
	Add game speed menu (similar to PC version)

,,,,,,

.. _BLADERUNNER:

**BLADERUNNER**
=================

Sitcom mode
	Game will add laughter after actor's line or narration

Shorty mode
	Game will shrink the actors and make their voices high pitched

Frame limiter high performance mode
	This mode may result in high CPU usage! It avoids use of delayMillis() function.

Max frames per second limit
	This mode targets a maximum of 120 fps. When disabled, the game targets 60 fps

Disable McCoy's quick stamina drain
	When running, McCoy won't start slowing down as soon as the player stops clicking the mouse

,,,,,,,,

.. _CGE:

**CGE**
=================

Color Blind Mode
	Enable Color Blind Mode by default

,,,,,,

.. _CINE:

**CINE**
=================

Use original save/load screen
	Use the original save/load screens instead of the ScummVM one

Use transparent dialog boxes in 16 color scenes
	Use transparent dialog boxes in 16 color scenes even if the original game version did not support them

,,,,,,,,,

.. _DRASCULA:

**DRASCULA**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

,,,,,,,,,,

.. _DREAMWEB:

**DREAMWEB**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

Use bright palette mode
	Display graphics using the game's bright palette

,,,,,,,,,,

.. _HDB:

**HDB**
=================

Enable cheat mode
	Debug info and level selection becomes available

,,,,,,,,,,

.. _HOPKINS:

**HOPKINS**
=================

Gore Mode
	Enable Gore Mode when available

,,,,,,,,,,

.. _KYRA:

**KYRA**
=================

Enable studio audience
	Studio audience adds an applause and cheering sounds whenever Malcolm makes a joke

Skip support
	This option allows the user to skip text and cutscenes.

Enable helium mode
	Helium mode makes people sound like they've inhaled Helium.

Enable smooth scrolling when walking
	When enabled, this option makes scrolling smoother when changing from one screen to another.

Enable floating cursors
	When enabled, this option changes the cursor when it floats to the  edge of the screen to a directional arrow. The player can then click to walk towards that direction.

Suggest save names
	When enabled, this option will fill in an autogenerated savegame escription into the input prompt.

HP bar graphs
	Enable hit point bar graphs

Fight Button L/R Swap
	Left button to attack, right button to pick up items

,,,,,,,,,,

.. _LURE:

**LURE**
=================

TTS Narrator
	Use TTS to read the descriptions (if TTS is available)

,,,,,,,,,,

.. _MADS:

**MADS**
=================

Easy mouse interface
	Shows object names when hovering the mouse over them

Animated inventory items
	Animated inventory items

Animated game interface
	Animated game interface

Naughty game mode
	Naughty game mode

Graphics dithering
	Graphics dithering

,,,,,,,,,,

.. _NEVERHOOD:

**NEVERHOOD**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

Skip the Hall of Records storyboard scenes
	Allows the player to skip past the Hall of Records storyboard scenes

Scale the making of videos to full screen
	Scale the making of videos, so that they use the whole screen

,,,,,,,,,,

.. _SCI:

**SCI**
=================

Skip EGA dithering pass (full color backgrounds)
	Skip dithering pass in EGA games, graphics are shown with full colors

Enable high resolution graphics
	Enable high resolution graphics/content

Enable black-lined video
	Draw black lines over videos to increase their apparent sharpness

Use high-quality video scaling
	Use linear interpolation when upscaling videos, where possible

Use high-quality "LarryScale" cel scaling
	Use special cartoon scaler for drawing character sprites

Prefer digital sound effects
	Prefer digital sound effects instead of synthesized ones

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

Use IMF/Yamaha FB-01 for MIDI output
	Use an IBM Music Feature card or a Yamaha FB-01 FM synth module for MIDI output

Use CD audio
	Use CD audio instead of in-game audio, if available

Use Windows cursors
	Use the Windows cursors (smaller and monochrome) instead of the DOS ones

Use silver cursors
	Use the alternate set of silver cursors instead of the normal golden ones

Upscale videos
	Upscale videos to double their size

,,,,,,,,,,

.. _SCUMM:

**SCUMM**
=================

Show Object Line
	Show the names of objects at the bottom of the screen

Use NES Classic Palette
	Use a more neutral color palette that closely emulates the NES Classic

,,,,,,,,,,

.. _SHERLOCK:

**SHERLOCK**
=================

Enable content censoring
	Enable the game's built-in optional content censoring

Upscale videos
	Upscale videos to double their size

Pixellated scene transitions
	When changing scenes, a randomized pixel transition is done

Don't show hotspots when moving mouse
	Only show hotspot names after you actually click on a hotspot or action button

Show character portraits
	Show portraits for the characters when conversing

Slide dialogs into view
	Slide UI dialogs into view, rather than simply showing them immediately

Transparent windows
	Show windows with a partially transparent background

TTS Narrator
	Use TTS to read the descriptions (if TTS is available)

,,,,,,,,,,

.. _SKY:

**SKY**
=================

Floppy intro
	Use the floppy version's intro (CD version only)

,,,,,,,,,,

.. _SUPERNOVA:

**SUPERNOVA**
=================

Improved mode
	Improved mode

,,,,,,,,,,

.. _TOLTECS:

**TOLTECS**
=================

Use original save/load screens
	Use the original save/load screens instead of the ScummVM ones

,,,,,,,,,,

.. _WINTERMUTE:

**WINTERMUTE**
=================

Show FPS-counter
	Show the current number of frames per second in the upper left corner

Sprite bilinear filtering (SLOW)
	Apply bilinear filtering to individual sprites

,,,,,,,,,,

.. _XEEN:

**XEEN**
=================

Show item costs in standard inventory mode
	Shows item costs in standard inventory mode, allowing the value of items to be compared

More durable armor
	Armor won't break until character is at -80HP, rather than merely -10HP

,,,,,,,,,,
