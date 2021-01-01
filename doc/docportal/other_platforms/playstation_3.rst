=============================
PlayStation 3
=============================

This page contains all the information you need to get ScummVM up and running on a PlayStation 3.

What you'll need
===================

- A Homebrew enabled Playstation 3 console. How to enable homebrew is outside the scope of this documentation.
- A USB drive
- A computer

Installing ScummVM
=====================================

Download the Playstation 3 package from the `ScummVM Downloads page <https://www.scummvm.org/downloads/>`_. Copy the ``.pkg`` file to a USB drive. 

Plug the USB drive into the PS3.  Go to the XMB, then go to **Games > Install Package** to install the ScummVM package.

To launch ScummVM, go to the XMB, then go to **Games > PlayStation 3 > ScummVM**. 


Transferring game files
=======================

Games can be launched from either a USB drive, or from the internal hard drive. The hard drive has better performance.  

To copy game files to the hard drive, use the USB drive, or use ScummVM's :doc:`cloud functionality <../use_scummvm/connect_cloud>`. 

See :doc:`../use_scummvm/game_files` for more information about game file requirements.

Controls
=================

Controls can also be manually configured in the :doc:`Keymaps tab <../settings/keymaps>`. See the :doc:`../use_scummvm/keyboard_shortcuts` page for common keyboard shortcuts. 


Default control map
*************************

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
    :header-rows: 2

        Touchscreen:,
        Buttons/Control,Action
        Left stick,Moves pointer
        |x|,Left mouse button
        |circle|,Right mouse button
        |triangle|,Game menu 
        |square|,Esc
        START,Global Main Menu 
        SELECT,Toggle virtual keyboard
        L1 button,AGI predictive input dialog


Paths 
=======

Saved games 
*******************

```/hdd0/game/SCUM12000/USRDIR/saves/``

Configuration file 
**************************

``/hdd0/game/SCUM12000/USRDIR/scummvm.ini``

Settings
==========

For more information, see the Settings section of the documentation. Only platform-specific differences are listed here. 

Audio
*********
Supported audio file formats:

- MP3
- OGG
- FLAC
- Uncompressed audio