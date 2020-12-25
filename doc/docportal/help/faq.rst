
============================
Frequently Asked Questions
============================

.. contents::


General
==================

About
*******

Is ScummVM an emulator?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
No. ScummVM replaces the original executable files that shipped with the game. This means that your games can run on many different platforms they were never designed to run on! For an in-depth look at how ScummVM works, see the `About ScummVM <https://wiki.scummvm.org/index.php?title=About>`_ wiki page. 

Is ScummVM free? 
^^^^^^^^^^^^^^^^^^^^
ScummVM is released under the GPL (General Public License), so it's more than just free. ScummVM source code is freely available for you to do whatever you want with it. If you make modifications and redistribute your work, you must make your source code available. 

The ScummVM team would be delighted if you send them your modifications, so that the changes can be merged into the main source code. See the `Developer Central <https://wiki.scummvm.org/index.php/Developer_Central>`_ wiki page. 

How do I install ScummVM onto my device?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See :ref:`firststeps`.

How do I get game files onto my device?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For general guidance, see the :doc:`../use_scummvm/add_play_games` page. For specific information, find the page for your device on the sidebar. 

ScummVM now has cloud and LAN functionality to simplify the file transfer process. For more information, see :doc:`../use_scummvm/connect_cloud` and :doc:`../use_scummvm/LAN`.

On what platform(s) will ScummVM run?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See our `Platforms list <https://wiki.scummvm.org/index.php/Platforms>`_ wiki page. 

Games
********

Can I only play LucasArts SCUMM games?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Although the ScummVM project started by reverse-engineering just the LucasArts SCUMM games, the project now supports hundreds of games from many different game developers. See the `Supported games <https://wiki.scummvm.org/index.php?title=Category:Supported_Games>`_ page on our wiki for a comprehensive list. There is a caveat; not all supported games are playable on all platforms. Sometimes this is because the game is simply too CPU intensive for the device, or there might be many other reasons. If a game is not available on a platform, you will not be able to add it to ScummVM. 

.. _getgames:

Where do I get the games? 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See the :doc:`../use_scummvm/add_play_games` page.


.. _installgames:

How do I install games from my floppy discs/CDs?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Generally speaking, you do not need to install the games. You only need to point ScummVM to the game files contained on these discs. See :doc:`../use_scummvm/add_play_games`. 

Do I need the original game discs?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
If you do not have any hard copy games, there are some digital options available, including some games that have been released as freeware. See :ref:`getgames`.

How do I know if I can play my game using ScummVM?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The best place to check is the `Compatibility page <https://www.scummvm.org/compatibility/>`_ on the ScummVM website.

I have saved games from when I played the original game - can I use these with ScummVM?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
This is only supported for a select number of games. See the `wiki <https://wiki.scummvm.org/index.php/Category:Supported_Games>`_ page for the game you are playing. 

How do I launch a game straight from the game folder?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Games must be added to and started from :doc:`../use_scummvm/the_launcher`, they cannot be started directly from the game files. See :doc:`../use_scummvm/add_play_games`. 

What are the in-game keyboard shortcuts?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
A list of default shortcuts can be found on the :doc:`../use_scummvm/keyboard_shortcuts` page. Since version 2.2.0 you can create custom shortcuts on the :doc:`../settings/keymaps` tab. 

Troubleshooting
===================

ScummVM
***********

ScummVM crashes, but the console window disappears too quickly to see the error message. 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you are using a computer, you can run ScummVM from the :doc:`command line <../advanced_topics/command_line>`. By doing this, error messages remain visible even after ScummVM exits. 

You can also find error messages in the ScummVM log file. See :ref:`logfile`. 

I think I found a bug. What do I do?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See :doc:`report_bugs`. 

Games
********

Help! My game won't run! 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Make sure your game is supported. Check the `Compatibility page <https://www.scummvm.org/compatibility/>`_ on the ScummVM website, and the `Platform Overview <https://wiki.scummvm.org/index.php?title=Platforms/Overview>`_ page on the wiki. 

2. Check that you have all the required datafiles. See the :doc:`../use_scummvm/add_play_games` page for more information.

3. Ask for advice on the ScummVM forums or on DIscord. See the :doc:`contact` page. 

4. If you think the game should run, and it doesn't, report it as a bug. See :doc:`report_bugs`. 

I installed my game but ScummVM can't find it... What do I do?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Installing the game does not necessarily provide ScummVM with the files it needs. In most cases you will need to copy the files from the disc into a folder ScummVM can access.  See :ref:`installgames`. 

Why are the subtitles in my non-English game messed up?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
You need to specify the correct :ref:`language <lang>` in the game-specific settings.  

Audio
*******

.. _nosound:

Help! There's no sound!
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Sometimes it's worth checking the obvious. 

1. Are your speakers on? Are your headphones properly connected? 
2. Try playing an audio clip from another source to see if you have sound in general. 
3. If you narrow it down to an issue with ScummVM, check the :doc:`audio settings <../settings/audio>`. ScummVM falls back on an audio setting that works, but if for some reason it doesn't, you might need to change the settings yourself. A safe bet is usually to set the **Preferred device** to **<default>** and allow ScummVM to choose for you. 

Help! There's no sound on my iPhone/iPad!
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
ScummVM will not play any sound if your device is in Silent Mode. If this is not the problem, see  :ref:`nosound`.

I have a "talkie" version of a LucasArts game but I can't hear the voices. What's the problem?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The original games shipped with an uncompressed ``MONSTER.SOU`` file, which is the voice file. If you have compressed this file to either a ``MONSTER.SO3`` (MP3 file), ``MONSTER.SOG`` (OGG file), or ``MONSTER.SOF`` (FLAC file) format, make sure that the ScummVM you're using has support for those formats. 

The audio is really glitchy. What can I do?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
There might be a few things at play here. 

- Try to increase the :ref:`audio buffer size <buffer>` in the configuration file. 
- If you are using the :ref:`MT-32 emulator <MT-32>`, your CPU might not have the processing power to keep up. In this case, you might have some success with running an external MT-32 emulator (Munt), as described in `this forum post <https://forums.scummvm.org/viewtopic.php?f=2&t=15251>`_, as long as your platform supports it. 
- If you are using :ref:`FluidSynth <FS>`, in particular with a large Soundfont, your CPU might not have the processing power to keep up. Try selecting a different :ref:`Preferred device <device>`.
- If you are using the :ref:`adlib` emulator, try selecting the least CPU-intensive option; MAME. 

I have a CD version of a game, how do I get the sound to work without running the game from the CD?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See :ref:`cdaudio`.

Graphics
***********

There are so many options... How do I know what to pick?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Start by checking out our :doc:`../advanced_topics/understand_graphics` page. It has comprehensive information on how it all works. 

Can I just make the image larger (for example, 1 pixel becomes 4 pixels) without any smoothing or antialiasing?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Yes. Using the **Normal (no scaling)** graphics mode in conjunction with pixel-perfect scaling will result in a larger image without any smoothing. The **2x** and **3x** graphics modes also do not involve any filtering.  Also check that **Filter graphics** is not enabled. 
 

The game colors are messed up, how do I fix them?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Ensure the correct game platform has been detected. For example, with Amiga game files, check that the :ref:`platform <platform>` is set to Amiga. 