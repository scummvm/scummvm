
==============
iOS
==============

Install ScummVM
=====================
There are two methods to install ScummVM onto an iOS device, and the method use depends on whether or not the device is jailbroken. 

No Jailbreak
^^^^^^^^^^^^^^^^

This method is a little complex, however it is currently the only way to install ScummVM onto an iOS device that is not jailbroken. While a rudimentary knowledge of the command line would be useful, the following instructions, if followed carefully, can be completed by anyone. 

Prerequisites
****************

- A Mac computer with Xcode installed. Xcode is a free App, available from the Mac App Store.
- An Apple Developer account. You can sign up for a free account on the `Apple Developer Member Center <https://developer.apple.com/membercenter/>`_ with your Apple ID. 
- The ScummVM `iOS Libraries <https://www.scummvm.org/frs/build/scummvm-ios7-libs-v2.zip>`_ downloaded. 

.. note::

    If you have a free Apple developer account, each build will only be valid for 7 days, which means you will need to repeat these steps and rebuild ScummVM every week.

Create the Xcode project
***************************

The first step is to download the repository containing the code required to build the app. The next steps require use of the command line. Go to **Applications > Utilities > Terminal** and copy and paste the following, then press :kbd:`return` :

.. code-block:: bash

    git clone https://github.com/scummvm/scummvm.git

.. note::
    The default branch of the ScummVM repository will be the master branch. To build a specific version of ScummVM, checkout the appropriate branch. For example, to build version 2.2.0, use the following command:

    .. code-block::

        git checkout branch-2-2-0

If you haven't done so before now, open Xcode from the Applications folder, and accept the End User License Agreement. Starting Xcode for the first time will also install the command line tools required. 

The next step changes the present working directory and then generates the tools required to create the Xcode project.

.. code-block:: bash

    cd scummvm/devtools/create_project/xcode
    xcodebuild

The next command creates a new directory called ``build`` at the same level as the repository:

.. code-block:: bash

    cd ../../../..
    mkdir build
    cd build

The present working directory will now be the new ``build`` directory. Extract the contents of the downloaded iOS libraries package into the build folder by using the following command:

.. code-block:: bash

    unzip ~/Downloads/scummvm-ios7-libs-v2.zip

.. note::

    In newer versions of macOS, the archive is extracted automatically. If this is the case, the ``unzip`` command will not work as expected. Instead, copy the contents of the ``scummvm-ios7-libs-v2`` folder into the ``build`` directory. It is simplest to do this in the Finder. 


Generate the Xcode project
*****************************

It's time to generate the Xcode project. Ensure the present working directory is still the ``build`` directory, and then run the following on the command line:

.. code::

    ../scummvm/devtools/create_project/xcode/build/Release/create_project ../scummvm --xcode --enable-fluidsynth --disable-nasm --disable-opengl --disable-theora --disable-taskbar --disable-tts --disable-fribidi

The resulting directory structure looks like this:

.. code-block:: bash

    Home
     |--scummvm
     '--build
         |-- include 
         |-- lib
         |-- engines
         '-- scummvm.xcodeproj

.. tip::

    To view the Home directory in the Finder, select **Go > Home** in the menu bar, or press :kbd:`command+shift+H`.

Open the ``scummvm.xcodeproj`` file from the Finder, or from the command line:

.. code-block:: bash

    open scummvm.xcodeproj

Build ScummVM
*****************

When Xcode is open, connect a device to install ScummVM onto. At the top of the window, select **ScummVM-iOS** and then select the device you just connected. You will need to "Trust" the device for it to show up on the list of connected devices. 

.. figure:: ../images/ios/choose_device.gif

   
The last step before building ScummVM is to change the bundle identifier and manage signing. Click on the **scummvm** project in the left pane, and then click on **ScummVM - iOS** under **TARGETS**. On the menu bar above, click on **General**. Under **Identity** there is a field labeled **Bundle Identifier**. Enter a unique identifier in reverse-DNS format. This can be as simple as com.\ *yournamehere*. 

.. figure:: ../images/ios/identifier.gif


Next to **General**, click on **Signing & Capabilities**. Under **Signing**, tick the **Automatically manage signing** box, and select your developer account from the **Team** dropdown menu. 

.. figure:: ../images/ios/signing.gif

   

If you have not added your developer account to Xcode, do this now. Click **Add an Account** in the dropdown menu.

