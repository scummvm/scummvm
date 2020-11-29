======================================================
Understanding the audio settings
======================================================

This guide expands on the information contained on the :doc:`audio settings <../settings/audio>` page. 


How do the audio settings work together?
==========================================

The **Preferred device** in the global settings Audio tab, or the **Music device** in the game-specific settings Audio tab, specifies the device that ScummVM uses to output audio. This device can be an emulated sound device or a software synthesizer, or an actual hardware device such as a soundcard or a hardware MIDI synthesizer. When set to **<default>**, ScummVM will automatically choose the most appropriate option for the played game. 

If **Preferred device** or **Music device** is set to **<default>** and ScummVM chooses either an MT-32 or General MIDI device automatically, the settings on the MT-32 or MIDI tabs also apply, and the devices selected on these tabs are the ones that will be used. If **Preferred device** or **Music device** is set to either an MT-32 device or a GM device, ScummVM will use that device and ignore the devices chosen on the MT-32 and MIDI tabs. 

Not all settings are applicable to all games. For example, changing the General MIDI settings will not have any effect on a game that only has CD audio. 


The evolution of PC audio
=============================

For a look at the evolution of PC audio, see `this video on YouTube <https://www.youtube.com/watch?v=a324ykKV-7Y>`_, or `this excellent post <http://www.oldskool.org/sound/pc>`_. 

Many of these old-school audio devices can be emulated by ScummVM:

- PC Speaker: emulates the built-in PC speaker. This is mostly supported by older games, and was the only option before sound cards became widely used. 

    *pcspk*

- IBM PCjr: emulates the sound of the 1984 IBM PCjr computer, which, enhanced with a Texas Instruments chip, provided three-voice sound and a white noise generator. 

    *pcjr*

- Creative Music System `(C/MS) <https://en.wikipedia.org/wiki/Sound_Blaster#Creative_Music_System>`_: emulates the first sound card developed by Creative Technology (later Creative Labs), the precursor to the SoundBlaster line of sound cards. The C/MS provided 12 channels of square-wave stereo sound.   

    *cms*

- C64 Audio: emulates the sound chip `(Sound Interface Device) <https://theconversation.com/the-sound-of-sid-35-years-of-chiptunes-influence-on-electronic-music-74935>`_ in the Commodore 64 computer. The SID was a three-voice synthesizer module, with a fourth voice for sampled drums or speech. 

    *C64*

- Amiga Audio: emulates the Amiga audio chip, `Paula <https://en.wikipedia.org/wiki/Original_Chip_Set#Audio>`_, which had four 8-bit PCM sound channels. 

    *CAMD*

- FM-Towns Audio: emulates the audio of the `FM Towns PC  <https://en.wikipedia.org/wiki/FM_Towns#Sound>`_. Games on FM Towns computers often used audio CD standard tracks. The soundchips were capable of eight PCM voices and six FM channels.  

    *towns*

- PC-98 Audio: emulates the audio of the NEC PC-9801-26 and PC-9801-86 sound cards. 

    *pc98*

- SegaCD Audio: emulates the audio of the Sega CD add-on for the Sega Genesis/32x. 

    *segacd*

To find out which emulation is compatible with the game you're playing, have a look at the manual that comes with the game.

What is MIDI? 
======================

MIDI is a communications protocol for musical information; it can be likened to digital sheet music. By itself, MIDI is not sound. Hardware or software synthesizers create (synthesize) audio by interpreting the information given to them by the MIDI protocol. 

While some older soundcards (and a few modern ones) have their own hardware-based synthesizers, this is relatively rare. Generally, soundcard drivers work with software synthesizers to interpret MIDI and output audio. 

Some games only contain MIDI audio data. In the past this prevented audio for these games from working on platforms that did not support MIDI, or with soundcards that did not provide MIDI drivers. ScummVM can now convert MIDI data to sampled audio using MIDI device emulators. 


What is General MIDI?
------------------------------

General MIDI is a MIDI standard which is implemented by a large number of devices. While using MIDI as its protocol, it also specifies an instrument map and some other information that devices must implement.


.. _FS:

General MIDI device emulation (FluidSynth)
*************************************************

