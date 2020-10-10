
=============
Audio
=============

ScummVM has several ways to adjust the audio playback of games, and these settings can be found in the Audio, MIDI, MT-32 and Volume tabs of both the :doc:`global <../using_scummvm/global_settings>` and :doc:`game-specific <../using_scummvm/game_settings>` settings. They can also be changed directly in the :doc:`configuration file <../advanced_options/configuration_file>`, or passed as an option on the :doc:`command line <../advanced_options/command_line>`.  

For a look at the evolution of PC audio, see `this video on YouTube <https://www.youtube.com/watch?v=a324ykKV-7Y>`_, or `this excellent post <http://www.oldskool.org/sound/pc>`_. Many of these devices can be emulated by ScummVM. 

See the game manual to find out what sound hardware is supported by your game. 


General audio settings overview
---------------------------------

The **Preferred device** in the global settings Audio tab, or the **Music device** in the game-specific settings Audio tab, specifies the device that ScummVM uses to output audio. This device can be an emulated sound device or a software synthesizer, or an actual hardware device such as a soundcard or a hardware MIDI synthesizer. When set to **<default>**, ScummVM will automatically choose the most appropriate option for the played game. 

If **Preferred device** or **Music device** is set to **<default>** and ScummVM chooses either an MT-32 or General MIDI device automatically, the settings on the MT-32 or MIDI tabs also apply, and the devices selected on these tabs are the ones that will be used. If **Preferred device** or **Music device** is set to either an MT-32 device or a GM device, ScummVM will use that device and ignore the devices chosen on the MT-32 and MIDI tabs. 

Many of the settings described in this section are highly dependent on the soundtrack(s) of the game you are playing; some offer only MIDI, while others have MP3 soundtracks, and so on. If MIDI is not available for a game, the MT-32 and MIDI tabs are not available in game-specific settings. 

MIDI 
------

MIDI is a communications protocol for musical information; it can be likened to digital sheet music. By itself, MIDI is not sound. Hardware or software synthesizers create (synthesize) audio by interpreting the information given to them via the MIDI protocol. 

,,,,,,,,,,,,,,,,,,,,,,

General MIDI
******************

General MIDI is a MIDI standard which is implemented by a large number or devices. While using MIDI as its protocol, it also specifies an instrument map and some other information that devices must implement.

MT-32
******************

The MT-32 is a `Roland sound module <https://en.wikipedia.org/wiki/Roland_MT-32>`_, although the term also commonly refers to a range of devices that are compatible with the way instruments are mapped for the MT-32. MT-32 devices also use MIDI as the communications protocol.

,,,,,,,,,,,,,,,,,,,,


General MIDI and MT-32 MIDI data are not compatible. 

Some games only contain MIDI audio data. In the past this prevented audio for these games from working on platforms that did not support MIDI, or with soundcards that did not provide MIDI drivers. 

ScummVM can now convert MIDI data to sampled audio using MIDI device emulators. 

General MIDI device emulation (FluidSynth)
********************************************

If the ScummVM you're using has libfluidsynth support it will be able to play MIDI music by using the FluidSynth emulator if set as the **Preferred device** or **Music device**, or if specified in the MIDI tab when **Preferred device** or **Music device** is set to **<default>** and ScummVM chooses General MIDI output automatically. 

You will have to specify a SoundFont in the MIDI tab for ScummVM to use FluidSynth. MIDI is like digital sheet music; it needs a library of sound samples known as a SoundFont to draw from in order to synthesize music. See the `ScummVM forum <https://forums.scummvm.org/viewtopic.php?t=14541>`_ for an example of a great SoundFont.  


Since the default output volume from FluidSynth can be fairly low, ScummVM will automatically set the gain to get a stronger signal. This can be further adjusted using the :ref:`MIDI gain <gain>` setting. 

See the global settings page for more :ref:`FluidSynth settings <fluid>`.

The processor requirements for FluidSynth are quite high; a fast CPU is recommended.

MT-32 device emulation
************************

Some games which contain MIDI music data have tracks designed specifically for the Roland MT-32. ScummVM can emulate the MT-32 device, however you must provide the original MT-32 ROMs, taken from the MT-32 module, for the emulator to work. These files are:

- MT32_PCM.ROM - IC21 (512KB)
- MT32_CONTROL.ROM - IC26 (32KB) and IC27 (32KB)

Place these ROMs in the game directory, in your extrapath, or in the directory where your ScummVM executable resides. ScummVM will also look for ``CM32L_PCM.ROM`` and ``CM32LCONTROL.ROM`` - the ROMs from the CM-32L device - and will use these instead of the MT32 ROMs if they are available. 

