===============
Global settings
===============

Games will follow the global game settings unless `game-specific
settings <using/game_settings>`__ are applied.

To open the global settings window, click **Options** in the main
Launcher window.

There are many settings, and they are separated into tabs:

`Graphics tab`_ | `Audio tab`_ | `Volume tab`_ | `Midi tab`_ | `MT-32 tab`_ | `Paths tab`_ | `Misc tab`_ | `Cloud tab`_ | `LAN tab`_ | `Accessibility tab`_ |

Settings may also be edited directly in the configuration file. These configuration keywords are listed below in italics. For more information, see :doc:`../advanced_options/configuration_file` .

_`Graphics tab`
---------------

Use the graphics tab to change how games look when they are played.

,,,,,,,

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


,,,,,,,,,,,,,,,,,,

_`Audio tab`
-------------------

Use the audio tab to change the sound output for games.

For more information including comprehensive explanation of the audio settings, see the :doc:`../advanced_options/audio` page.

,,,,,,,,,,,,,,,,

**Music device**
	The method ScummVM uses to output MIDI music. 

	*music_driver* (string)

**AdLib Emulator**
	The emulator used by ScummVM to generate the music when the AdLib music driver is selected. 

	*opl_driver* (string)

**Text and Speech**
	For games with digitized speech, this setting allows the user to decide whether to play the game with speech and without any subtitles, or with subtitles displaying the words spoken in the digitized speech but without the speech, or with both.

	*speech_mute* (boolean)

	*subtitles* (boolean)
	
**Subtitle speed**
	Adjusts the length of time that the subtitles are displayed on screen. The lower the speed is set, the longer the subtitles are displayed.

	*talkspeed* (number)
,,,,,,,,,,,,,,,

_`Volume tab`
---------------

Use the volume tab to set the relative volumes for various sounds.

,,,,,,,,,,,,,

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

,,,,,,,,,,,,,,,,,,


_`MIDI tab`
----------------

Use the MIDI tab to change settings about the MIDI music.

,,,,,,,,,,,,,

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

FluidSynth settings	
	Opens a new dialog, with three further tabs:
	`Reverb`_ | `Chorus`_ | `Misc`_

,,,,,,,,,,,,,,,,,,

_`Reverb`
*************

Reverberation, or reverb, describes the pattern of echoes and reflections that occur when a sound is heard in an enclosed closed space, such as a room or a concert hall. 

Active
	If ticked, reverb is added to the software synthesizer output. 

	*fluidsynth_reverb_activate* (boolean)

Room
	Sets the room size, which determines how much reverb there is. 

	*fluidsynth_reverb_roomsize* (number)

Damp
	Dampens higher-frequency reverb, which produces a warmer sound. 

	*fluidsynth_reverb_damping* (number)

Width
	Sets the stereo spread of the reverb signal. 

	*fluidsynth_reverb_width* (number)

Level	
	Sets the reverb output amplitude. 

	*fluidsynth_reverb_level*

,,,,,,,,,,,,,,,,,

_`Chorus`
**********
The chorus effect thickens and colors the audio signal so that it sounds as if there are a chorus of instruments playing one part. 

Active	
	If ticked, chorus effects are added to the software synthesizer output. 

	*fluidsynth_chorus_activate* (boolean)

N
	Sets the voice count of the chorus, in other words, how many instruments are playing one part.

	*fluidsynth_chorus_nr* (number)

Level
	Specifies the output amplitude of the chorus signal.

	*fluidsynth_chorus_level* (number)

Speed
	Sets the modulation speed of the output, in Hz.

	*fluidsynth_chorus_speed* (number)

Depth
	Specifies the modulation depth of the chorus.

	*fluidsynth_chorus_depth* (number)

Type
	++Find reference for this

	*fluidsynth_chorus_waveform* (string)

,,,,,,,,,,,,,

_`Misc`
*********
Interpolation
	Sets the interpolation method used by the software synthesizer. 

	*fluidsynth_misc_interpolation* (string)
,,,,,,,,,,,,,,,


_`MT-32 tab`
---------------

