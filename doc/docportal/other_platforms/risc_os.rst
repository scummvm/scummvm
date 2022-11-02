=============================
RISC OS
=============================

This page contains all the information you need to get ScummVM up and running on the RISC OS operating system.

What you'll need
===================

- A system running RISC OS 5.
- A minimum of 64 MB RAM. 32 MB may work in some circumstances, but is not generally recommended.
- The `SharedUnixLibrary <https://www.riscos.info/index.php/SharedUnixLibrary>`_ and `DRenderer <https://www.riscos.info/packages/LibraryDetails.html#DRendererarm>`_ modules installed. These can be installed using `PackMan <https://www.riscos.info/index.php/PackMan>`_.

Installing ScummVM
======================================

Download the RISC OS package from the `ScummVM Downloads page <https://www.scummvm.org/downloads/>`_.

Use a tool such as SparkFS to extract the archive. It is important that the archive is not extracted on any other system, because this results in a loss of file type information. ScummVM requires the file type information to run properly.



Transferring game files
=======================

Copy data files directly from the original discs on machines that have CD and/or floppy drives, or use a USB drive to transfer the files from another system.

See :doc:`../use_scummvm/game_files` for more information about game file requirements.

Controls
=================

Controls can be manually configured in the :doc:`Keymaps tab <../settings/keymaps>`. See the :doc:`../use_scummvm/keyboard_shortcuts` page for common default keyboard shortcuts.


Paths
=======

Saved games
*******************

``<Choices$Write>.ScummVM.Saves``

Configuration file
**************************
``<Choices$Write>.ScummVM.scummvmrc``

The ``<Choices$Write>`` environment variable is usually ``$.!Boot.Choices``.


Settings
==========


For more information about settings, see the Settings section of the documentation. Only platform-specific differences are listed here.

.. _reporter:

There is one additional configuration option, *enable_reporter*. When set to true in :doc:`../advanced_topics/configuration_file`, log messages are sent to the `!Reporter <http://www.avisoft.force9.co.uk/Reporter.htm>`_ application. This is useful mostly for developers.


Known issues
==============

- ScummVM for RISC OS does not have cloud or LAN functionality.
- FluidSynth is not supported.
- ScummVM is not compatible with RISC OS 3, 4 or 6.

