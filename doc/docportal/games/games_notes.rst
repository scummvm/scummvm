
==========================
Games notes
==========================

Blade Runner 
-------------------
Supported versions are English (both CD and DVD), French, German, Italian, Russian (by Fargus Multimedia) and Spanish. Other unofficial versions exist but they are based on the English version.

All ``*.mix``, ``*.tlk`` and ``*.dat`` files are needed from all 4 CDs/DVD. Files named ``CDFRAMES.DAT`` need to be renamed to ``CDFRAMES1.DAT``, ``CDFRAMES2.DAT``, ``CDFRAMES3.DAT`` and ``CDFRAMES4.DAT`` according on the CD they came from. Other files with the same name on different CDs are identical and it doesn't matter which one is used.

To enable subtitles, download a file from our Downloads page and place file ``SUBTITLES.MIX`` in the same folder as the other game files.

"Blade Runner with restored content" is not yet supported and might contain game breaking bugs.

Broken Sword 
----------------
The instructions for the Broken Sword games are for the Sold-Out Software versions, with each game on two CDs, since these were the versions most easily available at the time ScummVM gained support for them. Hopefully they are general enough to be useful to other releases as well.

Broken Sword
***************

For this game, you will need all of the files from the clusters directories on both CDs. For the Windows and Macintosh versions, you will also need the speech.clu files from the speech directories, but since they are not identical you will need to rename them ``speech1.clu`` and ``speech2.clu`` for CD 1 and 2 respectively. The PlayStation version requires the ``speech.tab``, ``speech.dat``, ``speech.lis``, and ``speech.inf``.

In addition, the Windows and Macintosh versions require a music subdirectory with all of the files from the music subdirectories on both CDs. Some of these files appear on both CDs, but in these cases they are either identical or, in one case, so nearly identical that it makes little difference. The PlayStation version requires ``tunes.dat`` and ``tunes.tab``.

Broken Sword II
******************
For this game, you will need all of the files from the clusters directories on both CDs. Rename the ``speech.clu`` and ``music.clu`` files ``speech1.clu``, ``speech2.clu``, ``music1.clu`` and ``music2.clu`` so that ScummVM can tell which ones are from CD 1 and which ones are from CD 2. Any other files that appear in both cluster directories are identical and eithe can be used. 

In addition, you will need the ``cd.inf`` and, optionally, the ``startup.inf`` files from the sword2 directory on CD 1.

Broken Sword games cutscenes
**********************************

The cutscenes for the Broken Sword games have a bit of a history, but in general all you need to do is to copy the ``.SMK`` files from the ``SMACKS`` or ``SMACKSHI`` directories on the CDs to the same directory as the other game data files. Broken Sword has a ``SMACKSLO`` directory with the same cutscenes, but these are of lower quality. You can put them in a subdirectory called "video" if you find that neater.

For the PlayStation versions, you can dump the original videos off the disc. For each of the files ending in an ``.STR`` extension, you should dump them as raw sectors off the disc (all 2352 bytes per sector). You may also use the re-encoded cutscenes mentioned below instead, but this will not work for all videos in Broken Sword II. For more information, see the `wiki page <https://wiki.scummvm.org/index.php/HOWTO-PlayStation_Videos>`_.

Some re-releases of the games, as well as the PlayStation version, do not have Smacker videos. Revolution Software has kindly allowed us to provide re-encoded cutscenes for `download <https://www.scummvm.org/downloads/>`_ on our website.

These cutscenes are provided in DXA format with FLAC audio. Their quality is equal to the original games due to the use of lossless compression. Viewing these cutscenes requires a version of ScummVM compiled with both FLAC and zlib support.

For systems that are too slow to handle the decoding of FLAC audio, the audio for these cutscenes is also provided separately as OGG Vorbis audio. Viewing these cutscenes with OGG Vorbis audio requires a version of ScummVM compiled with both libVorbis and zlib support.

