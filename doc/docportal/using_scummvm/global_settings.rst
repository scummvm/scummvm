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

.. _gfxmode:

Graphics mode
	Changes the resolution of the game. For example, a 2x graphics mode will take a 320x200 resolution game and scale it up to 640x400. 

	*gfx_mode* 

		
.. _render:

Render mode
	Changes how the game is rendered, which affects how the game looks. 

	*render_mode* 
			
.. _stretchmode:

Stretch mode
	Changes the way the game is displayed in relation to the window size.

	*stretch_mode* 

.. _ratio:

Aspect ratio correction
	If ticked, corrects the aspect ratio so that games appear the same as they would on original 320x200 resolution displays. 

	*aspect_ratio* 

.. _fullscreen:

Fullscreen mode
	Switches between playing games in a window, or playing them in fullscreen mode. Switch between the two by using :kbd:`Alt + F5` while in a game.

	*fullscreen* 

.. _filtering:

Filter graphics
	If ticked, uses bilinear interpolation instead of nearest neighbor resampling for the aspect ratio correction and stretch mode. It does not affect the graphics mode. 

	*filtering* 


,,,,,,,,,,,,,,,,,,

_`Audio tab`
-------------------

Use the audio tab to change the sound output for games.

For more information including comprehensive explanation of the audio settings, see the :doc:`../advanced_options/audio` page.

,,,,,,,,,,,,,,,,

.. _device:

**Preferred device**
	Specifies the device ScummVM uses to output audio. When set to <default>, ScummVM will automatically choose the most appropriate option for the played game. If set to an MT-32 or a MIDI device, or if ScummVM chooses one of these automatically, the settings on the MT-32 or MIDI tabs also apply.

	*music_driver* 

.. _opl:

**AdLib Emulator**
	Chooses which emulator is used by ScummVM when the AdLib emulator is chosen as the preferred device.

	*opl_driver* 

.. _speechmute:

**Text and Speech**
	For games with digitized speech, this setting allows the user to decide whether to play the game with speech only, subtitles only, or both. 

	*speech_mute* 

	*subtitles* 

.. _talkspeed:

**Subtitle speed**
	Adjusts the length of time that the subtitles are displayed on screen. The lower the speed is set, the longer the subtitles are displayed.

	*talkspeed* 

,,,,,,,,,,,,,,,

_`Volume tab`
---------------

Use the volume tab to set the relative volumes for various sounds.

,,,,,,,,,,,,,

.. _music:

**Music volume**
	Adjusts the volume of the music played back in the game. 

	*music_volume* 

**SFX volume**
	Adjusts the volume of the sound effects within the game.

	*sfx_volume* 


**Speech volume**
	Adjusts the volume of the digitized speech in the game, if it has any.

	*speech_volume* 

.. _mute:

**Mute All**
	Mutes all game audio. 

	*mute* 

,,,,,,,,,,,,,,,,,,


_`MIDI tab`
----------------

Use the MIDI tab to change the settings of General MIDI devices.

,,,,,,,,,,,,,

.. _gm:

**GM Device**
	Specifies a preferred General MIDI (GM) device, which ScummVM uses whenever General MIDI playback is required.   

	*gm_device* 

**Soundfont**
	Specifies the path to a soundfont file, if this is required by the GM device. 

	*soundfont* 

.. _multi:

**Mixed AdLib/MIDI mode**
	Combines MIDI music with AdLib sound effects. 

	*multi_midi* 

.. _gain:

**MIDI gain**
	Adjusts the relative volume of the MIDI music. This is only supported by some music devices.
	 
	*midi_gain* 

FluidSynth settings	
	Opens a new dialog, with three further tabs:
	`Reverb`_ | `Chorus`_ | `Misc`_

,,,,,,,,,,,,,,,,,,

_`Reverb`
*************

Reverberation, or reverb, describes the pattern of echoes and reflections that occur when a sound is heard in an enclosed closed space, such as a room or a concert hall. 

.. _revact:

Active
	If ticked, reverb is added to the software synthesizer output. 

	*fluidsynth_reverb_activate* 

.. _revroom:

Room
	Sets the room size, which determines how much reverb there is. 

	*fluidsynth_reverb_roomsize* 

.. _revdamp:

Damp
	Dampens higher-frequency reverb, which produces a warmer sound. 

	*fluidsynth_reverb_damping* 

.. _revwidth:

Width
	Sets the stereo spread of the reverb signal. 

	*fluidsynth_reverb_width* 

.. _revlevel:

