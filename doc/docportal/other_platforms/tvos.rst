
==============
Apple TV OS
==============

.. note::

 This page is under development. Only basic information is given to enable ScummVM on Apple TV.

This page contains all the information you need to get ScummVM up and running on an Apple TV.


Installing ScummVM
=====================
There are two ways to install ScummVM on an Apple TV depending on if building the project or downloading a bundle.

Building the project
************************************

Since the port for Apple TV shares the same code base as the iOS port the instructions for building the project is the same as for iOS. Follow the :doc:`./ios` with the differences that the `tvOS libraries <https://github.com/larsamannen/scummvm-tvos-libs-v1>`_ should be used instead and the ``ScummVM-tvOS`` target should be chosen in Xcode. Use the libraries in the ``appletv`` folder if building for real target and ``appletvsimulator`` if building for simulator. Note that the libraries are built without bitcode since it's not required since tvOS 16.

Downloading and installing ScummVM
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Instructions will be added when packages are available for download.


Transferring game files
========================

.. note::

 Note that Apple TV applications doesn't have a ``Documents`` folder in which games can be stored. All games are stored in the ``cache`` folder which can be deleted when the system requires the needed space. This could happen if having limited available space and streaming high definition content or installing other applications. Please see https://developer.apple.com/library/archive/documentation/General/Conceptual/AppleTV_PG/OnDemandResources.html for more information.

 Make sure to make backups of savegames.

ScummVM has built-in cloud functionality, which lets you connect your Google Drive, OneDrive, Box or Dropbox account. For more information, see the :doc:`../use_scummvm/connect_cloud` page. ScummVM also has the ability to run a local web server. For more information, see the :doc:`../use_scummvm/LAN` page.

.. note::

 ScummVM's cloud functionality does not currently support iCloud, however you can upload game folders to your iCloud and then use the Files app on your iOS device to copy these folders into the local ScummVM folder.


See :doc:`../use_scummvm/game_files` for more information about game file requirements.

Controls
============

The Apple TV supports several controllers however no mouse support. External keyboards can be used to help entering text input. Usually the Apple TV remote controller will be used.

Game controllers
********************
If running tvOS 14 and later there is support for connected gamepad controllers using the Apple Game Controller framework. "Micro Gamepad Controllers ""Extended Gamepad Controllers" are supported at the moment. For more information visit
https://developer.apple.com/documentation/gamecontroller/gcmicrogamepad
https://developer.apple.com/documentation/gamecontroller/gcextendedgamepad

Key mappings on Apple TV remote (also remote app on iOS)
************************************************************

.. csv-table::
    :widths: 40 60
    :header-rows: 1
    :class: controls

        Button, Action
        Swipe on Touch area, Controls pointer
        Press Touch area, Left mouse click
        Play/Pause, Right mouse click
        Back/Menu in game, Global Main menu
        Back/Menu in launcher, Apple TV Home
        Hold Play/Pause, Show keyboard with extra keys
        Touch (not press) on top of Touch area, Up arrow key
        Touch (not press) on left of Touch area, Left arrow key
        Touch (not press) on right of Touch area, Right arrow key
        Touch (not press) on bottom of Touch area, Down arrow key

Key mappings on Extended gamepad controller
***********************************************

.. csv-table::
    :widths: 40 60
    :header-rows: 1
    :class: controls

        Button, Action
        Left analog joystick, Controls pointer
        D-Pad, Up/Left/Right/Down arrow buttons
        A, Left mouse click
        B, Right mouse click
        Hold X, Show keyboard with extra keys
        Menu in game, Global Main menu
        Menu in launcher, Apple TV Home
        L1, show game original menu

Paths
=======

Saved games
**************

``Savegames/`` in the cache root folder. Access this folder through :doc:`../use_scummvm/LAN`.

Configuration file
*********************

``Preferences`` in the cache folder. Access this folder through :doc:`../use_scummvm/LAN`.
