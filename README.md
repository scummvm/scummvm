ResidualVM: A 3D game interpreter
=================================


  1. [ What is ResidualVM?         ](#1-what-is-residualvm)
  2. [ Current state               ](#2-current-state)
  3. [ Running GrimE games         ](#3-running-grime-games)
  4. [ Running Myst III            ](#4-running-myst-iii)
  5. [ Running The Longest Journey ](#5-running-the-longest-journey)
  6. [ Configuration               ](#6-configuration)
  7. [ Troubleshooting             ](#7-troubleshooting-known-bugs-issues)
  8. [ Debugging                   ](#8-debugging)
  9. [ Bug Reports                 ](#9-bug-reports)
 10. [ Contact                     ](#10-contact)


## 1. What is ResidualVM?
ResidualVM is a game engine reimplementation that allows you
to play 3D adventure games such as Grim Fandango, Escape from Monkey
Island, Myst III and The Longest Journey.

ResidualVM utilizes OpenGL for 3D graphics hardware acceleration.
A software renderer is also included for machines without hardware OpenGL.


## 2. Current state
At this point ResidualVM is fit for normal use, when playing supported
games, it is however worth noting that you should still save early, and
save often, as problems or dead-ends might still exist. (Grim Fandango
itself originally had a few unintentional ways to get the game stuck).

### 2.1. Which games does ResidualVM support? ###

Currently ResidualVM supports Grim Fandango and Escape From Monkey Island,
as well Myst III and The Longest Journey.

#### 2.1.1. GrimE-games support ####

Game                             | Status
-------------------------------- | -------------------------
Grim Fandango                    | Completable with glitches
Grim Fandango (demo)             | Completable with glitches
Escape From Monkey Island        | Completable with glitches
Escape From Monkey Island (demo) | Completable with glitches
Escape From Monkey Island (PS2)  | Untested

#### 2.1.2. Other games support ####

Game                             | Status
-------------------------------- | -------------------------
Myst III Exile                   | Completable
The Longest Journey              | Untested
The Longest Journey (demo)       | Untested

Specifics can be found at
http://www.residualvm.org/compatibility/

For more information, see the page on ResidualVM at the wiki page:
http://wiki.residualvm.org/


## 3. Running GrimE games

### 3.1. Required files

For both Grim Fandango and Escape from Monkey Island, you will need the original
game files as well as the official update patch.

#### 3.1.1. Grim Fandango ####

You will need to copy the data files from your Grim Fandango CDs into one
directory. Specifically, you'll need:
  * All of the .LAB files from both CDs
  * A copy of the Grim Fandango 1.01 update EXE
        The patch can be downloaded from:
        http://demos.residualvm.org/patches/gfupd101.exe

#### 3.1.2. Escape from Monkey Island ####

You will need to copy the data files from your Escape from Monkey Island CDs
into one directory. Specifically, you'll need:
  * All of the .M4B files from both CDs.
    One of the files is easy to miss:
    local.m4b is located on CD1 in Monkey4/MonkeyInstall.
        Note: The file voiceAll.m4b is repeated on both CDs. Use the
              copy from the first CD, it contains all of the required
              voice data
  * The Textures directory, combined from both CDs. When copying, rename
    the FullMonkeyMap.imt files to FullMonkeyMap1.imt and
    FullMonkeyMap2.imt from CDs 1 and 2 respectively.
  * The Movies directory from each CD
  * A copy of the Escape from Monkey Monkey Island update EXE
    You will need a patch specific to the EMI version you're using:
  * "EFMI Installer" if you have the Mac version of EMI.

Language   | URL
---------- |---------------------------------------------------------
English    | http://demos.residualvm.org/patches/MonkeyUpdate.exe
Portuguese | http://demos.residualvm.org/patches/MonkeyUpdate_BRZ.exe
German     | http://demos.residualvm.org/patches/MonkeyUpdate_DEU.exe
Spanish    | http://demos.residualvm.org/patches/MonkeyUpdate_ESP.exe
French     | http://demos.residualvm.org/patches/MonkeyUpdate_FRA.exe
Italian    | http://demos.residualvm.org/patches/MonkeyUpdate_ITA.exe

#### 3.1.3. Escape from Monkey Island (PS2) ####

You will need to copy the data files from your Escape from Monkey Island DVD
into one directory. Specifically, you'll need:
  * All of the m4b files from the DVD
  * The Videos, demos, jambalay, lucre, melee and monkey directores

### 3.2. Running the game ###

1. Prepare the game directory as specified above
2. Open ResidualVM
3. Choose "Add Game"
4. Select the folder you created in Step 1
5. Click Start

If you want to play with software-rendering, see the configuration-section
below, or use the ingame settings to disable 3D Acceleration.

When launching the game for the first time, ResidualVM will perform a full
integrity check of your game data files. This can take a bit of time, but it
will not happen again on successive runs. This prevents bugs from incompatible
game data, and helps us to find game variants that we're not aware of.

### 3.3. Default Keyboard-settings ###

Key         | Binding
----------- | ---------------------------------------------
e,u,p,i     | Examine, Use, Pickup, Inventory
Arrow keys  | Movement
Shift       | Hold to run
Enter       | Selects items in inventory, conversation, etc
Escape      | Skips cutscenes, exits certain screens
.           | Skips dialogue
q           | Exit Dialog Menu
Ctrl + c    | Force Quit (from command-line)
Alt + x     | Quit (ingame)
Alt + enter | Switch between windowed-mode and fullscreen
F1          | Menu

### 3.4. Joystick/gamepad support ###

If you want to use a joystick or gamepad for navigation, the joystick support
of the engine needs to be enabled using one of the following two options:

  * start residualvm with "--joystick" parameter
  * add "joystick\_num=0" to the "[residualvm]" section of the configuration file
    (see section 5.1. how to find the file)


## 4. Running Myst III

### 4.1. Required files ###

You will need these elements from your game CDs or DVD :
  * M3Data directory
  * Data directory
  * The menu language file '[LANGUAGE].m3u' (DVD only)

When using the CD version, you will need to merge the 'Data' directories from
all four discs. Playing directly from the CDs is not supported.

The game must be at least version 1.1. For most releases of the game, the
update is already applied on the installation media, no action is required.
Otherwise, ResidualVM asks for the update to be installed and refuses to run
the game. The updates can be downloaded from
http://demos.residualvm.org/patches/.

The DVD version is multilingual, you can change the in-game language from the
game menu. However, you must choose the language of the menus by copying the
appropriate files. You have to copy the menu language file from your chosen
language folder on the disc. On the DVD, the menu language file can be named
'language.m3u' or '[LANGUAGE].m3u' depending on the release. It should
be copied to the 'M3Data/TEXT' folder. If the file is named 'language.m3u',
it should be renamed to the explicit language e.g. ENGLISH.m3u for English.

The required files must be organized in the following manner to be recognized:

    ├── Data
    │   └── *.m3a
    └── M3Data
        └── Various files and folders (including the DVD version's menu language file)

### 4.2. Running the game ###

1. Copy the necessary files to a folder on your Hard Drive
2. Open ResidualVM
3. Choose "Add Game"
4. Select the folder you created in step 1
5. Click Start

### 4.3. Input controls ###

The mouse is used to look around and interact with the ages.

Available keyboard shortcuts:

Key        | Binding
---------- | ------------------------------
Escape     | Original Myst III Menu
Ctrl + F5  | ResidualVM Menu
Space      | Skip cutscenes, interact
Ctrl + c   | Force Quit (from command-line)
Ctrl + q   | Quit (ingame)


## 5. Running The Longest Journey

### 5.1. Required files ###

You will need to copy the data files from your The Longest Journey CDs,
DVD or digital distributiion into one directory. Specifically, you'll
need:
  * 1a—79 directories (4f only for demo version)
  * global directory
  * static directory
  * fonts directory (not critical, but recommended – see below)
  * x.xarc and all the .ini files

Steam version and demo from Steam are missing the fonts folder. The
required fonts can be copied over from demo version obtained from
different sources or a GOG or retail version.

The 2-CD and DVD versions have some of the data files packed in installer
archives. The archives need to be unpacked before they can be used.

### 5.2. Running the game ###

1. Copy the necessary files to a folder on your Hard Drive
2. Open ResidualVM
3. Choose "Add Game"
4. Select the folder you created in step 1
5. Click Start

### 5.3. Input controls ###

The mouse is used to interact with objects and menu elements.

Available keyboard shortcuts:

Key         | Binding
----------- | -------------------------------------------------------------------------------------------
Escape      | Skip video sequence or current line of dialogue, skip time if *Time Skip* option is enabled
Ctrl + F5   | ResidualVM Menu
Ctrl + c    | Force Quit (from command-line)
Ctrl + q    | Quit (ingame)
Alt + enter | Switch between windowed-mode and fullscreen


## 6. Configuration
Currently, not all the settings for ResidualVM are available through the GUI,
if you have problems with getting anything to work, first try to pass the
settings from the command line, then, if that doesn't work, try to change your
configuration file manually.

### 6.1. Location of configuration file ###

By default, the configuration file is saved in, and loaded from:

Operating System     | Location
-------------------- | ---------------------------------------------------------------------------
Windows Vista/7/8/10 | \Users\username\AppData\Roaming\ResidualVM\residualvm.ini
Windows 2000/XP      | \Documents and Settings\username\Application Data\ResidualVM\residualvm.ini
Unix                 | ~/.residualvmrc
Mac OS X             | ~/Library/Preferences/ResidualVM Preferences
Others               | residualvm.ini in the current directory

### 6.2. Interesting settings for GrimE games ###

The following settings are currently available in the config-file,
however some of them might not work with your current build. And
some of them might make ResidualVM crash, or behave in weird ways.

Setting        | Values            | Effect
-------------- | ----------------- | ---------------------------------------------------
show_fps       | [true/false]      | If true, then ResidualVM will show the current FPS-rate, while you play.
last_set       | [set-name]        | The set you were last on, ResidualVM will try to continue from there.
last_save      | [save-number]     | The save you last saved, ResidualVM will have that selected the next time you try to load a game.
use_arb_shaders| [true/false]      | If true, and if you are using the OpenGL renderer ResidualVM will use ARB shaders. While fast they may be incompatible with some graphics drivers.
fullscreen_res | [desktop/WWWxHHH] | If set to "desktop" (the default), ResidualVM will use your desktop resolution in fullscreen mode. If set to a resolution such as "640x480" or "1280x720", that resolution will be used.


## 7. Troubleshooting, Known Bugs, Issues
Grim Fandango had a few issues when it came out, and a few new and exciting
issues when you try to run it on newer hardware. Some of these have been
fixed in ResidualVM, but ResidualVM itself also has a few new issues that we
know about:
http://github.com/residualvm/residualvm/blob/master/KNOWN_BUGS

Look below for help with these issues, and if you can't find help here, try
either the forums at our homepage, or IRC: #residualvm at freenode.

### 7.1. I played a bit, but can't start a new game! ###

This is because the last save and visited scene is stored in your configuration
file, either delete grim-fandango from the ResidualVM-menu, and re-add it, or
go to your configuration file, and clean out the last-save and last-set entries.

### 7.2. My Save Games don't work any more ###

Did you recently update to a newer build of ResidualVM?
ResidualVM is still a work in progress, which means that the save format might
change between builds. While attempts are made to keep save file compatibility,
this isn't always possible.

### 7.3. In fullscreen mode, the picture is stretched! ###

This is know issue, in future versions it will be resolved.


## 8. Debugging
WARNING: This section contains information about the various tools that
are included for debugging ResidualVM, this should not be necessary for
normal play at all! However, the curious might like to know how to access
these tool. Please use at your own risk!

### 8.1. Debugging GrimE Games ###

The development console can be used to debug both Grim Fandango and Escape From
Monkey Island. To enter the debug console, press Ctrl + d. Use the `help`
command to display a list of the available commands.

Some of the useful commands are:

Command       | Description
------------- | -----------------------------------------------------------
jump          | Jump to a section of the game
lua_do        | Execute a lua command
openlog       | Show the log of errors/warnings/information from the engine
set_renderer  | Select a renderer (software, OpenGL or OpenGL with shaders)

The `jump` targets can be found at:
  * http://wiki.residualvm.org/index.php/Grim_Fandango_Debug_Mode#jump_targets
  * http://wiki.residualvm.org/index.php/Escape_From_Monkey_Island_Debug_Mode

### 8.2. Debugging Grim Fandango ###

Grim Fandango also includes a built in debug mode. To enable debug-keys and
debug-mode, you will have to add the following line to your configuration file
under the Grim Fandango-entry:

game_devel_mode=true

Development/debug keys from the original game:

Key        | Binding
---------- | ------------------------------------
Ctrl + e   | Enter lua string to execute
Ctrl + g   | Jump to set
Ctrl + i   | Toggle walk boxes
Ctrl + l   | Toggle lighting
Ctrl + n   | Display background name
Ctrl + o   | Create a door
Ctrl + p   | Execute patch file
Ctrl + s   | Turn on cursor
Ctrl + u   | Create a new object
Ctrl + v   | Print the value of a variable
Alt + n    | Next viewpoint
Apt + p    | Prev viewpoint
Alt + s    | Run lua script
Shift + n  | Next set
Shift + p  | Prev set
Shift + o  | Toggle object names
F3         | Toggle sector editor
Home       | Go to default position in current set
j          | Enter jump number

Note that these are only available after enabling debug-mode.

### 8.3. Debugging Myst III ###

The debug console can be triggered using Ctrl + d. Use the `help` command to
display a list of the available commands.

The most useful commands are:

Command     | Description
----------- | ------------------------------------------------------------
dumpArchive | Extract a game archive
infos       | Display the name of a node and show the associated scripts
go          | Jump to a node
var         | Display or alter the value of a variable from the game state

### 8.4. Debugging The Longest Journey ###

The debug console can be triggered using Ctrl + d. Use the `help` command to
display a list of the available commands.

The most useful commands are:

Command             | Description
------------------- | ------------------------------------------------------------
changeChapter       | Change the current chapter
changeKnowledge     | Change the value of some knowledge
changeLocation      | Change the current location
chapter             | Display the current chapter
decompileScript     | Decompile a script
dumpArchive         | Extract a game archive
dumpGlobal          | Print the global level's resource sub-tree to stdout
dumpKnowledge       | Print the current knowledge to stdout
dumpLevel           | Print the current level's resource sub-tree to stdout
dumpLocation        | Print the current location's resource sub-tree to stdout
dumpRoot            | Print the resource tree root to stdout
dumpStatic          | Print the static level's resource sub-tree to stdout
enableInventoryItem | Enable a specific inventory item
enableScript        | Enable or disable script
forceScript         | Force the execution of a script
listInventory       | List all inventory items in the game
listLocations       | List all the locations in the game
listScripts         | List all the scripts in current level
location            | Display the current location
openlog             | Show the log of errors/warnings/information from the engine
testDecompiler      | Test decompilation of all the scripts in game


## 9. Bug Reports
ResidualVM still has a few bugs, many might already have been reported,
but should you find a new one, don't hesitate to report it.

### 9.1. How, and where do I report bugs? ###

You can report bugs at our github-issue-tracker:
http://www.github.com/residualvm/residualvm/issues

Please read the Wiki regarding how to report bugs properly first though:
http://wiki.residualvm.org/index.php?title=Reporting_Bugs

Remember to always have the following information in your bug reports:
  * Information about the game (Escape from Monkey Island, PS2 version)
  * Language of game (English, German, ...)
  * Platform and Compiler (Win32, Linux, Mac OS X, ...)
  * Bug details, including instructions on reproducing it
  * Preferably also a link to a save game right before the bug happened.


## 10. Contact
  * Homepage: http://www.residualvm.org/
  * Wiki: http://wiki.residualvm.org/
  * Forums: http://forums.residualvm.org/
  * IRC: #residualvm on freenode
