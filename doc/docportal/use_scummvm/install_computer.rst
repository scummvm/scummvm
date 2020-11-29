
=====================================
Installing ScummVM 
=====================================

This page explains how to install ScummVM on a computer. For all other platforms, see the relevant :ref:`platform guide <platformspecific>`.

.. tabbed:: Windows

    There are two ways to install ScummVM on Windows: use the installer, or install manually. 

    .. dropdown:: Installing ScummVM using the installer
        :open:

        Download the Windows installer for your operating system from the `ScummVM downloads page <https://www.scummvm.org/downloads/>`_, and double click the downloaded file. The installer guides you through the install process, and adds a shortcut to the Start Menu. There is also an option to add a shortcut to the desktop. 

        To run ScummVM, either navigate to desktop and double click the ScummVM shortcut, or go to **Start > All Apps > ScummVM**. For Windows XP, go to **Start > All Apps > ScummVM**.

    .. dropdown:: Installing ScummVM manually
          
        Download the Windows zip file for your operating system (32bit or 64bit). To extract the files, right-click the folder and select **Extract All**. 

        To run ScummVM from the extracted folder, find the ``scummvm.exe`` file and double click it. 

.. tabbed:: macOS

    .. dropdown:: Installing ScummVM using the disk image
        :open:

        Download the recommended disk image file from the `ScummVM downloads page <https://www.scummvm.org/downloads/>`_. After the download has completed, double click the file to mount the disk image file. A window containing the ScummVM icon opens. Drag this icon into your Applications folder to install ScummVM.

        To run ScummVM, click on the icon in the Applications folder.

        .. note::

            macOS includes technology called Gatekeeper, which checks to ensure only trusted software is run on your Mac. ScummVM is not available from the App Store, so follow the steps on this `Apple support page <https://support.apple.com/en-us/HT202491>`_ to allow ScummVM to run. 
    
.. tabbed:: Linux


   There are multiple ways to install ScummVM on Linux: use the Snap Store, Flathub or the software repository, or manually install the release binary. 

    .. dropdown::  Installing ScummVM using the Snap Store
        :open:

        A Snap is an app that is bundled with its dependencies, which makes the install on any Linux operating system very easy. Snap comes pre-installed on Debian and Ubuntu-based distributions, but can be installed on any Linux distribution by following the instructions on the `Snapcraft website <https://snapcraft.io/>`_.

        The ScummVM Snap comes with a selection of freeware games and demos pre-loaded. 

        Enter the following on the command line to install the ScummVM Snap:

        .. code:: bash

            sudo snap install scummvm

        To run ScummVM, enter ``scummvm`` on the command line, or launch ScummVM through the desktop interface by clicking **Menu > Games > ScummVM**.

    .. dropdown:: Installing ScummVM using Flathub

        Flathub is another way to install apps for Linux, by using Flatpak. Flatpak comes standard with Fedora-based distributions, but can be installed on any Linux operating system.  The `Flathub website <https://flatpak.org/setup/>`_ has excellent install instructions.

        When Flatpak is installed, enter the following on the command line to install ScummVM:

        .. code:: bash

            flatpak install flathub org.scummvm.ScummVM

        Some distributions have the option to install Flatpaks through the graphical desktop interface. Navigate to the `ScummVM Flatpak page <https://flathub.org/apps/details/org.scummvm.ScummVM>`_ , click the **INSTALL** button and then follow the install process. 

        To run ScummVM, enter the following on the command line:

        .. code:: bash

            flatpak run org.scummvm.ScummVM

        To pass :doc:`Command line arguments <../advanced_topics/command_line>`, add them after the Flatpak ``run`` command.

        .. note:: 

            The Flatpak version of ScummVM is sandboxed, meaning that any games need to be copied into the Documents folder to be accessible by ScummVM. 

      
    .. dropdown:: Installing ScummVM using the software repository

        ScummVM is found in the software repositories of many Linux distributions. 

        .. caution::

            The repositories might not contain the most up-to-date version of ScummVM. 

        To run ScummVM, enter ``scummvm`` on the command line, or launch ScummVM through the desktop interface by clicking **Menu > Games > ScummVM**.


    .. dropdown:: Installing ScummVM using the release binaries
        
        Binary packages are only released for Debian and Ubuntu. On the `ScummVM downloads page <https://www.scummvm.org/downloads/>`_, find and download the ScummVM package that corresponds to your operating system and architecture. To install a DEB package, either double click on the downloaded DEB file to use the graphical installer, or, if that's not available, use the command line.

        .. code:: bash

            sudo apt install /path/to/downloaded/file.deb

        Replace ``/path/to/downloaded/file.deb`` with the actual path to the downloaded DEB package. The APT software manager handles the installation. 

        To run ScummVM, enter ``scummvm`` on the command line, or launch ScummVM through the desktop interface by clicking **Menu > Games > ScummVM**.

