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
  9. [ Bug reports                 ](#9-bug-reports)
 10. [ Changelog                   ](#10-changelog)
 11. [ Credits                     ](#11-credits)
 12. [ Contact                     ](#12-contact)


## 1. What is ResidualVM?
ResidualVM is a game engine reimplementation that allows you to play 3D
adventure games such as Grim Fandango, Escape from Monkey Island, Myst III
and The Longest Journey.

ResidualVM utilizes OpenGL for 3D graphics hardware acceleration.
A software renderer is also included for machines without hardware OpenGL.


## 2. Current state
At this point ResidualVM is fit for normal use, when playing supported
games, it is however worth noting that you should still save early and
save often, as problems or dead ends might still exist (Grim Fandango
itself originally had a few unintentional ways to get the game stuck).

### 2.1. Which games does ResidualVM support? ###

Currently ResidualVM supports Grim Fandango and Escape from Monkey Island,
as well as Myst III and The Longest Journey.

#### 2.1.1. GrimE games support ####

Game                             | Status
-------------------------------- | -------------------------
Grim Fandango                    | Completable with glitches
Grim Fandango (demo)             | Completable with glitches
Escape from Monkey Island        | Completable with glitches
Escape from Monkey Island (demo) | Completable with glitches
Escape from Monkey Island (PS2)  | Untested

#### 2.1.2. Other games support ####

Game                             | Status
-------------------------------- | -------------------------
Myst III Exile                   | Completable
The Longest Journey              | Untested
The Longest Journey (demo)       | Untested

Specifics can be found at
https://www.residualvm.org/compatibility/

For more information, see the page on ResidualVM at the wiki page:
https://wiki.residualvm.org/


## 3. Running GrimE games

### 3.1. Required files

For both Grim Fandango and Escape from Monkey Island, you will need the
original game files as well as the official update patch.

#### 3.1.1. Grim Fandango ####

You will need to copy the data files from your Grim Fandango CDs into one
directory. Specifically, you'll need:
  * All of the `LAB` files from both CDs.
  * A copy of the Grim Fandango 1.01 update EXE.
        The patch can be downloaded from:
        https://demos.residualvm.org/patches/gfupd101.exe

#### 3.1.2. Escape from Monkey Island ####

You will need to copy the data files from your Escape from Monkey Island
CDs into one directory. Specifically, you'll need:
  * All of the `M4B` files from both CDs.
    One of the files is easy to miss:
    `local.m4b` is located on CD1 in `Monkey4/MonkeyInstall`.
        Note: The file `voiceAll.m4b` is repeated on both CDs. Use the
              copy from the first CD, it contains all of the required
              voice data.
  * The `Textures` directory, combined from both CDs. When copying,
    rename the `FullMonkeyMap.imt` files to `FullMonkeyMap1.imt` and
    `FullMonkeyMap2.imt` from CDs 1 and 2 respectively.
  * The `Movies` directory from each CD.
  * A copy of the Escape from Monkey Monkey Island update EXE.
    You will need a patch specific to the EMI version you're using:

Language   | URL
---------- |---------------------------------------------------------
English    | https://demos.residualvm.org/patches/MonkeyUpdate.exe
Portuguese | https://demos.residualvm.org/patches/MonkeyUpdate_BRZ.exe
German     | https://demos.residualvm.org/patches/MonkeyUpdate_DEU.exe
Spanish    | https://demos.residualvm.org/patches/MonkeyUpdate_ESP.exe
French     | https://demos.residualvm.org/patches/MonkeyUpdate_FRA.exe
Italian    | https://demos.residualvm.org/patches/MonkeyUpdate_ITA.exe

  * "EFMI Installer" if you have the Mac version of EMI.

#### 3.1.3. Escape from Monkey Island (PS2) ####

You will need to copy the data files from your Escape from Monkey Island
DVD into one directory. Specifically, you'll need:
  * All of the `M4B` files from the DVD.
  * The `Videos`, `demos`, `jambalay`, `lucre`, `melee` and `monkey`
    directores.

### 3.2. Running the game ###

1. Prepare the game directory as specified above.
2. Open ResidualVM.
3. Click `Add Game...`.
4. Select the directory you created in step 1.
5. Click `Start`.

If you want to play with software rendering, see the configuration section
below, or use the in-game settings to disable 3D acceleration.

When launching the game for the first time, ResidualVM will perform a full
integrity check of your game data files. This can take a bit of time, but
it will not happen again on successive runs. This prevents bugs from
incompatible game data and helps us to find game variants that we're not
aware of.

### 3.3. Default keyboard settings ###

Key             | Binding
--------------- | ---------------------------------------------
Arrow keys      | Movement
`Shift`         | Hold to run
`Enter`         | Selects items in inventory, conversation, etc
`Escape`        | Skips cutscenes, exits certain screens
`e`             | Examine
`u`             | Use
`p`             | Pickup
`i`             | Inventory
`q`             | Exit Dialog Menu
`.`             | Skips dialogue
`F1`            | Menu
`Alt` + `x`     | Quit (in-game)
`Ctrl` + `c`    | Force quit (from command line)
`Alt` + `Enter` | Switch between windowed mode and fullscreen
`Alt` + `s`     | Save a screenshot

### 3.4. Joystick/gamepad support ###

If you want to use a joystick or gamepad for navigation, the joystick
support of the engine needs to be enabled using one of the following two
options:
  * start ResidualVM with `--joystick` parameter,
  * add `joystick\_num=0` to the `[residualvm]` section of the
    configuration file (see section 6.1. how to find it).


## 4. Running Myst III

### 4.1. Required files ###

You will need to copy the data files from your Myst III CDs or DVD into
one directory. Specifically, you'll need:
  * The `M3Data` directory.
  * The `Data` directory.
  * The menu language file `[LANGUAGE].m3u` (DVD only).

The game must be at least version 1.1. For most releases of the game, the
update is already applied on the installation media and no action is
required.
Otherwise, ResidualVM asks for the update to be installed and refuses to
run the game. The updates can be downloaded from
https://demos.residualvm.org/patches/

The DVD version is multilingual, you can change the in-game language from
the game menu. However, you must choose the language of the menus by
copying the appropriate files. You have to copy the menu language file
from your chosen language directory on the disc. On the DVD, the menu
language file can be named `language.m3u` or `[LANGUAGE].m3u` depending on
the release. It should be copied to the `M3Data/TEXT` directory. If the
file is named `language.m3u`, it should be renamed to the explicit
language e.g. `ENGLISH.m3u` for English.

The required files must be organized in the following manner to be
recognized:

    ├── Data
    │   └── *.m3a
    └── M3Data
        └── Various files and directories (including the DVD version's menu language file)

### 4.2. Running the game ###

1. Prepare the game directory as specified above.
2. Open ResidualVM.
3. Click `Add Game...`.
4. Select the directory you created in step 1.
5. Click `Start`.

### 4.3. Input controls ###

The mouse is used to look around and interact with the ages.

Available keyboard shortcuts:

Key           | Binding
------------- | ------------------------------
`Escape`      | Original Myst III menu
`Space`       | Skip cutscenes, interact
`Ctrl` + `F5` | ResidualVM menu
`Ctrl` + `c`  | Force quit (from command line)
`Ctrl` + `q`  | Quit (in-game)
`Alt`  + `s`  | Save a screenshot


## 5. Running The Longest Journey

### 5.1. Required files ###

You will need to copy the data files from your The Longest Journey CDs,
DVD or digital distribution into one directory. Specifically, you'll
need:
  * The `1a`—`79` directories (only `4f` for demo version).
  * The `global` directory.
  * The `static` directory.
  * The `fonts` directory (not critical, but recommended – see below).
  * `x.xarc` and all the `INI` files.
  * `game.exe` (not critical, but recommended for a styled message dialog)

The 2-CD and DVD versions have some of the data files packed in installer
archives. The archives need to be unpacked before they can be used.

Steam version and demo from Steam are missing the `fonts` directory.
The required fonts can be copied over from demo version obtained from
different sources or a GOG or retail version.

Mixing files from different versions of the game is not supported.

### 5.2. Running the game ###

1. Prepare the game directory as specified above.
2. Open ResidualVM.
3. Click `Add Game...`.
4. Select the directory you created in step 1.
5. Click `Start`.

### 5.3. Input controls ###

The mouse is used to interact with objects and menu elements.

Available keyboard shortcuts:

Key             | Binding
--------------- | -------------------------------------------------------------------------------------------
`Escape`        | Skip video sequence or current line of dialogue, skip time if *Time Skip* option is enabled
`F1`            | Diary Menu
`F2`            | Save game
`F3`            | Load game
`F4`            | Conversation Log
`F5`            | April's Diary (initially disabled)
`F6`            | Video replay
`F7`            | Game settings
`F8`            | Save a screenshot
`F9`            | Toggle subtitles on and off
`F10`           | Quit game and return to main menu
`A`             | Cycle back through inventory cursor items
`S`             | Cycle forward through inventory cursor items
`I`             | Inventory
`P`             | Pause the game
`X`             | Display all exits on current location
`Page Up`       | Scroll up in dialogues and in your inventory
`Up arrow`      | Scroll up in dialogues and in your inventory
`Page Down`     | Scroll down in dialogues and in your inventory
`Down arrow`    | Scroll down in dialogues and in your inventory
`Enter`         | Select currently highlighted dialogue choice
`1` – `9`       | Select a dialogue choice
`Ctrl` + `F5`   | ResidualVM menu
`Alt` + `Enter` | Switch between windowed mode and fullscreen
`Ctrl` + `c`    | Force quit (from command line)
`Ctrl` + `q`    | Quit (in-game)
`Alt` + `x`     | Quit
`Alt` + `q`     | Quit
`Alt` + `s`     | Save a screenshot


## 6. Configuration
Currently, not all the settings for ResidualVM are available through the
GUI, if you have problems with getting anything to work, first try to pass
the settings from the command line, then, if that doesn't work, try to
change your configuration file manually.

### 6.1. Location of configuration file ###

By default, the configuration file is saved in and loaded from:

Operating System     | Location
-------------------- | -----------------------------------------------------------------------------
Windows Vista/7/8/10 | `\Users\username\AppData\Roaming\ResidualVM\residualvm.ini`
Windows 2000/XP      | `\Documents and Settings\username\Application Data\ResidualVM\residualvm.ini`
Linux                | `~/.config/residualvm/residualvm.ini`
macOS/OS X           | `~/Library/Preferences/ResidualVM Preferences`
Others               | `residualvm.ini` in the current directory

### 6.2. Location of saved screenshots ###

By default, screenshots will be saved to:

Operating System    | Location
------------------- | -----------------------------------------------------------------
Windows             | `\Users\username\My Pictures\ResidualVM Screenshots`
macOS X             | On the Desktop
Other unices        | In the XDG Pictures user directory, e.g. `~/Pictures/ResidualVM Screenshots`
Any other OS        | In the current directory

Alternatively, you can specify the directory where the screenshots will be saved in the configuration file. To do so, add a screenshotpath value under the [residualvm] section:

```
[residualvm]
screenshotpath=/path/to/screenshots/
```

### 6.3. Interesting settings for GrimE games ###

The following settings are currently available in the config file, however
some of them might not work with your current build and some of them might
make ResidualVM crash, or behave in weird ways.

Setting           | Values                   | Effect
----------------- | ------------------------ | ---------------------------------------------------
`show_fps`        | `true`/`false`           | If set to `true`, ResidualVM will show the current FPS rate while you play
`last_set`        | set name                 | The set you were last on, ResidualVM will try to continue from there
`last_save`       | save number              | The save you last saved, ResidualVM will have that selected the next time you try to load a game
`use_arb_shaders` | `true`/`false`           | If set to `true` and if you are using the OpenGL renderer ResidualVM will use ARB shaders. While fast, they may be incompatible with some graphics drivers
`fullscreen_res`  | `desktop`/width`x`height | If set to `desktop` (the default), ResidualVM will use your desktop resolution in fullscreen mode. If set to a resolution such as `640x480` or `1280x720`, that resolution will be used


## 7. Troubleshooting, known bugs, issues
Grim Fandango had a few issues when it came out and a few new and
exciting issues when you try to run it on newer hardware. Some of these
have been fixed in ResidualVM, but ResidualVM itself also has a few new
issues that we know about:
https://github.com/residualvm/residualvm/blob/master/KNOWN_BUGS

Look below for help with these issues and if you can't find help here,
try contacting us at [Contact](#10-contact) section.

### 7.1. I played a bit, but can't start a new game! ###

This is because the last save and visited scene is stored in your
configuration file, either delete your Grim Fandango entry from the
ResidualVM menu and readd it, or go to your configuration file and clean
out the `last_save` and `last_set` entries.

### 7.2. My saved games don't work any more! ###

Did you recently update to a newer build of ResidualVM?
ResidualVM is still a work in progress, which means that the save format
might change between builds. While attempts are made to keep save file
compatibility, this isn't always possible.


## 8. Debugging
**WARNING:** This section contains information about the various tools that
are included for debugging ResidualVM, this should not be necessary for
normal play at all! However, the curious might like to know how to access
these tool. Please use at your own risk!

To enter the debug console, press `Ctrl` + `Alt`  + `d`. Use the `help` command to
display a list of the available commands. To exit, press `Escape` or type
`exit` or `quit`.

Debug console commands common to all engines:

Command             | Description
------------------- | -----------------------------------------------------------
`openlog`           | Show the log of errors/warnings/information from the engine
`debuglevel`        | List or change verbosity of debug output
`debugflag_list`    | List the available debug flags and their status
`debugflag_enable`  | Enable a given debug flag
`debugflag_disable` | Disable a given debug flag
`md5`               | Calculates MD5 checksum of a file
`md5mac`            | Calculates MD5 checksum of a file (Mac format)

### 8.1. Debugging GrimE games ###

The development console can be used to debug both Grim Fandango and Escape
from Monkey Island.

Some of the useful debug console commands:

Command        | Description
-------------- | -----------------------------------------------------------
`jump`         | Jump to a section of the game
`lua_do`       | Execute a lua command
`set_renderer` | Select a renderer (software, OpenGL or OpenGL with shaders)

The `jump` targets can be found at:
  * https://wiki.residualvm.org/index.php/Grim_Fandango_Debug_Mode#Jump_targets
  * https://wiki.residualvm.org/index.php/Escape_From_Monkey_Island_Debug_Mode#Jump_targets

### 8.2. Debugging Grim Fandango ###

Grim Fandango also includes a built in debug mode. To enable debug mode
and debug keys, you will have to add the following line to your
configuration file under the Grim Fandango entry:

`game_devel_mode=true`

Development/debug keys from the original game:

Key           | Binding
------------- | ------------------------------------
`Ctrl` + `e`  | Enter lua string to execute
`Ctrl` + `g`  | Jump to set
`Ctrl` + `i`  | Toggle walk boxes
`Ctrl` + `l`  | Toggle lighting
`Ctrl` + `n`  | Display background name
`Ctrl` + `o`  | Create a door
`Ctrl` + `p`  | Execute patch file
`Ctrl` + `s`  | Turn on cursor
`Ctrl` + `u`  | Create a new object
`Ctrl` + `v`  | Print the value of a variable
`Alt` + `n`   | Next viewpoint
`Alt` + `p`   | Prev viewpoint
`Alt` + `s`   | Run lua script
`Shift` + `n` | Next set
`Shift` + `p` | Previous set
`Shift` + `o` | Toggle object names
`F3`          | Toggle sector editor
`Home`        | Go to default position in current set
`j`           | Enter jump number

Note that these are only available after enabling debug mode.

### 8.3. Debugging Myst III ###

The most useful debug console commands are:

Command       | Description
------------- | ------------------------------------------------------------
`dumpArchive` | Extract a game archive
`infos`       | Display the name of a node and show the associated scripts
`go`          | Jump to a node
`var`         | Display or alter the value of a variable from the game state

### 8.4. Debugging The Longest Journey ###

The debug console commands are:

Command               | Description
--------------------- | --------------------------------------------------------
`changeChapter`       | Change the current chapter
`changeKnowledge`     | Change the value of some knowledge
`changeLocation`      | Change the current location
`chapter`             | Display the current chapter
`decompileScript`     | Decompile a script
`dumpArchive`         | Extract a game archive
`dumpGlobal`          | Print the global level's resource sub-tree to stdout
`dumpKnowledge`       | Print the current knowledge to stdout
`dumpLevel`           | Print the current level's resource sub-tree to stdout
`dumpLocation`        | Print the current location's resource sub-tree to stdout
`dumpRoot`            | Print the resource tree root to stdout
`dumpStatic`          | Print the static level's resource sub-tree to stdout
`enableInventoryItem` | Enable a specific inventory item
`enableScript`        | Enable or disable script
`extractAllTextures`  | Extract the textures used by the 3d models to `dump/`
`forceScript`         | Force the execution of a script
`forceAnimation`      | Force an animation to play
`listAnimations`      | List all the animations in the current level
`listInventoryItems`  | List all inventory items in the game
`listLocations`       | List all the locations in the game
`listScripts`         | List all the scripts in current level
`location`            | Display the current location
`testDecompiler`      | Test decompilation of all the scripts in game

### 8.5. Modding The Longest Journey ###

ResidualVM can load replacement assets instead of the original files for
some of the asset types. By leveraging this capability, users can create
mods for the game. These are the currently supported modding features:

  * Load mods from the `mods` directory inside the game data path.
  Each mod should be in its own directory in the `mods` subdirectory.
  Mods are loaded in alphabetical order.

  * Load external PNG files instead of the XMG files inside the game
  archives.
    The replacement PNG files can have larger dimensions when compared to
  the original XMG images, enabling the creation of a high resolution mod.
    The game looks for the replacement files in a mod directory and then
  in the `xarc` subdirectory of the directory containing the archive in
  which the XMG picture to be replaced is located. For instance:
  `mods/[my_mod]/1e/00/xarc/fountain_layercenter.png` needs to be used for
  the Venice park background.
    ResidualVM expects PNGs to be in pre-multiplied alpha format for improved
  load times. However the `replacement_png_premultiply_alpha` `residualvm.ini`
  setting allows to load regular transparency PNGs when set to `true` for
  convenience when testing.

  * Load replacement video files for the Smacker animations.
    The replacement files can be either in Smacker or Bink encoding. With
  Smacker, only 1-bit transparency can be used. Transparent pixels must have
  the Cyan color (#00FFFF). When using Bink, 8-bit transparency can be used.
  The alpha channel should be encoded in the pre-multiplied alpha format.
    The replacement videos can have larger dimensions than the originals
  but must have the same number of frames and the same frame rate.
    Like with PNG files, replacement video files are loaded from mod folders:
  for instance `mods/[my_mod]/08/02/xarc/011001.bik` is the animation where
  the tree spirit lifts the egg back into the nest.

  * Load replacement textures for the 3d models.
    Each original `tm` file contains several textures, each with its
  associated mipmaps. The replacement files are `zip` archives containing
  `dds` packaged textures. The replacement archives must be placed at the root
  of the mod directory and be named after the `tm` file they replace:
  `mods/[my_mod]/april_waitress.tm.zip`.
    Each `zip` archive must contain all the textures from the replaced `tm`
  file. The textures need to be encoded in uncompressed RGB or RGBA `dds`
  files with mipmaps. Files inside the archive must be named according
  to the replaced texture name, but with the `bmp` extension replaced with
  `dds`: `backdress-highres-battic.dds`
    The `extractAllTextures` console command can be used to extract the `tm`
  files to `png` files.

Contact us if you need further capabilities for your mod.

## 9. Bug reports
ResidualVM still has a few bugs, many might already have been reported,
but should you find a new one, don't hesitate to report it.

### 9.1. How and where do I report bugs? ###

You can report bugs at our GitHub issue tracker:
https://github.com/residualvm/residualvm/issues

Please read the Wiki regarding how to report bugs properly first though:
https://wiki.residualvm.org/index.php?title=Reporting_Bugs

Remember to always provide the following information in your bug reports:
  * Information about the game (e.g. *Escape from Monkey Island, PS2
    version*).
  * Language of game (*English*, *German*, *...*).
  * Platform (*Windows*, *Linux*, *macOS*, *...*).
  * Bug details, including instructions on reproducing it.
  * Preferably also a link to a save game right before the bug happened.


## 10. Changelog

Please refer to our extensive Changelog [here](NEWS.md).

## 11. Credits

Please refer to our extensive Credits list [here](AUTHORS).

## 12. Contact
  * Homepage: https://www.residualvm.org/
  * Wiki: https://wiki.residualvm.org/
  * Discord: https://discord.gg/VFQjRfa
  * Forums: https://forums.residualvm.org/
  * IRC: #residualvm on freenode
