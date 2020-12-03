=====================
PlayStation Portable
=====================

This page contains all the information you need to get ScummVM up and running on a PSP.

What you'll need
===================

- A homebrew-enabled PSP. How to enable homebrew is outside the scope of this documentation.
- A computer to download an extract the install files. 

Installing ScummVM
=======================================

The PSP package is available for download ScummVM's `Downloads page <https://www.scummvm.org/downloads>`_.

Download the zip file to a computer, and extract the files. 

Connect the PSP to the computer by USB cable. Go to **Settings > USB Connection** on the PSP. Ensure the **USB Device** option is set to **Memory Stick**.  The computer automatically opens the root folder of the PSP. Alternatively, if you have a Memory Stick adapter, copy the files directly to the Memory Stick. 

Copy the extracted ``scummvm`` folder to your PSP, into the ``/PSP/GAME/`` folder. 

Go to **Game > Memory Stick** and choose **ScummVM** to launch ScummVM!

Transferring game files
==========================

Transfer game files onto the PSP as described in the installation section above. They can be copied to any folder; location doesn't matter. 

See :doc:`../use_scummvm/game_files` for more information about game file requirements.

Controls
=================

Default control map
*********************************

These controls can also be manually configured in the :doc:`Keymaps tab <../settings/keymaps>`. See the :doc:`../use_scummvm/keyboard_shortcuts` page for common keyboard shortcuts. 


.. |square| raw:: html

   <svg xmlns="http://www.w3.org/2000/svg" viewBox="5 2 16 16" width="20" height="20"><path fill-rule="evenodd" d="M6 7.75C6 6.784 6.784 6 7.75 6h8.5c.966 0 1.75.784 1.75 1.75v8.5A1.75 1.75 0 0116.25 18h-8.5A1.75 1.75 0 016 16.25v-8.5zm1.75-.25a.25.25 0 00-.25.25v8.5c0 .138.112.25.25.25h8.5a.25.25 0 00.25-.25v-8.5a.25.25 0 00-.25-.25h-8.5z"></path></svg>

.. |circle| raw:: html

   <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" width="16" height="16"><path fill-rule="evenodd" d="M3.404 3.404a6.5 6.5 0 109.192 9.192 6.5 6.5 0 00-9.192-9.192zm-1.06 10.253A8 8 0 1113.656 2.343 8 8 0 012.343 13.657z"></path></svg>

.. |x| raw:: html

    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" width="16" height="16"><path fill-rule="evenodd" d="M3.72 3.72a.75.75 0 011.06 0L8 6.94l3.22-3.22a.75.75 0 111.06 1.06L9.06 8l3.22 3.22a.75.75 0 11-1.06 1.06L8 9.06l-3.22 3.22a.75.75 0 01-1.06-1.06L6.94 8 3.72 4.78a.75.75 0 010-1.06z"></path></svg>

.. |triangle| raw:: html

   <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24"><path d="M11.574 3.712c.195-.323.662-.323.857 0l9.37 15.545c.2.333-.039.757-.429.757l-18.668-.006c-.385 0-.629-.422-.428-.758l9.298-15.538zm.429-2.483c-.76 0-1.521.37-1.966 1.111l-9.707 16.18c-.915 1.523.182 3.472 1.965 3.472h19.416c1.783 0 2.879-1.949 1.965-3.472l-9.707-16.18c-.446-.741-1.205-1.111-1.966-1.111z"/></svg>


.. csv-table:: 
  	:header-rows: 2

        Game Controller Mapping,
        Button,Action
        R button,Modifier key 
        L button,ESC 
        Analog stick,Mouse movement
        Analog stick + R button,Fine mouse movement
        Directional buttons,Arrow keys 
        Directional buttons + R button,Diagonal arrow keys 
        |triangle|,Enter 
        |x|,Left Mouse Button 
        |circle|,Right Mouse Button 
        |square|,Period
        |square| + R button,Spacebar
        SELECT,Shows/Hides the virtual Keyboard. Hold down with Directional buttons to move the keyboard. 
        SELECT + R button ,Shows Image Viewer 
        START,Global Main Menu
        START + R button,F5 

.. csv-table:: 
  	:header-rows: 1

        Virtual Keyboard Mode,
        START,Enter. Also exits virtual keyboard mode
        SELECT,Exits the virtual keyboard mode
        R button,Input: lowercase/uppercase letters (press to toggle)
        L button,Input: numbers/symbols (press to toggle)
        Directional buttons,"Selects square of characters (up, down, left or right)"
        |triangle| |x| |circle| |square| and L/R buttons,Chooses a specific character in the square. The four center characters are chosen by the button in the corresponding position. The 2 top characters are chosen by the L/R buttons.
        Analog stick,Moves in a direction (left/right/up/down) 

.. csv-table:: 
  	:header-rows: 1

        Image Viewer,
        Left/right directional buttons,Previous/next image
        Up/down directional buttons ,Zooms in/out
        Analog stick,Moves around the image
        "L/R buttons + START",Exits image viewer

.. csv-table:: 
  	:header-rows: 1

        1st Person Game Mode ,
        |square| + R button + L button,Enables 1st Person Mode
        |square|,Modifier key (instead of R button)
        L/R button,Strafes left/right
        Directional buttons left/right,Turns left/right
        |square| + Directional buttons,F1/F2/F3/F4
        |square| + SELECT,Image Viewer
        |square| + START,Esc 

Settings
===========================

For more information, see the Settings section of the documentation. Only platform-specific differences are listed here. 

Audio
*******

Supported audio file formats:

- MP3
- OGG
- Uncompressed audio

.. tip::

   Games run faster if audio files are in the ``.mp3`` file format.


Paths
========================

Saved games
************

``ms0:/scummvm_savegames/``

Configuration file
********************

``ms0:/scummvm.ini``

Known issues
==============

- Plugin files are not interchangeable between ScummVM versions.  You must copy all the plugin files found in the ``/PSP/GAME/scummvm/plugin/`` folder for every new version you install. If ScummVM crashes on startup, delete the existing plugin files in the plugin directory and copy the new ones again.
- If the virtual keyboard does not show up on the screen, ensure the ``kbd.zip`` file is in the ``scummvm`` folder. 