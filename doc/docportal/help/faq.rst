
============================
Frequently Asked Questions
============================

.. contents::


1. General
==================

1.1 About
***********

1.1.1. Is ScummVM an emulator?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
No! ScummVM actually replaces the original executable file that shipped with the game. This means that your games can run on platforms they were never designed for! For an in-depth look at how ScummVM works, see the `About ScummVM <https://wiki.scummvm.org/index.php?title=About>`_ wiki page.

1.1.2. Is ScummVM free?
^^^^^^^^^^^^^^^^^^^^^^^^^
ScummVM is released under the GPL (General Public License), so it's more than just free. ScummVM source code is available for you to do whatever you want with it, but if you make modifications and redistribute your work, you must make your source code available.

The ScummVM team would be delighted if you send them your modifications, so that the changes you've made can be merged into the main source code. See the `Developer Central <https://wiki.scummvm.org/index.php/Developer_Central>`_ wiki page for contributing guidelines.

1.1.3. Can I create my own games using ScummVM?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
A few engines supported by ScummVM have publicly available authoring tools. See this `wiki page <https://wiki.scummvm.org/index.php/HOWTO-Fangames>`_ for more information.

1.1.4. I want to compile ScummVM myself from the source code. How do I do this?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See the `Compiling ScummVM wiki page <https://wiki.scummvm.org/index.php?title=Compiling_ScummVM>`_. 

1.2 Using ScummVM
**********************

1.2.1. How do I install ScummVM onto my device?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See :ref:`firststeps`.

1.2.2. How do I get game files onto my device?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For general guidance, see the :doc:`../use_scummvm/game_files` page. For platform-specific information, see the relevant page in the **OTHER PLATFORMS** section of the sidebar.

ScummVM has cloud and LAN functionality to simplify the file transfer process. For more information, see :doc:`../use_scummvm/connect_cloud` and :doc:`../use_scummvm/LAN`.

1.2.3. On what platform(s) will ScummVM run?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See our `Platforms <https://wiki.scummvm.org/index.php/Platforms>`_ wiki page for a full list. We have guides available for many of the supported platforms, see the relevant page in the **OTHER PLATFORMS** section of the sidebar.

1.2.4. Where do I find the configuration file?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See :doc:`../advanced_topics/configuration_file`.


1.3 Playing games
********************

1.3.1. Can I only play LucasArts SCUMM games?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Although the ScummVM project started by reverse-engineering just the LucasArts SCUMM games, the project now supports hundreds of games from many different game developers. See the full list of supported games `here <https://wiki.scummvm.org/index.php?title=Category:Supported_Games>`_. There is a caveat; not all supported games are playable on all platforms. Often this is because the game is simply too CPU intensive for the device, or because of some other hardware or software limitation. If a game is not available on a platform, you will not be able to add it to ScummVM.

.. _getgames:

1.3.2. Where do I get the games?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See the ScummVM `Where to get the games <https://wiki.scummvm.org/index.php/Where_to_get_the_games>`_ wiki page.


.. _installgames:

1.3.3. How do I install games from my floppy discs/CDs?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Generally speaking, you do not need to install the games. You only need to point ScummVM to the game files contained on these discs. For a complete guide, see :doc:`../use_scummvm/game_files`.

1.3.4. Do I need the original game discs?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Ideally yes, however we know that a lot of people don't! If you do not have any hard copy games, there are some digital options available, including some games that have been released as freeware. See the ScummVM `Where to get the games <https://wiki.scummvm.org/index.php/Where_to_get_the_games>`_ wiki page.

1.3.5. How do I get the Macintosh versions of games to work?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See :doc:`../use_scummvm/mac_game_files`

1.3.6. How do I know if I can play my game using ScummVM?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The best place to check is the `Compatibility page <https://www.scummvm.org/compatibility/>`_ on the ScummVM website. You can also have a look at the full list of supported games `here <https://wiki.scummvm.org/index.php?title=Category:Supported_Games>`_.

1.3.7. I have saved games from when I played the original game - can I use these with ScummVM?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
This is only supported for a select number of games. See the `wiki <https://wiki.scummvm.org/index.php/Category:Supported_Games>`_ page for the game you are playing.

1.3.8. Where are my games saved?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See :doc:`../use_scummvm/save_load_games`.

1.3.9. How do I launch a game straight from the game folder?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Games must be added to and started from :doc:`the ScummVM Launcher <../use_scummvm/the_launcher>`; they cannot be started directly from the game files. See :doc:`../use_scummvm/add_play_games`.

1.3.10. What are the in-game keyboard shortcuts?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
A list of default shortcuts can be found on the :doc:`../use_scummvm/keyboard_shortcuts` page. From version 2.2.0 you can also create custom shortcuts on the :doc:`../settings/keymaps` tab.


2. Troubleshooting
===================

2.1. General
**************

2.1.1. ScummVM crashes, but the console window disappears too quickly to see the error message.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you are using a computer, you can run ScummVM from the :doc:`command line <../advanced_topics/command_line>`. By doing this, error messages remain visible even after ScummVM exits.

You can also find error messages in the ScummVM log file. See :ref:`logfile`.

2.1.2. I get "Failed to save game state to file:  ". What's going on?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Check that your :ref:`save path <savepath>` is a writeable directory. 

2.1.3. I think I found a bug. What do I do?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See :doc:`report_bugs`.

