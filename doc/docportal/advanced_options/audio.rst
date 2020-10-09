
=============
Audio
=============

ScummVM has several ways to adjust the audio playback of games, and these settings can be found in the Audio, MIDI MT-32 and Volume tabs of both the :doc:`global <../using_scummvm/global_settings>` and :doc:`game-specific <../using_scummvm/game_settings>` settings. They can also be changed directly in the :doc:`configuration file <../advanced_options/configuration_file>`, or passed as an option on the :doc:`command line <../advanced_options/command_line>`.  

General audio settings
------------------------

The **Preferred device** in the Audio tab specifies the device that ScummVM uses to output audio. This can be either an emulated sound device, or an actual device such as a soundcard. When set to <default>, ScummVM will automatically choose the most appropriate option for the played game. 

If **Preferred device** is set to an MT-32 or a General MIDI device (emulated or actual), or if ScummVM chooses one of these automatically, the settings on the MT-32 or MIDI tabs also apply. The MT-32 device chosen in the MT-32 tab is the device that ScummVM will use if the game has an MT-32 soundtrack. Similarly, the GM device chosen in the MIDI tab only applies when ScummVM plays a General MIDI soundtrack. 

Many of the settings described in this section are highly dependent on the soundtrack(s) of the game you are playing; some offer only MIDI, while others have MP3 soundtracks, and so on. If nothing changes when you select an option, chances are the game does not support that setting. 

For a look at the evolution of PC audio, see `this video on YouTube <https://www.youtube.com/watch?v=a324ykKV-7Y>`_, or `this excellent post <http://www.oldskool.org/sound/pc>`_. Many of these soundcards and devices can be emulated by ScummVM. 


MIDI Emulation
------------------

Some games only contain MIDI music data. In the past this prevented music for these games from working on platforms that did not support MIDI, or with soundcards that did not provide MIDI drivers. 

ScummVM can now emulate MIDI using sampled waves and either the AdLib, FluidSynth or MT-32 MIDI emulator. 

AdLib emulation
****************

By default an AdLib soundcard will be emulated and the music output as sampled waves. This offers the best compatibility between machines and games.

MT-32 emulation
********************

Some games which contain MIDI music data also have improved tracks designed for the `Roland MT-32 MIDI device <https://en.wikipedia.org/wiki/Roland_MT-32>`_. ScummVM can emulate the MT-32 device, however you must provide the original MT-32 ROMs that come with the game to make it work:

- MT32_PCM.ROM - IC21 (512KB)
- MT32_CONTROL.ROM - IC26 (32KB) and IC27 (32KB)

Place these ROMs in the game directory, in your extrapath, or in the directory where your ScummVM executable resides. Choose **MT-32 emulator** as your **Preferred device** in the Audio settings tab and as the **MT-32 Device** in the MT-32 settings tab.  You don't need to enable **True Roland MT-32** in the MT-32 tab, ScummVM does this automatically. 

NOTE: The processor requirements for the MT-32 emulator are quite high; a fast CPU is strongly recommended.

FluidSynth MIDI emulation
**************************

If the ScummVM you're using has libfluidsynth support it will be able to play MIDI music by using the FluidSynth emulator as the **Preferred device**. 

You will have to specify a SoundFont in the MIDI tab. MIDI is like digital sheet music; it needs a library of sound samples known as a SoundFont to draw from in order to synthesize music. See the `ScummVM forum <https://forums.scummvm.org/viewtopic.php?t=14541>`_ for an example of a great SoundFont.  


Since the default output volume from FluidSynth can be fairly low, ScummVM will automatically set the gain to get a stronger signal. This can be further adjusted using the :ref:`MIDI gain <gain>` setting. 

See the global settings page for more :ref:`FluidSynth settings <fluid>`.

NOTE: The processor requirements for FluidSynth are quite high; a fast CPU is recommended.


Native MIDI support
---------------------

If you have a soundcard or device capable of using native MIDI and you have selected this as the **Preferred device** in the Audio tab, you will need to specify what type of device this is with the options in the :ref:`MT-32 <mt32>` tab. Enabling the True Roland MT-32 option tells ScummVM that the MIDI device you have selected is an MT-32 device. Conversely, selecting Roland GS device tells ScummVM that the selected device is a GS device that can accept MT-32 mapping.  

These settings only apply to games where there is an MT-32 soundtrack and an MT-32 device is required. If neither of the MT-32 settings is enabled, ScummVM will initialize your chosen device in General MIDI mode and use General MIDI emulation for games with MT-32 soundtracks.

Some games contain sound effects that are exclusive to the AdLib soundtrack, or the AdLib soundtrack may provide better sound effects. For these games, you can combine MIDI music with AdLib sound effects by using the :ref:`mixed AdLib/MIDI mode <multi>`.


