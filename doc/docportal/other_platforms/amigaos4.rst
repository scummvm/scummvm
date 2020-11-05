=============================
Amiga OS 4
=============================

Install ScummVM
===================

Prerequisites
****************

- A system running Amiga OS 4

Download and install ScummVM
*********************************

There are three ways to install ScummVM on AmigaOS 4:

- Use AmiUpdate 
- Use the installer script that comes with every release
- Download and unarchive the ``.lha`` file. It does not matter where the files are kept. 


Transfer game files
=======================

See :doc:`../using_scummvm/add_play_games` for more information. 

For games on PC or Mac formatted discs, use the native platform to access the data files, and then transfer these to Amiga. Alternatively, if you have external hardware such as a Catweazel floppy disc controller, you may be able to use this to access the files. 

ScummVM for Amiga OS has Cloud and LAN functionality, to help manage the transfer of files. See :doc:`../guides/connect_cloud` and :doc:`../guides/local_webserver`. 

Controls
=================

Controls can also be manually configured in the :doc:`Keymaps tab <../settings/keymaps>`.

See the :doc:`../using_scummvm/keyboard_shortcuts` page for common keyboard shortcuts. 


Paths 
=======

Saved games 
*******************
``Games:ScummVM/saves``

Configuration file 
**************************
``Games:ScummVM/scummvm.ini``


Settings
==========

For more information, see the Settings section of the documentation. Only platform-specific differences are listed here. 

Audio
******

The CAMD driver allows the use of MIDI hardware. Select CAMD as the :ref:`Preferred device <device>`.


Known issues
==============

Due to the Big Endian nature of Amiga OS, there can sometimes be graphics and sounds issues with games. Check out the Platform Overview `wiki page <https://wiki.scummvm.org/index.php?title=Platforms/Overview>`_ for a list of compatible engines. 

If a game crashes and it's listed as compatible, report it as a :doc:`bug <../help/report_bugs>`.