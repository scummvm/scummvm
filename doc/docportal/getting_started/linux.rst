
====================================================================
Linux 
====================================================================

Downloading and installing ScummVM
===================================
There are multiple ways to install ScummVM onto a computer running Linux, and all are accessible from the `ScummVM Downloads page <https://www.scummvm.org/downloads/>`_. 

The Snap Store
---------------
A Snap is an app that is bundled with its dependencies, which makes the install on any Linux operating system very easy. Snap comes pre-installed on Debian and Ubuntu-based distributions, but can be installed on any Linux distribution by following the instructions on the `Snapcraft website <https://snapcraft.io/>`_.

The ScummVM Snap comes with a selection of freeware games and demos pre-loaded. 

Enter the following on the command line (using your preferred Terminal application) to install the ScummVM Snap:

.. code:: bash

   sudo snap install scummvm

To run ScummVM, enter ``scummvm`` on the command line, or launch ScummVM through the desktop interface by clicking **Menu > Games > ScummVM**.

Flathub
----------
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
--------------------

ScummVM can be found in the software repositories of many Linux distributions. Please note that these may not contain the most up to date version of ScummVM. 

If you are unsure how to install software packages from the software repository for your distribution, you may find `this article <https://www.maketecheasier.com/install-software-in-various-linux-distros/>`_ useful.

To run ScummVM, enter ``scummvm`` on the command line, or launch ScummVM through the desktop interface by clicking **Menu > Games > ScummVM**


Installing release binaries
----------------------------

At this time, binary packages are only released for Debian and Ubuntu. Find and download the ScummVM version that corresponds to your operating system and architecture. To install a ``.deb`` package:

.. code:: bash

   sudo apt install /path/to/package/name.deb

In this situation, replace the ``/path/to/package/name.deb`` with the actual path to the downloaded ``.deb`` package. The APT software manager will handle the installation. 

To run ScummVM, enter ``scummvm`` on the command line, or launch ScummVM through the desktop interface by clicking **Menu > Games > ScummVM**.

**Congratulations!**

ScummVM is now installed and ready for you to :doc:`add your games <../getting_started/adding_games>`!