2.2 Games
**********

2.2.1. Help! My game won't run!
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Make sure your game is supported. Check the `Compatibility page <https://www.scummvm.org/compatibility/>`_ on the ScummVM website, and the `Platform Overview <https://wiki.scummvm.org/index.php?title=Platforms/Overview>`_ page on the wiki.

2. Check that you have all the required datafiles. See the :doc:`../use_scummvm/game_files` page.

3. Ask for advice on the ScummVM forums or on Discord. See the :doc:`contact` page.

4. If you think the game should run, and it doesn't, report it as a bug. See :doc:`report_bugs`.

2.2.2. I installed my game but ScummVM can't find it... What do I do?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Installing the game does not necessarily provide ScummVM with the files it needs. In most cases you will need to copy the files from the disc into a folder ScummVM can access.  See :doc:`../use_scummvm/game_files`.

2.2.3. Why are the subtitles in my non-English game messed up?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
You need to specify the correct :ref:`language <lang>` in the game-specific settings.

2.2.4. My game crashes at a specific point. What do I do?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
First, check the `Compatibility page <https://www.scummvm.org/compatibility/>`_ on the ScummVM website to see if the game has any known issues. If not, and the crash can be reproduced, report the crash as a bug. See :doc:`report_bugs`.

2.2.5. I downloaded the Broken Sword packs from your website but the game won't run. 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

These are not full games, they are re-encoded cutscene (video) packs. To run the games you still need the original disks. See the `Broken Sword wiki page <https://wiki.scummvm.org/index.php/Broken_Sword_1#Cutscenes>`_.

2.3 Audio
***********

.. _nosound:

2.3.1 Help! There's no sound!
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Sometimes it's worth checking the obvious.

1. Are your speakers on? Are your headphones properly connected?
2. Try playing an audio clip from another source to see if you have sound in general.
3. If you narrow it down to an issue with ScummVM, check the :doc:`audio settings <../settings/audio>`. ScummVM falls back on an audio setting that works, but if for some reason it doesn't, you might need to change the settings yourself. A safe bet is usually to set the **Preferred device** to **<default>** and allow ScummVM to choose for you.

2.3.2 Help! There's no sound on my iPhone/iPad!
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
ScummVM will not play any sound if your device is in Silent Mode. If this is not the problem, see :ref:`nosound`.

2.3.3 I have a "talkie" version of a LucasArts game but I can't hear the voices. What's the problem?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The original games shipped with an uncompressed voice file (``MONSTER.SOU``). If you have compressed this file to an mp3 file (``MONSTER.SO3``), an Ogg Vorbis file (``MONSTER.SOG``), or a FLAC file (``MONSTER.SOF``), make sure that the ScummVM you're using has support for those formats.

2.3.4 The audio is really glitchy. What can I do?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
There are a few things you can try:

- Try to increase the :ref:`audio buffer size <buffer>` in the configuration file.
- If you are using the :ref:`MT-32 emulator <MT-32>`, your CPU might not have the processing power to keep up. In this case, you might have some success with running an external MT-32 emulator (Munt), as described in `this forum post <https://forums.scummvm.org/viewtopic.php?f=2&t=15251>`_, provided your platform supports it.
- If you are using :ref:`FluidSynth <FS>`, in particular with a large Soundfont, your CPU might not have the processing power to keep up. Try selecting a different :ref:`Preferred device <device>`.
- If you are using the :ref:`AdLib <adlib>` emulator, try selecting the least CPU-intensive option; MAME.

2.3.5. I have a CD version of a game, how do I get the sound to work without running the game from the CD?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See :ref:`cdaudio`.

2.4 Graphics
**************

2.4.1. There are so many options... How do I know what to pick?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Start by checking out our :doc:`../advanced_topics/understand_graphics` page. It has comprehensive information on how all this stuff works.

2.4.2. Can I just make the image larger (for example, 1 pixel becomes 4 pixels) without any smoothing or antialiasing?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Yes. Using the **OpenGL** graphics mode or the **SDL Surface** graphics mode with **Normal** scaler in conjunction with pixel-perfect stretch will result in a larger image without any smoothing. The **Normal** scaler also has options to scale by **2x**, **3x**, or **4x**. Also check that **Filter graphics** is not enabled. If you want to use aspect ratio correction, it is recommended to use the **OpenGL** graphics mode with the **Even pixels scaling** stretch mode.


2.4.3. The game colors are messed up, how do I fix them?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Ensure the correct game platform has been detected. For example, with Amiga game files, check that the :ref:`platform <platform>` is set to Amiga.

3. Features
================

3.1 Requests
*****************

3.1.1. When will you add support for ZIP archives?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
We won't. There are two main reasons: firstly, we believe that it would ease illegal distribution of games, and secondly, we already support compression of sound and speech to reduce file sizes. 

3.1.2. Will ScummVM support other games in the future?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
That depends on a few factors. Firstly, it has to fit within the scope of ScummVM. Secondly, there has to be a developer who is interested and willing to carry out the work. 

ScummVM developers are all volunteers who work on ScummVM in their spare time, solely for fun, and not for profit. Reverse engineering a completely new game without the source code is a long and difficult process. Even with source it can be tedious and time consuming. 

Unless you work for a company interested in providing us with source code for one of their classic titles, or want to do the work yourself, please do not ask us to add support for a new game. 