=============================
MorphOS
=============================

This page contains all the information you need to get ScummVM up and running on MorphOS.

What you'll need
===================

- A system running MorphOS 3.15
- SDL2 library installed, download last version : <https://www.morphos-storage.net/?find=SDL_2>

Installing ScummVM
=====================================

ScummVM is available for download from the `ScummVM Downloads page <https://www.scummvm.org/downloads/>`_ or the `MorphOS Storage <https://www.morphos-storage.net/?find=scummvm>`_.

Download and extract the ``.lha`` file.

Transferring game files
=========================

For games on PC or Mac formatted discs, use the original platform to access the data files, and then transfer these to Amiga. Alternatively, if you have external hardware such as a Catweazel floppy disc controller, you might be able to use this to access the files.

ScummVM for MorphOS has Cloud and LAN functionality, to help manage the transfer of files. See :doc:`../use_scummvm/connect_cloud` and :doc:`../use_scummvm/LAN`.

See :doc:`../use_scummvm/game_files` for more information about game file requirements.

Controls
=================

Controls can be manually configured in the :doc:`Keymaps tab <../settings/keymaps>`. See the :doc:`../use_scummvm/keyboard_shortcuts` page for common default keyboard shortcuts.


Paths
=======

Saved games
*******************
``PROGDIR:saves``

Configuration file
**************************
``PROGDIR:scummvm.ini``


Settings
==========

For more information about Settings, see the Settings section of the documentation. Only platform-specific differences are listed here.

Audio
******

Supported devices:
- MT-32 emulator
- FluidSynth (General MIDI emulator)
- AdLib emulator
- CAMD: The CAMD driver allows the use of MIDI hardware. Select CAMD as the :ref:`Preferred device <device>`.

Supported audio file formats:
- MP3
- OGG
- FLAC
- Uncompressed audio

Known issues
==============

Due to the big-endian nature of Amiga OS, there can sometimes be graphics and sounds issues with games. Check out the Platform Overview `wiki page <https://wiki.scummvm.org/index.php?title=Platforms/Overview>`_ for a list of compatible engines.

If a game crashes and it's listed as compatible, report it as a :doc:`bug <../help/report_bugs>`.
