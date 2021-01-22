=============================
Nintendo Switch
=============================

This page contains all the information you need to get ScummVM up and running on a Nintendo Switch.

What you'll need
===================

- A homebrew-enabled Nintendo Switch console. How to enable homebrew is outside the scope of this documentation.

Installing ScummVM
=======================================

The Nintendo Switch package is available for download on the ScummVM `Downloads page <https://www.scummvm.org/downloads>`_.

Download the zip file to a computer, and extract the files. 

Copy the extracted ``scummvm`` folder to your Nintendo Switch microSD card, into the ``/switch/`` folder. 


Transferring game files
=======================

Copy the folders containing game files into the ``/switch/scummvm/`` folder on the microSD card. 

ScummVM on the Nintendo switch supports :doc:`cloud functionality <../use_scummvm/connect_cloud>`. 

See :doc:`../use_scummvm/game_files` for more information about game file requirements.

Controls
=================

Default control map
*********************************

These controls can also be manually configured in the :doc:`Keymaps tab <../settings/keymaps>`. See the :doc:`../use_scummvm/keyboard_shortcuts` page for common keyboard shortcuts. 

.. |plus| raw:: html

    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" width="16" height="16"><path fill-rule="evenodd" d="M8 2a.75.75 0 01.75.75v4.5h4.5a.75.75 0 010 1.5h-4.5v4.5a.75.75 0 01-1.5 0v-4.5h-4.5a.75.75 0 010-1.5h4.5v-4.5A.75.75 0 018 2z"></path></svg>

.. |minus| raw:: html

    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" width="16" height="16"><path fill-rule="evenodd" d="M2 8a.75.75 0 01.75-.75h10.5a.75.75 0 010 1.5H2.75A.75.75 0 012 8z"></path></svg>

.. csv-table:: 
  	:header-rows: 1

        Button,Action
        Left stick,Mouse
        Left stick + R button,Slow Mouse
        Ⓑ,Left mouse button
        Ⓐ,Right mouse button
        Directional buttons,"Keypad 'Cursor' Keys"
        Directional buttons + R button ,"Diagonal Keypad 'Cursor' Keys"
        L button,Game menu (F5)
        R button,Shift 
        Ⓨ,Period
        Ⓨ + R button,Space 
        Ⓧ,Escape 
        Ⓧ + R button,Return
        |plus|,Global main menu (GMM)
        |minus|,Toggle virtual keyboard
        |minus| + R button,AGI predictive input dialog
    
.. csv-table::
    :header-rows: 2

        "Touch Controls (Touchpad Mouse Mode Off, Default)",
        Touch,Action
        Short single finger tap,Move pointer to finger position and left mouse button click
        Short second finger tap while holding one finger,Move pointer to first finger position and right mouse button click
        Single finger drag,Move pointer directly with finger
        `Two finger drag`_ ,Move pointer while left mouse button is held down (drag and drop). The pointer directly follows the first finger. The drag is ended when the last finger is removed.
        `Three finger drag`_ ,Move pointer while right mouse button is held down (drag and drop). The pointer directly follows the first finger. The drag is ended when the last finger is removed.
    
.. csv-table::
    :header-rows: 2

        Touch Controls (Touchpad Mouse Mode On),
        Touch,Action
        Short single finger tap,Left mouse button click
        Short second finger tap while holding one finger,Right mouse button click
        Single finger drag,"Move pointer indirectly with finger, pointer speed can be adjusted in control settings"
        `Two finger drag`_ ,Move pointer while left mouse button is held down (drag-and-drop). The pointer motion is affected by the first finger. The drag is ended when the last finger is removed.
        `Three finger drag`_ ,Move pointer while right mouse button is held down (drag-and-drop). The pointer motion is affected by the first finger. The drag is ended when the last finger is removed.

Keyboard and mouse support
****************************
Physical USB mice and keyboards work on the Switch and are supported by ScummVM. All keyboards work but not all mice work. A mouse compatibility list is available on this `Google Sheet <https://docs.google.com/spreadsheets/d/1Drbo5-QuSX901MwtOytSMuqRGxeIkq2HELM806I9dj0/edit#gid=0>`_.

Touch support
****************

For multi-touch gestures, the fingers have to be far enough apart from each other that the Switch does not recognize them as a single finger, otherwise the pointer jumps around.

The touch control scheme can be switched in the global settings. From the Launcher, go to **Options > Control > Touchpad mouse mode**.

    - When touchpad mouse mode is off (default), the touch controls are direct. The pointer always jumps to where the finger touches the screen.
    - When touchpad mouse mode is on, the touch controls are indirect. The finger can be far away from the pointer and still move it, like on a laptop touchpad. 
    - The pointer speed setting in the :doc:`Controls tab <../settings/control>` affects how far the pointer moves in response to a finger movement.

Two finger drag
^^^^^^^^^^^^^^^^^^^

Hold down two fingers to initiate a drag-and-drop feature. The two finger drag simulates the left mouse button being held down. 

- The first finger affects mouse motion during the drag, the second finger can be removed without affecting the drag. 
- When the last finger is removed, the left mouse button is released and the drag is over. 
- The second finger can be anywhere, it doesn't have to be close to the first finger, and it doesn't have to be put down simultaneously with the first finger.


Three finger drag
^^^^^^^^^^^^^^^^^^^^

Hold down the three fingers to initiate a drag-and-drop feature. The three finger drag simulates the right mouse button being held down. 

Paths 
=======

Saved games 
*******************
``/switch/scummvm/saves`` 

Configuration file 
**************************
``/switch/scummvm/scummvm.ini``


Settings
==========

For more information, see the Settings section of the documentation. Only platform-specific differences are listed here. 

Audio
******

Supported devices:

- MT-32 emulator
- FluidSynth (General MIDI emulator)
- AdLib emulator

Supported audio file formats:

- MP3
- OGG 
- FLAC
- Uncompressed audio

Controls
*********

See the `Touch support`_ section. 

