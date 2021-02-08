
===========================
Handling game files
===========================

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



For games on floppies or CDs, if the data files are accessible, the external media can be accessed directly by ScummVM. Alternatively, copy the game files to a folder as described above. If the data files are not accessible, run the installer on the platform the game was designed for, to extract the game files. 

For games downloaded from digital distributors or from various `freeware sources <https://wiki.scummvm.org/index.php?title=Where_to_get_the_games#Freeware_Games>`__ run the installer to extract the data files, or if the data files are supplied as a zip file, extract the files.

For macOS or Linux users without access to a Windows machine, you can get the data files from games that only have Windows installers. Use `Wine <https://www.winehq.org/>`_ to run the Windows installer. Alternatively, for GOG.com games, use `innoextract <https://constexpr.org/innoextract/>`_ to unpack game files from the installer without actually running it.


.. tip::

   Use ScummVM's Cloud functionality to share game files and automatically back up and sync your saved game states. For detailed instructions see the :doc:`../use_scummvm/connect_cloud` guide. For devices on the same local network, ScummVM can also run a web server to allow hassle-free transfer of files. For detailed instructions, see the :doc:`../use_scummvm/LAN` guide. This is a good option if you do not wish to connect a Cloud service.


Multi-disc games
===================

ScummVM needs access to all the data files, so it will not be able to run directly from a CD if the game has multiple discs. To add a multi-CD game, copy the required data files from the CD to a folder on your hard drive. For a comprehensive list of required files, see the `ScummVM Wiki Datafiles <https://wiki.scummvm.org/index.php?title=Datafiles>`__ page.

As a general guideline, where there are duplicate, identical files (same name, same file size) on more than one disc, only one copy of the file is required. Where there are files that have the same name but are different on each disc, rename them. For example, where there is a ``music.clu`` on two discs, rename the file on the first disc to ``music1.clu`` and the file on the second disc to ``music2.clu``. 

.. _cdaudio:

CD audio
============

In most cases, ScummVM can use CD audio directly from the game CD. If you don't always want to insert the game CD to use the CD audio, you can extract the audio tracks from the CD and save them locally: 

1. Extract the CD audio tracks in WAV or AIFF format. 
2. Convert the tracks to either MP3, FLAC or OGG file formats. 
3. Place the converted audio files in the same folder that contains the other game datafiles. 

.. tip::

    `fre:ac <https://www.freac.org/>`_ is a multi-platform, open-source software, with the ability to both rip CDs and convert audio between MP3, OGG, FLAC and WAV  file formats. 


.. _macgames:

Macintosh games
==================

All LucasArts SCUMM games also exist in versions for the Macintosh. Most of the newer games shipped with only a single data file; ScummVM can use this file like it does any other data file. Some discs shipped with hidden files. To view hidden files in macOS, press :kbd:`Cmd+Shift+.` in a Finder window.  

Mac uses a different file system to other systems (HFS+), so to access the CD or floppy disc on a Windows or Linux system you need additional software to view and copy the files. 

For Windows, `HFS Explorer <http://www.catacombae.org/hfsexplorer/>`_  is a basic and free option, which gives you read-only access to HFS drives. Use the installer rather than the zip file, to ensure it is installed correctly. 

For Linux, hfsplus gives you read-only access to HFS drives. To use hfsplus, use the command line: 

   1. Install hfsplus using the software manager. On Debian-based distributions, use ``sudo apt install hfsplus``.
   2. Find the game disc by running ``sudo fdisk -l`` and finding the one with type ``Apple HFS/HFS+``. In this example, this is ``/dev/fd0``.
   3. Create a mount point, for example: ``sudo mkdir /media/macgamedrive``
   4. Mount the device to that moint point: ``sudo mount -t hfsplus /dev/fd0 /media/macgamedrive``
   5. Access the device at ``/media/macgamedrive``.


