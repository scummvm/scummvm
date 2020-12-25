
===============================
Saving and loading a game
===============================

This guide explains how to save and load a game from the Global Main Menu, or load a game directly from the Launcher. 

.. note::

	Some games use their own load or save entry points within the game, or use their own menu instead of the GMM.

Saving a game
==============

When you're playing a game and you're ready to save, press :kbd:`Ctrl+F5` (:kbd:`Ctrl+fn+F5` on a Mac) to access the Global Main Menu. Select **Save**.  

.. figure:: ../images/Launcher/gmm_save.png

	The Global Main Menu (GMM). 

There are two views: list view and tile view. Click the list and tile icons at the bottom left of the window to toggle between the two.  

In tile view, click on the **New Save** tile. Enter a description for the game, then click **OK**. 

.. figure:: ../images/Launcher/save_game_tile.png

	Saving a game, tile view.

.. figure:: ../images/Launcher/save_game_tile_desc.png

	Save game decription, tile view. 

In list view, click on a slot to save the game to that slot. Enter a description for the game, then click **Save**.

.. figure:: ../images/Launcher/save_game_list.png

	Saving a game, list view.  


Autosave
------------

ScummVM automatically saves your game every 5 minutes. The :ref:`autosave period <autosave>` can be changed in the Misc settings tab, or in the :doc:`configuration file <../advanced_topics/configuration_file>`.

Location of saved game files
===============================

The save directory can be changed with the savepath setting in the :doc:`Paths tab <../settings/paths>`. It can also be changed in the :doc:`configuration file <../advanced_topics/configuration_file>`.

Default saved game paths are shown below. 

.. tabbed:: macOS

	.. panels::
		:column: col-lg-12 mb-2

		macOS/Mac OSX
		^^^^^^^^^^
		``~/Documents/ScummVM Savegames/``, where ``~`` is your Home directory. 
	
.. tabbed:: Windows

	.. panels::
		:column: col-lg-12 mb-2

		Windows NT4 
		^^^^^^^^^^^^

		``<windir>\Profiles\username\Application Data\ScummVM\Saved games\`` where ``<windir>`` refers to the Windows directory. Most commonly, this is ``C:\WINDOWS``.

		---
		:column: col-lg-12 mb-2

		Windows 2000/XP
		^^^^^^^^^^^^^^^^^
		``\Documents and Settings\username\Application Data\ScummVM\Saved games\``

		---
		:column: col-lg-12 mb-2

		Window Vista/7/8/10
		^^^^^^^^^^^^^^^^^^^^^^
		``%APPDATA%\ScummVM\Saved games``
 

.. tabbed:: Linux/Unix

	.. panels::
		:column: col-lg-12 mb-2

		
		ScummVM follows the XDG Base Directory Specification, so by default the saved games can be found at ``~/.local/share/scummvm/saves/``, but this location might vary depending on the value of the ``XDG_DATA_HOME`` environment variable.
		
		``.local`` is a hidden directory. To view it use ``ls -a`` on the command line.

		If ScummVM was installed using Snap, the saves can be found at ``~/snap/scummvm/current/.local/share/scummvm/saves/``

	

.. tabbed:: Other

	.. panels::
		:column: col-lg-12 mb-2

		See the relevant Platform page for details. 



Loading a game
===============

If you want to load your saved game without having to start the game first, select the **Load** button directly from the Launcher. To load a saved game while you're playing a game, press :kbd:`Ctrl+F5` (:kbd:`Ctrl+fn+F5` on a Mac) to access the Global Main Menu, then select **Load**.

There are two views: list view and tile view. Click the list and tile icons at the bottom left of the window to toggle between the two.  

In list view, the pane on the left shows a list of your saved games. Click on a saved game to highlight it.  In the pane on the right you can see timestamps, screenshots and playtime for that saved game. 

Once you have highlighted the game you want to play, click **Load**.

To delete a saved game, highlight the saved game and click **Delete**.

.. figure:: ../images/Launcher/load_game_list.png

	Loading a game, list view. 

In tile view, each tile shows a screenshot of the saved state, as well as a description and a save slot number. Click on the tile to load that saved game.

.. figure:: ../images/Launcher/load_game_tile.png
  
  Loading a game, tile view. 


.. _savelocation:

