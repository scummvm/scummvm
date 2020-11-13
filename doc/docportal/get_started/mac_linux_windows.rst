
========================
Mac, Linux or Windows
========================

All downloads are available from the `ScummVM downloads page <https://www.scummvm.org/downloads/>`_.

macOS/Mac OSX
------------------

Download the recommended Disk Image. Once the download has completed, double click the file to mount the Disk Image. A window will open with the ScummVM icon. Drag this icon into your Applications folder to install ScummVM.

To run ScummVM, click on the icon in the Applications folder.

.. note::

   macOS includes technology called Gatekeeper, which checks to ensure only trusted software is run on your Mac. Since ScummVM is not available from the App Store, follow the steps on this `Apple support page <https://support.apple.com/en-us/HT202491>`_ to allow ScummVM to run. 


Linux
--------

There are multiple ways to install ScummVM onto a computer running Linux; use the Snap Store, Flathub or the software repository, or install the release binary. 

The Snap Store
*********************

A Snap is an app that is bundled with its dependencies, which makes the install on any Linux operating system very easy. Snap comes pre-installed on Debian and Ubuntu-based distributions, but can be installed on any Linux distribution by following the instructions on the `Snapcraft website <https://snapcraft.io/>`_.

The ScummVM Snap comes with a selection of freeware games and demos pre-loaded. 

Enter the following on the command line to install the ScummVM Snap:

.. code:: bash

   sudo snap install scummvm

To run ScummVM, enter ``scummvm`` on the command line, or launch ScummVM through the desktop interface by clicking :menuselection:`Menu --> Games --> ScummVM`.

Flathub
**********

Flathub is another way to quickly install apps for Linux by using Flatpak. Flatpak comes standard with Fedora-based distributions, but can be installed on any Linux operating system.  The `Flathub website <https://flatpak.org/setup/>`_ has excellent install instructions.

Once Flatpak is installed, enter the following on the command line to install ScummVM:

.. code:: bash

   flatpak install flathub org.scummvm.ScummVM

Some distributions have the option to install Flatpaks via the graphical desktop interface. To use this option, navigate to the `ScummVM Flatpak page <https://flathub.org/apps/details/org.scummvm.ScummVM>`_ and click the **INSTALL** button. A dialog opens; choose the **Open with Software Install (default)** option, and then click **OK**. 

To run ScummVM, enter the following on the command line:

.. code:: bash

   flatpak run org.scummvm.ScummVM

To pass :doc:`Command line arguments <../advanced_topics/command_line>`, add them after the Flatpak ``run`` command.

.. note:: 

   The Flatpak version of ScummVM is sandboxed, meaning that any games need to be copied into the Documents folder to be accessible by ScummVM. 

Software Repository
*********************************

ScummVM is found in the software repositories of many Linux distributions. 

.. caution::

   The repositories may not contain the most up-to-date version of ScummVM. 

If you are unsure about how to install software packages from the software repository for your distribution, you may find `this help article <https://www.maketecheasier.com/install-software-in-various-linux-distros/>`_ useful.

To run ScummVM, enter ``scummvm`` on the command line, or launch ScummVM through the desktop interface by clicking **Menu > Games > ScummVM**.


Release binaries
*********************

At this time, binary packages are only released for Debian and Ubuntu. Find and download the ScummVM version that corresponds to your operating system and architecture. To install a ``.deb`` package, either double click on the downloaded ``.deb`` file to use the graphical installer, or, if that's not available, use the command line.

.. code:: bash

   sudo apt install /path/to/downloaded/file.deb

Replace ``/path/to/downloaded/file.deb`` with the actual path to the downloaded ``.deb`` package. The APT software manager will handle the installation. 

To run ScummVM, enter ``scummvm`` on the command line, or launch ScummVM through the desktop interface by clicking **Menu > Games > ScummVM**.


Windows
---------

There are two ways to install ScummVM onto a computer running Windows; download and use the installer, or install manually. 

Installer 
************

Download the Windows installer for your operating system, and double click the downloaded file. The installer guides you through the install process, and adds a shortcut to the Start Menu. There is also an option to add a shortcut to the desktop. 

To run ScummVM, either navigate to desktop and double click the ScummVM shortcut, or go to **Start > All Apps > ScummVM**. For Windows XP, go to **Start > All Apps > ScummVM**.

Manual 
**********

Download the Windows ``.zip`` file for your operating system (32bit or 64bit). To unzip the contents, right-click the folder and select **Extract All**. 

To run ScummVM from the extracted folder, find the ``scummvm.exe`` file and double click it. 
