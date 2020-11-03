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


.. |square| raw:: html

   <svg xmlns="http://www.w3.org/2000/svg" viewBox="5 2 16 16" width="20" height="20"><path fill-rule="evenodd" d="M6 7.75C6 6.784 6.784 6 7.75 6h8.5c.966 0 1.75.784 1.75 1.75v8.5A1.75 1.75 0 0116.25 18h-8.5A1.75 1.75 0 016 16.25v-8.5zm1.75-.25a.25.25 0 00-.25.25v8.5c0 .138.112.25.25.25h8.5a.25.25 0 00.25-.25v-8.5a.25.25 0 00-.25-.25h-8.5z"></path></svg>

.. |circle| raw:: html

   <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" width="16" height="16"><path fill-rule="evenodd" d="M3.404 3.404a6.5 6.5 0 109.192 9.192 6.5 6.5 0 00-9.192-9.192zm-1.06 10.253A8 8 0 1113.656 2.343 8 8 0 012.343 13.657z"></path></svg>

.. |x| raw:: html

    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" width="16" height="16"><path fill-rule="evenodd" d="M3.72 3.72a.75.75 0 011.06 0L8 6.94l3.22-3.22a.75.75 0 111.06 1.06L9.06 8l3.22 3.22a.75.75 0 11-1.06 1.06L8 9.06l-3.22 3.22a.75.75 0 01-1.06-1.06L6.94 8 3.72 4.78a.75.75 0 010-1.06z"></path></svg>

.. |triangle| raw:: html

   <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24"><path d="M11.574 3.712c.195-.323.662-.323.857 0l9.37 15.545c.2.333-.039.757-.429.757l-18.668-.006c-.385 0-.629-.422-.428-.758l9.298-15.538zm.429-2.483c-.76 0-1.521.37-1.966 1.111l-9.707 16.18c-.915 1.523.182 3.472 1.965 3.472h19.416c1.783 0 2.879-1.949 1.965-3.472l-9.707-16.18c-.446-.741-1.205-1.111-1.966-1.111z"/></svg>

.. csv-table:: 
    :widths: 50 50
    :width: 100%
    :header-rows: 2

        Game Controller Mapping,
        Button,Action
        Left stick,Mouse
        Left stick + R button,Slow Mouse
        |x|,Left mouse button
        |circle|,Right mouse button
        Directional buttons,Keypad 'Cursor' Keys 
        Directional buttons + R button,Diagonal Keypad 'Cursor' Keys
        L button,Game menu (F5)
        R button,Shift 
        |square|,Period (.)
        R button + |square|,Spacebar 
        |triangle|,Escape 
        R button + |triangle|,Return
        START,Global main menu (GMM)
        SELECT,Toggle virtual keyboard
        SELECT + R button,AGI predictive input dialog

.. csv-table:: 
  	:header-rows: 2

        "Front Touchscreen Controls, always enabled",
        Touch,Action
        Short single finger tap,Move pointer to finger position and left mouse button click
        Short second finger tap while holding one finger,Move pointer to first finger position and right mouse button click
        Single finger drag,Move pointer directly with finger
        `Two finger drag`_ ,Move pointer while left mouse button is held down (drag and drop). The pointer directly follows the first finger. The drag is ended when the last finger is removed.
        `Three finger drag`_ ,Move pointer while right mouse button is held down (drag and drop). The pointer directly follows the first finger. The drag is ended when the last finger is removed.

.. csv-table::  
    :width: 100%
    :widths: 50 50
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

For multi-touch gestures, the fingers have to be far enough apart from each other that the Vita will not recognize them as a single finger, otherwise the pointer will jump around.

The touch control scheme can be switched in the global settings. Go to **Options > Control**:

    - When **Touchpad mouse mode** is off:
    
        - The touch controls are direct, which means the pointer always jumps to the finger position. 
        - The rear touch pad is disabled. 

    - When **Touchpad mouse mode** is on:
        
        - The touch controls are indirect. The finger can be far away from the pointer and still move it, like on a laptop touchpad. 
        - The pointer speed setting in the :doc:`Control tab <../settings/control>` affects how far the pointer moves in response to a finger movement.
        - The rear touch pad is enabled. 


The front touchscreen uses the direct mode by default, but this can be changed by editing the :doc:`configuration file <../advanced_options/configuration_file>` with the following keyword:

.. _frontpanel:

*frontpanel_touchpad_mode*

When set to true, the touch controls on the front touchscreen are indirect. 

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