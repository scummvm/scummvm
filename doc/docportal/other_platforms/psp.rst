=====================
Playstation Portable
=====================

Installing ScummVM
===================

Prerequisites
****************

- A homebrew-enabled PSP. How to enable homebrew is outside the scope of this documentation.
- At least one ScummVM supported game. See the `compatibility page <https://www.scummvm.org/compatibility/>`_ on our website for a list of compatible games. 

Installing ScummVM
*******************


The PSP package is available for download on our website's `Downloads page <https://www.scummvm.org/downloads>`_.

Download the ``.zip`` archive to a computer, and extract the files. 

Connect the PSP to the computer by USB cable. Go to **Settings > USB Connection** on the PSP. The computer should automatically open the root folder of the PSP. Alternatively, if you have a Memory Stick adapter, copy the files directly to the Memory Stick. 

Copy the extracted ``scummvm`` folder to your PSP, into the ``/PSP/GAME/`` folder. 

Go to **Game > Memory Stick** and choose **ScummVM** to launch ScummVM!

Transferring game files
==========================

Transfer game files onto the PSP as described in the installation section above. They can be copied to any folder; location doesn't matter. 

For more information, see :doc:`../using_scummvm/adding_games`. 

Controls
=================

Default control mapping
*********************************

These controls can also be manually configured in the :doc:`Keymaps tab<../settings/keymaps>`.

See the :doc:`../using_scummvm/keyboard_shortcuts` page for common keyboard shortcuts. 

.. csv-table:: 
  	:widths: 40 60 
  	:header-rows: 2

        Game Controller Mapping,
        Button,Action
        Right trigger,Modifier key 
        Left trigger,ESC 
        Analog,Mouse movement
        Right trigger + Analog,Fine mouse movement
        D-Pad,Arrow keys 
        D-Pad + Right Trigger,Diagonal arrow keys 
        Triangle,Enter 
        Cross,Left Mouse Button 
        Circle,Right Mouse Button 
        Square,Period
        Right trigger + Square,Spacebar
        Right trigger + Start,F5 
        Select,Show/Hide Virtual Keyboard. Hold down to move keyboard onscreen (with D-Pad).
        Right trigger + Select,Show Image Viewer 
        Start,Global main menu

.. csv-table:: 
  	:widths: 40 60 
  	:header-rows: 1

        Virtual Keyboard Mode,
        Start,Enter. Also exits virtual keyboard mode
        Select,Exit the virtual keyboard mode
        Right trigger,Input: lowercase/uppercase letters (press to toggle)
        Left trigger,Input: numbers/symbols (press to toggle)
        D-Pad,"Select square of characters (up, down, left or right)"
        Buttons/Triggers,Choose a specific character in the square. The four center characters are chosen by the button in the corresponding position. The 2 top characters are chosen by the triggers.
        Analog,Moves in a direction (left/right/up/down) 

.. csv-table:: 
  	:widths: 40 60 
  	:header-rows: 1

        Image Viewer,
        Left/Right,Previous/next image
        Up/down,Zoom in/out
        Analog,Move around the image
        "Triggers, Start",Exit image viewer

.. csv-table:: 
  	:widths: 40 60 
  	:header-rows: 1

        1st Person Game Mode ,
        Right Trigger + Left Trigger + Square,Enable 1st Person Mode
        Square,Modifier key (instead of Right Trigger)
        Left/Right Trigger,Strafe left/right
        D-Pad Left/Right,Turn left/right
        Square + D-Pad,F1/F2/F3/F4
        Square + Select,Image Viewer
        Square + Start,Esc 

Settings
===========================

For more information, see the Settings section of the documentation. Only platform-specific differences are listed here. 

Audio
*******
Supported audio file formats:

- ``.mp3``
- ``.ogg`` 
- Uncompressed audio

Games will generally run faster if audio files are in the ``.mp3`` file format.


Paths
========================

Saved games
************

``ms0://scummvm_savegames/``

Configuration file
********************

``ms0:/scummvm.ini``

Known issues
==============

- Plugin files are not interchangeable between ScummVM versions.  You must copy all the plugin files found in the ``PSP/GAME/scummvm/plugin/`` folder for every new version you install. If ScummVM crashes on startup, delete the existing plugin files in the plugin directory and copy the new ones again.

- The PSP does not support audio files in the ``.flac`` file format. For games with voice (talkies), ensure the ``monster.sou`` file is not compressed to ``monster.sof``.