
===============
Android
===============

This page contains all the information you need to get ScummVM up and running on an Android device. 

What you'll need
================

- An Android device running Android 4.1 (Jelly Bean) or newer.

.. note::

    ScummVM has been tested to work up to Android 10. Currently there are some file browsing limitations with Android 11. The ScummVM team are working to resolve these.  

Installing ScummVM
====================================

There are two ways to install ScummVM on an Android device; install it from the Google Play Store, or manually download and install the APK package from the ScummVM downloads page. 

Installing from the Google Play Store
***************************************

The latest supported version of ScummVM is on the Google Play Store; search for ScummVM on the Google Play Store and select **Install**. 


Manually installing the APK package 
*************************************

On your device, use a web browser app to navigate to the `ScummVM downloads page <https://www.scummvm.org/downloads>`_. The recommended download section on the page suggests the correct ``.apk`` package, however if it does not, scroll down to select the correct package. 

.. tip:: 

    To find the correct package for your device, find out whether the device has a 32 or 64-bit operating system, and which chipset the device has. Do an internet search using the device model number to find these details.

Click on the correct package to start the download. After it has downloaded, go to the file and tap on it to start the install process. Use either a file manager app, or select the file directly from the web browser downloads. The file manager or the web browser asks for permission to install the package, and after permission is granted, returns to the install. 

.. note:: 

    The downloaded APK package is treated as an app from an unknown source, because it doesn't originate in the Google Play store. To allow unknown apps:

    - For Android 7 or lower, go to **Settings > Security > Device Administration**, scroll down to **Device Administration** and enable **Unknown Sources**.
    - For Android 8 or higher, go to **Settings > Apps and Notification** and select **Install Unknown Apps**. 
    
Transferring game files 
========================================

Transfer all the required game data files to your device, into a folder accessible by the ScummVM app. 

ScummVM has built-in Cloud functionality, which lets you connect your Google Drive, OneDrive, Box or Dropbox account. For more information, see the :doc:`../use_scummvm/connect_cloud` page. ScummVM also has the ability to run a local web server. For more information, see the :doc:`../use_scummvm/LAN` page. 

There are a few other options to transfer the game files:

 - Copy the files directly onto the microSD card, if the device has one.
 - USB transfer from a computer. To complete file transfer by USB cable, tap the **Charging this device via USB notification** when the device is connected. Select **Use USB for** and then **File Transfer**. An Android File Transfer window opens.   
 - Download games such as freeware games or those from digital distributors directly to your device.

See :doc:`../use_scummvm/game_files` for more information about game file requirements.



Controls
=============

Default control mapping
****************************

Controls can also be manually configured in the :doc:`Keymaps tab <../settings/keymaps>`. See the :doc:`../use_scummvm/keyboard_shortcuts` page for common keyboard shortcuts. 

.. csv-table:: 
    :header-rows: 1

        Touch screen control, Action
        One finger tap, Left mouse click
        Two finger tap, Right mouse click
        One finger touch & hold, Middle mouse click 
        Long press system Back button, Opens Global Main Menu
        Short press system Back button, "Skip, or Cancel/Quit in the Launcher"
        Small keyboard icon (top right), Opens/closes Virtual keyboard

Touch controls
****************
The touch control scheme can be switched in the global settings. From the Launcher, go to **Options > Control > Touchpad mouse mode**.

    - When touchpad mouse mode is off (default), the touch controls are direct. The pointer jumps to where the finger touches the screen.
    - When touchpad mouse mode is on, the touch controls are indirect. The finger can be far away from the pointer and still move it, like on a laptop touchpad. 
    - The pointer speed setting in the :doc:`Controls tab <../settings/control>` affects how far the pointer moves in response to a finger movement.

Two finger tap
^^^^^^^^^^^^^^^^^

To do a two finger tap, hold one finger down and then tap with a second finger. 


Immersive Sticky fullscreen mode
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

ScummVM for Android uses the Immersive Sticky fullscreen mode, which means that the Android system bar is hidden until the user swipes from an edge with a system bar. Swipe from the edge to reveal the system bars.  They remain semi-transparent and disappear after a few seconds unless you interact with them. Your swipe also registers in the game, so if you need to swipe from an edge with system bars, your game play is not interrupted. 

Virtual keyboard
^^^^^^^^^^^^^^^^^^^^^

To open the virtual keyboard, tap on the small keyboard icon at the top right of the screen, or tap on any editable text field. To hide the virtual keyboard, tap the small keyboard icon again, or tap outside the text field. 

To display or hide the small keyboard icon, from the Launcher select **Options** and then the **Control** tab. Tick the **Show on-screen control** box to enable the keyboard icon. 


Paths
=======

Saved games
**************

By default, the saved games are saved in the internal app folder. To change the path, go to the :doc:`../settings/paths` tab in either the global or game-specific settings. 


Configuration file
************************

The configuration file is in the internal app folder, and this path cannot be changed. 

Accessing the internal app folder
************************************

To access the internal app folder, use ScummVM's built-in LAN functionality:

1. From the Launcher, select **Options** and then the **LAN** tab. 
2. Select **/root/Path**.
3. Use the file browser to **Go up** to the root of the ScummVM internal app folder. 
4. Select the **ScummVM data (int)** shortcut, and tap **Choose**. 
5. Run the server to access the ``scummvm.ini``` configuration file. 

For more information, see the :doc:`../use_scummvm/LAN` page. 


Known issues
===============

- On some newer devices, there is an issue if the saved path is outside the app's internal (or external) storage. The ScummVM team is currently working on a fix.

- If ScummVM is uninstalled or downgraded, its internal and external app spaces are fully deleted. If you want to keep saved games use ScummVM's :doc:`cloud <../use_scummvm/connect_cloud>` or LAN functionality to keep those files. Alternatively, change the saved game path to a shared location such as an SD card. 




