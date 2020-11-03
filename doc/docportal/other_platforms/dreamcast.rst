=============================
Sega Dreamcast
=============================

Installing ScummVM
===================

Prerequisites
****************
- A CD-R disc (80min/700mb).
- A CD-RW drive.
- A computer running Windows.  
- CD burning software capable of creating a disc image, such as Bootdreams, Nero Burning ROM, or ImgBurn. 

Installing ScummVM
*******************

There are two ways to install ScummVM on the Dreamcast:

1. Download a ready-to-go Nero Image and Demos package  and burn the extracted ``.nrg`` image to a CD-R using Nero Burning ROM or ImgBurn software.
2. Download the Dreamcast plain files package and use the extracted files to create a custom disc image to burn to a CD-R.

Both packages are available from the ScummVM `Downloads page <https://www.scummvm.org/downloads/>`_

Dreamcast Nero Image and Demos
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Use Nero Burning ROM or ImgBurn to burn the ``.nrg`` file contained in the package to a CD-R disc. 

Dreamcast plain files
^^^^^^^^^^^^^^^^^^^^^^^^^^
Create a directory on your computer. In this example, we will call the directory ``ScummVMDC``.

Copy the downloaded ``SCUMMVM.BIN`` and ``IP.BIN`` files into the ScummVMDC directory. These are the main boot and engine files required for ScummVM to run.

The ``.PLG`` files in the plain files package are the plugins that are required to run the games. For the best performance, only include the plugins for games you want to play. For example, if you want to play LucasArts games, include ``SCUMM.PLG``. 

These plugin files are also placed directly into the ScummVMDC directory. 

Next, make a new directory for each game. In this example, we will add Day of the Tentacle into a subdirectory named ``DOTT``. 

Our file structure now looks like this:

::

    ScummVMDC
        |--DOTT
        |    |-- MONSTER.SOU
        |    |-- TENTACLE.000
        |    `-- TENTACLE.001
        |--IP.BIN
        |-- SCUMM.PLG
        `-- SCUMMVM.BIN

Now that we have the ScummVMDC directory complete with all the required files, we need to create a disc image. The recommended disc image type is ``.cdi`` or ``.nrg``. A ``.iso`` image will not work.

Bootdreams is the software often recommended for creating and burning a ``.cdi`` image. It's an old piece of software, but it is free and still available for download. 

ImgBurn is also free, and can also be used to create and burn ``.cdi`` images. It is compatible with all Windows operating systems. 

Alternatively, you can use Nero Burning ROM to create and burn a ``.nrg`` image.

.. caution::

    Use a lower burning speed (4x) for best results; the Dreamcast is an older system and as such can have problems reading discs burned at high speeds.  

Transferring game files
=======================

Game files can be included on the same CD as the ScummVM binary file, or on separate CDs. 

The Dreamcast port uses its own custom Graphical User Interface, which automatically scans the disc for games, and adds the games to the list through ScummVM's built-in autodetection. 

ScummVM for the Dreamcast supports disc swapping from the GUI. It will rescan once a new disc is inserted and repopulate the games list. 

Controls
=================

The mouse cursor can be controlled with the standard Dreamcast controller. 
There is a virtual keyboard that can be brought up by pressing the left trigger button, that will emulate keyboard input. 
The Dreamcast mouse and keyboard peripherals are also supported. 

.. csv-table::
    :header-rows: 2

    Game Controller Mapping:,,
    Button,Action,Notes
    Joystick,Moves pointer,
    Directional pad,Moves pointer slowly,Also controls the on-screen keyboard
    A,Left Click,
    B,Right Click,
    Y,Spacebar,Pause
    Start,F5,Load and save
    Trigger L,Toggles the on-screen keyboard on/off,
    X,5,For boxing in Indiana Jones and the Last Crusade
    Directional pad + X,"1, 2, 3, 4, 6, 7, 8, 9",For boxing in Indiana Jones and the Last Crusade

Paths 
=======

Saved games 
*****************

The Visual Memory Unit (VMU) is used for saving and loading games. 

Autosave is enabled on the Dreamcast, so each game will use some memory blocks (between 2 and 13) for the autosave in save slot 0.

When a game is saved, the VMU beeps, and a prompt is displayed on the screen letting you know that the game has been saved.

Configuration file 
*********************



Settings
==========



Known issues
==============

- The Dreamcast has no external storage medium, and only has 16MB of RAM. As such, the Dreamcast port of ScummVM does not support The Curse of Monkey Island or the Dig. The resources required for these games are too great for the Dreamcast to handle. Full Throttle is not completable. 


- There is currently no way to manually add games to the games list. The autodetect feature usually works well, but sometimes games will show up on the list twice, incorrect games will show up with the correct game, and, quite rarely, the game will not show up on the list at all. 


- Some games, for example SPY Fox 1: Dry Cereal, use filenames which are incompatible with the ISO 9660 filesystem standard. The Dreamcast port does not support filename extensions like RockRidge or Joliet, so the CD mastering software needs to be instructed not to change the filenames to comply to ISO 9660, or the engine will not be able to open the files.