MT-32 Device
	++Clarify? Would an external MIDI synthesizer listed below show up here??

**True Roland MT-32 (disable GM emulation)**
	Enable this option if you are using an actual Roland MT-32, LAPC-I, CM-64, CM-32L, CM-500, or a GS device with an MT-32 map.


**Roland GS device (enable MT-32 mappings)**
	 Enable this if you are using a GS device that has an MT-32 map, such as a SC-55, SC-88 or SC-8820. Roland GS mode may be disabled for games that use General MIDI natively. 

,,,,,

_`Paths tab`
--------------------

Use the paths tab to tell ScummVM where to look for particular files.

,,,,,,,,,,,,,,,,,

Save Path
	The default folder in which ScummVM will store saved games. If this is not set, saved games will generally be stored in the current directory. Exceptions to this include:

	* Windows Vista and up ``\Users\username\AppData\Roaming\ScummVM\Saved games\``

	* Windows 2000 and XP ``\Documents and Settings\username\ApplicationData\ScummVM\Saved games\``

	* Mac OS X ``$HOME/Documents/ScummVM Savegames/``

	* Other UNIX variants ``$HOME/.scummvm/``

	* iPhone ``/private/var/mobile/Library/ScummVM/Savegames``
	
	*savepath* (string - path)

Theme Path
	The folder that additional themes for the ScummVM Launcher are stored in.

	*themepath* (string - path)

Extra Path
	This is the folder that ScummVM will look in for various extra files. These could include one or more of:

	* Additional datafiles required for certain games
	* Soundfonts 
	* MT-32 ROMs 

	*extrapath* (string - path)

,,,,,,,,,,,,,,

_`Misc tab`
----------------------

The Misc tab contains options that don’t belong on any of the other tabs.

,,,,,,,,,,,,,,,

Theme
	Changes the visual appearance of the ScummVM Launcher

	*gui_theme* (string)

GUI Renderer
	Defines how the ScummVM GUI is rendered; normal or antialiased.

	*gui_renderer* (string)

Autosave
	Adjusts the time period that ScummVM waits between autosaves. The default setting is 5 minutes. This is not available for all games. 

	*autosave_period* (number)

GUI Language
	Choose the language of the ScummVM Launcher

	*gui_language* (string)

Switch the GUI language to the game language
	If ticked, the Launcher language will be the same as the game language. 

	*gui_use_game_language* (boolean)

,,,,,,,,,,,,,

_`Cloud tab`
--------------

The Cloud tab contains options for connecting Cloud-based services to enable a sync of games and saved states across multiple devices. This tab may not be available on all platforms. 

,,,,,,,,,,,,,,,,,,,,,,,,,,

Active storage
	Choose from Dropbox, OneDrive, Google Drive, or Box. 

For more information on how to connect a Cloud storage service to ScummVM, see the :doc:`cloud_and_lan` page. 

Once a Cloud service has been selected, further options are available. 

Sync Now
	Manually sync saved games with the Cloud service. Saved games automatically sync on launch, after saving, on on game load. 

Download game files
	Use this option to download game files from your Cloud ScummVM folder.

Disconnect
	Use this option to disconnect the Cloud storage account. To change accounts, disconnect and connect again. 

,,,,,,,,,,,,,,,,,,,,,,,

_`LAN tab`
----------

The LAN tab contains options for starting a local network web server which allows browser-based file management, including uploading and downloading game files. The web server will only run while the Options window remains open.

For more information, see the :doc:`cloud_and_lan` page. 

,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,

Run server
	Starts the web server. Once running, this displays the URL at which the web server can be accessed. 

Server's port
	The port on which the web server is available. 
	
	*local_server_port* (number)

/root/ Path	
	Allows you to specify the root path. Any sub-directories will be accessible. 

	*rootpath* (string - path)

,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,

_`Accessibility tab`
--------------------

Use Text to Speech
	If ticked, enables text to speech. As the cursor hovers over any text field in the Launcher it is converted to speech. Choose from a variety of voices. 

	*tts_enabled* (boolean)

,,,,,,,,,,,,,,,,,,,,,