For Broken Sword, we also provide a subtitles add-on. Simply unpack it and follow the instructions in its ``readme.txt`` file. The subtitle pack currently does not work when running PlayStation videos. Broken Sword II already has subtitles; no extra work is needed for them.

Broken Sword games cutscenes, in retrospect
********************************************

The original releases of the Broken Sword games used RAD Game Tools's Smacker(tm) format. As RAD was unwilling to open the older legacy versions of this format to us, and had requested we not reverse engineer it, an alternative solution had to be found.

In Broken Sword II, it was possible to play back the voice-over without playing the video itself. This remained a fallback until ScummVM 1.0.0, but was never the only solution for any stable release.

In ScummVM 0.6.0 we used MPEG, which provided a reasonable trade-off between size and quality. In ScummVM 0.10.0 this was superseded by DXA (originally added for AdventureSoft's "The Feeble Files"). This gave us a way of providing the cutscenes in the exact same quality as the originals, at the cost of being larger.

Finally, in early 2006, the Smacker format was reverse engineered for the FFmpeg project. Thanks to their hard work, ScummVM 1.0.0 and later support the original cutscenes. At the same time, MPEG support was dropped. From a technical standpoint, this was a good thing since decoding MPEG movies added a lot of complexity, and they didn't look as good as the Smacker and DXA versions anyway.

Day of the Tentacle
----------------------

At one point in the game, you come across a computer that allows you to play the original Maniac Mansion as an easter egg. 

ScummVM will scan your configuration file for a game that's in a Maniac sub-folder of your Day of the Tentacle folder. If you've copied the data files from the CD version, this should already be the case but you have to add the game to ScummVM as well.

To return to Day of the Tentacle, press F5 and select **Return to Launcher**.

In theory any game can be used as the easter egg. Indeed, there is a "secret" configuration setting, easter_egg, to override the ID of the game to run. Be aware, though, that not all games support returning to the launcher, and setting it up to use Day of the Tentacle itself as the easter egg game is not recommended.

Dragon History
----------------------

There are 4 language variants of the game: Czech, English, Polish and German. Each of them is distributed in a separate archive. The only official version is the Czech one, and the English, Polish and German ports have always been a work in progress and never officially released. Although all texts are fully translated, it is known that some of them contain typos.

There exists an optional Czech dubbing for the game. For bandwidth reasons, you can download it separately and then unpack it to the directory of the game. You can listen to the Czech dubbing with all language variants of the game, while reading the subtitles.

All game files and the walkthrough can be `downloaded <http://www.ucw.cz/draci-historie/index-en.html>`_.

Flight of the Amazon Queen 
-----------------------------

Only the original non-freeware version of Flight of the Amazon Queen (from original CD), requires the ``queen.tbl`` datafile available from the ScummVM `Downloads page <https://www.scummvm.org/downloads/#extras>`_  in either the directory containing the ``queen.1`` game data file, in your extrapath, or in the directory where your ScummVM executable resides.

Alternatively, you can use the compress_queen tool from the :doc:`tools package <../advanced_options/tools>` to rebuild the FOTAQ data file to include the table for that specific version, and thus removing the run-time dependency on the ``queen.tbl`` file. This tool also allows you to compress the speech and sound effects with MP3, OGG or FLAC.

Gobliiins 
---------------

The CD versions of the Gobliiins series contain one big audio track which you need to rip and copy into the game directory if you want to have in-game music without the CD in the drive all the time. See the :ref:`audio page <cd>` for more information.

The speech is also in that track and its volume is therefore changed with the music volume control as well.

Inherit the Earth: Quest for the Orb 
---------------------------------------

In order to run the Mac OSX Wyrmkeep re-release of the game you will need to copy over data from the CD to your hard disk. If you're on a PC or Linux machine, see the :ref:`adding games <macgames>` section about accessing games from Mac discs or drives.  

The speech data "Inherit the Earth Voices" needs to be copied into the same directory as the game data, which is stored in ``Inherit the Earth.app/Contents/Resources``.

For the old Mac OS 9 release you need to copy the files in MacBinary format, as they should include both resource and data forks. Copy all ``ITE.*`` files.

Mickey's Space Adventure 
--------------------------

To run Mickey's Space Adventure under ScummVM, the original executable of the game (mickey.exe) is needed together with the game's data files.

There is extensive mouse support for the game under ScummVM, even though there wasn't any mouse support in the original game. Menu items can be selected using the mouse, and it is possible to move to other locations using the mouse as well. When the mouse cursor is hovered on the edges of the screen, it changes color to red if it is possible to walk towards that direction. The player can then simply click on the edges of the game's screen to change location, similar to many adventure games, which is simpler and more straightforward than moving around using the menu.

Might and Magic Xeen games notes
------------------------------------

To properly play the World of Xeen CD Talkie using original discs, use LAME or some other encoder to rip the cd audio tracks to files, either mp3 or ogg. Whichever you choose, the tracks of the first CD should be named from ``track02`` to ``track31``, whereas the second CD's audio tracks should be encoded and renamed as ``track32`` through to ``track60``.

For the GOG Might and Magic 4-5 installation, install the game to your computer, then complete the following steps:

    - The ``game1.inst`` (CUE) and ``game1.gog`` (BIN) file from the game folder is a CD image. Use software like Virtual CloneDrive to mount it as a drive. Linux and MacOS users can use bchunk to convert it to an ISO.
    - Copy all the ``.cc`` files from the subfolder in the mounted drive to a new empty game folder that you create for the game.
    - Copy all the music (``*.ogg``) files from the GOG installation to your game folder. You'll then need to rename all of them from ``xeen??.ogg`` to ``track??.ogg``
    - Point ScummVM to this new game folder, and the CD talkie version should be detected.

Savegames from either Clouds or Darkside of Xeen games can be transferred across to World of Xeen, which combines both games, by setting up and detecting World of Xeen. This can be done either by manually combining the two games or using the GOG World of Xeen installer, and then renaming the savegames to use the World of Xeen savegame format, which is by default ``worldofxeen.*``

The Xeen engine also offers :ref:`two custom options <XEEN>` in the Engine tab for the games in the ScummVM launcher. They are:

    - To change the threshold armor breaks at for characters from -10HP to -80HP 
    - To show values for inventory items, even outside of the blacksmith, allowing the relative strength/value of armor and weapons to be compared.

Myst 
------

Myst will autosave to slot 0 if no save or an autosave is present in slot 0.

Quest for Glory 
-----------------------

It is possible to import characters, beginning with Quest for Glory II, from past games to future games and continue from the stats earned from those games.

For example, a character can be imported from Quest for Glory I directly to Quest for Glory III without having to play Quest for Glory II.

Characters cannot be imported from future games to past games, nor can a character be imported to the same game that was just completed. In other words, a character from Quest for Glory II cannot be imported into Quest for Glory II.

If you want to use a saved character from the original Sierra interpreter, you will need to rename the character file to ``qfg[game-number]-[character-filename].sav``, for example ``qfg2-thief.sav``, and place it in the ScummVM :ref:`save path <savepath>`,  otherwise the file won't get listed on the import screen.


Riven 
------------

Riven will autosave to slot 0 if no save or an autosave is present in slot 0.

Simon the Sorcerer
------------------------

If you have the dual version of Simon the Sorcerer 1 or 2 on CD, you will find the Windows version in the main directory of the CD and the DOS version in the DOS directory of the CD.

Starship Titanic 
----------------------------

For the purposes of solving the starfield puzzle, only mouse clicks, L and Tab are really needed, though the action glyph in the PET can be used instead of Tab.

The Curse of Monkey Island 
-----------------------------------------
For this game, you will need the ``comi.la0``, ``comi.la1`` and ``comi.la2 files``. The ``comi.la0`` file can be found on either CD, but since they are identical it doesn't matter which one of them you use.

In addition, you will need to create a ``resource`` subdirectory containing all of the files from both ``resource`` subdirectories on the two CDs. Some of the files appear on both CDs.

The Feeble Files
--------------------

Amiga/Macintosh
 You need to install a small pack of cutscenes that are missing in both of these versions of The Feeble Files. It's called "The Feeble Files - Omni TV and epilogue cutscenes for the Amiga and Macintosh versions". They are available from `here <https://www.scummvm.org/games/#feeble>`_ on the ScummVM website. 


Windows
    If you have the Windows version of The Feeble Files, there are several things to note.

    Many of the files necessary for the game are stored in an InstallShield file called ``data1.cab``, which ScummVM is unable to unpack. You will need to use the original installer or i5comp to unpack the contents of this file. The i5comp decompression tool, can be found via a search on the internet.

To use the speech files with ScummVM, they need to be renamed. Rename ``voices.wav`` on CD1 to ``voices1.wav``,``voices.wav`` on CD2 to ``voices2.wav``, and so on for all 4 CDs. 


The Legend of Kyrandia
--------------------------

To run The Legend of Kyrandia under ScummVM you need the ``kyra.dat`` file. The file should already be included in official ScummVM packages. In case ScummVM complains that the file is missing you can find it on the `Downloads page <https://www.scummvm.org/downloads/#extras>`_ of the ScummVM website. Note that the current Windows release of ScummVM should contain the file embedded into the executable, thus you only need to grab it in case ScummVM complains about the file being missing.

Troll's Tale
------------------

The original game came in a PC booter disk, therefore it is necessary to dump the contents of that disk in an image file and name it ``troll.img`` to be able to play the game under ScummVM.

Winnie the Pooh
----------------------

It is possible to import saved games from the original interpreter of the game into ScummVM.

There is extensive mouse support for the game under ScummVM, even though there wasn't any mouse support in the original game. Menu items can be selected using the mouse, and it is possible to move to other locations using the mouse as well. When the mouse cursor is hovered on the edges of the screen, it changes color to red if it is possible to walk towards that direction. The player can then simply click on the edges of the game's screen to change location, similar to many adventure games, which is simpler and more straightforward than moving around using the menu.

Sierra AGI games
-------------------

Predictive Input Dialog
**************************

The Predictive Input Dialog is a ScummVM aid for running AGI engine games, which notoriously require command line input, on devices with limited keyboard support. In these situations, since typing with emulated keyboards is quite tedious, commands can be entered quickly and easily via the Predictive Input Dialog.

In order to enable predictive input in AGI games, you need to copy the ``pred.dic`` file into the ScummVM :ref:`extras directory <extrapath>` or the directory of the game you wish to play. This dictionary has been created by parsing through all known AGI games and contains the maximum set of common words.

If the dictionary is detected, the Predictive Input Dialog is displayed either when you click on the command line area, or in some ports by pressing a designated hot key.

The predictive input dialog operates in three modes, switchable by the (*)Pre/123/Abc button. 

- The primary input method is the predictive mode (Pre) which resembles the way "fast typing" is performed on older non-smartphones. 

    - The alphabet is divided into 9 sets which naturally map to the 9 number keys of the numeric keypad (0 is space). To type in a word, you press once the number of the set which contains the letter of the word you intend to type, then move on to the next. 
    - For example, to type the command **look**, you press 5665. As you gradually type the intended word's numeric code, the dictionary is accessed for known words matching your input up to that point. As you press more keys, the prediction converges to the correct word. This is why the printed word may change dramatically between key presses. 
    - There exist situations where more than one word share the same numeric representation. For example, the words "quit" and "suit" both map to the same number, namely 7848. In these cases the (#) next button lights up. By pressing it, you can cycle through the list of words sharing the same code and finally accept the correct one by pressing (0)space or the Ok button.

- The second input method (123) is the numeric input: Each key you press is entered verbatim as a number.

- The third input method (Abc) is the Multi-tap Alpha input mode. 

    - This mode is intended for entering free text, without assistance from the dictionary scheme of predictive (Pre) mode. 
    - The text is entered one letter at the time. For each letter first press the number of the set which contains the letter you want, then use the (#)next button to cycle through the letters and repeat with another number. 
    - For example, to enter the word look you must press the following: 5##6##6##5#

The dialog is fully usable with the mouse, but a few provisions have been made in some ScummVM ports to make its use more comfortable by naturally mapping the functionality to the numeric keypad. Also, the dialog's buttons can be navigated with the arrow and the enter keys.

Sierra SCI games
------------------

Simultaneous speech and subtitles
***********************************

Certain CD versions of Sierra SCI games had both speech and text resources. Some have an option to toggle between the two, but there are some cases where there wasn't any option to enable both simultaneously. In ScummVM, it is possible to enjoy a combined mode, where both speech and text are shown at the same time. This mode can be toggled in the ScummVM audio options, but each game has different behavior in-game regarding speech and text toggling.

- EcoQuest 1 CD: 
    - Speech and text can be toggled via the in-game **Mode** option in the options dialog, or via the ScummVM global or game settings.

- Freddy Pharkas CD: .
    - There is no in-game option to toggle speech and text. Only ScummVM's audio options can be used to toggle this feature. Note that some spoken dialog is missing from the game texts.

- Gabriel Knight CD: 
    - Speech and text can be toggled via the **Text** and **Voice** buttons in the game's settings dialog, or via the ScummVM global or game settings.

- King's Quest 6 CD: 
    - Speech and text can be toggled via the **Mode** button in the options dialog (with an extra "Dual" setting added in ScummVM), or via the ScummVM global or game settings.

- King's Quest VII CD: 
    - There is no in-game option to toggle speech and text. Only ScummVM's global or game settings can be used to toggle this feature. Note that the subtitles were disabled in the official release of this game, so some subtitles may be incorrect or missing.

- Laura Bow 2 CD: 
    - Speech and text can be toggled via the **Mode** button in the options dialog, with an extra "Dual" setting added in ScummVM, or via the ScummVM global or game settings.

- Leisure Suit Larry 6 CD: 
    - Either speech only or speech and text can be selected. There is no in-game option to toggle text only. Only the ScummVM global or game settings can be used to enable the text only mode.

- Leisure Suit Larry 6 (hires) CD: 
    - Text can be toggled by selecting the **Text On/Off** option from the in-game **Game** menu, or via the ScummVM global or game settings. Speech cannot be disabled.

- Police Quest 4 CD: 
    - Either speech only or text only can be selected from the game's settings dialog. Only the ScummVM global or game settings can be used to enable text+speech mode.

- Shivers CD: 
    - Text can be toggled by selecting the **Text** option from the game's settings dialog, or via the ScummVM global or game settings. Note that only videos have subtitles in this game.

- Space Quest 4 CD: 
    - Speech and text can be toggled via the **Display Mode** button in the options dialog, or via the ScummVM global or game settings.

- Space Quest 6 CD: 
    - Speech and text can be toggled via the **Speech** and **Text** buttons in the game's settings dialog, or via the ScummVM global or game settings.

- Torin's Passage CD: 
    - Text can be toggled by selecting **Closed Captioning** from the in-game **Game** menu. Speech can be disabled by selecting **Audio Mixer** from the in-game **Game** menu and setting the speech volume to zero.

Zork games notes
-----------------

To run the supported Zork games (Zork Nemesis: The Forbidden Lands and Zork: Grand Inquisitor) you need to copy some (extra) data to its corresponding destination.


Zork: Grand Inquisitor
***************************

Download the `Liberation(tm) fonts package <https://releases.pagure.org/liberation-fonts/liberation-fonts-ttf-2.00.1.tar.gz>`_ and unpack all the ``.ttf`` files into your ScummVM :ref:`extras directory<extrapath>`. Alternatively, ScummVM comes with the GNU FreeFont fonts, though at the time of writing these fonts cause some text rendering issues. 

Zork Nemesis: The Forbidden Lands
*********************************
As for Grand Inquisitor, but also Download the `subtitles patch <https://www.thezorklibrary.com/installguides/znpatch.zip>`_ and unzip the addon directory into the game directory.
