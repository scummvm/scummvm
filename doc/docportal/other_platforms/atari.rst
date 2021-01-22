=============================
Atari/FreeMiNT
=============================

This page contains all the information you need to get ScummVM up and running on an Atari system. 

What you'll need
=================

- A powerful Atari system or clone; recommended minimum CPU 68060. 
- FreeMiNT, TOS or MagiC operating system.

Installing ScummVM
=====================================

Binary packages are provided for the m68000 CPU, the m68020 to m68060 range of CPUs, and the Coldfire (FireBee) CPU. All are available for download from the ScummVM `Downloads page <https://www.scummvm.org/downloads>`_.  To install ScummVM, extract the selected ScummVM archive into a folder on the hard disk.

Run ``scummvm.gtp`` to start Scummvm.  The ``scummvm.gtp`` command will take :doc:`command line options <../advanced_topics/command_line>` to set parameters for the ScummVM session. 

To run ScummVM from a FreeMiNT Command Line Interface, make the program executable with the following command:  
    
.. code-block:: bash

    chmod +x scummvm.gtp

For the best performance, run ScummVM under FreeMiNT, from the command line, and without the AES started. 

Transferring game files
=========================

Transfer files to the Atari as you normally would, such as through a network connection or by optical disc. 

See :doc:`../use_scummvm/game_files` for more information about game file requirements.

Controls
=================

Controls can be manually configured in the :doc:`Keymaps tab <../settings/keymaps>`. See the :doc:`../use_scummvm/keyboard_shortcuts` page for common keyboard shortcuts. 


Paths 
=======

FreeMiNT
***************
Saved games and the configuration file are found in the user’s defined home directory. 

MagiC/TOS
***************
Saved games and the configuration file are found in the ScummVM installation directory.



Settings
==========

For more information, see the Settings section of the documentation. Only platform-specific differences are listed here. 

Graphics
************

For video rendering, define the variable ``SDL_VIDEODRIVER=XBIOS`` in your environmental variables. Either XBIOS or GEM might be set by default. This is not a ScummVM setting, but rather a system-wide setting. 

.. tip::

    XBIOS works better than GEM, because GEM rendering is generally slower.

Audio
*******

Supported audio file formats:

- MP3
- OGG 
- FLAC
- Uncompressed audio

Known issues
==============

- There is no cloud/LAN functionality.

- The Atari built-in MIDI driver (STMIDI) does not work. 

- FLAC, OGG and MP3 decoding libraries are extremely CPU intensive and greatly slow down the user experience.  ScummVM is best run with uncompressed audio, when available.

- While all supported engines are included in the Atari ScummVM release, not all games will run well due to the lack of resources.  For example, the Curse of Monkey Island will run on a CT60-equipped Atari Falcon, but very poorly. 