
===========================
Handling game files
===========================

ScummVM needs access to the data files for each game. A list of required data files for each game is found on the `wiki entry <https://wiki.scummvm.org/index.php?title=Category:Supported_Games>`__ for that game. ScummVM can run the game data files from any directory it has access to, including external media.

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

ScummVM needs access to all the data files, so it will not be able to run directly from a CD if the game has multiple discs. To add a multi-CD game, copy the required data files from the CD to a folder on your hard drive. For a list of required files, see the `wiki entry <https://wiki.scummvm.org/index.php?title=Category:Supported_Games>`__ for the game you're playing.

As a general guideline, where there are duplicate, identical files (same name, same file size) on more than one disc, only one copy of the file is required. Where there are files that have the same name but are different on each disc, rename them. For example, where there is a ``music.clu`` on two discs, rename the file on the first disc to ``music1.clu`` and the file on the second disc to ``music2.clu``.

.. _cdaudio:

CD audio
============

In most cases, ScummVM can use CD audio directly from the game CD. If you don't always want to insert the game CD to use the CD audio, you can extract the audio tracks from the CD and save them locally:

1. Extract the CD audio tracks in WAV or AIFF format.
2. Convert the tracks to either MP3, FLAC, M4A or OGG file formats.
3. Place the converted audio files in the same folder that contains the other game datafiles.
4. You can name them in several ways:

   1. track01.ogg - track12.ogg
   2. track1.ogg - track12.ogg
   3. track_01.ogg - track_12.ogg
   4. track_1.ogg - track_12.ogg

.. tip::

    `fre:ac <https://www.freac.org/>`_ is a multi-platform, open-source software, with the ability to both rip CDs and convert audio between MP3, OGG, FLAC and WAV  file formats.


.. _macgames:

Macintosh games
==================
Many of the supported games also exist in versions for the Macintosh, however it can be tricky to extract the correct files from a Macintosh floppy or CD, especially if you are using Windows or Linux. See :doc:`../use_scummvm/mac_game_files` for an in-depth how-to guide.