ScummVM will use the MT-32 emulator if it is set as the **Preferred device** or **Music device**, or if it is specified in the MT-32 tab when **Preferred device** or **Music device** is set to **<default>** and ScummVM chooses MT-32 output automatically. 

You don't need to enable **True Roland MT-32** in the MT-32 tab, ScummVM does this automatically. 

.. tip::

    Some games work better with some MT-32 devices than others. As an example, Lure of the Temptress makes use of extra sound effects included with the CM-32L and won't sound right with an MT-32. Likewise, The Colonel's Bequest exploits some bugs in the early MT-32 modules, which means that later devices will play incorrect sound effects! 
    
    `This Wikipedia article <https://en.wikipedia.org/wiki/List_of_MT-32-compatible_computer_games>`_ provides a comprehensive list of MT-32 compatible games, including which games work best with which device.  

The processor requirements for the MT-32 emulator are quite high; a fast CPU is strongly recommended.


Native MIDI support
***********************

If you have a MIDI-capable device (hardware or software synthesizer) connected, it will show up in the **Preferred device** or **Music device** dropdown selector. If you have selected this device, you will also need to specify what type of device this is with the options in the :ref:`MT-32 <mt32>` tab. 

- Enabling the **True Roland MT-32** option tells ScummVM that the MIDI device you have selected is an MT-32 (or compatible) device. 
- Enabling **Roland GS device** tells ScummVM to use an MT-32 soundtrack on a GS device. This is not supported by all games.
- If no options are selected, this tells ScummVM that the selected device is General MIDI.  

Selecting an option that does not match the MIDI device selected may have unintended consequences; for example, if a game only has support for MT-32 and you have a General MIDI device selected as the **Preferred device** or **Music device**, ScummVM will convert the MT-32 MIDI data to GM-compatible MIDI data. However, GM devices do not support custom sound programming like MT-32 devices do, and the audio will not be correct. The opposite is also true, and is not supported by all games. 

.. note::

    Soundcards or audio interfaces do not necessarily have anything to do with MIDI; while some older soundcards (and a few modern ones) have their own hardware-based synthesizers, it is relatively rare. Generally, soundcard drivers work with software synthesizers to interpret MIDI and output audio. 


Mac OSX 
^^^^^^^^^^^^

Mac has a built-in MIDI synthesizer; Apple DLS software synthesizer. It uses the Mac's built-in sounds (based on Roland GS), and shows up independently from any connect MIDI devices. 

The `Apple Support page <https://support.apple.com/en-nz/guide/audio-midi-setup/ams875bae1e0/mac>`_ has further information about setting up MIDI devices on a Mac. 

Windows
^^^^^^^^^

Windows has a generic built-in MIDI synthesizer; GS WaveTable Synth.

For an in-depth look at audio and MIDI device setup on a Windows computer, see this `very helpful article <http://donyaquick.com/midi-on-windows/>`_.

Linux
^^^^^^^^^^

MIDI device setup may vary depending on your Linux distro. 

Here are a couple of helpful articles from the Ubuntu community documentation to get you started. These instructions should work for any Debian-based distro. 

`How to: Software Synthesizers <https://help.ubuntu.com/community/Midi/SoftwareSynthesisHowTo>`_

`How to: Hardware synthesizer soundcard setup <https://help.ubuntu.com/community/Midi/HardwareSynthesisSetup?action=show&redirect=MidiHardwareSynthesisSetup>`_



AdLib 
--------

AdLib devices do not use MIDI. They instead have a chip that produces sound via FM synthesis. While some games do store their audio data using a MIDI-derived format, this is converted by the game to work with the AdLib chip. ScummVM emulates a few different AdLib configuations, and selects the most appropriate for the game:

- The original AdLib and SoundBlaster card had one OPL2 chip. 
- The SoundBlaster Pro 1 had two OPL2 chips
- The SoundBlaster Pro 2 and 16 had an OPL3 chip. 

The AdLib emulator setting offers MAME, DOSBox and Nuked emulation, with MAME being the least accurate and using the least CPU power, and Nuked being the most accurate and also using the most CPU power - DOSBox is somewhere in between. 

There is also the option to select the OPL2LPT and OPL3LPT devices, which are external hardware devices with a real OPL chip, connected via the parallel port of a computer. 

Mixed AdLib/MIDI mode
------------------------
Some games contain sound effects that are exclusive to the AdLib soundtrack, or the AdLib soundtrack may provide better sound effects. For these games, you can combine MIDI music with AdLib sound effects by using the :ref:`mixed AdLib/MIDI mode <multi>`.

.. note::

    Mixed AdLib/MIDI mode is not supported by all games. 


