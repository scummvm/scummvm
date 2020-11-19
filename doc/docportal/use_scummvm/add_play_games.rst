
=================================
Add and play games
=================================

The games
----------------

One of the most frequently asked questions is, "Where do I get the games?!?". If you still have your old floppy discs or CDs lying around, and have a way to read them, then you can use the game files from those original discs. 

If you haven't had a floppy disc or even a CD in your possession for a while, there is a ScummVM `wiki page <https://wiki.scummvm.org/index.php?title=Where_to_get_the_games>`_ dedicated to helping you find some games to play. As a quick summary, there are a few freeware games available for download on the ScummVM website's `Downloads page <https://www.scummvm.org/downloads/>`_, or there are digital distributors such as `GOG.com <https://www.gog.com/>`__ and `Steam <https://store.steampowered.com/>`_. ScummVM has an affiliate referrer programme with GOG.com, so if you are going to buy a game, please use the links on the `wiki page <https://wiki.scummvm.org/index.php?title=Where_to_get_the_games>`_ to help the ScummVM project! 

Make sure to check out the `compatibility list <https://www.scummvm.org/compatibility/>`_ on our website, and the `Supported Games page <https://wiki.scummvm.org/index.php?title=Category:Supported_Games>`_ and `Platform Overview page <https://wiki.scummvm.org/index.php/Platforms/Overview>`_ on the ScummVM wiki before you buy a game, to ensure it can run with ScummVM. 


A note about copyright 
------------------------

The ScummVM team does not condone piracy, however there are cases where the game companies themselves bundled 'cracked' executables with their games. In these cases the data files still contain the copy protection scripts, but the interpreter bypasses them; similar to what an illegally cracked version might do, only that here the producer of the game did it. There is no way for ScummVM to tell the difference between legitimate and pirated data files, so for the games where a cracked version of the original interpreter was sold at some point, ScummVM has to bypass the copy protection.

In some cases ScummVM still shows the copy protection screen. Enter any answer; chances are that it will work.


The game files
------------------------

ScummVM needs access to the data files for each game. A comprehensive list of required data files for each game is found on the `ScummVM Wiki Datafiles <https://wiki.scummvm.org/index.php?title=Datafiles>`__ page. ScummVM can run the game data files from any directory it has access to, including external media. 

.. note::

    For ease of use, you can create a dedicated games folder into which folder containing game files can be copied. The resulting directory structure will look somewhat like this::

        ScummVM Games
        |-- Day of the Tentacle
        |   |-- MONSTER.SOU
        |   |-- TENTACLE.000
        |   `-- TENTACLE.001
        `-- Flight of the Amazon Queen
            |-- QUEEN.1
            `-- queen.tbl


   The exact layout of files and folders within the game folder is not important, as long as all the files are there.

For games on floppies or CDs:

- If the data files are accessible, the external media can be accessed directly by ScummVM. Alternatively, copy the game files to a folder as described above. 
- If the data files are not accessible, run the installer on the platform the game was designed for, to extract the game files. 

For games downloaded from digital distributors or from various `freeware sources <https://wiki.scummvm.org/index.php?title=Where_to_get_the_games#Freeware_Games>`__:

- Run the installer to extract the game files, or
- If the data files are supplied as a zip file, extract the files.

For macOS or Linux users without access to a Windows machine, you can get the game files from games that only have Windows installers. Use `Wine <https://www.winehq.org/>`_ to run the Windows installer on macOS or Linux. Alternatively, for GOG.com games, use `innoextract <https://constexpr.org/innoextract/>`_ to unpack game files from the installer without actually running it.



.. tip::

   Use ScummVM's Cloud functionality to share game files and automatically back up and sync your saved game states. For detailed instructions see the :doc:`../use_scummvm/connect_cloud` guide. For devices on the same local network, ScummVM can run a web server to allow hassle-free transfer of files. For detailed instructions, see the :doc:`../use_scummvm/LAN` guide. This is a good option if you do not wish to connect a Cloud service.


Multi-disc games
*****************

ScummVM needs access to all the data files, so it will not be able to run directly from a CD if the game has multiple discs. To add a multi-CD game, copy the required data files from the CD to a folder on your hard drive. 

- Where there are duplicate, identical files (same name, same file size) on more than one disc, only one copy of the file is required. 
- Where there are files that have the same name but are different on each disc, rename them. For example, where there is a ``music.clu`` on two discs, rename the file on the first disc to ``music1.clu`` and the file on the second disc to ``music2.clu``. 

.. _cdaudio:

CD audio
**********

Some games contain one or more separate audio tracks on CD. To use these with ScummVM, extract the audio tracks from the CD in WAV or AIFF format, and then convert them to either MP3, FLAC or OGG file formats. 

.. tip::

    Software suggestion:

    `fre:ac <https://www.freac.org/>`_ is a multi-platform, open-source software, with the ability to both rip CDs and convert audio between MP3, OGG, FLAC and WAV  file formats. 

Place the converted audio files in the same folder that contains the other game datafiles. 


.. _macgames:

Macintosh games
******************

All LucasArts SCUMM games also exist in versions for the Macintosh. Most of the newer games shipped with only a single data file; ScummVM can use this file like it does any other data file. Some discs shipped with hidden files. To view hidden files in macOS, press :kbd:`Cmd+Shift+.` in a Finder window.  

Mac uses a different file system to other systems (HFS+), so to access the CD or floppy disc on a Windows or Linux system you will need additional software to view and copy the files. 

For Windows, `HFS Explorer <http://www.catacombae.org/hfsexplorer/>`_  is a basic and free option, which gives you read-only access to HFS drives. Use the installer rather than the zip file, to ensure it is installed correctly. 

For Linux, hfsplus gives you read-only access to HFS drives. To use hfsplus, use the command line: 

   1. Install hfsplus using the software manager. On Debian-based distributions, use ``sudo apt install hfsplus``.
   2. Find the game disc by running ``sudo fdisk -l`` and finding the one with type ``Apple HFS/HFS+``. In this example, this is ``/dev/fd0``.
   3. Create a mount point, for example: ``sudo mkdir /media/macgamedrive``
   4. Mount the device to that moint point: ``sudo mount -t hfsplus /dev/fd0 /media/macgamedrive``
   5. Access the device at ``/media/macgamedrive``.

.. _add and play games:

Add games to the Launcher
---------------------------------

1. Run ScummVM to open :doc:`the Launcher <../use_scummvm/the_launcher>`.

2. In the Launcher window, click **Add Game**. This opens a file browser; either the system file browser or the ScummVM file browser. To add multiple games at once, click the expander arrow next to **Add Game**, and select **Mass Add**.

.. image:: ../images/Launcher/add_game.png
   :class: with-shadow


3.  Use the file browser to locate the folder containing the game data files. For the ScummVM file browser: Double click on a folder to open it, and use **Go up** to go back up one level. Click **Choose** to select the folder. 

.. image:: ../images/Launcher/choose_game_directory.png
   :class: with-shadow

.. note::

   The **Use Native file browser**  option in the :doc:`Misc <../settings/misc>` tab of the global settings controls whether ScummVM uses the system file browser, or its own file browser. 

5.  The **Edit Game** dialog opens to allow configuration of various settings for the game. These can be reconfigured at any time, but for now everything is OK at the default settings. Click **Ok**. 

.. image:: ../images/Launcher/game_settings.png
   :class: with-shadow

6. Games are now ready to play! To play, highlight a game in the games list and click **Start**.

.. image:: ../images/Launcher/start_game.png
   :class: with-shadow

Games can also be launched directly from the command line. For more information, see the `Command line options <../advanced_topics/command_line>`_ page.


