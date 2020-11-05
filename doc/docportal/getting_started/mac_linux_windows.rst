
========================
Mac, Linux or Windows
========================

All downloads referenced on this page are available from the `ScummVM downloads page <https://www.scummvm.org/downloads/>`_.

macOS/Mac OSX
------------------

Download the Disk Image. Once the download has completed, double click the file to mount the Disk Image. A window will open with the ScummVM icon. Drag this icon into your Applications folder to install ScummVM.

To run ScummVM, click on the icon in the Applications folder.

.. note::

   macOS includes a technology called Gatekeeper, which checks to ensure only trusted software is run on your Mac. 

   Since ScummVM is not available from the App Store, see this `Apple support page <https://support.apple.com/en-us/HT202491>`_ if you have any difficulty running ScummVM. 


Linux
--------


There are multiple ways to install ScummVM onto a computer running Linux.

The Snap Store
*********************

A Snap is an app that is bundled with its dependencies, which makes the install on any Linux operating system very easy. Snap comes pre-installed on Debian and Ubuntu-based distributions, but can be installed on any Linux distribution by following the instructions on the `Snapcraft website <https://snapcraft.io/>`_.

The ScummVM Snap comes with a selection of freeware games and demos pre-loaded. 

Enter the following on the command line (using your preferred Terminal application) to install the ScummVM Snap:

.. code:: bash

   sudo snap install scummvm

To run ScummVM, enter ``scummvm`` on the command line, or launch ScummVM through the desktop interface by clicking **Menu > Games > ScummVM**.

Flathub
******************

Flathub is another way to easily install apps for Linux by using Flatpak. Flatpak comes standard with Fedora-based distributions, but can be installed on any Linux operating system.  The `Flathub website <https://flatpak.org/setup/>`_ has excellent install instructions.

Once Flatpak has been installed, enter the following on the command line to install ScummVM:

.. code:: bash

   flatpak install flathub org.scummvm.ScummVM

To run ScummVM, enter the following on the command line:

.. code:: bash

   flatpak run org.scummvm.ScummVM

.. note:: 

   The Flatpak version of ScummVM is sandboxed, meaning that any games will need to be copied into the Documents folders to be accessible by ScummVM. 

   :doc:`Command line arguments <../advanced_options/command_line>` can still be passed by adding them after the Flatpak run command.

Some distributions have the option to install Flatpaks via the graphical desktop interface. To use this option, navigate to the `ScummVM Flatpak page <https://flathub.org/apps/details/org.scummvm.ScummVM>`_ and click the **INSTALL** button. A dialog will appear; choose the **Open with Software Install (default)** option, and click **OK**. You will then be guided through the install process. 

Software Repository
*********************************

ScummVM can be found in the software repositories of many Linux distributions. Please note that these may not contain the most up to date version of ScummVM. 

If you are unsure how to install software packages from the software repository for your distribution, you may find `this article <https://www.maketecheasier.com/install-software-in-various-linux-distros/>`_ useful.

To run ScummVM, enter ``scummvm`` on the command line, or launch ScummVM through the desktop interface by clicking **Menu > Games > ScummVM**


Installing release binaries
*********************************

At this time, binary packages are only released for Debian and Ubuntu. Find and download the ScummVM version that corresponds to your operating system and architecture. To install a ``.deb`` package:

.. code:: bash

   sudo apt install /path/to/package/name.deb

In this situation, replace the ``/path/to/package/name.deb`` with the actual path to the downloaded ``.deb`` package. The APT software manager will handle the installation. 

To run ScummVM, enter ``scummvm`` on the command line, or launch ScummVM through the desktop interface by clicking **Menu > Games > ScummVM**.


Windows
---------

The easiest way to install ScummVM is to download the Windows installer. This installer will guide you through the install process, as well as adding a shortcut to the Start Menu. There is also an option to add a shortcut to the desktop. To run ScummVM, navigate to desktop and double click the ScummVM shortcut. To run ScummVM from the Start menu, go to **Start > All Apps > ScummVM**. For Windows XP, go to **Start > All Programs > Games > ScummVM**.

Alternatively, you can download the Windows zipped file appropriate to your operating system (32bit or 64bit). To unzip the contents, right-click the folder, select Extract All, and then follow the instructions. To run ScummVM from this folder, find the ``scummvm.exe`` file and double click it. 

Next steps...
-----------------------

ScummVM is now installed and ready for you to :doc:`add your games <../using_scummvm/add_play_games>`!
