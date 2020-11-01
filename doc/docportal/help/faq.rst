
============================
Frequently Asked Questions
============================

General
==================

About
*******

Q: Is ScummVM an emulator?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** No. ScummVM replaces the original executable files that shipped with the game. This means that your games can run on many different platforms they were never designed to run on! For an in-depth look at how ScummVM works, see our `About ScummVM <https://wiki.scummvm.org/index.php?title=About>`_ wiki page. 

Q: Is ScummVM free? 
^^^^^^^^^^^^^^^^^^^^
**A:** ScummVM is released under the GPL (General Public License), so it's more than just free. ScummVM source code is freely available and you can do whatever you want with it. If you make modifications and redistribute your work, you must make the source code available. 

The ScummVM team would be delighted if you send them your modifications, so that the changes can be merged into the main source code. See :doc:`../contributing/code`.

How do I install ScummVM onto my [device]?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** See :ref:`firststeps`.

On what platform(s) will ScummVM run?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** See our `Platforms list <https://wiki.scummvm.org/index.php/Platforms>`_ wiki page. 

Games
********

Q: Can I only play LucasArts SCUMM games?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** Although the ScummVM project started by reverse-engineering just the LucasArts SCUMM games, the project now supports hundreds of games from many different game developers. See the `Supported games <https://wiki.scummvm.org/index.php?title=Category:Supported_Games>`_ page on our wiki for a comprehensive list. There is a caveat; not all supported games are playable on all platforms. Sometimes this is because the game is simply too CPU intensive for the device, or there may be many other reasons. If a game is not available on a platform, you will not be able to add it to ScummVM. 

.. _getgames:

Q: Where do I get the games? 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** See `Where to get the games <https://wiki.scummvm.org/index.php?title=Where_to_get_the_games>`_ on our wiki. 

.. _installgames:

Q: How do I install games from my floppy discs/CDs?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** Generally speaking, you do not need to install the games. You only need to point ScummVM to the game files contained on these discs. See :doc:`../using_scummvm/adding_games`. 

Q: Do I need the original game discs?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** If you do not have any hard copy games, there are some digital options, including some games that have been released as freeware. See :ref:`getgames`.

Q: How do I know if I can play my game using ScummVM?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The best place to check is the `Compatibility page <https://www.scummvm.org/compatibility/>`_ on our website.

Q: I have saved games from when I played the original game - can I use these with ScummVM?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** This is only supported for a select number of games. See :ref:`originalsaves`.

Q: How do I launch a game from the game folder?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** Games must be added to and started from :doc:`../using_scummvm/the_launcher`, they cannot be started directly from the game files. See :doc:`../using_scummvm/adding_games`. 


Troubleshooting
===================

ScummVM
***********

Q: ScummVM crashes, but the console window disappears too quickly to see the error message. 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**A:** If you are using a computer, you can run ScummVM from the :doc:`command line <../advanced_options/command_line>`. Error messages remain visible even after ScummVM exits. 

You can also find error messages in the ScummVM log file. See :ref:`logfile`. 

Q: I think I found a bug. What do I do?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** See :doc:`reporting_bugs`. 

Games
********

Q: Help! My game won't run! 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A: First, make sure your game is supported. See the `Compatibility page <https://www.scummvm.org/compatibility/>`_ on our website. 

Check that you have all the required datafiles. See :doc:`../using_scummvm/adding_games`.

Ask on the forums or on Discord whether your game is supported on the platform you're trying to play it on. See :doc:`contact`.

If you think the game should run, and it doesn't, report it as a bug. See :doc:`reporting_bugs`. 

Q: I installed my game but ScummVM can't find it... What do I do?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Installing the game does not necessarily provide ScummVM with the files it needs. See :ref:`installgames`. 

Q: Why are the subtitles in my non-English game messed up?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** You need to specify the correct :ref:`language <lang>` in the game-specific settings.  

Audio
*******

.. _nosound:

Q: Help! There's no sound!
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** Sometimes it's worth checking the obvious. Are your speakers on? Are your headphones properly connected? Try playing an audio clip from another source to see if you have sound in general. If you narrow it down to an issue with ScummVM, check the :doc:`audio settings <../settings/audio>`. ScummVM should fall back on an audio setting that works, but if it doesn't, you may need to do so yourself. A safe bet is usually to set the **Preferred device** to **<default>** and allow ScummVM to choose for you. 

Q: Help! There's no sound on my iPhone/iPad!
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** ScummVM will not play any sound if your device is in Silent Mode. If this is not the problem, see  :ref:`nosound`.

Q: I have a "talkie" version of a LucasArts game but I can't hear the voices. What's the problem?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** The original games shipped with an uncompressed ``MONSTER.SOU`` file - this is the voice file. If you have compressed this file to either a ``MONSTER.SO3`` (``.mp3``), ``MONSTER.SOG`` (``.ogg``), or ``MONSTER.SOF`` (``.flac``) format, make sure that the ScummVM you're using has support for those formats. 

Q: The audio is really glitchy. What can I do?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** There may be a few things at play here:

- Try to increase the :ref:`audio buffer size <buffer>` in the :doc:`configuration file <../advanced_options/configuration_file>`. 
- If you are using the :ref:`MT-32 emulator <MT-32>`, your CPU may not have the processing power to keep up. In this case, you may have some success with running an external MT-32 emulator (Munt), as described in `this forum post <https://forums.scummvm.org/viewtopic.php?f=2&t=15251>`_, as long as your platform supports it. 
- If you are using :ref:`FluidSynth <FS>`, in particular with a large Soundfont, your CPU may not have the processing power to keep up. Try selecting a different :ref:`Preferred device <device>`.
- If you are using the :ref:`adlib` emulator, try selecting the least CPU-intensive option; MAME. 

Q: I have a CD version of a game, how do I get the sound to work without running the game from the CD?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** See using :ref:`cd`.

Graphics
***********

Q: There are so many options... How do I know what to pick?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** Start by checking out our :doc:`../settings/graphics` settings page. It has comprehensive information on how it all works. 

Q: Can I just make the image larger (for example, 1 pixel becomes 4 pixels) without any smoothing or antialiasing?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** Yes. Using the **Normal (no scaling)** graphics mode in conjunction with pixel-perfect scaling will result in a larger image without any smoothing. The **2x** and **3x** graphics modes also do not involve any filtering.  

Ensure **Filter graphics** is not enabled. 

See the :doc:`../settings/graphics` settings for a more detailed explanation. 

Q: The game colors are messed up, how do I fix them?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**A:** Ensure the correct game has been detected. For example, with Amiga game files, check that the :ref:`platform <platform>` is set to Amiga. 