In the upper left-hand corner, press the play button to build ScummVM. When the build is finished, it launches on your connected device. 


Jailbreak
^^^^^^^^^^^^


Prerequisites
***************

- A jailbroken iOS device. How to jailbreak a device is outside the scope of this documentation.
- A file manager app such as Filza installed on the device.


Install ScummVM
*******************

Go to the `ScummVM downloads page <https://www.scummvm.org/downloads>`_ and download the recommended ``.deb`` or ``.ipa`` package. When the device asks how to open the file, choose Filza.

Ensure the package is saved somewhere in the ``/var/mobile/`` directory. It defaults to ``/var/mobile/Documents/``, which is fine. Tap on the file and then tap **Install** in the upper right corner to install ScummVM.

Restart your device for the install to complete. 

Transfer game files 
========================

There are multiple ways to transfer game files to your iOS device. 

ScummVM has built-in cloud functionality, which lets you connect your Google Drive, OneDrive, Box or Dropbox account. For more information, see the :doc:`../use_scummvm/connect_cloud` page. ScummVM also has the ability to run a local web server. For more information, see the :doc:`../use_scummvm/LAN` page. 

.. note::

 ScummVM's cloud functionality does not currently support iCloud, however you can upload game folders to your iCloud and then use the Files app on your iOS device to copy these folders into the local ScummVM folder.

Another way to transfer files (for macOS Catalina and newer) to your device is by using the Finder while your device is connected to your Mac. In the Finder app on your Mac, navigate to **Locations** in the left pane, and click on the connected device. Click on **Files**, then drag the folder containing the game files into the **ScummVM** folder. For older versions of macOS, and for Windows users, file transfer can be done in iTunes. 

.. image:: ../images/ios/ios_transfer_files.gif
   

For more information on which files ScummVM requires, and how to add and play games, see :doc:`../use_scummvm/add_play_games`. 

Controls
============

.. csv-table:: 
  	:widths: 40 60 
  	:header-rows: 2

        Game Controller Mapping,
        Touch screen control, Action
        One finger tap, Left mouse click
        :ref:`twofinger`, Right mouse click
        :ref:`twofingerdouble`,ESC
        Two finger swipe (bottom to top), Toggles `Click and drag mode`_
        Two finger swipe (left to right),Toggles between touch direct mode and touchpad mode. 
        Two finger swipe (top to bottom),Global Main Menu  
        Three finger swipe, Arrow keys  
       
        Pinch gesture, Enables/disables keyboard
        Keyboard spacebar, Pause
       

Touch controls
^^^^^^^^^^^^^^^^^
The touch control scheme can be switched in the global settings. From the Launcher, go to **Options > Control > Touchpad mouse mode**.

    - When touchpad mouse mode is off, the touch controls are direct. The pointer jumps to where the finger touches the screen.
    - When touchpad mouse mode is on, the touch controls are indirect. The finger can be far away from the pointer and still move it, like on a laptop touchpad. 
    - The pointer speed setting in the :doc:`Controls tab <../settings/control>` affects how far the pointer moves in response to a finger movement.

.. _twofinger:

Two finger tap
******************

For a two finger tap, hold one finger down and then tap with a second finger. 

.. _twofingerdouble:

Two finger double tap
************************

For a two finger double tap, hold one finger down and then double tap with a second finger.


Click and drag mode
*********************

Simulates a left mouse click when you touch the screen. The mouse pointer drags to wherever you slide your finger, and releases where you lift your finger.

If you press down a second finger in this mode, it releases the simulated left mouse button, and presses down the simulated right mouse button. The right mouse button is released when you lift your finger again. 

Keyboard
**********
If no external keyboard is connected, the pinch gesture shows and hides the onscreen keyboard. When an external keyboard is connected, the pinch gesture enables/disables inputs from the external keyboard.

Paths
=======

Saved games
^^^^^^^^^^^^^^^^^

``/var/mobile/Library/ScummVM/Savegames/`` if the device is jailbroken, or ``Savegames/`` in the ScummVM folder for a non-jailbroken device. Access this folder through the Finder or iTunes. 

Configuration file
^^^^^^^^^^^^^^^^^^^^^^^

``/var/mobile/Library/ScummVM/Preferences`` if the device is jailbroken, or ``Preferences`` in the ScummVM folder for a non-jailbroken device. Access this folder through the Finder or iTunes. 