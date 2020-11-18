=============================
RISC OS
=============================

Install ScummVM
===================

Prerequisites
****************

- A system running RISC OS 5
- The `SharedUnixLibrary <https://www.riscos.info/index.php/SharedUnixLibrary>`_, `DRenderer <https://www.riscos.info/packages/LibraryDetails.html#DRendererarm>`_ and `Iconv <https://www.netsurf-browser.org/projects/iconv/>`_ modules installed. SharedUnixLibrary and DRenderer can be installed using `Packman <https://www.riscos.info/index.php/PackMan>`_.

Download and install ScummVM
*********************************

Download the RISC OS package from the `ScummVM Downloads page <https://www.scummvm.org/downloads/>`_. 

Use a tool such as SparkFS to extract the archive. It is important that the archive is not extracted on any other system, since this results in a loss of file type information. ScummVM requires this file type information to run properly. 



Transfer game files
=======================

Copy data files directly from the original discs on machines that have CD and/or floppy drives, or transfer the files from another system by using a USB stick. 

See :doc:`../use_scummvm/add_play_games` for more information. 

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

There is one additional configuration option, *enable_reporter*. When set to true in :doc:`../advanced_topics/configuration_file`, log messages are sent to the `!Reporter <http://www.avisoft.force9.co.uk/Reporter.html>`_ application, which can be useful to developers. 


Known issues
==============

- Due to ongoing issues with executable size, releases are built without Cloud or LAN functionality. 
- FluidSynth is not supported. 
- The releases are only compatible with RISC OS 5, and not RISC OS 3, 4, or 6. 

 