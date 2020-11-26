=========================
Phones and tablets
=========================

.. tabbed:: Android

    There are two ways to install ScummVM on an Android device; install it from the Google Play Store, or manually download and install the APK package from the ScummVM downloads page. 

    .. dropdown:: Installing ScummVM from the Google Play Store
        :open:

        Go to the Google Play Store to download and install ScummVM. 

        .. note:: 

            ScummVM has been tested to work up to Android 10. Currently there are some file browsing limitations with Android 11. The ScummVM team are working to resolve these.  

    .. dropdown:: Downloading and manually installing the APK package 

        On your device, use a web browser app to navigate to the `ScummVM downloads page <https://www.scummvm.org/downloads>`_. The recommended download section on the page suggests the correct ``.apk`` package, however if it does not, scroll down to select the correct package. 

        .. tip:: 

            To find the correct package for your device, find out whether the device has a 32 or 64-bit operating system, and which chipset the device has. Do an internet search using the device model number to find these details.

        Click on the correct package to start the download. After it has downloaded, go to the file and tap on it to start the install process. Use either a file manager app, or select the file directly from the web browser downloads. The file manager or the web browser asks for permission to install the package, and after permission is granted, returns to the install. 

        .. note:: 

            The downloaded APK package is treated as an app from an unknown source, because it doesn't originate in the Google Play store. To allow unknown apps:

            - For Android 7 or lower, go to **Settings > Security > Device Administration**, scroll down to **Device Administration** and enable **Unknown Sources**.
            - For Android 8 or higher, go to **Settings > Apps and Notification** and select **Install Unknown Apps**. 
    

    .. dropdown:: Transferring game files 
  
        Transfer all the required game data files to your device, into a folder accessible by the ScummVM app. 

        ScummVM has built-in Cloud functionality, which lets you connect your Google Drive, OneDrive, Box or Dropbox account. For more information, see the :doc:`../use_scummvm/connect_cloud` page. ScummVM also has the ability to run a local web server. For more information, see the :doc:`../use_scummvm/LAN` page. 

        There are a few other options to transfer the game files:

        - Copy the files directly onto the microSD card, if the device has one.
        - USB transfer from a computer. To complete file transfer by USB cable, tap the **Charging this device via USB notification** when the device is connected. Select **Use USB for** and then **File Transfer**. An Android File Transfer window opens.   
        - Download games such as freeware games or those from digital distributors directly to your device.

        For more information on which files ScummVM requires, see the :doc:`../use_scummvm/game_files` page. To learn how to add and play games, see the :doc:`../use_scummvm/add_play_games`.

    .. dropdown:: Controls


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

        **Touch controls**
        
        The touch control scheme can be switched in the global settings. From the Launcher, go to **Options > Control > Touchpad mouse mode**.

            - When touchpad mouse mode is off (default), the touch controls are direct. The pointer jumps to where the finger touches the screen.
            - When touchpad mouse mode is on, the touch controls are indirect. The finger can be far away from the pointer and still move it, like on a laptop touchpad. 
            - The pointer speed setting in the :doc:`Controls tab <../settings/control>` affects how far the pointer moves in response to a finger movement.

        **Two finger tap**
        
        To do a two finger tap, hold one finger down and then tap with a second finger. 


        **Immersive Sticky fullscreen mode**
        
        ScummVM for Android uses the Immersive Sticky fullscreen mode, which means that the Android system bar is hidden until the user swipes from an edge with a system bar. Swipe from the edge to reveal the system bars.  They remain semi-transparent and disappear after a few seconds unless you interact with them. Your swipe also registers in the game, so if you need to swipe from an edge with system bars, your game play is not interrupted. 

        **Virtual keyboard**
      
        To open the virtual keyboard, tap on the small keyboard icon at the top right of the screen, or tap on any editable text field. To hide the virtual keyboard, tap the small keyboard icon again, or tap outside the text field. 

        To display or hide the small keyboard icon, from the Launcher select **Options** and then the **Control** tab. Tick the **Show on-screen control** box to enable the keyboard icon. 

    .. dropdown:: Paths

        **Saved games**
        
        By default, the saved games are saved in the internal app folder. To change the path, go to the :doc:`../settings/paths` tab in either the global or game-specific settings. 

        **Configuration file**
 
        The configuration file is in the internal app folder, and this path cannot be changed. 

        **Accessing the internal app folder**

        To access the internal app folder, use ScummVM's built-in LAN functionality:

        1. From the Launcher, select **Options** and then the **LAN** tab. 
        2. Select **/root/Path**.
        3. Use the file browser to **Go up** to the root of the ScummVM internal app folder. 
        4. Select the **ScummVM data (int)** shortcut, and tap **Choose**. 
        5. Run the server to access the ``scummvm.ini``` configuration file. 

        For more information, see the :doc:`../use_scummvm/LAN` page. 
           
            
    .. dropdown:: Known issues

        - On some newer devices, there is an issue if the saved path is outside the app's internal (or external) storage. The ScummVM team is currently working on a fix.

        - If ScummVM is uninstalled or downgraded, its internal and external app spaces are fully deleted. If you want to keep saved games use ScummVM's :doc:`cloud <../use_scummvm/connect_cloud>` or LAN functionality to keep those files. Alternatively, change the saved game path to a shared location such as an SD card. 

.. tabbed:: iOS


    There are two ways to install ScummVM onto an iOS device, and the method you use depends on whether or not the device is jailbroken. 
    
    .. dropdown:: Installing ScummVM onto a device without a jailbreak

        To install ScummVM onto a device without a jailbreak requires you to build the app yourself, using the source code and an app called Xcode. While a rudimentary knowledge of the command line would be helpful, this is not required. 

         **What you'll need:**

        - A Mac computer with Xcode installed. Xcode is a free App, available from the Mac App Store.
        - An Apple Developer account. You can sign up for a free account on the `Apple Developer Member Center <https://developer.apple.com/membercenter/>`_ with your Apple ID. 
        - The ScummVM `iOS Libraries <https://www.scummvm.org/frs/build/scummvm-ios7-libs-v2.zip>`_ downloaded. 

            .. note::

                If you have a free Apple developer account, each build will only be valid for 7 days, which means you will need to repeat these steps and rebuild ScummVM every week.

                
        **Creating the Xcode project**


        The first step is to download the repository containing the code required to build the app. The following steps require use of the command line. Go to **Applications > Utilities > Terminal** and copy and paste the following, then press :kbd:`return` :

        .. code-block:: bash

            git clone --depth 1 -b branch-2-2-0 https://github.com/scummvm/scummvm.git


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


        **Generating the Xcode project**

        It's time to generate the Xcode project. Ensure the present working directory is still the ``build`` directory, and then run the following on the command line:

        .. code::

            ../scummvm/devtools/create_project/xcode/build/Release/create_project ../scummvm --xcode --enable-fluidsynth --disable-nasm --disable-opengl --disable-theora   --disable-taskbar --disable-tts --disable-fribidi

        The resulting directory structure should look like this:

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

        **Building ScummVM**

        When Xcode is open, connect a device to install ScummVM onto. At the top of the window, select **ScummVM-iOS** and then select the device you just connected. You will need to "Trust" the device for it to show up on the list of connected devices. 

        .. figure:: ../images/ios/choose_device.gif

                
        The last step before building ScummVM is to change the bundle identifier and manage signing. Click on the **scummvm** project in the left pane, and then click on **ScummVM - iOS** under **TARGETS**. On the menu bar above, click on **General**. Under **Identity** there is a field labeled **Bundle Identifier**. Enter a unique identifier in reverse-DNS format. This can be as simple as com.\ *yournamehere*. 

        .. figure:: ../images/ios/identifier.gif


        Next to **General**, click on **Signing & Capabilities**. Under **Signing**, tick the **Automatically manage signing** box, and select your developer account from the **Team** dropdown menu. 

        .. figure:: ../images/ios/signing.gif

                

        If you have not added your developer account to Xcode, do this now. Click **Add an Account** in the dropdown menu.

        In the upper left-hand corner, press the play button to build ScummVM. When the build is finished, it launches on your connected device. 

    .. dropdown:: Installing ScummVM onto a jailbroken device

        **What you'll need:**

        - A jailbroken iOS device. How to jailbreak a device is outside the scope of this documentation.
        - A file manager app such as Filza installed on the device.


        **Installing ScummVM**

        Go to the `ScummVM downloads page <https://www.scummvm.org/downloads>`_ and download the recommended ``.deb`` or ``.ipa`` package. When the device asks how to open the file, choose Filza.

        Ensure the package is saved somewhere in the ``/var/mobile/`` directory. It defaults to ``/var/mobile/Documents/``, which is fine. Tap on the file and then tap **Install** in the upper right corner to install ScummVM.

        Restart your device for the install to complete. 

    .. dropdown:: Transferring game files 


        There are multiple ways to transfer game files to your iOS device. 

        ScummVM has built-in cloud functionality, which lets you connect your Google Drive, OneDrive, Box or Dropbox account. For more information, see the :doc:`../use_scummvm/connect_cloud` page. ScummVM also has the ability to run a local web server. For more information, see the :doc:`../use_scummvm/LAN` page. 

        .. note::

            ScummVM's cloud functionality does not currently support iCloud, however you can upload game folders to your iCloud and then use the Files app on your iOS device to copy these folders into the local ScummVM folder.

        Another way to transfer files (for macOS Catalina and newer) to your device is by using the Finder while your device is connected to your Mac. In the Finder app on your Mac, navigate to **Locations** in the left pane, and click on the connected device. Click on **Files**, then drag the folder containing the game files into the **ScummVM** folder. For older versions of macOS, and for Windows users, file transfer can be done in iTunes. 

        .. image:: ../images/ios/ios_transfer_files.gif
        
        For more information on which files ScummVM requires, and how to add and play games, see :doc:`../use_scummvm/add_play_games`. 

    .. dropdown:: Controls

        .. csv-table:: 
            :widths: 40 60 
            :header-rows: 2

                Game Controller Mapping,
                Touch screen control, Action
                One finger tap, Left mouse click
                Two finger tap, Right mouse click
                Two finger double tap,ESC
                Two finger swipe (bottom to top), Toggles :ref:`Click and drag mode <clickdrag>`.
                Two finger swipe (left to right),Toggles between touch direct mode and touchpad mode. 
                Two finger swipe (top to bottom),Global Main Menu  
                Three finger swipe, Arrow keys  
            
                Pinch gesture, Enables/disables keyboard
                Keyboard spacebar, Pause
        

        **Touch controls**
    
        The touch control scheme can be switched in the global settings. From the Launcher, go to **Options > Control > Touchpad mouse mode**.

            - When touchpad mouse mode is off, the touch controls are direct. The pointer jumps to where the finger touches the screen.
            - When touchpad mouse mode is on, the touch controls are indirect. The finger can be far away from the pointer and still move it, like on a laptop touchpad. 
            - The pointer speed setting in the :doc:`Controls tab <../settings/control>` affects how far the pointer moves in response to a finger movement.

        .. _twofinger:

        **Two finger tap**


        For a two finger tap, hold one finger down and then tap with a second finger. 

        .. _twofingerdouble:

        **Two finger double tap**
    

        For a two finger double tap, hold one finger down and then double tap with a second finger.

        .. _clickdrag:

        **Click and drag mode**
    

        Simulates a left mouse click when you touch the screen. The mouse pointer drags to wherever you slide your finger, and releases where you lift your finger.

        If you press down a second finger in this mode, it releases the simulated left mouse button, and presses down the simulated right mouse button. The right mouse button is released when you lift your finger again. 

        **Keyboard**

        If no external keyboard is connected, the pinch gesture shows and hides the onscreen keyboard. When an external keyboard is connected, the pinch gesture enables/disables inputs from the external keyboard.

    .. dropdown:: Paths


        **Saved games**

        ``/var/mobile/Library/ScummVM/Savegames/`` if the device is jailbroken, or ``Savegames/`` in the ScummVM folder for a non-jailbroken device. Access this folder through the Finder or iTunes. 

        **Configuration file**

        ``/var/mobile/Library/ScummVM/Preferences`` if the device is jailbroken, or ``Preferences`` in the ScummVM folder for a non-jailbroken device. Access this folder through the Finder or iTunes. 


