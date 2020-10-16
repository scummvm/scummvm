
===============================
Saving and loading games
===============================

Games can be saved and loaded directly from the Global Main Menu (GMM), which can be accessed during game play by using the default :kbd:`Ctrl` + :kbd:`F5` keyboard shortcut, or the shortcut set in the :ref:`global keymap <keyglobal>`.

Saved games can also be loaded directly from the :ref:`Launcher <loadlauncher>` by clicking on the desired game, and then clicking the **Load** button. 

Autosave
------------

For some games, ScummVM will automatically save the game every 5 minutes. The :ref:`autosave period <autosave>` can be changed in the Misc tab of the :doc:`global_settings`, or in the :doc:`configuration file <../advanced_options/configuration_file>`


The command line
--------------------

Viewing saved games
*********************

``--list-saves`` can be used to display the list of current saved games for the specified game, as well as the corresponding save slot. Note that this option is not available for all game engines. 

Usage:

.. code-block::

	scummvm --list-saves --game=GAMEID


For example, to see the saved games for the Curse of Monkey Island, run the following on the command line:

.. code-block::

	scummvm --list-saves --game=comi

Loading saved games
*********************

``--save-slot`` or ``-x`` can be used to load a saved gamed directly from the command line. Where no save slot is specified, the default is the autosave. Note that this option is not available for all game engines. 

Usage:

.. code-block::

	scummvm --save-slot=SLOT GAMEID
	scummvm -x=SLOT GAMEID

For example, to load the saved game at slot 0, for Flight of the Amazon Queen, run the following on the command line:

.. code-block::

	scummvm --save-slot=0 queen

For more information on how to use ScummVM with the command line, see the :doc:`command line <../advanced_options/command_line>` page.  	

Location of saved game files
-------------------------------

Saved game files can be found in the current ScummVM directory for some platforms, and in default directories for others. The defaults for Windows, Mac and Linux/Unix are shown below. 

The save directory can be changed via the savepath setting in the Paths tab of the :ref:`global settings <savepath>` or it can be changed for a specific game in the :ref:`game settings <savepathgame>`. It can also be changed in the :doc:`configuration file <../advanced_options/configuration_file>`.

.. tabs::

	.. tab:: Mac OSX

		``~/Documents/ScummVM Savegames/``, where ``~`` is your Home directory. To see the Libraries folder you will need to view hidden files.
	
	.. tab:: Windows

		**Windows NT4**: ``<windir>\Profiles\username\Application Data\ScummVM\Saved games\`` where ``<windir>`` refers to the Windows directory. Most commonly, this is ``C:\WINDOWS``.
		
		**Windows 2000/XP**: ``\Documents and Settings\username\Application Data\ScummVM\Saved games\``
		
		**Window 7/Vista**: ``\Users\username\AppData\Roaming\ScummVM\Saved games\``
		
		The Application Data/AppData directory is hidden. To view it, use the Run utility or the command prompt, and enter ``%APPDATA%\ScummVM\Saved Games``, or enable hidden files in Windows Explorer. 

	.. tab:: Linux/Unix

		We follow the XDG Base Directory Specification. This means our configuration can be found in ``$XDG_DATA_HOME/scummvm/saves/``

		If XDG_DATA_HOME is not defined or empty, ``~/.local/share/`` will be used, where ``~`` is your Home directory. 
		
		Note that ``.local`` is a hidden directory; to view it use ``ls -a`` on the command line. If you are using a GUI file manager, go to **View > Show Hidden Files**, or use the keyboard shortcut :kbd:`Ctrl + H`.

		If ScummVM was installed using Snap, the saves can be found at ``~/snap/scummvm/current/.local/share/scummvm/saves/``


