=====================
Playstation Vita
=====================

Installing ScummVM
=======================

Prerequisites
*******************

- A homebrew-enabled PS Vita console with `Vitashell <https://github.com/TheOfficialFloW/VitaShell/releases/tag/v2.02>`_ installed. How to enable homebrew is outside the scope of this documentation.
- At least one ScummVM supported game. See the `compatibility page <https://www.scummvm.org/compatibility/>`_ on our website for a list of compatible games. 

Installing ScummVM
*******************

The PS Vita package is available for download on our website's `Downloads page <https://www.scummvm.org/downloads>`_.

Copy the ``.vpk`` to the PS Vita and install it using VitaShell. 


Transferring game files
=======================

There are a few ways to transfer game files to the PS Vita. 

Using VitaShell:
    
    -  Transfer over FTP
    -  Transfer via USB

If you're using SD2Vita:
    
    - Transfer the files directly to the microSD card. 

Folders containing game files can be copied into any folder.

ScummVM on the PS Vita supports :doc:`Cloud functionality <../guides/connect_cloud>`. 

For more information on how to add and play games, see the :doc:`../using_scummvm/add_play_games` page.


Controls
=================

Default control mapping
*********************************

These controls can also be manually configured in the :doc:`Keymaps tab <../settings/keymaps>`.

See the :doc:`../using_scummvm/keyboard_shortcuts` page for common keyboard shortcuts. 

.. csv-table:: 
  	:widths: 40 60 
  	:header-rows: 2

        Game Controller Mapping,
        Button,Action
        Left stick,Mouse
        R + Left stick,Slow Mouse
        Cross,Left mouse button
        Circle,Right mouse button
        DPad,Keypad 'Cursor' Keys 
        R + DPad,Diagonal Keypad 'Cursor' Keys
        L Trigger,Game menu (F5)
        R Trigger,Shift 
        Square,Period 
        R + Square,Space '
        Triangle,Escape 
        R + Triangle,Return
        Start,Global main menu (GMM)
        Select,Toggle virtual keyboard
        R + Select,AGI predictive input dialog

.. csv-table:: 
  	:widths: 40 60 
  	:header-rows: 2

        "Front Touchscreen Controls, always enabled",
        Touch,Action
        Short single finger tap,Move pointer to finger position and left mouse button click
        Short second finger tap while holding one finger,Move pointer to first finger position and right mouse button click
        Single finger drag,Move pointer directly with finger
        `Two finger drag`_ ,Move pointer while left mouse button is held down (drag and drop). The pointer directly follows the first finger. The drag is ended when the last finger is removed.
        `Three finger drag`_ ,Move pointer while right mouse button is held down (drag and drop). The pointer directly follows the first finger. The drag is ended when the last finger is removed.

.. csv-table:: 
  	:widths: 40 60 
  	:header-rows: 2

        "Back Touchpad Controls, enabled via settings",
        Touch,Action
        Short single finger tap,Left mouse button click
        Short second finger tap while holding one finger,Right mouse button click
        Single finger drag,"Move pointer indirectly with finger, pointer speed can be adjusted in control settings"
        `Two finger drag`_,Move pointer while left mouse button is held down (drag and drop). The pointer motion is affected by the first finger. The drag is ended when the last finger is removed.
        `Three finger drag`_ ,Move pointer while right mouse button is held down (drag and drop). The pointer motion is affected by the first finger. The drag is ended when the last finger is removed.

Keyboard and mouse support
****************************

Real bluetooth mice and keyboards work on the Vita and are supported by ScummVM. Go to **Settings > Devices** on the Vita home screen to pair your devices.

.. note::

    Not all bluetooth keyboards or mice pair successfully with the Vita. 

    We tested the Jelly Bean BT keyboard and mouse combo (ASIN:B06Y56BBYP) and with the standalone Jelly Comb Bluetooth Wireless Mouse (ASIN:B075HBDWCF).

Touch support
****************

For multi-touch gestures, the fingers have to be far enough apart from each other that the Switch will not erroneously recognize them as a single finger, otherwise the pointer will jump around.

The touch control scheme can be switched in the global settings. Go to **Options > Controls**:

    - When **Touchpad mouse mode** is off:
    
        - The touch controls are direct, which means the pointer always jumps to the finger position. 
        - The rear touchpad is disabled. 

    - When **Touchpad mouse mode** is on:
        
        - The touch controls are indirect. The finger can be far away from the pointer and still move it, like on a laptop touchpad. 
        - The pointer speed setting in the :doc:`Controls tab <../settings/control>` affects how far the pointer moves in response to a finger movement.
        - The rear touchpad is enabled. 


The front touchscreen uses the direct mode by default, but this can be changed by editing the :doc:`configuration file <../advanced_options/configuration_file>` with the following keyword:

.. _frontpanel:

*frontpanel_touchpad_mode*

When set to true, this forces the front touchscreen into an indirect mode similar to the rear touchpad. 

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
============================

Saved games 
*******************
``ux0:/data/scummvm/saves`` 

Configuration file 
**************************
``ux0:/data/scummvm/scummvm.ini``


Settings
===========================

For more information, see the Settings section of the documentation. Only platform-specific differences are listed here. 

Audio
******

Supported audio file formats:

- ``.mp3``
- ``.ogg`` 
- ``.flac``
- Uncompressed audio

Controls
*********

See the `Touch support`_ section. 

Known Issues
===============

MT-32 emulation is not available on the PS Vita.