Level	
	Sets the reverb output amplitude. 

	*fluidsynth_reverb_level*

,,,,,,,,,,,,,,,,,

_`Chorus`
**********
The chorus effect thickens and colors the audio signal so that it sounds as if there are a chorus of instruments playing one part. 

.. _chact:

Active	
	If ticked, chorus effects are added to the software synthesizer output. 

	*fluidsynth_chorus_activate* 

.. _chnr:

N
	Sets the voice count of the chorus, in other words, how many instruments are playing one part.

	*fluidsynth_chorus_nr* 

.. _chlevel:

Level
	Specifies the output amplitude of the chorus signal.

	*fluidsynth_chorus_level* 

.. _chspeed:

Speed
	Sets the modulation speed of the output, in Hz.

	*fluidsynth_chorus_speed* 

.. _chdepth:

Depth
	Specifies the modulation depth of the chorus.

	*fluidsynth_chorus_depth* 

.. _chwave:

Type
	Specifies the chorus modulation waveform type. 

	*fluidsynth_chorus_waveform* 

,,,,,,,,,,,,,

_`Misc`
*********
.. _interp:

Interpolation
	Sets the interpolation method used by the software synthesizer. 

	*fluidsynth_misc_interpolation* 

,,,,,,,,,,,,,,,


_`MT-32 tab`
---------------
Use the MT-32 tab to change the settings of MT-32 devices.

,,,,,,,,,,,,,

.. _mt32:

MT-32 Device
	Specifies a preferred MT-32 device, which ScummVM uses whenever MT-32 playback is required.  

	*mt32_device*

.. _nativemt32:

**True Roland MT-32 (disable GM emulation)**
	Enable this option only if you are using an actual Roland MT-32, LAPC-I, CM-64, CM-32L, CM-500 or other MT-32 compatible device. Note that this cannot be used in conjuntion with the Roland GS device option. 

	*native_mt32*

.. _gs:

**Roland GS device (enable MT-32 mappings)**
	 Enable this option if you are using a GS device that has an MT-32 map, such as an SC-55, SC-88 or SC-8820. Note that this cannot be used in conjunction with the True Roland MT-32 option. 

,,,,,

_`Paths tab`
--------------------

Use the paths tab to tell ScummVM where to look for particular files.

,,,,,,,,,,,,,,,,,

.. _savepath:

Save Path
	The default folder in which ScummVM will store saved games. If this is not set, saved games will generally be stored in the current directory. Exceptions to this include:

	* Windows Vista and up ``\Users\username\AppData\Roaming\ScummVM\Saved games\``

	* Windows 2000 and XP ``\Documents and Settings\username\ApplicationData\ScummVM\Saved games\``

	* Mac OS X ``$HOME/Documents/ScummVM Savegames/``

	* Other UNIX variants ``$HOME/.scummvm/``

	* iPhone ``/private/var/mobile/Library/ScummVM/Savegames``
	
	*savepath* 

.. _themepath:

Theme Path
	The folder that additional themes for the ScummVM Launcher are stored in.

	*themepath* 

.. _extra:

Extra Path
	This is the folder that ScummVM will look in for various extra files. These could include one or more of:

	* Additional datafiles required for certain games
	* Soundfonts 
	* MT-32 ROMs 

	*extrapath* 

,,,,,,,,,,,,,,

_`Misc tab`
----------------------

The Misc tab contains options that don’t belong on any of the other tabs.

,,,,,,,,,,,,,,,

Theme
	Changes the visual appearance of the ScummVM Launcher

	*gui_theme* 

GUI Renderer
	Defines how the ScummVM GUI is rendered; normal or antialiased.

	*gui_renderer* 

Autosave
	Adjusts the time period that ScummVM waits between autosaves. The default setting is 5 minutes. This is not available for all games. 

	*autosave_period* 

GUI Language
	Choose the language of the ScummVM Launcher

	*gui_language* 

Switch the GUI language to the game language
	If ticked, the Launcher language will be the same as the game language. 

	*gui_use_game_language* 

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

.. _serverport:

Server's port
	The port on which the web server is available. 
	
	*local_server_port* 

.. _rootpath:

/root/ Path	
	Allows you to specify the root path. Any sub-directories will be accessible. 

	*rootpath* 

,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,

_`Accessibility tab`
--------------------

.. _ttsenabled:

Use Text to Speech
	If ticked, enables text to speech. As the cursor hovers over any text field in the Launcher it is converted to speech. Choose from a variety of voices. 

	*tts_enabled* 

,,,,,,,,,,,,,,,,,,,,,