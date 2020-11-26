=============================
Consoles
=============================

.. tabbed:: Nintendo 3DS

    There are two ways to install ScummVM onto a Nintendo 3DS: using the 3DSX format, and using the CIA format. 

    **What you'll need:**

    - A Nintendo 3DS with a Homebrew Launcher or custom firmware. How to enable homebrew is outside the scope of this documentation. 
    - The Nintendo 3DS package downloaded from the `ScummVM Downloads page <https://www.scummvm.org/downloads/>`_, and extracted.


    .. dropdown:: Installing ScummVM using the 3DSX format
        :open:

        Copy the extracted files to your SD card into the ``/3ds/scummvm`` directory. 

        Launch ScummVM using the Homebrew Launcher. 


    .. dropdown:: Installing ScummVM using the CIA format

        Use any CIA installation software to install the ``scummvm.cia`` file.

        The CIA format requires a DSP binary dump saved on your SD card as ``/3ds/dspfirm.cdc``. This is not an optional step, because without this you will not have proper audio support. Documenting this process is outside the scope of this documentation.  

    .. dropdown:: Transferring game files

        Transfer game folders onto the SD card. The exact location is not important. 

        ScummVM for the Nintendo 3DS also has cloud functionality. See :doc:`../use_scummvm/connect_cloud`. 

        See :doc:`../use_scummvm/add_play_games` for more information about how to add games to ScummVM. 

    .. dropdown:: Controls

        Controls can also be manually configured in the :doc:`Keymaps tab <../settings/keymaps>`. See the :doc:`../use_scummvm/keyboard_shortcuts` page for common keyboard shortcuts. 

        .. csv-table::
            :widths: 50 50
            :header-rows: 1

            Game Controller Mapping:,
            Button,Action
            Circle Pad,Moves the cursor
            R + Circle Pad,Slow Mouse
            Ⓐ,Left mouse button
            Ⓑ,Right mouse button
            Ⓧ,Opens the virtual keyboard
            Ⓨ,ESC 
            Control Pad,Keypad cursor keys
            L Button,Toggles magnify mode on/off
            R Button,Toggles between hover/drag modes
            START,Opens global main menu
            SELECT,Opens 3DS config menu

        .. _hover:

        **Hover mode**
 

        When using the touchscreen, hover mode simulates the movement of the mouse. You can click with taps, however you can't drag or hold down a mouse button unless you use the buttons mapped to the right/left mouse buttons. 

        **Drag mode**
      

        Simulates the click and release of the mouse buttons every time you touch and release the touchscreen. At the moment, this is only a left mouse button click.

        **Magnify mode**
        
        Due to the low resolutions of the 3DS screens (400x240 for the top, and 320x240 for the bottom), games that run at a higher resolution will inevitably lose some visual detail from being scaled down. This can result in situations where essential information, such as text, is indiscernable.

        Magnify mode increases the scale factor of the top screen back to 1, but the bottom screen remains unchanged. The touchscreen can then be used to change which part of the game display is being magnified. This can be done even in situations where the cursor is disabled, such as during full-motion video segments.

        When activating magnify mode, touchscreen controls are automatically switched to :ref:`hover` mode to reduce the risk of the user accidentally inputting a click when changing the magnified area with the stylus. Clicking can still be done as in normal hover mode. Turning off magnify mode will revert controls back to what was used previously. It will also restore the top screen's previous scale factor.

        Currently magnify mode can only be used when the following conditions are met:

        - In the 3DS config menu, **Use Screen** is set to **Both**
        - A game is played
        - The horizontal and/or vertical resolution in-game is greater than that of the top screen

        Magnify mode cannot be used in the Launcher menu.

    .. dropdown:: Paths 

        **Configuration file** 
    
        ``sdmc:/3ds/scummvm/scummvm.ini``

        **Saved games** 

        ``sdmc:/3ds/scummvm/saves/``

    .. dropdown:: Settings

        For more information about Settings, see the Settings section of the documentation. Only platform-specific differences are listed here. 

        **3DS config menu**


        .. figure:: ../images/3ds/config.png

            The 3DS config menu. 

        Show mouse cursor
            Toggles a visible mouse cursor on/off.

        Stretch to fit
            Stretches the image to fit the screen

        Snap to edges
            The 3DS touch screen has difficulty reaching the very end of the screen. With this option enabled, if you're within a few pixels of the edge the cursor snaps to the edge. 
            
        Use Screen:
            Sets whether ScummVM uses the **Top**, **Bottom** or **Both** screens.
            
        C-Pad Sensitivity
            Sets the sensitivity of the Circle Pad. 

        **Audio**
       
        Supported audio file formats:

        - MP3 
        - OGG 
        - OGG
        - Uncompressed audio

    .. dropdown:: Known issues

        Some games are not playable due to the slow CPU speed on the 3DS. If there are any games that run really slowly, this is considered a hardware limitation, not a bug. 
