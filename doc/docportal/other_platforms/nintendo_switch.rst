=============================
Nintendo Switch
=============================

Installing ScummVM
===================

Prerequisites
****************

- A homebrew-enabled Nintendo Switch console. How to enable homebrew is outside the scope of this documentation.
- At least one ScummVM supported game. See the `compatibility page <https://www.scummvm.org/compatibility/>`_ on our website for a list of compatible games. 

Installing ScummVM
*******************

The Nintendo Switch package is available for download on our website's `Downloads page <https://www.scummvm.org/downloads>`_.

Download the ``.zip`` archive to a computer, and extract the files. 

Copy the extracted ``scummvm`` folder to your Nintendo Switch microSD card, into the ``/switch/`` folder. 


Transferring game files
=======================

Copy the folders containing game files into the ``/switch/scummvm/`` folder on the microSD card. 

ScummVM on the Nintendo switch supports :ref:`Cloud functionality <howtocloud>`. 

For more information on how to add and play games, see the :doc:`../using_scummvm/adding_games` page.

Controls
=================

Default control mapping
*********************************

These controls can also be manually configured in the :doc:`Keymaps tab <../settings/keymaps>`.

See the :doc:`../using_scummvm/keyboard_shortcuts` page for common keyboard shortcuts. 

.. csv-table:: 
  	:widths: 40 60 
  	:header-rows: 1

        Button,Action
        Left stick,Mouse
        R + Left stick,Slow Mouse
        B,Left mouse button
        A,Right mouse button
        DPad,"Keypad 'Cursor' Keys"
        R + DPad,"Diagonal Keypad 'Cursor' Keys"
        L Trigger,Game menu (F5)
        R Trigger,Shift 
        Y,Period
        R + Y,Space 
        X,Escape 
        R + X,Return
        Plus,Global main menu (GMM)
        Minus,Toggle virtual keyboard
        R + Minus,AGI predictive input dialog
    
.. csv-table::
    :widths: 40 60
    :header-rows: 2

        "Touch Controls (Touchpad Mouse Mode Off, Default)",
        Touch,Action
        Short single finger tap,Move pointer to finger position and left mouse button click
        Short second finger tap while holding one finger,Move pointer to first finger position and right mouse button click
        Single finger drag,Move pointer directly with finger
        `Two finger drag`_ ,Move pointer while left mouse button is held down (drag and drop). The pointer directly follows the first finger. The drag is ended when the last finger is removed.
        `Three finger drag`_ ,Move pointer while right mouse button is held down (drag and drop). The pointer directly follows the first finger. The drag is ended when the last finger is removed.
    
.. csv-table::
    :widths: 40 60 
    :header-rows: 2

        Touch Controls (Touchpad Mouse Mode On),
        Touch,Action
        Short single finger tap,Left mouse button click
        Short second finger tap while holding one finger,Right mouse button click
        Single finger drag,"Move pointer indirectly with finger, pointer speed can be adjusted in control settings"
        `Two finger drag`_ ,Move pointer while left mouse button is held down (drag and drop). The pointer motion is affected by the first finger. The drag is ended when the last finger is removed.
        `Three finger drag`_ ,Move pointer while right mouse button is held down (drag and drop). The pointer motion is affected by the first finger. The drag is ended when the last finger is removed.

Keyboard and mouse support
****************************
Physical USB mice and keyboards work on the Switch and are supported by ScummVM. 

- All keyboards work. 
- Not all mice work. A mouse compatibility list is available on this `Google Sheet <https://docs.google.com/spreadsheets/d/1Drbo5-QuSX901MwtOytSMuqRGxeIkq2HELM806I9dj0/edit#gid=0>`_.

Touch support
*************** 

For multi-touch gestures, the fingers have to be far enough apart from each other that the Switch will not erroneously recognize them as a single finger, otherwise the pointer will jump around.

The touch control scheme can be switched in the global settings. Go to **Options > Controls > Touchpad mouse mode**.

    - When touchpad mouse mode is off (default), the touch controls are direct. The pointer always jumps to the finger.

    - When touchpad mouse mode is on, the touch controls are indirect. The finger can be far away from the pointer and still move it, like on a laptop touchpad. 
    - The pointer speed setting in the :doc:`Controls tab <../settings/control>` affects how far the pointer moves in response to a finger movement.

Two finger drag
^^^^^^^^^^^^^^^^^^^

The two finger drag works in the following way: holding down two fingers for a while initiates a drag and drop feature, where the left mouse button is being held down. 

- The first finger affects mouse motion during the drag, the second finger can be removed without affecting the drag. 
- When the last finger is removed, the left mouse button is released and the drag is over. 
- The second finger can be anywhere, it doesn't have to be close to the first finger, and it doesn't have to be put down simultaneously with the first finger.

Typical usage of the two finger drag and drop in Full Throttle: 
    - Hold down one finger over an object. 
    - Hold down a second finger somewhere on the screen for a little bit to initiate a two finger drag. This brings up the action menu. 
    - Let go of the second finger. The two finger drag continues since one finger is still down. 
    - Select an action with the first finger, and let go of the first finger to end the drag.

Three finger drag
^^^^^^^^^^^^^^^^^^^^
The three finger drag works in a similar way to the `Two finger drag`_, except that instead of holding down a single second finger, you hold down two extra fingers. Holding down the three fingers for a while initiates a drag and drop feature, where the right mouse button is being held down.


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

- ``.mp3``
- ``.ogg`` 
- ``.flac``
- Uncompressed audio

Controls
*********

See the `Touch support`_ section. 

Known issues
==============

