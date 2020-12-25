
=================================
Adding and playing a game
=================================

The games
------------

One of the most frequently asked questions is, "Where do I get the games?!?". If you still have your old floppy discs or CDs lying around, and have a way to read them, then you can use the game files from those original discs. 

If you haven't had a floppy disc or even a CD in your possession for a while, there is a ScummVM `wiki page <https://wiki.scummvm.org/index.php?title=Where_to_get_the_games>`_ dedicated to helping you find some games to play. 

.. tip::

   ScummVM has an affiliate referrer programme with GOG.com, so if you are going to buy a game, please use this `GOG.com <https://www.gog.com/?pp=22d200f8670dbdb3e253a90eee5098477c95c23d">`_ link to help the project!

Make sure to check out the `compatibility list <https://www.scummvm.org/compatibility/>`_ on the ScummVM website, and the `Supported Games page <https://wiki.scummvm.org/index.php?title=Category:Supported_Games>`_ and `Platform Overview page <https://wiki.scummvm.org/index.php/Platforms/Overview>`_ on the ScummVM wiki before you buy a game, to ensure it can run with ScummVM. 

.. _add and play games:

Adding games to the Launcher
---------------------------------

The following steps explain how to add a game to ScummVM, and assume you have access to the data files for each game. If you're not sure how to access the game data files, see the :doc:`game_files` page. 


1. Run ScummVM to open :doc:`the Launcher <../use_scummvm/the_launcher>`.

2. In the Launcher window, click **Add Game**. This opens a file browser; either the system file browser or the ScummVM file browser. 

.. image:: ../images/Launcher/add_game.png

To add multiple games at once, click the expander arrow next to **Add Game**, and select **Mass Add**. 

.. image:: ../images/Launcher/mass_add.png

3.  Use the file browser to locate the folder containing the game data files. For the ScummVM file browser: Double click on a folder to open it, and use **Go up** to go back up one level. Click **Choose** to select the folder. 

.. image:: ../images/Launcher/choose_game_directory.png
   :class: with-shadow

.. note::

   The **Use Native file browser**  option in the :doc:`Misc <../settings/misc>` tab of the global settings controls whether ScummVM uses the system file browser, or its own file browser. 

3a. If you are using **Mass Add**, select **Yes** to run the mass game detector. A dialog opens to advise which games were added to the games list.

.. image:: ../images/Launcher/mass_add_confirm.png

.. image:: ../images/Launcher/mass_add_success.png

4.  The **Edit Game** dialog opens to allow configuration of various settings for the game. These can be reconfigured at any time, but for now everything should be OK at the default settings. Any settings changed here override the global settings. Click **Ok**. 

.. image:: ../images/Launcher/game_settings.png
   :class: with-shadow

.. note::
   
   The **Edit Game** dialog does not open when **Mass Add** is used. 

5. Games are now ready to play! To play, highlight a game in the games list and click **Start**, or double-click the game in the games list.

.. image:: ../images/Launcher/start_game.png
   :class: with-shadow

Games can also be launched directly from the command line. For more information, see the `Command line options <../advanced_topics/command_line>`_ page.

A note about copyright 
------------------------

ScummVM has a strict anti-piracy stance and the team wil not tolerate discussions around pirated games in any part of the project, including on the Forum or on Discord. 

Copy protection screen
************************

There are cases where the game companies themselves bundled 'cracked' executables with their games. In these cases the data files still contain the copy protection scripts, but the interpreter bypasses them; similar to what an illegally cracked version might do, only that here the producer of the game did it. There is no way for ScummVM to tell the difference between legitimate and pirated data files, so for the games where a cracked version of the original interpreter was sold at some point, ScummVM has to bypass the copy protection.

In some cases ScummVM still shows the copy protection screen. Enter any answer; chances are that it will work.