If the ScummVM you're using has libfluidsynth support it will be able to play MIDI music by using the FluidSynth emulator if set as the **Preferred device** or **Music device**, or if specified in the MIDI tab when **Preferred device** or **Music device** is set to **<default>** and ScummVM chooses General MIDI output automatically. 

You will have to specify a SoundFont in the MIDI tab for ScummVM to use FluidSynth. MIDI is like digital sheet music; it needs a library of sound samples known as a SoundFont to draw from to synthesize music. See the `ScummVM forum <https://forums.scummvm.org/viewtopic.php?t=14541>`_ for an example of a great SoundFont.  

The default output volume from FluidSynth can be fairly low, so ScummVM automatically sets the gain to get a stronger signal. Use the :ref:`MIDI gain <gain>` setting to further adjust this. 

The processor requirements for FluidSynth are quite high; a fast CPU is recommended.

What is MT-32?
---------------------

The MT-32 is a `Roland sound module <https://en.wikipedia.org/wiki/Roland_MT-32>`_, although the term also commonly refers to a range of devices that are fully compatible with the MT-32. MT-32 devices also use MIDI as the communications protocol.


.. _MT-32:

MT-32 device emulation
********************************

Some games which contain MIDI music data have tracks designed specifically for the Roland MT-32. ScummVM can emulate the MT-32 device, however you must provide the original MT-32 ROMs, taken from the MT-32 module, for the emulator to work. These files are:

- MT32_PCM.ROM - IC21 (512KB)
- MT32_CONTROL.ROM - IC26 (32KB) and IC27 (32KB)

Place these ROMs in the game directory, in your extrapath, or in the directory where your ScummVM executable resides. ScummVM also looks for ``CM32L_PCM.ROM`` and ``CM32L_CONTROL.ROM``—the ROMs from the CM-32L device—and uses these instead of the MT32 ROMs if they are available. 

.. note::

    The MT-32 ROMs are copyrighted, and are not provided by ScummVM. These must be taken from your own MT-32 module. 

ScummVM uses the MT-32 emulator if it is set as the **Preferred device** or **Music device**, or if it is specified in the MT-32 tab when **Preferred device** or **Music device** is set to **<default>** and ScummVM chooses MT-32 output automatically. 

You don't need to enable **True Roland MT-32** in the MT-32 tab, ScummVM does this automatically. 

.. tip::

    Some games work better with some MT-32 devices than others. As an example, Lure of the Temptress makes use of extra sound effects included with the CM-32L and won't sound right with an MT-32. Likewise, The Colonel's Bequest uses some bugs in the early MT-32 modules, which means that later devices will play incorrect sound effects! 
    
    `This Wikipedia article <https://en.wikipedia.org/wiki/List_of_MT-32-compatible_computer_games>`_ provides a comprehensive list of MT-32 compatible games, including which games work best with which device.  

The processor requirements for the MT-32 emulator are quite high; a fast CPU is strongly recommended.


Built-in MIDI support
--------------------------

All MIDI ports show up in the **Preferred device** or **Music device** dropdown selector. If you have selected a MIDI port, you need to specify what type of MIDI device this is with the options in the :ref:`MT-32 <mt32>` tab. 

- Enable **True Roland MT-32** to tell ScummVM that the MIDI device is an MT-32 (or fully compatible) device. 
- Enable **Roland GS device** to tell ScummVM to use an MT-32 soundtrack on a GS device. This is not supported by all games.
- If no options are selected, ScummVM treats the device on the port as a General MIDI device.  

If you select an option that does not match the actual device, this might have unintended consequences. For example, if a game only has support for MT-32 and you have a General MIDI device selected as the **Preferred device** or **Music device**, ScummVM will convert the MT-32 MIDI data to GM-compatible MIDI data. While this might work fine for some games, it really depends on how the game has made use of the MT-32. 



macOS/Mac OSX 
***************

Mac has a built-in MIDI synthesizer; Apple DLS software synthesizer. It uses the Mac's built-in sounds (which are based on Roland GS).

The `Apple Support page <https://support.apple.com/en-nz/guide/audio-midi-setup/ams875bae1e0/mac>`_ has further information about setting up MIDI devices on a Mac. 

Windows
**********

Windows has a generic built-in MIDI synthesizer—GS WaveTable Synth—also based on Roland's GS sounds. 

For an in-depth look at audio and MIDI device setup on a Windows computer, see this `very helpful article <http://donyaquick.com/midi-on-windows/>`_.

Linux
******

MIDI device setup might vary depending on your Linux distro. 

If you do not have a hardware MIDI device, there are two options: FluidSynth and TiMidity. FluidSynth is recommended as TiMidity might have some lag, depending on the system. 

Here are a couple of helpful articles from the Ubuntu community documentation to get you started. 

`How to: Software Synthesizers <https://help.ubuntu.com/community/Midi/SoftwareSynthesisHowTo>`_

`How to: Hardware synthesizer soundcard setup <https://help.ubuntu.com/community/Midi/HardwareSynthesisSetup?action=show&redirect=MidiHardwareSynthesisSetup>`_

,,,,,,,,,,,,,,,,,,,,

.. _adlib:

What is AdLib? 
================

AdLib devices do not use MIDI. They instead have a chip that produces sound through FM synthesis. While some games do store their audio data using a MIDI-derived format, this is converted by the game to work with the AdLib chip. ScummVM emulates a few different AdLib configurations, and selects the most appropriate for the game:

- The original AdLib and SoundBlaster card had one OPL2 chip. 
- The SoundBlaster Pro 1 had two OPL2 chips
- The SoundBlaster Pro 2 and 16 had an OPL3 chip. 

The AdLib emulator setting offers MAME, DOSBox and Nuked emulation, with MAME being the least accurate and using the least CPU power, and Nuked being the most accurate and also using the most CPU power - DOSBox is somewhere in between. 

There is also the option to select the OPL2LPT and OPL3LPT devices, which are external hardware devices with a real OPL chip, connected through the parallel port of a computer. 

AdLib does not require a SoundFont or ROMs, so for many games it might be the easiest to configure. However, if an MT-32 or GS emulator or device is available, ScummVM will prioritize this over AdLib. 

Mixed AdLib/MIDI mode
------------------------
Some games contain sound effects that are exclusive to the AdLib soundtrack, or the AdLib soundtrack might provide better sound effects. For these games, you can combine MIDI music with AdLib sound effects by using the :ref:`mixed AdLib/MIDI mode <multi>`.

.. note::

    Mixed AdLib/MIDI mode is not supported by all games. 

Digital Sound effects
=======================

Some games have both sampled and synthesized sound effects. ScummVM will usually use the sampled sound effects, even if you select Adlib, MT-32 or GM as your audio device. Some games allow you to choose between sampled and synthesized sound effects by using the **Prefer digital sound effects** option in the Engine tab. 

.. _outputrate:

Sample output rate
========================

The output sample rate tells ScummVM how many sound samples to play per channel per second. 

Most of the sounds were originally sampled at either 22050Hz or 11025Hz, so using a higher sample rate in these cases will not improve the quality of the audio.

For games that use CD audio, the sounds were probably sampled at 44100Hz, so that is a better sample rate to choose for these games.

ScummVM generates the samples when using AdLib, FM-Towns, PC Speaker or IBM PCjr emulated sound. 22050Hz will usually be fine for these options, although for Beneath a Steel Sky 44100Hz is recommended.

ScummVM has to resample all sounds to the selected output frequency. It is recommended to choose an output frequency that is a multiple of the original frequency. Choosing an in-between number might not be supported by your sound card.

.. _buffer:

Audio buffer size
==========================

There is no option to control audio buffer size through the GUI, but the default value can be overridden in the the :doc:`configuration file <../advanced_topics/configuration_file>` with the *audio_buffer_size* configuration keyword. The default value is calculated based on output sampling frequency to keep audio latency below 45ms. 

Appropriate values are normally between 512 and 8192, but the value must be one of: 256, 512, 1024, 2048, 4096, 8192, 16384, or 32768. 

Smaller values yield faster response time, but can lead to stuttering if your CPU isn't able to catch up with audio sampling when using the sound emulators. Large buffer sizes might lead to minor audio delays (high latency).


