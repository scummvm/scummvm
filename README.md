# [ScummVM README](https://www.scummvm.org/) · [![Build Status](https://travis-ci.org/scummvm/scummvm.svg?branch=master)](https://travis-ci.org/scummvm/scummvm) ![CI](https://github.com/scummvm/scummvm/workflows/CI/badge.svg) [![Translation status](https://translations.scummvm.org/widgets/scummvm/-/scummvm/svg-badge.svg)](https://translations.scummvm.org/engage/scummvm/?utm_source=widget) [![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md#pull-requests) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/200a9bd3f7a647b48415efb484cc8bdc)](https://www.codacy.com/app/sev-/scummvm?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=scummvm/scummvm&amp;utm_campaign=Badge_Grade)

For more information, compatibility lists, details on donating, the
latest release, progress reports and more, please visit the ScummVM home
page at: <https://www.scummvm.org/>

## Table of Contents:

  - [1.0) Introduction](#10-introduction)
      - [1.1) About ScummVM](#11-about-scummvm)
      - [1.2) Quick start](#12-quick-start)
      - [1.3) F.A.Q.](#13-faq)
  - [2.0) Contact](#20-contact)
      - [2.1) Reporting Bugs](#21-reporting-bugs)
  - [3.0) Supported Games](#30-supported-games)
      - [3.1) Copy Protection](#31-copy-protection)
      - [3.2) Datafiles](#32-datafiles)
      - [3.3) Multi-CD games notes](#33-multi-cd-games-notes)
      - [3.4) Known Problems](#34-known-problems)
      - [3.5) Extra Data Files](#35-extra-data-files)
      - [3.6) Blade Runner notes](#36-blade-runner-notes)
      - [3.7) Broken Sword games notes](#37-broken-sword-games-notes)
          - [3.7.1) Broken Sword](#371-broken-sword)
          - [3.7.2) Broken Sword II](#372-broken-sword-ii)
          - [3.7.3) Broken Sword games
            cutscenes](#373-broken-sword-games-cutscenes)
          - [3.7.4) Broken Sword games cutscenes, in
            retrospect](#374-broken-sword-games-cutscenes-in-retrospect)
      - [3.8) Day of the Tentacle notes](#38-day-of-the-tentacle-notes)
      - [3.9) Dragon History notes](#39-dragon-history-notes)
      - [3.10) Flight of the Amazon Queen
        notes](#310-flight-of-the-amazon-queen-notes)
      - [3.11) Gobliiins notes](#311-gobliiins-notes)
      - [3.12) Inherit the Earth: Quest for the Orb
        notes](#312-inherit-the-earth-quest-for-the-orb-notes)
      - [3.13) Mickey's Space Adventure
        notes](#313-mickeys-space-adventure-notes)
      - [3.14) Might and Magic Xeen games
        notes](#314-might-and-magic-xeen-games-notes)
      - [3.15) Myst game notes](#315-myst-game-notes)
      - [3.16) Quest for Glory notes](#316-quest-for-glory-notes)
      - [3.17) Riven game notes](#317-riven-game-notes)
      - [3.18) Simon the Sorcerer games
        notes](#318-simon-the-sorcerer-games-notes)
      - [3.19) Starship Titanic game
        notes](#319-starship-titanic-game-notes)
      - [3.20) The Curse of Monkey Island
        notes](#320-the-curse-of-monkey-island-notes)
      - [3.21) The Feeble Files notes](#321-the-feeble-files-notes)
      - [3.22) The Legend of Kyrandia
        notes](#322-the-legend-of-kyrandia-notes)
      - [3.23) Troll's Tale notes](#323-trolls-tale-notes)
      - [3.24) Winnie the Pooh notes](#324-winnie-the-pooh-notes)
      - [3.25) Sierra AGI games: Predictive Input
        Dialog](#325-sierra-agi-games-predictive-input-dialog)
      - [3.26) Sierra SCI games: Simultaneous speech and
        subtitles](#326-sierra-sci-games-simultaneous-speech-and-subtitles)
      - [3.27) Zork games notes](#327-zork-games-notes)
          - [3.27.1) Zork Nemesis: The Forbidden
            Lands](#3271-zork-nemesis-the-forbidden-lands)
          - [3.27.2) Zork: Grand
            Inquisitor](#3272-zork-grand-inquisitor)
      - [3.28) Commodore64 games notes](#328-commodore64-games-notes)
      - [3.29) Macintosh games notes](#329-macintosh-games-notes)
  - [4.0) Supported Platforms](#40-supported-platforms)
  - [5.0) Running ScummVM](#50-running-scummvm)
      - [5.1) Command Line Options](#51-command-line-options)
      - [5.2) Global Menu](#52-global-menu)
      - [5.3) Graphics filters](#53-graphics-filters)
      - [5.4) Hotkeys](#54-hotkeys)
      - [5.5) Language options](#55-language-options)
  - [6.0) Saved Games](#60-saved-games)
      - [6.1) Autosaves](#61-autosaves)
      - [6.2) Converting Saved Games](#62-converting-saved-games)
      - [6.3) Viewing/Loading saved games from the command
        line](#63-viewingloading-saved-games-from-the-command-line)
  - [7.0) Music and Sound](#70-music-and-sound)
      - [7.1) AdLib emulation](#71-adlib-emulation)
      - [7.2) FluidSynth MIDI emulation](#72-fluidsynth-midi-emulation)
      - [7.3) MT-32 emulation](#73-mt-32-emulation)
      - [7.4) MIDI emulation](#74-midi-emulation)
      - [7.5) Native MIDI support](#75-native-midi-support)
          - [7.5.1) Using MIDI options to customize Native MIDI
            output](#751-using-midi-options-to-customize-native-midi-output)
      - [7.6) UNIX native, ALSA and dmedia sequencer
        support](#76-unix-native-alsa-and-dmedia-sequencer-support)
          - [7.6.1) ALSA sequencer \[UNIX
            ONLY\]](#761-alsa-sequencer-unix-only)
          - [7.6.2) IRIX dmedia sequencer: \[UNIX
            ONLY\]](#762-irix-dmedia-sequencer-unix-only)
      - [7.7) TiMidity++ MIDI server
        support](#77-timidity-midi-server-support)
      - [7.8) Using compressed audio
        files](#78-using-compressed-audio-files)
          - [7.8.1) Using MP3 files for CD
            audio](#781-using-mp3-files-for-cd-audio)
          - [7.8.2) Using Ogg Vorbis files for CD
            audio](#782-using-ogg-vorbis-files-for-cd-audio)
          - [7.8.3) Using Flac files for CD
            audio](#783-using-flac-files-for-cd-audio)
          - [7.8.4) Compressing MONSTER.SOU with
            MP3](#784-compressing-monstersou-with-mp3)
          - [7.8.5) Compressing MONSTER.SOU with Ogg
            Vorbis](#785-compressing-monstersou-with-ogg-vorbis)
          - [7.8.6) Compressing MONSTER.SOU with
            Flac](#786-compressing-monstersou-with-flac)
          - [7.8.7) Compressing music/sfx/speech in AGOS
            games](#787-compressing-musicsfxspeech-in-agos-games)
          - [7.8.8) Compressing speech/music in Broken
            Sword](#788-compressing-speechmusic-in-broken-sword)
          - [7.8.9) Compressing speech/music in Broken Sword
            II](#789-compressing-speechmusic-in-broken-sword-ii)
      - [7.9) Output sample rate](#79-output-sample-rate)
  - [8.0) Configuration file](#80-configuration-file)
      - [8.1) Recognized configuration
        keywords](#81-recognized-configuration-keywords)
      - [8.2) Custom game options that can be toggled via the
        GUI](#82-custom-game-options-that-can-be-toggled-via-the-gui)
  - [9.0) Screenshots (SDL backend
    only)](#90-screenshots-sdl-backend-only)
  - [10.0) Compiling](#100-compiling)
  - [11.0) Changelog](#110-changelog)
  - [12.0) Credits](#120-credits)

## 1.0) Introduction

### 1.1) About ScummVM

ScummVM is a program which allows you to run certain classic graphical
point-and-click adventure games, provided you already have their data
files. The clever part about this: ScummVM just replaces the executables
shipped with the game, allowing you to play them on systems for which
they were never designed\!

Originally it was designed to run LucasArts' SCUMM games, such as Maniac
Mansion, Monkey Island, Day of the Tentacle or Sam and Max. SCUMM stands
for 'Script Creation Utility for Maniac Mansion', which was the first
game for which LucasArts designed this system. And much later it gave
its name to ScummVM ('VM' meaning Virtual Machine).

Over time support for a lot of non-SCUMM games has been added, and
ScummVM now also supports many of Sierra's AGI and SCI games (such as
King's Quest 1-7, Space Quest 1-6, ...), Discworld 1 and 2, Simon the
Sorcerer 1 and 2, Beneath A Steel Sky, Lure of the Temptress, Broken
Sword I and II, Flight of the Amazon Queen, Gobliiins 1-3, The Legend of
Kyrandia series, many of Humongous Entertainment's children's SCUMM
games (including Freddi Fish and Putt Putt games) and many more. You can
find a full list with details on which adventures are supported and how
well on the compatibility page. ScummVM is continually improving, so
check back often.

Among the systems on which you can play those games are regular desktop
computers (running Windows, Linux, Mac OS X, ...), game consoles
(Dreamcast, Nintendo DS & Wii, PSP, ...), smartphones (Android, iPhone,
Symbian ...) and more.

At this time ScummVM is still under heavy development. Be aware that
whilst we attempt to make sure that many games can be completed with few
major bugs, crashes can happen and we offer no warranty. That being
said, some of the games have been supported for a long time and should
work fine with any recent stable release. You can get a feeling of how
well each game is working in ScummVM by looking at the compatibility
page. Actually, if you browse a bit around you might discover that
ScummVM is even being used commercially to re-release some of the
supported games on modern platforms. This shows that several companies
are happy with the quality of the software and how well it can run some
of the games.

If you enjoy ScummVM feel free to donate using the PayPal button on the
ScummVM homepage. This will help us buy utilities needed to develop
ScummVM easier and quicker. If you cannot donate, help and contribute a
patch\!

### 1.2) Quick start

For the impatient among you, here is how to get ScummVM running in five
simple steps.

1.  Download ScummVM from <https://www.scummvm.org/downloads/> and
    install it.

2.  Create a directory on your hard drive and copy the game datafiles
    from the original media to this directory. Repeat this for every
    game you want to play.

3.  Start ScummVM, choose 'Add game', select the directory with the game
    datafiles (do not try to select the datafiles themselves\!) and
    press `Choose`.

4.  A dialog should pop up allowing you to configure various settings if
    you wish to (it should be just fine to leave everything at its
    default, though). Confirm the dialog.

5.  Select the game you want to play in the list, and press `Start`.

In the future, you should be able to directly skip to step 5, unless you
want to add more games.

Hint: If you want to add multiple games in one go, try pressing and
holding the shift key before clicking 'Add game' -- its label will
change to 'Mass Add' and if you press it, you are again asked to select
a directory, only this time ScummVM will search through all
subdirectories for supported games.

### 1.3) F.A.Q.

We've compiled a list of F.A.Q. at:

<https://www.scummvm.org/faq/>

## 2.0) Contact

The easiest way to contact the ScummVM team is by submitting bug reports
(see section 2.1) or by using our forums at <https://forums.scummvm.org>.
You can also join and e-mail the scummvm-devel mailing list, or chat
with us on IRC (\#scummvm on irc.freenode.net) Please do not ask us to
support an unsupported game -- read the FAQ on our web site first.

### 2.1) Reporting Bugs

To report a bug, please follow the "Bug Tracker" link from our homepage
and log in with your GitHub account. Please make sure the bug is
reproducible, and still occurs in the latest git/Daily build version.
Also check the known problems list (below) and the compatibility list on
our website for that game, to ensure the issue is not already known:

<https://www.scummvm.org/compatibility/>

Please do not report bugs for games that are not listed as being
completeable in the 'Supported Games' section, or compatibility list. We
_know_ those games have bugs.

Please include the following information:

  - ScummVM version (PLEASE test the latest git/Daily build)
  - Bug details, including instructions on reproducing
  - Language of game (English, German, ...)
  - Version of game (talkie, floppy, ...)
  - Platform and Compiler (Win32, Linux, FreeBSD, ...)
  - Attach a saved game if possible - If this bug only occurred
    recently, please note the last version without the bug, and the
    first version including the bug. That way we can fix it quicker by
    looking at the changes made.

Finally, please report each issue separately; do not file multiple
issues on the same ticket. (Otherwise, it gets difficult to track the
status of each individual bug).

## 3.0) Supported Games

At the moment the following games have been reported to work, and should
be playable to the end: A more detailed compatibility list of the
supported games can be found here:

<https://www.scummvm.org/compatibility/>

| LucasArts (SCUMM) Games:                |              |
| --------------------------------------- | ------------ |
| Maniac Mansion                          | \[maniac\]   |
| Zak McKracken and the Alien Mindbenders | \[zak\]      |
| Indiana Jones and the Last Crusade      | \[indy3\]    |
| Loom                                    | \[loom\]     |
| Passport to Adventure                   | \[pass\]     |
| The Secret of Monkey Island             | \[monkey\]   |
| Monkey Island 2: LeChuck's Revenge      | \[monkey2\]  |
| Indiana Jones and the Fate of Atlantis  | \[atlantis\] |
| Day of the Tentacle                     | \[tentacle\] |
| Sam & Max Hit the Road                  | \[samnmax\]  |
| Full Throttle                           | \[ft\]       |
| The Dig                                 | \[dig\]      |
| The Curse of Monkey Island              | \[comi\]     |

| Activision (MADE) Games:      |             |
| ----------------------------- | ----------- |
| Leather Goddesses of Phobos 2 | \[lgop2\]   |
| The Manhole                   | \[manhole\] |
| Return to Zork                | \[rtz\]     |
| Rodney's Funscreen            | \[rodney\]  |

| Adventuresoft/Horrorsoft (AGOS) Games:                |              |
| ----------------------------------------------------- | ------------ |
| Elvira - Mistress of the Dark                         | \[elvira1\]  |
| Elvira II - The Jaws of Cerberus                      | \[elvira2\]  |
| Personal Nightmare                                    | \[pn\]       |
| Simon the Sorcerer 1                                  | \[simon1\]   |
| Simon the Sorcerer 2                                  | \[simon2\]   |
| Simon the Sorcerer's Puzzle Pack - Demon In My Pocket | \[dimp\]     |
| Simon the Sorcerer's Puzzle Pack - Jumble             | \[jumble\]   |
| Simon the Sorcerer's Puzzle Pack - NoPatience         | \[puzzle\]   |
| Simon the Sorcerer's Puzzle Pack - Swampy Adventures  | \[swampy\]   |
| The Feeble Files                                      | \[feeble\]   |
| Waxworks                                              | \[waxworks\] |

| Coktel Vision (GOB) Games:                           |                 |
| ---------------------------------------------------- | --------------- |
| Bargon Attack                                        | \[bargon\]      |
| Fascination                                          | \[fascination\] |
| Geisha                                               | \[geisha\]      |
| Gobliiins                                            | \[gob1\]        |
| Gobliins 2                                           | \[gob2\]        |
| Goblins 3                                            | \[gob3\]        |
| Lost in Time                                         | \[lostintime\]  |
| Once Upon A Time: Little Red Riding Hood             | \[littlered\]   |
| Playtoons: Bambou le sauveur de la jungle            | \[bambou\]      |
| The Bizarre Adventures of Woodruff and the Schnibble | \[woodruff\]    |
| Urban Runner                                         | \[urban\]       |
| Ween: The Prophecy                                   | \[ween\]        |

| Revolution Software (Various) Games:     |            |
| ---------------------------------------- | ---------- |
| Beneath a Steel Sky                      | \[sky\]    |
| Broken Sword: The Shadow of the Templars | \[sword1\] |
| Broken Sword II: The Smoking Mirror      | \[sword2\] |
| Lure of the Temptress                    | \[lure\]   |

| Sierra (AGI/preAGI) Games:                           |                 |
| ---------------------------------------------------- | --------------- |
| The Black Cauldron                                   | \[bc\]          |
| Gold Rush\!                                          | \[goldrush\]    |
| King's Quest I                                       | \[kq1\]         |
| King's Quest II                                      | \[kq2\]         |
| King's Quest III                                     | \[kq3\]         |
| King's Quest IV                                      | \[kq4\]         |
| Leisure Suit Larry in the Land of the Lounge Lizards | \[lsl1\]        |
| Mixed-Up Mother Goose                                | \[mixedup\]     |
| Manhunter 1: New York                                | \[mh1\]         |
| Manhunter 2: San Francisco                           | \[mh2\]         |
| Police Quest I: In Pursuit of the Death Angel        | \[pq1\]         |
| Space Quest I: The Sarien Encounter                  | \[sq1\]         |
| Space Quest II: Vohaul's Revenge                     | \[sq2\]         |
| Fanmade Games                                        | \[agi-fanmade\] |
| Mickey's Space Adventure                             | \[mickey\]      |
| Troll's Tale                                         | \[troll\]       |
| Winnie the Pooh in the Hundred Acre Wood             | \[winnie\]      |

| Sierra (SCI) Games:                        |                      |
| ------------------------------------------ | -------------------- |
| Castle of Dr. Brain                        | \[castlebrain\]      |
| Codename: ICEMAN                           | \[iceman\]           |
| Conquests of Camelot                       | \[camelot\]          |
| Conquests of the Longbow                   | \[longbow\]          |
| EcoQuest: The Search for Cetus             | \[ecoquest\]         |
| EcoQuest 2: Lost Secret of the Rainforest  | \[ecoquest2\]        |
| Freddy Pharkas: Frontier Pharmacist        | \[freddypharkas\]    |
| Gabriel Knight: Sins of the Fathers        | \[gk1\]              |
| Hoyle's Book of Games 1                    | \[hoyle1\]           |
| Hoyle's Book of Games 2                    | \[hoyle2\]           |
| Hoyle's Book of Games 3                    | \[hoyle3\]           |
| Hoyle Classic Card Games                   | \[hoyle4\]           |
| Jones in the Fast Lane                     | \[jones\]            |
| King's Quest I                             | \[kq1sci\]           |
| King's Quest IV                            | \[kq4sci\]           |
| King's Quest V                             | \[kq5\]              |
| King's Quest VI                            | \[kq6\]              |
| King's Quest VII                           | \[kq7\]              |
| King's Questions                           | \[kquestions\]       |
| Laura Bow: The Colonel's Bequest           | \[laurabow\]         |
| Laura Bow 2: The Dagger of Amon Ra         | \[laurabow2\]        |
| Leisure Suit Larry 1                       | \[lsl1sci\]          |
| Leisure Suit Larry 2                       | \[lsl2\]             |
| Leisure Suit Larry 3                       | \[lsl3\]             |
| Leisure Suit Larry 5                       | \[lsl5\]             |
| Leisure Suit Larry 6                       | \[lsl6\]             |
| Leisure Suit Larry 6 (hires)               | \[lsl6hires\]        |
| Leisure Suit Larry 7                       | \[lsl7\]             |
| Lighthouse: The Dark Being                 | \[lighthouse\]       |
| Mixed-up Fairy Tales                       | \[fairytales\]       |
| Mixed-up Mother Goose                      | \[mothergoose\]      |
| Mixed-up Mother Goose Deluxe               | \[mothergoosehires\] |
| Pepper's Adventures in Time                | \[pepper\]           |
| Phantasmagoria                             | \[phantasmagoria\]   |
| Phantasmagoria 2: A Puzzle of Flesh        | \[phantasmagoria2\]  |
| Police Quest 1                             | \[pq1sci\]           |
| Police Quest 2                             | \[pq2\]              |
| Police Quest 3                             | \[pq3\]              |
| Police Quest 4                             | \[pq4\]              |
| Quest for Glory 1/Hero's Quest             | \[qfg1\]             |
| Quest for Glory 1                          | \[qfg1vga\]          |
| Quest for Glory 2                          | \[qfg2\]             |
| Quest for Glory 3                          | \[qfg3\]             |
| Quest for Glory 4                          | \[qfg4\]             |
| RAMA                                       | \[rama\]             |
| Slater & Charlie Go Camping                | \[slater\]           |
| Shivers                                    | \[shivers\]          |
| Space Quest I                              | \[sq1sci\]           |
| Space Quest III                            | \[sq3\]              |
| Space Quest IV                             | \[sq4\]              |
| Space Quest V                              | \[sq5\]              |
| Space Quest 6                              | \[sq6\]              |
| The Island of Dr. Brain                    | \[islandbrain\]      |
| The Beast Within: A Gabriel Knight Mystery | \[gk2\]              |
| Torin's Passage                            | \[torin\]            |

| Other Games:                                                        |                 |
| ------------------------------------------------------------------- | --------------- |
| 3 Skulls of the Toltecs                                             | \[toltecs\]     |
| Amazon: Guardians of Eden                                           | \[access\]      |
| Beavis and Butt-head in Virtual Stupidity                           | \[bbvs\]        |
| Blade Runner                                                        | \[bladerunner\] |
| Blue Force                                                          | \[blueforce\]   |
| Broken Sword: The Return of the Templars                            | \[sword25\]     |
| Bud Tucker in Double Trouble                                        | \[tucker\]      |
| Chivalry is Not Dead                                                | \[chivalry\]    |
| Cruise for a Corpse                                                 | \[cruise\]      |
| DreamWeb                                                            | \[dreamweb\]    |
| Discworld                                                           | \[dw\]          |
| Discworld 2: Missing Presumed ...\!?                                | \[dw2\]         |
| Dragon History                                                      | \[draci\]       |
| Drascula: The Vampire Strikes Back                                  | \[drascula\]    |
| Eye of the Beholder                                                 | \[eob\]         |
| Eye of the Beholder II: The Legend of Darkmoon                      | \[eob2\]        |
| Flight of the Amazon Queen                                          | \[queen\]       |
| Future Wars                                                         | \[fw\]          |
| Hopkins FBI                                                         | \[hopkins\]     |
| Hugo's House of Horrors                                             | \[hugo1\]       |
| Hugo 2: Whodunit?                                                   | \[hugo2\]       |
| Hugo 3: Jungle of Doom                                              | \[hugo3\]       |
| I Have No Mouth, and I Must Scream                                  | \[ihnm\]        |
| Inherit the Earth: Quest for the Orb                                | \[ite\]         |
| Lands of Lore: The Throne of Chaos                                  | \[lol\]         |
| Mortville Manor                                                     | \[mortevielle\] |
| Myst / Myst: Masterpiece Edition                                    | \[myst\]        |
| Nippon Safes Inc.                                                   | \[nippon\]      |
| Rex Nebular and the Cosmic Gender Bender                            | \[nebular\]     |
| Ringworld: Revenge Of The Patriarch                                 | \[ringworld\]   |
| Riven: The Sequel to Myst                                           | \[riven\]       |
| Return to Ringworld                                                 | \[ringworld2\]  |
| Sfinx                                                               | \[sfinx\]       |
| Soltys                                                              | \[soltys\]      |
| Starship Titanic                                                    | \[titanic\]     |
| The Journeyman Project: Pegasus Prime                               | \[pegasus\]     |
| The Labyrinth of Time                                               | \[lab\]         |
| The Legend of Kyrandia                                              | \[kyra1\]       |
| The Legend of Kyrandia: The Hand of Fate                            | \[kyra2\]       |
| The Legend of Kyrandia: Malcolm's Revenge                           | \[kyra3\]       |
| The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel | \[scalpel\]     |
| The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo      | \[rosetattoo\]  |
| The Neverhood                                                       | \[neverhood\]   |
| The 7th Guest                                                       | \[t7g\]         |
| TeenAgent                                                           | \[teenagent\]   |
| Toonstruck                                                          | \[toon\]        |
| Tony Tough and the Night of Roasted Moths                           | \[tony\]        |
| Touche: The Adventures of the Fifth Musketeer                       | \[touche\]      |
| U.F.O.s / Gnap: Der Schurke aus dem All                             | \[gnap\]        |
| Versailles: 1685                                                    | \[versailles\]  |
| Voyeur                                                              | \[voyeur\]      |
| Zork: Grand Inquisitor                                              | \[zgi\]         |
| Zork Nemesis: The Forbidden Lands                                   | \[znemesis\]    |

| Humongous Entertainment (SCUMM) Games:                         |                   |
| -------------------------------------------------------------- | ----------------- |
| Backyard Baseball                                              | \[baseball\]      |
| Backyard Baseball 2001                                         | \[baseball2001\]  |
| Backyard Baseball 2003                                         | \[baseball2003\]  |
| Backyard Football                                              | \[football\]      |
| Backyard Football 2002                                         | \[football2002\]  |
| Bear Stormin'                                                  | \[brstorm\]       |
| Big Thinkers First Grade                                       | \[thinker1\]      |
| Big Thinkers Kindergarten                                      | \[thinkerk\]      |
| Blue's 123 Time Activities                                     | \[Blues123Time\]  |
| Blue's ABC Time Activities                                     | \[BluesABCTime\]  |
| Blue's Art Time Activities                                     | \[arttime\]       |
| Blue's Birthday Adventure                                      | \[BluesBirthday\] |
| Blue's Reading Time Activities                                 | \[readtime\]      |
| Fatty Bear's Birthday Surprise                                 | \[fbear\]         |
| Fatty Bear's Fun Pack                                          | \[fbpack\]        |
| Freddi Fish 1: The Case of the Missing Kelp Seeds              | \[freddi\]        |
| Freddi Fish 2: The Case of the Haunted Schoolhouse             | \[freddi2\]       |
| Freddi Fish 3: The Case of the Stolen Conch Shell              | \[freddi3\]       |
| Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch | \[freddi4\]       |
| Freddi Fish 5: The Case of the Creature of Coral Cove          | \[freddicove\]    |
| Freddi Fish and Luther's Maze Madness                          | \[maze\]          |
| Freddi Fish and Luther's Water Worries                         | \[water\]         |
| Let's Explore the Airport with Buzzy                           | \[airport\]       |
| Let's Explore the Farm with Buzzy                              | \[farm\]          |
| Let's Explore the Jungle with Buzzy                            | \[jungle\]        |
| Pajama Sam: Games to Play on Any Day                           | \[pjgames\]       |
| Pajama Sam 1: No Need to Hide When It's Dark Outside           | \[pajama\]        |
| Pajama Sam 2: Thunder and Lightning Aren't so Frightening      | \[pajama2\]       |
| Pajama Sam 3: You Are What You Eat From Your Head to Your Feet | \[pajama3\]       |
| Pajama Sam's Lost & Found                                      | \[lost\]          |
| Pajama Sam's Sock Works                                        | \[socks\]         |
| Putt-Putt Enters the Race                                      | \[puttrace\]      |
| Putt-Putt Goes to the Moon                                     | \[puttmoon\]      |
| Putt-Putt Joins the Circus                                     | \[puttcircus\]    |
| Putt-Putt Joins the Parade                                     | \[puttputt\]      |
| Putt-Putt Saves the Zoo                                        | \[puttzoo\]       |
| Putt-Putt Travels Through Time                                 | \[putttime\]      |
| Putt-Putt and Pep's Balloon-O-Rama                             | \[balloon\]       |
| Putt-Putt and Pep's Dog on a Stick                             | \[dog\]           |
| Putt-Putt & Fatty Bear's Activity Pack                         | \[activity\]      |
| Putt-Putt's Fun Pack                                           | \[funpack\]       |
| SPY Fox 1: Dry Cereal                                          | \[spyfox\]        |
| SPY Fox 2: Some Assembly Required                              | \[spyfox2\]       |
| SPY Fox 3: Operation Ozone                                     | \[spyozon\]       |
| SPY Fox in Cheese Chase                                        | \[chase\]         |
| SPY Fox in Hold the Mustard                                    | \[mustard\]       |

> The following games should load but are not yet fully playable. Play
> these at your own risk, and please do not file bug reports about them.
> If you want the latest updates on game compatibility, visit our web
> site and view the compatibility chart.

|                      |                       |
| -------------------- | --------------------- |
| Backyard Soccer      | \[soccer\]            |
| Backyard Soccer MLS  | \[soccermls\]         |
| Backyard Soccer 2004 | \[soccer2004\]        |
| Blue's Treasure Hunt | \[BluesTreasureHunt\] |

| Animation Magic (Composer) Games: |                 |
| --------------------------------- | --------------- |
| Darby the Dragon                  | \[darby\]       |
| Gregory and the Hot Air Balloon   | \[gregory\]     |
| Magic Tales: Liam Finds a Story   | \[liam\]        |
| The Princess and the Crab         | \[princess\]    |
| Sleeping Cub's Test of Courage    | \[sleepingcub\] |

| Living Books Games:                       |                |
| ----------------------------------------- | -------------- |
| Aesop's Fables: The Tortoise and the Hare | \[tortoise\]   |
| Arthur's Birthday                         | \[arthurbday\] |
| Arthur's Teacher Trouble                  | \[arthur\]     |
| Dr. Seuss's ABC                           | \[seussabc\]   |
| Green Eggs and Ham                        | \[greeneggs\]  |
| Harry and the Haunted House               | \[harryhh\]    |
| Just Grandma and Me                       | \[grandma\]    |
| Little Monster at School                  | \[lilmonster\] |
| Ruff's Bone                               | \[ruff\]       |
| Sheila Rae, the Brave                     | \[sheila\]     |
| Stellaluna                                | \[stellaluna\] |
| The Berenstain Bears Get in a Fight       | \[bearfight\]  |
| The Berenstain Bears in the Dark          | \[beardark\]   |
| The New Kid on the Block                  | \[newkid\]     |

The following games are based on the SCUMM engine, but NOT supported by
ScummVM (yet):

    Moonbase Commander

Please be aware that the engines may contain bugs and unimplemented
features that sometimes make it impossible to finish the game. Save
often, and please file a bug report (instructions on submitting bug
reports are above) if you encounter such a bug in a 'supported' game.

### 3.1) Copy Protection

The ScummVM team does not condone piracy. However, there are cases where
the game companies (such as LucasArts) themselves bundled 'cracked'
executables with their games -- in these cases the data files still
contain the copy protection scripts, but the interpreter bypasses them
(similar to what an illegally cracked version might do, only that here
the producer of the game did it). There is no way for us to tell the
difference between legitimate and pirated data files, so for the games
where we know that a cracked version of the original interpreter was
sold at some point, ScummVM will always have to bypass the copy
protection.

In some cases ScummVM will still show the copy protection screen. Try
entering any answer. Chances are that it will work.

ScummVM will skip copy protection in the following games:

  - Beneath a Steel Sky
      - bypassed with kind permission from Revolution Software.
  - Dreamweb
      - a list of available commands in the in-game terminals is now
        shown when the player uses the `help` command
  - Inherit the Earth: Quest for the Orb (Floppy version)
      - bypassed with kind permission from Wyrmkeep Entertainment, since
        it was bypassed in all CD releases of the game.
  - Loom (EGA DOS)
  - Lure of the Temptress
  - Maniac Mansion
  - Might and Magic: World of Xeen
  - Monkey Island 2: LeChuck's Revenge
  - Rex Nebular and The Cosmic Gender Bender
  - Simon the Sorcerer 1 (Floppy version)
  - Simon the Sorcerer 2 (Floppy version)
      - bypassed with kind permission from Adventure Soft, since it was
        bypassed in all CD releases of the game.
  - The Secret of Monkey Island (VGA)
  - Voyeur
  - Waxworks
  - Zak McKracken and the Alien Mindbenders

### 3.2) Datafiles

For a comprehensive list of required Datafiles for supported games
visit:

<https://wiki.scummvm.org/index.php/Datafiles>

### 3.3) Multi-CD games notes

In general, ScummVM does not deal very well with Multi-CD games. This is
because ScummVM assumes everything about a game can be found in one
directory. Even if ScummVM does make some provisions for asking the user
to change CD, the original game executables usually installed a small
number of files to the hard disk. Unless these files can be found on all
the CDs, ScummVM will be in trouble.

Fortunately, ScummVM has no problems running the games entirely from
hard disk, if you create a directory with the correct combination of
files. Usually, when a file appears on more than one CD you can pick
either of them.

### 3.4) Known Problems

This release has the following known problems. There is no need to
report them, although patches to fix them are welcome. If you discover a
bug that is not listed here, nor in the compatibility list on the web
site, please see the section on reporting bugs.

CD Audio Games:

  - When playing games that use CD Audio (FM-TOWNS games, Loom CD, etc)
    users of Microsoft Windows 2000/XP may experience random crashes.
    This is due to a long-standing Windows bug, resulting in corrupt
    game files being read from the CD. Please copy the game data to your
    hard disk to avoid this.

FM-TOWNS versions:

  - The Kanji versions require the FM-TOWNS Font ROM.

Loom:

  - Turning off the subtitles via the config file does not work reliably
    as the Loom scripts automatically turn them on again.
  - MIDI support in the EGA version requires the Roland update from
    LucasArts.
  - The PC-Engine Kanji version requires the system card rom.

The Secret of Monkey Island:

  - MIDI support in the EGA version requires the Roland update from
    LucasArts.

Beneath a Steel Sky:

  - Amiga versions aren't supported.
  - Floppy demos aren't supported.
  - Not a bug: CD version is missing speech for some dialogs, this is
    normal.

Elvira - Mistress of the Dark:

  - No music in the Atari ST version.

Elvira II - The Jaws of Cerberus

  - No music in the Atari ST version.
  - No sound effects in the PC version.
  - Palette issues in the Atari ST version.

Inherit the Earth: Quest for the Orb:

  - Amiga versions aren't supported.

Lure of the Temptress:

  - No Roland MT-32 support.
  - Sound support is incomplete and doesn't sound like original.

Simon the Sorcerer 1:

  - Subtitles aren't available in the English and German CD versions as
    they are missing the majority of subtitles.

Simon the Sorcerer 2:

  - Combined speech and subtitles will often cause speech to be cut off
    early, this is a limitation of the original game.
  - Only default language (English) of data files is supported in Amiga
    and Macintosh versions.

Simon the Sorcerer's Puzzle Pack:

  - No support for displaying, entering, loading or saving high scores.
  - No support for displaying names of items, when hovering over them in
    Swampy Adventures.

The Feeble Files:

  - Subtitles are often incomplete, they were always disabled in the
    original game.

The Legend of Kyrandia:

  - No music or sound effects in the Macintosh floppy versions.
  - Macintosh CD is using included DOS music and sound effects.

Humongous Entertainment games:

  - Only the original load and save interface can be used.
  - No support for multiplayer or printing images.

### 3.5) Extra Data Files

Some games require additional files that are not part of the original data. Those files can generally be found in our [Downloads](https://www.scummvm.org/downloads/#extras) page.

Games that require additional data:
  - Beneath a Steel Sky (sky.cpt)
  - Flight of the Amazon Queen
  - Kyrandia Series (kyra.dat)
  - Lands of Lore Series (kyra.dat)
  - Lure of the Temptress (lure.dat)
  - Versailles 1685 (cryomni3d.dat)

The most up to date list of Engine data files can be found in our [source code repository](https://github.com/scummvm/scummvm/tree/master/dists/engine-data)

### 3.6) Blade Runner notes

Supported versions are English (both CD and DVD), French, German,
Italian, Russian (by Fargus Multimedia) and Spanish. Other unofficial
versions exist but they are based on the English version.

All *.mix, *.tlk and *.dat files are needed from all 4 CDs/DVD.
Files named CDFRAMES.DAT need to be renamed to CDFRAMES1.DAT,
CDFRAMES2.DAT, CDFRAMES3.DAT and CDFRAMES4.DAT according on
the CD they came from. Other files with the same name on different
CDs are identical and it doesn't matter which one is used.

To enable subtitles, download a file from our [Downloads](https://www.scummvm.org/games/#bladerunner) page
and place file SUBTITLES.MIX next to the other game files.

"Blade Runner with restored content" is not yet supported and might
contain game breaking bugs.

### 3.7) Broken Sword games notes

The instructions for the Broken Sword games are for the Sold-Out
Software versions, with each game on two CDs, since these were the
versions most easily available at the time ScummVM gained support for
them. Hopefully they are general enough to be useful to other releases
as well.

#### 3.7.1) Broken Sword

For this game, you will need all of the files from the clusters
directories on both CDs. For the Windows and Macintosh versions, you
will also need the speech.clu files from the speech directories, but
since they are not identical you will need to rename them speech1.clu
and speech2.clu for CD 1 and 2 respectively. The PlayStation version
requires the speech.tab, speech.dat, speech.lis, and speech.inf.

In addition, the Windows and Macintosh versions require a music
subdirectory with all of the files from the music subdirectories on both
CDs. Some of these files appear on both CDs, but in these cases they are
either identical or, in one case, so nearly identical that it makes
little difference. The PlayStation version requires tunes.dat and
tunes.tab.

#### 3.7.2) Broken Sword II

For this game, you will need all of the files from the clusters
directories on both CDs. (Actually, a few of them may not be strictly
necessary, but the ones that I'm uncertain about are all fairly small.)
You will need to rename the speech.clu and music.clu files speech1.clu,
speech2.clu, music1.clu and music2.clu so that ScummVM can tell which
ones are from CD 1 and which ones are from CD 2. Any other files that
appear in both cluster directories are identical. Use whichever you
like.

In addition, you will need the cd.inf and, optionally, the startup.inf
files from the sword2 directory on CD 1.

#### 3.7.3) Broken Sword games cutscenes

The cutscenes for the Broken Sword games have a bit of a history (see
the next section, if you are interested), but in general all you need to
do is to copy the .SMK files from the "SMACKS" or "SMACKSHI" directories
on the CDs to the same directory as the other game data files. (Broken
Sword has a "SMACKSLO" directory with the same cutscenes, but these are
of lower quality.) You can put them in a subdirectory called "video" if
you find that neater.

For the PlayStation versions, you can dump the original videos off the
disc. For each of the files ending in an "STR" extension, you should
dump them as *raw* sectors off the disc (all 2352 bytes per sector). You
may also use the re-encoded cutscenes mentioned below instead, but this
will not work for all videos in Broken Sword II. For more information,
see:

<https://wiki.scummvm.org/index.php/HOWTO-PlayStation_Videos>

Some re-releases of the games, as well as the PlayStation version, do
not have Smacker videos. Revolution Software has kindly allowed us to
provide re-encoded cutscenes for download on our website:

<https://www.scummvm.org/downloads/>

These cutscenes are provided in DXA format with FLAC audio. Their
quality is equal to the original games due to the use of lossless
compression. Viewing these cutscenes requires a version of ScummVM
compiled with both FLAC and zlib support.

For systems that are too slow to handle the decoding of FLAC audio, the
audio for these cutscenes is also provided separately as OGG Vorbis
audio. Viewing these cutscenes with OGG Vorbis audio requires a version
of ScummVM compiled with both libVorbis and zlib support.

For Broken Sword, we also provide a subtitles add-on. Simply unpack it
and follow the instructions in its readme.txt file. The subtitle pack
currently does not work when running PlayStation videos. (Broken Sword
II already has subtitles; no extra work is needed for them.)

#### 3.7.4) Broken Sword games cutscenes, in retrospect

The original releases of the Broken Sword games used RAD Game Tools's
Smacker(tm) format. As RAD was unwilling to open the older legacy
versions of this format to us, and had requested we not reverse engineer
it, an alternative solution had to be found.

In Broken Sword II, it was possible to play back the voice-over without
playing the video itself. This remained a fallback until ScummVM 1.0.0,
but was never the only solution for any stable release.

In ScummVM 0.6.0 we used MPEG, which provided a reasonable trade-off
between size and quality. In ScummVM 0.10.0 this was superseded by DXA
(originally added for AdventureSoft's "The Feeble Files"). This gave us
a way of providing the cutscenes in the exact same quality as the
originals, at the cost of being larger.

Finally, in early 2006, the Smacker format was reverse engineered for
the FFmpeg project. Thanks to their hard work, ScummVM 1.0.0 now
supports the original cutscenes. At the same time, MPEG support was
dropped. From a technical standpoint, this was a good thing since
decoding MPEG movies added a lot of complexity, and they didn't look as
good as the Smacker and DXA versions anyway.

### 3.8) Day of the Tentacle notes

At one point in the game, you come across a computer that allows you to
play the original Maniac Mansion as an easter egg. ScummVM supports
this, with a few caveats:

ScummVM will scan your configuration file for a game that's in a
`Maniac` sub-folder of your Day of the Tentacle folder. If you've copied
the data files from the CD version, this should already be the case but
you have to add the game to ScummVM as well.

To return to Day of the Tentacle, press F5 and select "Return to
Launcher".

This means that you could in theory use any game as the easter egg.
Indeed, there is a "secret" configuration setting, `easter_egg`, to
override the ID of the game to run. Be aware, though, that not all games
support returning to the launcher, and setting it up to use Day of the
Tentacle itself as the easter egg game is not recommended.

### 3.9) Dragon History notes

There are 4 language variants of the game: Czech, English, Polish and
German. Each of them is distributed in a separate archive. The only
official version is the Czech one, and the English, Polish and German
ports have always been work in progress and never officially released.
Although all texts are fully translated, it is known that some of them
contain typos.

There exists an optional Czech dubbing for the game. For bandwidth
reasons, you can download it separately and then unpack it to the
directory of the game. You can listen to the Czech dubbing with all
language variants of the game, while reading the subtitles.

All game files and the walkthrough can be downloaded from:

<http://www.ucw.cz/draci-historie/index-en.html>

### 3.10) Flight of the Amazon Queen notes

Only the original non-freeware version of Flight of the Amazon Queen
(from original CD), requires the `queen.tbl` datafile (available from the `Downloads` page on our website) in either the
directory containing the `queen.1` game data file, in your extrapath, or
in the directory where your ScummVM executable resides.

Alternatively, you can use the `compress_queen` tool from the tools
package to 'rebuild' your FOTAQ data file to include the table for that
specific version, and thus removing the run-time dependency on the
`queen.tbl` file. This tool also allows you to compress the speech and
sound effects with MP3, OGG or FLAC.

### 3.11) Gobliiins notes

The CD versions of the Gobliiins series contain one big audio track
which you need to rip (see the section on using compressed audio files)
and copy into the game directory if you want to have in-game music
without the CD in the drive all the time. The speech is also in that
track and its volume is therefore changed with the music volume control
as well.

### 3.12) Inherit the Earth: Quest for the Orb notes

In order to run the Mac OS X Wyrmkeep re-release of the game you will
need to copy over data from the CD to your hard disk. If you're on a PC
then consult:

<https://wiki.scummvm.org/index.php/HOWTO-Mac_Games>

Although it primarily talks about SCUMM games, it mentions the
"HFSExplorer" utility which you need to extract the files. Note that you
have to put the speech data "Inherit the Earth Voices" in the same
directory as the game data which is stored in:

Inherit the Earth.app/Contents/Resources

For the old Mac OS 9 release you need to copy the files in MacBinary
format, as they should include both resource and data forks. Copy all
'ITE \*' files.

### 3.13) Mickey's Space Adventure notes

To run Mickey's Space Adventure under ScummVM, the original executable
of the game (mickey.exe) is needed together with the game's data files.

There is extensive mouse support for the game under ScummVM, even though
there wasn't any mouse support in the original game. Menu items can be
selected using the mouse, and it is possible to move to other locations
using the mouse as well. When the mouse cursor is hovered on the edges
of the screen, it changes color to red if it is possible to walk towards
that direction. The player can then simply click on the edges of the
game's screen to change location, similar to many adventure games, which
is simpler and more straightforward than moving around using the menu.

### 3.14) Might and Magic Xeen games notes

To properly play the World of Xeen CD Talkie using original discs, use
LAME or some other encoder to rip the cd audio tracks to files, either
mp3 or ogg. Whichever you choose, the tracks of the first CD should be
named from track02 to track31, whereas the second CD's audio tracks
should be encoded and renamed as track32 through to track60.

For the GOG Might and Magic 4-5 installation, install the game to your
computer, and do the following steps:

  - The game1.inst (CUE) and game1.gog (BIN) file from the game folder
    is a CD image. Use software like Virtual CloneDrive to mount it as a
    drive. Linux and MacOS users can use bchunk to convert it to an ISO.
  - Copy all the .cc files from the subfolder in the mounted drive to a
    new empty game folder that you create for the game.
  - Copy all the music/\*.ogg files from the GOG installation to your
    game folder. You'll then need to rename all of them from xeen??.ogg
    to track??.ogg
  - You should then be able to point ScummVM to this new game folder,
    and the CD talkie version should be detected.

Savegames from either Clouds or Darkside of Xeen games can be
transferred across to World of Xeen (that combines both games) simply by
setting up and detecting World of Xeen (either by manually combining the
two games or using the GOG World of Xeen installer), and then renaming
the savegames to use the World of Xeen savegame format, by default
'worldofxeen.\*'

The Xeen engine also offers two custom options in the Engine tab for the
games in the ScummVM launcher. They are:

  - To change the threshold armor breaks at for characters from -10HP to
    -80HP
  - To show values for inventory items, even outside of the blacksmith,
    allowing the relative strength/value of armor and weapons to be
    compared.

### 3.15) Myst game notes

Left Click: Move/action
Space: Pause the game
Esc: Skip cutscene
F5: Menu

Myst will autosave to slot 0 if no save or an autosave is present in
slot 0.

### 3.16) Quest for Glory notes

It is possible to import characters, beginning with Quest for Glory II,
from past games to future games and continue from the stats earned from
those games.

For example, a character can be imported from Quest for Glory I directly
to Quest for Glory III without having to necessarily play Quest for
Glory II.

Characters cannot be imported from future games to past games, nor can a
character be imported to the same game that was just completed. In other
words, a character from Quest for Glory II cannot be imported into Quest
for Glory II.

If you want to use a saved character from the original Sierra
interpreter, you will need to rename the character file to
"qfg\[game-number\]-\[character-filename\].sav" and place it in the
ScummVM save path (see section 6.0), otherwise the file won't get listed
on the import screen.

Example: qfg2-thief.sav

### 3.17) Riven game notes

Left Click: Move/action
Arrow Keys: Movement
Page Up: Look up
Page Down: Look down
Space: Pause the game
Esc: Skip cutscene
F5: Menu
Ctrl-o: Load game
Ctrl-s: Save game

Riven will autosave to slot 0 if no save or an autosave is present in
slot 0.

### 3.18) Simon the Sorcerer games notes

If you have the dual version of Simon the Sorcerer 1 or 2 on CD, you
will find the Windows version in the main directory of the CD and the
DOS version in the DOS directory of the CD.

### 3.19) Starship Titanic game notes

For the purposes of solving the starfield puzzle, only mouse clicks, L
and Tab are really needed, though the action glyph in the PET can be
used instead of Tab.

### 3.20) The Curse of Monkey Island notes

For this game, you will need the comi.la0, comi.la1 and comi.la2 files.
The comi.la0 file can be found on either CD, but since they are
identical it doesn't matter which one of them you use.

In addition, you will need to create a "resource" subdirectory
containing all of the files from -both- "resource" subdirectories on the
two CDs. Some of the files appear on both CDs, but again they're
identical.

### 3.21) The Feeble Files notes

Amiga/Macintosh: You need to install a small pack of cutscenes that are
missing in both of these versions of The Feeble Files. It's called "The
Feeble Files - Omni TV and epilogue cutscenes for the Amiga and
Macintosh versions" and you can get it here:

<https://www.scummvm.org/games/#feeble>

Windows: If you have the Windows version of The Feeble Files, there are
several things to note.

Many of the files necessary for the game are stored in an InstallShield
file called data1.cab, which ScummVM is unable to unpack. You will need
to use the original installer or i5comp to unpack the contents of this
file. The i5comp decompression tool, can be found via a search on the
internet.

To use the speech files with ScummVM, they need to be renamed as
follows:

  - Rename voices.wav on CD1 to voices1.wav
  - Rename voices.wav on CD2 to voices2.wav
  - Rename voices.wav on CD3 to voices3.wav
  - Rename voices.wav on CD4 to voices4.wav

### 3.22) The Legend of Kyrandia notes

To run The Legend of Kyrandia under ScummVM you need the `kyra.dat`
file. The file should already be included in official ScummVM packages.
In case ScummVM complains that the file is missing you can find it on
the `Downloads` page of the ScummVM website. Note that the current
Windows release of ScummVM should contain the file embedded into the
executable, thus you only need to grab it in case ScummVM complains
about the file being missing.

### 3.23) Troll's Tale notes

The original game came in a PC booter disk, therefore it is necessary to
dump the contents of that disk in an image file and name it "troll.img"
to be able to play the game under ScummVM.

### 3.24) Winnie the Pooh notes

It is possible to import saved games from the original interpreter of
the game into ScummVM.

There is extensive mouse support for the game under ScummVM, even though
there wasn't any mouse support in the original game. Menu items can be
selected using the mouse, and it is possible to move to other locations
using the mouse as well. When the mouse cursor is hovered on the edges
of the screen, it changes color to red if it is possible to walk towards
that direction. The player can then simply click on the edges of the
game's screen to change location, similar to many adventure games, which
is simpler and more straightforward than moving around using the menu.

### 3.25) Sierra AGI games: Predictive Input Dialog

The Predictive Input Dialog is a ScummVM aid for running AGI engine
games (which notoriously require command line input) on devices with
limited keyboard support. In these situations, since typing with
emulated keyboards is quite tedious, commands can be entered quickly and
easily via the Predictive Input Dialog.

In order to enable predictive input in AGI games, you need to copy the
pred.dic file in the ScummVM extras directory or the directory of the
game you wish to play. This dictionary has been created by parsing
through all known AGI games and contains the maximum set of common
words.

If the dictionary is detected, the Predictive Input Dialog is displayed
either when you click on the command line area (wherever keyboard input
is required, even in dialog boxes), or in some ports by pressing a
designated hot key.

The predictive input dialog operates in three modes, switchable by the
(\*)Pre/123/Abc button. The primary input method is the predictive mode
(Pre) which resembles the way "fast typing" is performed at phones. The
alphabet is divided into 9 sets which naturally map to the 9 number keys
of the numeric keypad (0 is space). To type in a word, you press once
the number of the set which contains the letter of the word you intend
to type, then move on to the next. For example, to type the command
`look`, you should press 5665. As you gradually type the intended word's
numeric code, the dictionary is accessed for known words matching your
input up to that point. As you press more keys, the prediction converges
to the correct word. This is why the printed word may change
dramatically between key presses. There exist situations though where
more than one words share the same numeric representation. For example
the words `quit` and `suit` map to the same number, namely 7848. In
these cases the (\#)next button lights up. By pressing it, you can cycle
through the list of words sharing the same code and finally accept the
correct one by pressing (0)space or the Ok button.

The second input method (123) is the numeric input: Each key you press
is entered verbatim as a number.

The third input method (Abc) is the Multi-tap Alpha input mode. This
mode is intended for entering free text, without assistance from the
dictionary scheme of predictive (Pre) mode. The text is entered one
letter at the time. For each letter first press the number of the set
which contains the letter you want, then use the (\#)next button to
cycle through the letters and repeat with another number. For example,
to enter the word `look` you must press the following:
5\#\#6\#\#6\#\#5\#

The dialog is fully usable with the mouse, but a few provisions have
been made in some ScummVM ports to make its use more comfortable by
naturally mapping the functionality to the numeric keypad. Also, the
dialog's buttons can be navigated with the arrow and the enter keys.

### 3.26) Sierra SCI games: Simultaneous speech and subtitles

Certain CD versions of Sierra SCI games had both speech and text
resources. Some have an option to toggle between the two, but there are
some cases where there wasn't any option to enable both simultaneously.
In ScummVM, it is possible to enjoy a combined mode, where both speech
and text are shown at the same time. This mode can be toggled in the
ScummVM audio options, but each game has different behavior in-game
regarding speech and text toggling.

The CD games where speech and subtitles can be shown simultaneously are:

  - EcoQuest 1 CD
  - Freddy Pharkas CD
  - Gabriel Knight CD
  - King's Quest 6 CD
  - King's Quest VII CD
  - Laura Bow 2 CD
  - Leisure Suit Larry 6 CD
  - Leisure Suit Larry 6 (hires) CD
  - Police Quest 4 CD
  - Shivers CD
  - Space Quest 4 CD
  - Space Quest 6 CD
  - Torin's Passage CD

**EcoQuest 1 CD:** Speech and text can be toggled via the game's "Mode"
option in the options dialog, or via ScummVM's audio options.

**Freddy Pharkas CD:** There is no in-game option to toggle speech and
text. Only ScummVM's audio options can be used to toggle this feature.
Note that some spoken dialog is missing from the game texts.

**Gabriel Knight CD:** Speech and text can be toggled via the "Text" and
"Voice" buttons in the game's settings dialog, or via ScummVM's audio
options.

**King's Quest 6 CD:** Speech and text can be toggled via the "Mode"
button in the options dialog (with an extra "Dual" setting added in
ScummVM), or via ScummVM's audio options.

**King's Quest VII CD:** There is no in-game option to toggle speech and
text. Only ScummVM's audio options can be used to toggle this feature.
Note that the subtitles were disabled in the official release of this
game, so some subtitles may be wrong or missing.

**Laura Bow 2 CD:** Speech and text can be toggled via the "Mode" button
in the options dialog (with an extra "Dual" setting added in ScummVM),
or via ScummVM's audio options.

**Leisure Suit Larry 6 CD:** Either speech only or speech and text can
be selected. There is no in-game option to toggle text only. Only
ScummVM's audio options can be used to enable the text only mode.

**Leisure Suit Larry 6 (hires) CD:** Text can be toggled by selecting
the "Text On/Off" option from the in-game "Game" menu, or via ScummVM's
audio options. Speech cannot be disabled.

**Police Quest 4 CD:** Either speech only or text only can be selected
from the game's settings dialog. Only ScummVM's audio options can be
used to enable text+speech mode.

**Shivers CD:** Text can be toggled by selecting the "Text" option from
the game's settings dialog, or via ScummVM's audio options. Note that
only videos have subtitles in this game.

**Space Quest 4 CD:** Speech and text can be toggled via the "Display
Mode" button in the options dialog, or via ScummVM's audio options.

**Space Quest 6 CD:** Speech and text can be toggled via the "Speech"
and "Text" buttons in the game's settings dialog, or via ScummVM's audio
options.

**Torin's Passage CD:** Text can be toggled by selecting "Closed
Captioning" from the in-game "Game" menu. Speech can be disabled by
selecting "Audio Mixer" from the in-game "Game" menu and setting the
speech volume to zero.

### 3.27) Zork games notes

To run the supported Zork games (Zork Nemesis: The Forbidden Lands and
Zork: Grand Inquisitor) you need to copy some (extra) data to its
corresponding destination.

#### 3.27.1) Zork Nemesis: The Forbidden Lands

Download the Liberation(tm) fonts package
<https://releases.pagure.org/liberation-fonts/liberation-fonts-ttf-2.00.1.tar.gz>
and unpack all the ttf files into your ScummVM extras directory.
Alternatively, ScummVM comes with the GNU FreeFont fonts, though at the
time of writing these fonts cause some text rendering issues. Download
the subtitles patch <https://www.thezorklibrary.com/installguides/znpatch.zip>
and unzip the addon directory into the game root directory

#### 3.27.2) Zork: Grand Inquisitor

Download the Liberation(tm) fonts package
<https://releases.pagure.org/liberation-fonts/liberation-fonts-ttf-2.00.1.tar.gz>
and unpack all the ttf files into your ScummVM extras directory.
Alternatively, ScummVM comes with the GNU FreeFont fonts, though at the
time of writing these fonts cause some text rendering issues.

### 3.28) Commodore64 games notes

Both Maniac Mansion and Zak McKracken run but Maniac Mansion is not yet
playable. Simply name the D64 disks "maniac1.d64" and "maniac2.d64"
respectively "zak1.d64" and "zak2.d64", then ScummVM should be able to
automatically detect the game if you point it at the right directory.

Alternatively, you can use `extract_mm_c64` from the tools package to
extract the data files. But then the game will not be properly
autodetected by ScummVM, and you must make sure that the platform is set
to Commodore64. We recommend using the much simpler approach described
in the previous paragraph.

### 3.29) Macintosh games notes

All LucasArts SCUMM based adventures, except COMI, also exist in
versions for the Macintosh. ScummVM can use most (all?) of them,
however, in some cases some additional work is required. First off, if
you are not using a Macintosh for this, accessing the CD/floppy data
might be tricky. The reason for this is that the mac uses a special disk
format called HFS which other systems usually do not support. However,
there are various free tools which allow reading such HFS volumes. For
example `HFSExplorer` for Windows and `hfsutils` for Linux and other
Unix-like operating systems.

Most of the newer games on the Macintosh shipped with only a single data
file (note that in some cases this data file was made invisible, so you
may need extra tools in order to copy it). ScummVM is able to directly
use such a data file; simply point ScummVM at the directory containing
it, and it should work (just like with every other supported game).

We also provide a tool called `extract_scumm_mac` in the tools package
to extract the data from these data files, but this is neither required
nor recommended.

For further information on copying Macintosh game files to your hard
disk see:

<https://wiki.scummvm.org/index.php/HOWTO-Mac_Games>

## 4.0) Supported Platforms

ScummVM has been ported to run on many platforms and operating systems.
Links to these ports can be found either on the ScummVM web page or by a
Google search. Many thanks to our porters for their efforts. If you have
a port of ScummVM and wish to commit it into the master git, feel free
to contact us\!

Supported platforms include (but are not limited to):

  - UNIX (Linux, Solaris, IRIX, \*BSD, ...)
  - Windows
  - Mac OS X
  - AmigaOS
  - Android
  - Atari/FreeMiNT
  - BeOS
  - Dreamcast
  - GP2x
  - Haiku
  - iPhone (also includes iPod Touch and iPad)
  - Maemo (Nokia Internet tablet N810)
  - Nintendo 64
  - Nintendo DS
  - Nintendo GameCube
  - Nintendo Wii
  - OpenPandora
  - OS/2
  - PlayStation 2
  - PlayStation 3
  - PlayStation Portable
  - PlayStation Vita
  - Raspberry Pi
  - RISC OS
  - Symbian
  - WebOS

The Dreamcast port does not support The Curse of Monkey Island, nor The
Dig. The Nintendo DS port does not support Full Throttle, The Dig, or
The Curse of Monkey Island. For more platform specific limitations,
please refer to our Wiki:

<https://wiki.scummvm.org/index.php/Platforms>

In the Macintosh port, the right mouse button is emulated via Cmd-Click
(that is, you click the mouse button while holding the
Command/Apple/Propeller key).

There are unofficial ports to a variety of platforms, including the
Xbox, and Xbox 360. Please note that these are not made
by us, so we neither endorse nor can we support them. Use at your own
risk\!

## 5.0) Running ScummVM

Please note that by default, ScummVM will save games in the directory it
is executed from, so you should refrain from running it from more than
one location. Further information, including how to specify a specific
save directory to avoid this issue, are in section 6.0.

ScummVM can be launched directly by running the executable. In this
case, the built-in launcher will activate. From this, you can add games
(click 'Add Game'), or launch games which have already been configured.
Games can also be added in mass quantities. By pressing shift + 'Add
Game' (Note that the image turns to 'Mass Add'), you can then specify a
directory to start in, and ScummVM will attempt to detect games in all
subdirectories of that directory.

ScummVM can also be launched into a game directly using Command Line
arguments -- see the next section.

### 5.1) Command Line Options

    Usage: scummvm [OPTIONS]... [GAME]

    [GAME]                   Short name of game to load. For example, 'scumm:monkey'
                              for Monkey Island. This can be either a built-in
                              gameid, or a user configured target.

    -v, --version            Display ScummVM version information and exit
    -h, --help               Display a brief help text and exit
    -z, --list-games         Display list of supported games and exit
    -t, --list-targets       Display list of configured targets and exit
    --list-engines           Display list of suppported engines and exit
    --list-saves             Display a list of saved games for the target specified
                              with --game=TARGET, or all targets if none is specified
    -a, --add                Add all games from current or specified directory.
                              If --game=ID is passed only the game with id ID is
                              added. See also --detect.
                              Use --path=PATH to specify a directory.
    --detect                 Display a list of games with their ID from current or
                              specified directory without adding it to the config.
                              Use --path=PATH to specify a directory.
    --game=ID                In combination with --add or --detect only adds or attempts to
                              detect the game with id ID.
    --auto-detect            Display a list of games from current or specified directory
                              and start the first one. Use --path=PATH to specify
                              a directory.
    --recursive              In combination with --add or --detect recurse down all
                              subdirectories
    --console                Enable the console window (default: enabled) (Windows only)

    -c, --config=CONFIG      Use alternate configuration file
    -p, --path=PATH          Path to where the game is installed
    -x, --save-slot[=NUM]    Saved game slot to load (default: autosave)
    -f, --fullscreen         Force full-screen mode
    -F, --no-fullscreen      Force windowed mode
    -g, --gfx-mode=MODE      Select graphics scaler (see also section 5.3)
    --stretch-mode=MODE      Select stretch mode (center, integral, fit, stretch)
    --filtering              Force filtered graphics mode
    --no-filtering           Force unfiltered graphics mode


    --gui-theme=THEME        Select GUI theme (default, modern, classic)
    --themepath=PATH         Path to where GUI themes are stored
    --list-themes            Display list of all usable GUI themes
    -e, --music-driver=MODE  Select music driver (see also section 7.0)
    --list-audio-devices     List all available audio devices
    -q, --language=LANG      Select game's language (see also section 5.5)
    -m, --music-volume=NUM   Set the music volume, 0-255 (default: 192)
    -s, --sfx-volume=NUM     Set the sfx volume, 0-255 (default: 192)
    -r, --speech-volume=NUM  Set the voice volume, 0-255 (default: 192)
    --midi-gain=NUM          Set the gain for MIDI playback, 0-1000 (default: 100)
                              (only supported by some MIDI drivers)
    -n, --subtitles          Enable subtitles (use with games that have voice)
    -b, --boot-param=NUM     Pass number to the boot script (boot param)
    -d, --debuglevel=NUM     Set debug verbosity level
    --debugflags=FLAGS       Enable engine specific debug flags
                              (separated by commas)
    -u, --dump-scripts       Enable script dumping if a directory called 'dumps'
                              exists in the current directory

    --cdrom=NUM              CD drive to play CD audio from (default: 0 = first
                              drive)
    --joystick[=NUM]         Enable joystick input (default: 0 = first joystick)
    --platform=WORD          Specify platform of game (allowed values: 2gs, 3do,
                              acorn, amiga, atari, c64, fmtowns, mac, nes, pc,
                              pce, segacd, windows)
    --savepath=PATH          Path to where saved games are stored
    --extrapath=PATH         Extra path to additional game data
    --soundfont=FILE         Select the SoundFont for MIDI playback (Only
                              supported by some MIDI drivers)
    --multi-midi             Enable combination of AdLib and native MIDI
    --native-mt32            True Roland MT-32 (disable GM emulation)
    --dump-midi              Dumps MIDI events to 'dump.mid', until quitting from game
                              (if file already exists, it will be overwritten)
    --enable-gs              Enable Roland GS mode for MIDI playback
    --output-rate=RATE       Select output sample rate in Hz (e.g. 22050)
    --opl-driver=DRIVER      Select AdLib (OPL) emulator (db, mame, nuked)
    --aspect-ratio           Enable aspect ratio correction
    --render-mode=MODE       Enable additional render modes (hercGreen, hercAmber,
                              cga, ega, vga, amiga, fmtowns, pc9821, pc9801, 2gs,
                              atari, macintosh)

    --alt-intro              Use alternative intro for CD versions of Beneath a
                              Steel Sky and Flight of the Amazon Queen
    --copy-protection        Enable copy protection in games, when
                              ScummVM disables it by default.
    --talkspeed=NUM          Set talk delay for SCUMM games, or talk speed for
                              other games (default: 60)
    --demo-mode              Start demo mode of Maniac Mansion (Classic version)
    --tempo=NUM              Set music tempo (in percent, 50-200) for SCUMM games
                              (default: 100)

The meaning of most long options (that is, those options starting with a
double-dash) can be inverted by prefixing them with "no-". For example,
`--no-aspect-ratio` will turn aspect ratio correction off. This is
useful if you want to override a setting in the configuration file.

The short game name ('game target') you see at the end of the command
line specifies which game is started. It either corresponds to an
arbitrary user defined target (from the configuration file), or to a
built-in gameid. A brief list of the latter can be found in section 3.0.

Examples:

  - Win32:
    Running Monkey Island, fullscreen, from a hard disk:
    `C:\Games\LucasArts\scummvm.exe -f -pC:\Games\LucasArts\monkey\
    scumm:monkey`
    Running Full Throttle from CD, fullscreen and with subtitles
    enabled:
    `C:\Games\LucasArts\scummvm.exe -f -n -pD:\resource\ scumm:ft`

  - Unix:
    Running Monkey Island, fullscreen, from a hard disk:
    `/path/to/scummvm -f -p/games/LucasArts/monkey/ scumm:monkey`
    Running Full Throttle from CD, fullscreen and with subtitles
    enabled:
    `/path/to/scummvm -f -n -p/cdrom/resource/ scumm:ft`

### 5.2) Global Menu

The Global Menu is a general menu which is available to all of the game
engines by pressing Ctrl-F5. From this menu there are the following
buttons: Resume, Options, About, Return to Launcher, and Quit. Selecting
`Options` will display a dialog where basic audio settings, such as
volume levels, can be adjusted. Selecting 'Return to Launcher' will
close the current game and return the user back to the ScummVM Launcher,
where another game may be selected to play.

Note: Returning to the Launcher is not supported by all of the engines,
and the button will be disabled in the Global Menu if it is not
supported.

Engines which currently support returning to the Launcher are:

    AGI
    AGOS
    BLADERUNNER
    CINE
    COMPOSER
    CRUISE
    CRYOMNI3D
    DRACI
    DRASCULA
    GOB
    GROOVIE
    HUGO
    KYRA
    LURE
    MADE
    MOHAWK
    PARALLACTION
    QUEEN
    SAGA
    SCI
    SCUMM
    SKY
    SWORD1
    SWORD2
    TEENAGENT
    TITANIC
    TOUCHE
    TSAGE
    TUCKER
    ZVISION

### 5.3) Graphics filters

ScummVM offers several anti-aliasing filters to attempt to improve
visual quality. These are the same filters used in many other emulators,
such as MAME. These filters take the original game graphics, and scale
it by a certain fixed factor (usually 2x or 3x) before displaying them
to you. So for example, if the game originally run at a resolution of
320x200 (typical for most of the SCUMM games), then using a filter with
scale factor 2x will effectively yield 640x400 graphics. Likewise with a
3x filter you will get 960x600.

They are:

    1x         - No filtering, no scaling. Fastest.
    2x         - No filtering, factor 2x (default for non 640x480 games).
    3x         - No filtering, factor 3x.
    2xsai      - 2xSAI filter, factor 2x.
    super2xsai - Enhanced 2xSAI filtering, factor 2x.
    supereagle - Less blurry than 2xSAI, but slower. Factor 2x.
    advmame2x  - Doesn't rely on blurring like 2xSAI, fast. Factor 2x.
    advmame3x  - Doesn't rely on blurring like 2xSAI, fast. Factor 3x.
    hq2x       - Very nice high quality filter but slow. Factor 2x.
    hq3x       - Very nice high quality filter but slow. Factor 3x.
    tv2x       - Interlace filter, tries to emulate a TV. Factor 2x.
    dotmatrix  - Dot matrix effect. Factor 2x.

To select a graphics filter, select it in the Launcher, or pass its name
via the '-g' option to scummvm, for example:

    scummvm -gadvmame2x scumm:monkey2

Note \#1: Not all backends support all (or even any) of the filters
listed above; some may support additional ones. The filters listed above
are those supported by the default SDL backend.

Note \#2: Filters can be very slow when ScummVM is compiled in a debug
configuration without optimizations. And there is always a speed impact
when using any form of anti-aliasing/linear filtering.

Note \#3: The FM-TOWNS version of Zak McKracken uses an original
resolution of 320x240, hence for this game scalers will scale to 640x480
or 960x720. Likewise, games that originally were using 640x480 (such as
Curse of Monkey Island or Broken Sword) will be scaled to 1280x960 and
1920x1440.

### 5.4) Hotkeys

ScummVM supports various in-game hotkeys. They differ between SCUMM
games and other games.

```
  Common:
    Ctrl-F5                - Displays the Global Menu
    Cmd-q                  - Quit (Mac OS X)
    Ctrl-q                 - Quit (other unices including Linux)
    Alt-F4                 - Quit (Windows)
    Ctrl-z                 - Quit (other platforms)
    Ctrl-u                 - Mute all sounds
    Ctrl-m                 - Toggle mouse capture
    Ctrl-Alt 1-8           - Switch between graphics filters
    Ctrl-Alt + and -       - Increase/Decrease the scale factor
    Ctrl-Alt a             - Toggle aspect-ratio correction on/off
                             Most of the games use a 320x200 pixel
                             resolution, which may look squashed on
                             modern monitors. Aspect-ratio correction
                             stretches the image to use 320x240 pixels
                             instead, or a multiple thereof
    Ctrl-Alt f             - Enable/disable graphics filtering
    Ctrl-Alt s             - Cycle through scaling modes
    Alt-Enter              - Toggles full screen/windowed
    Alt-s                  - Make a screenshot (SDL backend only)
    Ctrl-F7                - Open virtual keyboard (if enabled)
                             This can also be triggered by a long press
                             of the middle mouse button or wheel.

  SCUMM:
    Alt-x                  - Quit
    Ctrl 0-9 and Alt 0-9   - Load and save game state
    Ctrl-d                 - Starts the debugger
    Ctrl-f                 - Toggle fast mode
    Ctrl-g                 - Runs in really REALLY fast mode
    Ctrl-t                 - Switch between 'Speech only',
                             'Speech and Subtitles' and 'Subtitles only'
    Tilde (~)              - Show/hide the debugging console
    [ and ]                - Music volume, down/up
    - and +                - Text speed, slower/faster
    F5                     - Displays a save/load box
    Alt-F5                 - Displays the original save/load box, if the
                             game has one. You can save and load games using
                             this, however it is not intended for this purpose,
                             and may even crash ScummVM in some games.
    i                      - Displays IQ points (Indiana Jones and the Last
                             Crusade, and Indiana Jones and the Fate of
                             Atlantis)
    Space                  - Pauses
    Period (.)             - Skips current line of text in some games
    Enter                  - Simulate left mouse button press
    Tab                    - Simulate right mouse button press

  AGI/SCI (Sierra):
    Ctrl-Shift-d           - Starts the debugger

  Beneath a Steel Sky:
    Ctrl-d                 - Starts the debugger
    Ctrl-f                 - Toggle fast mode
    Ctrl-g                 - Runs in really REALLY fast mode
    F5                     - Displays a save/load box
    Escape                 - Skips the game intro
    Period (.)             - Skips current line of text

  Blade Runner:
    Ctrl-d                 - Starts the debugger
    Space                  - Toggle combat mode
    Enter or Escape        - Skips a video or a line of dialogue
    Escape                 - Show game menu
    Tab                    - Activate last database
    F1                     - Online help
    F2                     - Save game menu
    F3                     - Load game menu
    F4                     - Crime scene database
    F5                     - Suspect database
    F6                     - Clue database
    F10                    - Quit game

  Broken Sword:
    F5 or Escape           - Displays save/load box

  Broken Sword II:
    Ctrl-d                 - Starts the debugger
    Ctrl-f                 - Toggle fast mode
    p                      - Pauses

  Dragon History:
    F5                     - Displays the Global Menu
    left click             - Walk, explore
    right click            - Use, talk
    move mouse up, i       - Inventory
    move mouse down, m     - Map
    Escape                 - Skip the intro, exit map/inventory
    any click              - Skip the currently dubbed sentence
    q                      - Quick walking on/off

  Flight of the Amazon Queen:
    Ctrl-d                 - Starts the debugger
    Ctrl-f                 - Toggle fast mode
    F1                     - Use Journal (saving/loading)
    F11                    - Quicksave
    F12                    - Quickload
    Escape                 - Skips cutscenes
    Space                  - Skips current line of text

  Future Wars:
    F1                     - Examine
    F2                     - Take
    F3                     - Inventory
    F4                     - Use
    F5                     - Activate
    F6                     - Speak
    F9                     - "Activate" menu
    F10                    - "Use" menu
    Escape                 - Bring on command menu

  Nippon Safes:
    Ctrl-d                 - Starts the debugger
    l                      - Load game
    s                      - Save game

  Simon the Sorcerer 1 and 2:
    Ctrl 0-9 and Alt 0-9   - Load and save game state
    Ctrl-d                 - Starts the debugger
    Ctrl-f                 - Toggle fast mode
    F1 - F3                - Text speed, faster - slower
    F10                    - Shows all characters and objects you can
                             interact with
    Escape                 - Skip cutscenes
    - and +                - Music volume, down/up
    m                      - Music on/off
    s                      - Sound effects on/off
    b                      - Background sounds on/off
                             [Simon the Sorcerer 2 only]
    Pause                  - Pauses
    t                      - Switch between speech only and
                             combined speech and subtitles
                             [Simon the Sorcerer 1 CD (other than
                             English and German) and Simon the
                             Sorcerer 2 CD (all languages)]
    v                      - Switch between subtitles only and
                             combined speech and subtitles
                             [Simon the Sorcerer 2 CD only]

  Simon the Sorcerer's Puzzle Pack:
    Ctrl-d                 - Starts the debugger
    Ctrl-f                 - Toggle fast mode
    F12                    - High speed mode on/off in Swampy Adventures
    - and +                - Music volume, down/up
    m                      - Music on/off
    s                      - Sound effects on/off
    Pause                  - Pauses

  Starship Titanic:
    Ctrl-c                 - Open up the developer's cheat room
    Ctrl-d                 - Open up the ScummVM Debugger
    Left click             - Move action
    Shift-Left click       - Edit room glyph chevrons and
                              quick movement transitions
    Right click            - Edit room glyph chevrons
                              and quick transitions
    Mouse wheel            - Scroll through items (inventory, etc)
                              and conversation log
    Arrow keys             - Movement. Down arrow/back is only available if the
                              given view explicitly has a backwards movement
                              available.
    F1                     - Switch to Chat-O-Mat
    F2                     - Switch to Personal Baggage
    F3                     - Switch to Remote Thingummy
    F4                     - Switch to Designer Room Numbers (chevron list)
    F5                     - GMM save menu
    F6                     - Switch to Real Life
    F7                     - GMM restore menu

  Starship Titanic (Starfield Puzzle):
    Tab                    - Toggle between starmap and skyscape
    Mouse click:           - skyscape star selection and
                              starmap star fast travel
    Mouse movement         - starmap orientation
    SPACE                  - starmap stop movement
    z                      - starmap turn left
    x                      - starmap turn right
    Single quote (')       - starmap turn up
    Forward slash (/)      - starmap turn down
    Semicolon (;)          - starmap move forward
    Period (.)             - starmap move backward
    l                      - starmap lock coordinate
    d                      - starmap unlock coordinate
	b                      - starmap show boundary sphere
	c                      - starmap show constellations sphere

  The Feeble Files:
    Ctrl-d                 - Starts the debugger
    Ctrl-f                 - Toggle fast mode
    F7                     - Switch characters
    F9                     - Hitbox names on/off
    s                      - Sound effects on/off
    Pause                  - Pauses
    t                      - Switch between speech only and
                             combined speech and subtitles
    v                      - Switch between subtitles only and
                             combined speech and subtitles

  The Legend of Kyrandia:
    Ctrl 0-9 and Alt 0-9   - Load and save game state
    Ctrl-d                 - Starts the debugger

  TeenAgent
    F5                     - Displays the Global Menu

  Touche: The Adventures of the Fifth Musketeer:
    Ctrl-f                 - Toggle fast mode
    F5                     - Displays options
    F9                     - Turn fast walk mode on
    F10                    - Turn fast walk mode off
    Escape                 - Quit
    Space                  - Skips current line of text
    t                      - Switch between 'Voice only',
                             'Voice and Text' and 'Text only'

  Zork: Grand Inquisitor:
    Ctrl-s                 - Save
    Ctrl-r                 - Restore
    Ctrl-q                 - Quit
    Ctrl-p                 - Preferences
    F1                     - Help
    F5                     - Inventory
    F6                     - Spellbook
    F7                     - Score
    F8                     - Put away current object/forget spell
    F9                     - Extract coin (must have the coin bag)
    Space                  - Skips movies

  Zork Nemesis: The Forbidden Lands:
    Ctrl-s                 - Save
    Ctrl-r                 - Restore
    Ctrl-q                 - Quit
    Ctrl-p                 - Preferences
    Space                  - Skips movies
```

Note that using Ctrl-f or Ctrl-g is not recommended: games can crash
when being run faster than their normal speed, as scripts will lose
synchronisation.

### 5.5) Language options

ScummVM includes a language option for Maniac Mansion, Zak McKracken,
The Dig, The Curse of Monkey Island, Beneath a Steel Sky and Broken
Sword.

Note that with the exception of Beneath a Steel Sky, Broken Sword,
multilanguage versions of Goblins games and Nippon Safes Inc., using
this option does *not* change the language of the game (which usually is
hardcoded), but rather is only used to select the appropriate font (e.g.
for a German version of a game, one containing umlauts).

An exception are The Dig and The Curse of Monkey Island -- non-English
versions can be set to 'English.' This however only affects subtitles;
game speech will remain the same.

    Maniac Mansion and Zak McKracken
        en  - English (default)
        de  - German
        fr  - French
        it  - Italian
        es  - Spanish

    The Dig
        jp  - Japanese
        zh  - Chinese
        kr  - Korean

    The Curse of Monkey Island
        en  - English (default)
        de  - German
        fr  - French
        it  - Italian
        pt  - Portuguese
        es  - Spanish
        jp  - Japanese
        zh  - Chinese
        kr  - Korean

    Beneath a Steel Sky
        gb  - English (Great Britain) (default)
        en  - English (USA)
        de  - German
        fr  - French
        it  - Italian
        pt  - Portuguese
        es  - Spanish
        se  - Swedish

    Broken Sword
        en  - English (default)
        de  - German
        fr  - French
        it  - Italian
        es  - Spanish
        pt  - Portuguese
        cz  - Czech

## 6.0) Saved Games

Saved games are by default put in the current directory on some
platforms and preset directories on others. You can specify the save in
the config file by setting the savepath parameter. See the example
config file later in this README.

The platforms that currently have a different default directory are:

**Mac OS X:**

`$HOME/Documents/ScummVM Savegames/`

**Other unices:**

We follow the XDG Base Directory Specification. This means by default
saved games can be found in: `$XDG_DATA_HOME/scummvm/saves/`

If `XDG_DATA_HOME` is not defined or empty, `~/.local/share` will be
used as value of `XDG_DATA_HOME` in accordance with the specification.

If an earlier version of ScummVM was installed on your system, the
previous default location of `~/.scummvm` will be kept. This is detected
based on the presence of the path `~/.scummvm`.

**Windows Vista/7:**

`\Users\username\AppData\Roaming\ScummVM\Saved games\`

**Windows 2000/XP:**

`\Documents and Settings\username\Application Data\ScummVM\Saved games\`

**Windows NT4:**

`<windir>\Profiles\username\Application Data\ScummVM\Saved games\`

Saved games are stored under a hidden area in Windows
NT4/2000/XP/Vista/7, which can be accessed by running
`%APPDATA%\ScummVM\Saved Games` or by enabling hidden files in Windows
Explorer.

Note for Windows NT4/2000/XP/Vista/7 users: The default saved games
location changed in ScummVM 1.5.0. The migration batch file can be used
to copy saved games from the old default location, to the new default
location.

### 6.1) Autosaves

For some games ScummVM will by default automatically save the current
state every five minutes (adjustable via the `autosave_period` config
setting). The default autosave slot for many engines is slot 0.

The games/engines listed below have autosave support.

  - AGI games
  - Beneath a Steel Sky
  - Bud Tucker in Double Trouble
  - COMPOSER games
  - Flight of the Amazon Queen
  - Myst
  - Riven
  - SCUMM games
  - The Legend of Kyrandia I (slot 999)
  - ZVISION games

For the SCUMM engine, this saved game can then be loaded again via
Ctrl-0, or the F5 menu.

### 6.2) Converting Saved Games

Using saved games from original versions isn't supported by all game
engines. Only the following games can use saved games from their
original versions.

  - Blade Runner

      - Use the debugger console and command "save" to save the game to
        the original format and command "load" to load such a one
      - Saved games between different languages are interchangeable
      - It is not recommended to convert saved games from the version
        with restored content as they might behave unexpectedly
        or might cause game breaking bugs

  - Elvira 1

      - Add 8 bytes (saved game name) to the start of the saved game
        file
      - Rename the saved game to `elvira1.xxx`

  - Elvira 2

      - Add 8 bytes (saved game name) to the start of the saved game
        file
      - Rename the saved game to `elvira2-pc.xxx` (DOS version) or
        `elvira2.xxx` (Other versions)

  - Myst

      - Rename the saved game to `myst-xxx.mys`
      - Saves from the masterpiece edition and the regular edition are
        interchangeable

  - Riven

      - Rename the saved game to `riven-xxx.rvn`
      - Saves from the CD and DVD edition are not interchangeable

  - Simon the Sorcerer 1

      - Rename the saved game to `simon1.xxx`

  - Simon the Sorcerer 2

      - Rename the saved game to `simon2.xxx`

  - Starship Titanic

      - Rename the saved game to `titanic-win.xxx` for saves from the
        English version and `titanic-win-de.xxx` for saves from the
        German version
      - Saved games between different languages are not interchangeable

  - The Feeble Files

      - Rename the saved game to `feeble.xxx`

  - Waxworks

      - Add 8 bytes (saved game name) to the start of the saved game
        file
      - Rename the saved game to `waxworks-pc.xxx` (DOS version) or
        `waxworks.xxx` (Other versions)

Where `xxx` is exact the saved game slot (i.e., 001) under ScummVM

### 6.3) Viewing/Loading saved games from the command line

**`--list-saves`**

This switch may be used to display a list of the current saved games of
the specified target game and their corresponding save slots. If no
target is specified, it lists saved games for all known target.

Usage: `--list-saves --game=[TARGET]`, where \[TARGET\] is the target
game.

Engines which currently support `--list-saves` are:

  - AGI
  - AGOS
  - BLADERUNNER
  - CGE
  - CINE
  - CRUISE
  - CRYOMNI3D
  - DRACI
  - GROOVIE
  - HUGO
  - KYRA
  - LURE
  - MOHAWK
  - PARALLACTION
  - QUEEN
  - SAGA
  - SCI
  - SCUMM
  - SKY
  - SWORD1
  - SWORD2
  - TEENAGENT
  - TINSEL
  - TITANIC
  - TOON
  - TOUCHE
  - TSAGE
  - TUCKER
  - ZVISION

**`--save-slot/-x`**

This switch may be used to load a saved game directly from the command
line.

Usage: `--save-slot[SLOT]` or `-x[SLOT]`, where \[SLOT\] is the save
slot number.

Engines which currently support `--save-slot` / `-x are`:

  - AGI
  - BLADERUNNER
  - CGE
  - CINE
  - CRUISE
  - CRYOMNI3D
  - DRACI
  - GROOVIE
  - HUGO
  - KYRA
  - LURE
  - MOHAWK
  - QUEEN
  - SAGA
  - SCI
  - SCUMM
  - SKY
  - SWORD1
  - SWORD2
  - TEENAGENT
  - TINSEL
  - TITANIC
  - TOON
  - TOUCHE
  - TSAGE
  - TUCKER
  - ZVISION

## 7.0) Music and Sound

On most operating systems and for most games, ScummVM will by default
use MT-32 or AdLib emulation for music playback. MIDI may not be
available on all operating systems or may need manual configuration. If
you want to use MIDI, you have several different choices of output,
depending on your operating system and configuration.

    null       - Null output. Don't play any music.

    adlib      - Internal AdLib emulation
    fluidsynth - FluidSynth MIDI emulation
    mt32       - Internal MT-32 emulation
    pcjr       - Internal PCjr emulation (only usable in SCUMM games)
    pcspk      - Internal PC Speaker emulation
    towns      - Internal FM-TOWNS YM2612 emulation
                 (only usable in SCUMM FM-TOWNS games)

    alsa       - Output using ALSA sequencer device. See below.
    core       - CoreAudio sound, for Mac OS X users.
    coremidi   - CoreMIDI sound, for Mac OS X users. Use only if you have
                 a hardware MIDI synthesizer.
    seq        - Use /dev/sequencer for MIDI, *nix users. See below.
    timidity   - Connect to TiMidity++ MIDI server. See below.
    windows    - Windows MIDI. Uses built-in sequencer, for Windows users

To select a sound driver, select it in the Launcher, or pass its name
via the `-e` option to scummvm, for example:

`scummvm -eadlib scumm:monkey2`

### 7.1) AdLib emulation

By default an AdLib card will be emulated and ScummVM will output the
music as sampled waves. This is the default mode for several games, and
offers the best compatibility between machines and games.

### 7.2) FluidSynth MIDI emulation

If ScummVM was build with libfluidsynth support it will be able to play
MIDI music through the FluidSynth driver. You will have to specify a
SoundFont to use, however.

Since the default output volume from FluidSynth can be fairly low,
ScummVM will set the gain by default to get a stronger signal. This can
be further adjusted using the `--midi-gain` command-line option, or the
`midi_gain` config file setting.

The setting can take any value from 0 through 1000, with the default
being 100. (This corresponds to FluidSynth's gain settings of 0.0
through 10.0, which are presumably measured in decibel.)

NOTE: The processor requirements for FluidSynth can be fairly high in
some cases. A fast CPU is recommended.

### 7.3) MT-32 emulation

Some games which contain MIDI music data also have improved tracks
designed for the MT-32 sound module. ScummVM can now emulate this
device, however you must provide original MT-32 ROMs to make it work:

`MT32_PCM.ROM` - IC21 (512KB)
`MT32_CONTROL.ROM` - IC26 (32KB) and IC27 (32KB), interleaved byte-wise

Place these ROMs in the game directory, in your extrapath, or in the
directory where your ScummVM executable resides.

You don't need to specify `--native-mt32` with this driver, as it
automatically gets turned on.

NOTE: The processor requirements for the emulator are quite high; a fast
CPU is strongly recommended.

### 7.4) MIDI emulation

Some games (such as Sam & Max) only contain MIDI music data. This once
prevented music for these games from working on platforms that do not
support MIDI, or soundcards that do not provide MIDI drivers (e.g. many
soundcards will not play MIDI under Linux). ScummVM can now emulate MIDI
mode using sampled waves and AdLib, FluidSynth MIDI emulation or MT-32
emulation using the `-eadlib`, `-efluidsynth` or `-emt32` options
respectively. However, if you are capable of using native MIDI, we
recommend using one of the MIDI modes below for best sound.

### 7.5) Native MIDI support

Use the appropriate `-e<mode>` command line option from the list above
to select your preferred MIDI device. For example, if you wish to use
the Windows MIDI driver, use the `-ewindows` option.

#### 7.5.1) Using MIDI options to customize Native MIDI output

ScummVM supports a variety of MIDI modes, depending on the capabilities
of your MIDI device.

If `--native-mt32` is specified, ScummVM will treat your device as a
real MT-32. Because the instrument mappings and system exclusive
commands of the MT-32 vary from those of General MIDI devices, you
should only enable this option if you are using an actual Roland MT-32,
LAPC-I, CM-64, CM-32L, CM-500, or GS device with an MT-32 map.

If `--enable-gs` is specified, ScummVM will initialize your
GS-compatible device with settings that mimic the MT-32's reverb, (lack
of) chorus, pitch bend sensitivity, etc. If it is specified in
conjunction with `--native-mt32`, ScummVM will select the
MT-32-compatible map and drumset on your GS device. This setting works
better than default GM or GS emulation with games that do not have
custom instrument mappings (Loom and Monkey1). You should only specify
both settings if you are using a GS device that has an MT-32 map, such
as an SC-55, SC-88, SC-88 Pro, SC-8820, SC-8850, etc. Please note that
`--enable-gs` is automatically disabled in both DOTT and Samnmax, since
they use General MIDI natively.

If neither of the above settings is enabled, ScummVM will initialize
your device in General MIDI mode and use GM emulation in games with
MT-32 soundtracks.

Some games contain sound effects that are exclusive to the AdLib
soundtrack. For these games, you may wish to specify `--multi-midi` in
order to combine MIDI music with AdLib sound effects.

### 7.6) UNIX native, ALSA and dmedia sequencer support

If your soundcard driver supports a sequencer, you may set the
environment variable `SCUMMVM_MIDI` to your sequencer device -- for
example, to /dev/sequencer

If you have problems with not hearing audio in this configuration, you
may need to set the environment variable `SCUMMVM_MIDIPORT` to 1 or 2.
This selects the port on the selected sequencer to use. Then start
scummvm with the `-eseq` parameter. This should work on several cards,
and may offer better performance and quality than AdLib emulation.
However, for those systems where sequencer support does not work, you
can always fall back on AdLib emulation.

#### 7.6.1) ALSA sequencer \[UNIX ONLY\]

If you have installed the ALSA driver with sequencer support, then you
may set the environment variable `SCUMMVM_PORT` or the config file
variable `alsa_port` to specify your sequencer port. If neither is set,
the default behavior is to try both "65:0" and "17:0".

Here is a brief guide on how to use the ALSA sequencer with your
soundcard. In all cases, to obtain a list of all the sequencer ports you
have, try the command `aconnect -o -l`. This should give output similar
to:

```
    client 14: 'Midi Through' [type=kernel]
        0 'Midi Through Port-0'
    client 16: 'SBLive! Value [CT4832]' [type=kernel]
        0 'EMU10K1 MPU-401 (UART)'
    client 17: 'Emu10k1 WaveTable' [type=kernel]
        0 'Emu10k1 Port 0  '
        1 'Emu10k1 Port 1  '
        2 'Emu10k1 Port 2  '
        3 'Emu10k1 Port 3  '
    client 128: 'TiMidity' [type=user]
        0 'TiMidity port 0 '
        1 'TiMidity port 1 '
        2 'TiMidity port 2 '
        3 'TiMidity port 3 '
```

The most important bit here is that there are four WaveTable MIDI
outputs located at 17:0, 17:1, 17:2 and 17:3, and four TiMidity ports
located at 128:0, 128:1, 128:2 and 128:3.

If you have a FM-chip on your card, like the SB16, then you have to load
the SoundFonts using the sbiload software. Example:

    sbiload -p 17:0 /etc/std.o3 /etc/drums.o3

If you have a WaveTable capable sound card, you have to load a sbk or
sf2 SoundFont using the sfxload or asfxload software. Example:

    sfxload /path/to/8mbgmsfx.sf2

If you don't have a MIDI capable soundcard, there are two options:
FluidSynth and TiMidity. We recommend FluidSynth, as on many systems
TiMidity will 'lag' behind music. This is very noticeable in
iMUSE-enabled games, which use fast and dynamic music transitions.
Running TiMidity as root will allow it to setup real time priority,
which may reduce music lag.

Asking TiMidity to become an ALSA sequencer:

    timidity -iAqqq -B2,8 -Os1S -s 44100 &

(If you get distorted output with this setting, you can try dropping the
-B2,8 or changing the value.)

Asking FluidSynth to become an ALSA sequencer (using SoundFonts):

    fluidsynth -m alsa_seq /path/to/8mbgmsfx.sf2

Once either TiMidity or FluidSynth are running, use the 'aconnect -o -l'
command as described earlier in this section.

#### 7.6.2) IRIX dmedia sequencer: \[UNIX ONLY\]

If you are using IRIX and the dmedia driver with sequencer support, you
can set the environment variable `SCUMMVM_MIDIPORT` or the config file
variable `dmedia_port` to specify your sequencer port. The default is to
use the first port.

To get a list of configured midi interfaces on your system, run
"startmidi" without parameters. Example output:

```
  2 MIDI interfaces configured:
          Serial Port 2
          Software Synth
```

In this example, you can configure ScummVM to use the "Software Synth"
instead of the default "Serial Port 2" by adding a line

    dmedia_port=Software Synth

to your configuration file in the section \[scummvm\], or setting
`SCUMMVM_PORT=Software Synth` in your environment.

### 7.7) TiMidity++ MIDI server support

If your system lacks any MIDI sequencer, but you still want better MIDI
quality than default AdLib emulation can offer, you can try the
TiMidity++ MIDI server. See <http://timidity.sourceforge.net/> for
download and install instructions.

First, you need to start a daemon:

    timidity -ir 7777

Now you can start ScummVM and try selection TiMidity music output. By
default, it will connect to localhost:7777, but you can change host/port
via the `TIMIDITY_HOST` environment variable. You can also specify a
"device number" using the `SCUMMVM_MIDIPORT` environment variable.

### 7.8) Using compressed audio files

#### 7.8.1) Using MP3 files for CD audio

Use LAME or some other MP3 encoder to rip the cd audio tracks to files.
Name the files track1.mp3 track2.mp3 etc. ScummVM must be compiled with
MAD support to use this option. You will need to rip the file from the
CD as a WAV file, then encode the MP3 files in constant bit rate. This
can be done with the following LAME command line:

    lame -t -q 0 -b 96 track1.wav track1.mp3

#### 7.8.2) Using Ogg Vorbis files for CD audio

Use oggenc or some other vorbis encoder to encode the audio tracks to
files. Name the files track1.ogg track2.ogg etc. ScummVM must be
compiled with vorbis support to use this option. You will need to rip
the files from the CD as a WAV file, then encode the vorbis files. This
can be done with the following oggenc command line with the value after
q specifying the desired quality from 0 to 10:

    oggenc -q 5 track1.wav

#### 7.8.3) Using Flac files for CD audio

Use flac or some other flac encoder to encode the audio tracks to files.
Name the files track1.flac track2.flac etc. If your filesystem only
allows three letter extensions, name the files track1.fla track2.fla
etc. ScummVM must be compiled with flac support to use this option. You
will need to rip the files from the CD as a WAV file, then encode the
flac files. This can be done with the following flac command line:

    flac --best track1.wav

Remember that the quality is always the same, varying encoder options
will only affect the encoding time and resulting filesize.

#### 7.8.4) Compressing MONSTER.SOU with MP3

You need LAME, and our `compress_scumm_sou` utility from the
scummvm-tools package to perform this task, and ScummVM must be compiled
with MAD support.

    compress_scumm_sou monster.sou

Eventually you will have a much smaller monster.so3 file, copy this file
to your game directory. You can safely remove the monster.sou file.

#### 7.8.5) Compressing MONSTER.SOU with Ogg Vorbis

As above, but ScummVM must be compiled with OGG support. Run:

    compress_scumm_sou --vorbis monster.sou

This should produce a smaller monster.sog file, which you should copy to
your game directory. Ogg encoding may take a considerable longer amount
of time than MP3, so have a good book handy.

#### 7.8.6) Compressing MONSTER.SOU with Flac

As above, but ScummVM must be compiled with Flac support. Run:

    compress_scumm_sou --flac monster.sou

This should produce a smaller monster.sof file, which you should copy to
your game directory. Remember that the quality is always the same,
varying encoder options will only affect the encoding time and resulting
file size. Playing with the blocksize (`-b <value>`), has the biggest
impact on the resulting file size -- 1152 seems to be a good value for
those kind of soundfiles. Be sure to read the encoder documentation
before you use other values.

#### 7.8.7) Compressing music/sfx/speech in AGOS games

Use our `compress_agos` utility from the scummvm-tools package to
perform this task. You can choose between multiple target formats, but
note that you can only use each if ScummVM was compiled with the
respective decoder support enabled.

```
  compress_agos effects     (For Acorn CD version of Simon 1)
  compress_agos simon       (For Acorn CD version of Simon 1)
  compress_agos effects.voc (For DOS CD version of Simon 1)
  compress_agos simon.voc   (For DOS CD version of Simon 1)
  compress_agos simon.wav   (For Windows CD version of Simon 1)
  compress_agos simon2.voc  (For DOS CD version of Simon 2)
  compress_agos simon2.wav  (For Windows CD version of Simon 2)
  compress_agos mac         (For Macintosh version of Simon 2)

  compress_agos voices1.wav (For Windows 2CD/4CD version of Feeble)
  compress_agos voices2.wav (For Windows 2CD/4CD version of Feeble)
  compress_agos voices3.wav (For Windows 4CD version of Feeble)
  compress_agos voices4.wav (For Windows 4CD version of Feeble)

  compress_agos Music       (For Windows version of Puzzle Pack)
```

For Ogg Vorbis add `--vorbis` to the options, i.e.

    compress_agos --vorbis

For Flac add `--flac` and optional parameters, i.e.

    compress_agos --flac

Eventually you will have a much smaller \*.mp3, \*.ogg or \*.fla file,
copy this file to your game directory. You can safely remove the old
file.

#### 7.8.8) Compressing speech/music in Broken Sword

The `compress_sword1` tool from the scummvm-tools package can encode
music and speech to MP3, Ogg Vorbis as well as Flac. The easiest way to
encode the files is simply copying the executable into your BS1
directory (together with the lame encoder) and run it from there. This
way, it will automatically encode everything to MP3. Afterwards, you can
manually remove the SPEECH?.CLU files and the wave music files.

Running `compress_sword1 --vorbis` will compress the files using Ogg
Vorbis instead of MP3.

Running `compress_sword1 --flac` will compress the files using Flac
instead of MP3.

Use `compress_sword1 --help` to get a full list of the options.

#### 7.8.9) Compressing speech/music in Broken Sword II

Use our `compress_sword2` utility from the scummvm-tools package to
perform this task. You can choose between multiple target formats, but
note that you can only use each if ScummVM was compiled with the
respective decoder support enabled.

```
  compress_sword2 speech1.clu
  compress_sword2 music1.clu
```

For Ogg Vorbis add --vorbis to the options, i.e.

    compress_sword2 --vorbis

Eventually you will have a much smaller \*.cl3 or \*.clg file, copy this
file to your game directory. You can safely remove the old file.

It is possible to use Flac compression by adding the `--flac` option.
However, the resulting \*.clf file will actually be larger than the
original.

Please note that `compress_sword2` will only work with the four
speech/music files in Broken Sword II. It will not work with any of the
other \*.clu files, nor will it work with the speech files from Broken
Sword.

### 7.9) Output sample rate

The output sample rate tells ScummVM how many sound samples to play per
channel per second. There is much that could be said on this subject,
but most of it would be irrelevant here. The short version is that for
most games 22050 Hz is fine, but in some cases 44100 Hz is preferable.
On extremely low-end systems you may want to use 11025 Hz, but it is
unlikely that you have to worry about that.

To elaborate, most of the sounds ScummVM has to play were sampled at
either 22050 Hz or 11025 Hz. Using a higher sample rate will not
magically improve the quality of these sounds. Hence, 22050 Hz is fine.

Some games use CD audio. If you use compressed files for this, they are
probably sampled at 44100 Hz, so for these games that may be a better
choice of sample rate.

When using the AdLib, FM Towns, PC Speaker or IBM PCjr music drivers,
ScummVM is responsible for generating the samples. Usually 22050 Hz will
be plenty for these, but there is at least one piece of AdLib music in
Beneath a Steel Sky that will sound a lot better at 44100 Hz.

Using frequencies in between is not recommended. For one thing, your
sound card may not support it. In theory, ScummVM should fall back on a
sensible frequency in that case, but don't count on it. More
importantly, ScummVM has to resample all sounds to its output frequency.
This is much easier to do well if the output frequency is a multiple of
the original frequency.

## 8.0) Configuration file

By default, the configuration file is saved in, and loaded from:

**Windows Vista/7:**

`\Users\username\AppData\Roaming\ScummVM\scummvm.ini`

**Windows 2000/XP:**

`\Documents and Settings\username\Application Data\ScummVM\scummvm.ini`

**Windows NT4:**

`<windir>\Profiles\username\Application Data\ScummVM\scummvm.ini`

**Windows 95/98/ME:**

`<windir>\scummvm.ini`

If an earlier version of ScummVM was installed under Windows, the
previous default location of `<windir>\scummvm.ini` will be kept.

**Unix:**

We follow the XDG Base Directory Specification. This means our
configuration can be found in: `$XDG_CONFIG_HOME/scummvm/scummvm.ini`

If `XDG_CONFIG_HOME` is not defined or empty, `~/.config` will be used
as value for `XDG_CONFIG_HOME` in accordance with the specification.

If an earlier version of ScummVM was installed on your system, the
previous default location of `~/.scummvmrc` will be kept.

**Mac OS X:**

`~/Library/Preferences/ScummVM Preferences` (here, `~` refers to your
home directory)

**iOS:**


For sandboxed version: `/Preferences`
Otherwise: `/var/mobile/Library/ScummVM/Preferences`

**Others:**

`scummvm.ini` in the current directory

An example config file looks as follows:

```
    [scummvm]
    gfx_mode=supereagle
    fullscreen=true
    savepath=C:\saves\

    [sky]
    path=C:\games\SteelSky\

    [germansky]
    gameid=sky
    language=de
    path=C:\games\SteelSky\
    description=Beneath a Steel Sky w/ German subtitles

    [germandott]
    gameid=tentacle
    path=C:\german\tentacle\
    description=German version of DOTT

    [tentacle]
    path=C:\tentacle\
    subtitles=true
    music_volume=40
    sfx_volume=255

    [loomcd]
    cdrom=1
    path=C:\loom\
    talkspeed=5
    savepath=C:\loom\saves\

    [monkey2]
    path=C:\amiga_mi2\
    music_driver=windows
```

### 8.1) Recognized configuration keywords

The following keywords are recognized:

    path               string   The path to where a game's data files are
    autosave_period    number   The seconds between autosaving (default: 300)
    save_slot          number   The saved game number to load on startup.
    savepath           string   The path to where a game will store its
                                saved games.
    screenshotpath     string   The path to where screenshots are saved.
    iconspath          string   The path to where to look for icons to use as
                                overlay for the ScummVM icon in the Windows
                                taskbar or macOS X Dock when running a game.
                                The icon files should be named after the game
                                ids and be in ico format on Windows or png
                                format on macOS X.
    versioninfo        string   The version of the ScummVM that created the
                                configuration file.

    gameid             string   The real id of a game. Useful if you have
                                several versions of the same game, and want
                                different aliases for them. See the example.
    description        string   The description of the game as it will appear
                                in the launcher.

    language           string   Specify language (en, us, de, fr, it, pt, es,
                                jp, zh, kr, se, gb, hb, cz, ru)
    speech_mute        bool     If true, speech is muted
    subtitles          bool     Set to true to enable subtitles.
    talkspeed          number   Text delay in SCUMM games, or text speed in
                                other games.

    fullscreen         bool     Fullscreen mode
    aspect_ratio       bool     Enable aspect ratio correction
    gfx_mode           string   Graphics mode (normal, 2x, 3x, 2xsai,
                                super2xsai, supereagle, advmame2x, advmame3x,
                                hq2x, hq3x, tv2x, dotmatrix, opengl)
    filtering          bool     Enable graphics filtering

    confirm_exit       bool     Ask for confirmation by the user before
                                quitting (SDL backend only).
    console            bool     Enable the console window (default: enabled)
                                (Windows only).
    cdrom              number   Number of CD-ROM unit to use for audio. If
                                negative, don't even try to access the CD-ROM.
    joystick_num       number   Number of joystick device to use for input
    controller_map_db  string   A custom controller mapping file to load to
                                complete default database (SDL backend only).
                                Otherwise, file gamecontrollerdb.txt will be
                                loaded from extrapath.
    music_driver       string   The music engine to use.
    opl_driver         string   The AdLib (OPL) emulator to use.
    output_rate        number   The output sample rate to use, in Hz. Sensible
                                values are 11025, 22050 and 44100.
    audio_buffer_size  number   Overrides the size of the audio buffer. The
                                value must be one of: 256 512 1024 2048 4096
                                8192 16384 32768. The default value is
                                calculated based on the output_rate to keep
                                audio latency below 45ms.
    alsa_port          string   Port to use for output when using the
                                ALSA music driver.
    music_volume       number   The music volume setting (0-255)
    multi_midi         bool     If true, enable combination AdLib and native
                                MIDI.
    soundfont          string   The SoundFont to use for MIDI playback. (Only
                                supported by some MIDI drivers.)
    native_mt32        bool     If true, disable GM emulation and assume that
                                there is a true Roland MT-32 available.
    enable_gs          bool     If true, enable Roland GS-specific features to
                                enhance GM emulation. If native_mt32 is also
                                true, the GS device will select an MT-32 map
                                to play the correct instruments.
    sfx_volume         number   The sfx volume setting (0-255)
    tempo              number   The music tempo (50-200) (default: 100)
    speech_volume      number   The speech volume setting (0-255)
    midi_gain          number   The MIDI gain (0-1000) (default: 100) (Only
                                supported by some MIDI drivers.)

    copy_protection    bool     Enable copy protection in certain games, in
                                those cases where ScummVM disables it by
                                default.
    demo_mode          bool     Start demo in Maniac Mansion
    alt_intro          bool     Use alternative intro for CD versions of
                                Beneath a Steel Sky and Flight of the Amazon
                                Queen

    boot_param         number   Pass this number to the boot script

Sierra games using the AGI engine add the following non-standard keywords:

    originalsaveload   bool     If true, the original save/load screens are
                                used instead of the enhanced ScummVM ones
    altamigapalette    bool     Use an alternative palette, common for all
                                Amiga games. This was the old behavior
    mousesupport       bool     Enables mouse support. Allows to use mouse
                                for movement and in game menus

Sierra games using the SCI engine add the following non-standard keywords:

    disable_dithering  bool     Remove dithering artifacts from EGA games
    prefer_digitalsfx  bool     If true, digital sound effects are preferred
                                instead of synthesized ones
    originalsaveload   bool     If true, the original save/load screens are
                                used instead of the enhanced ScummVM ones
    native_fb01        bool     If true, the music driver for an IBM Music
                                Feature card or a Yamaha FB-01 FM synth module
                                is used for MIDI output
    use_cdaudio        bool     Use CD audio instead of in-game audio,
                                when available
    windows_cursors    bool     Use the Windows cursors (smaller and monochrome)
                                instead of the DOS ones (King's Quest 6)
    silver_cursors     bool     Use the alternate set of silver cursors,
                                instead of the normal golden ones (Space Quest 4)

Blade Runner adds the following non-standard keywords:
    shorty             bool     If true, game will shrink the actors and make
                                their voices high pitched
    sitcom             bool     If true, game will add laughter after actor's
                                line or narration

Broken Sword II adds the following non-standard keywords:

    gfx_details        number   Graphics details setting (0-3)
    music_mute         bool     If true, music is muted
    object_labels      bool     If true, object labels are enabled
    reverse_stereo     bool     If true, stereo channels are reversed
    sfx_mute           bool     If true, sound effects are muted

Flight of the Amazon Queen adds the following non-standard keywords:

    music_mute         bool     If true, music is muted
    sfx_mute           bool     If true, sound effects are muted

Hopkins FBI adds the following non-standard keyword:

    enable_gore        bool     If true, enable some optional gore content in
                                the game

Jones in the Fast Lane adds the following non-standard keyword:

    music_mute         bool     If true, CD audio is used, if available,
                                instead of in-game audio

King's Quest VI Windows adds the following non-standard keyword:

    windows_cursors    bool     If true, the original unscaled black and white
                                Windows cursors are used instead of the DOS
                                ones. If false, the DOS cursors are used in the
                                Windows version, upscaled to match the rest of
                                the upscaled graphics

Lands of Lore: The Throne of Chaos adds the following non-standard keywords:

    smooth_scrolling   bool     If true, scrolling is smoother when changing
                                from one screen to another
    floating_cursors   bool     If true, the cursor changes when it floats to
                                the edge of the screen to a directional arrow.
                                The player can then click to walk towards that
                                direction.

Maniac Mansion NES adds the following non-standard keyword:

    mm_nes_classic_palette  bool    If true, use a more natural palette that
                                    closely resembles the Nintendo NES Classic
                                    instead of the default NTSC palette

Space Quest IV CD adds the following non-standard keyword:

    silver_cursors     bool     If true, an alternate set of silver mouse
                                cursors is used instead of the original golden
                                ones

Simon the Sorcerer 1 and 2 add the following non-standard keywords:

    music_mute         bool     If true, music is muted
    sfx_mute           bool     If true, sound effects are muted

Soltys adds the following non-standard keyword:

    enable_color_blind bool     If true, original colors are replaced by a set
                                of greys

The Legend of Kyrandia adds the following non-standard keyword:

    walkspeed          number   The walk speed (0-4)

The Legend of Kyrandia: The Hand of Fate adds the following non-standard
keyword:

    walkspeed          number   The walk speed (3 or 5, resp. fast or
                                slow)

The Legend of Kyrandia: Malcolm's Revenge adds the following non-standard
keywords:

    walkspeed          number   The walk speed (3 or 5, resp. fast or
                                slow)
    studio_audience    bool     If true, applause and cheering sounds are heard
                                whenever Malcolm makes a joke
    skip_support       bool     If true, the player can skip text and cutscenes
    helium_mode        bool     If true, people sound like they've inhaled
                                Helium

The Neverhood adds the following non-standard keywords:

    originalsaveload   bool     If true, the original save/load screens are
                                used instead of the enhanced ScummVM ones
    skiphallofrecordsscenes     bool
                                If true, allows the player to skip
                                past the Hall of Records storyboard scenes
    scalemakingofvideos  bool   If true, the making of videos are scaled, so that
                                they use the whole screen

The 7th Guest adds the following non-standard keyword:

    fast_movie_speed   bool     If true, movies are played at an increased
                                speed, matching the speed of the iOS version.
                                Movies without sound are still played at their
                                normal speed, to avoid music synchronization
                                issues

Zork Nemesis: The Forbidden Lands adds the following non-standard keywords:

    originalsaveload   bool     If true, the original save/load screens are
                                used instead of the enhanced ScummVM ones
    doublefps          bool     If true, game FPS are increased from 30 to 60
    venusenabled       bool     If true, the in-game Venus help system is
                                enabled
    noanimwhileturning bool     If true, animations are disabled while turning
                                in panoramic mode

Zork: Grand Inquisitor adds the following non-standard keywords:

    originalsaveload   bool     If true, the original save/load screens are
                                used instead of the enhanced ScummVM ones
    doublefps          bool     If true, game FPS are increased from 30 to 60
    noanimwhileturning bool     If true, animations are disabled while turning
                                in panoramic mode
    mpegmovies         bool     If true, the hires MPEG movies are used in the
                                DVD version of the game, instead of the lowres
                                AVI ones

### 8.2) Custom game options that can be toggled via the GUI

A lot of the custom game options in the previous section can be toggled
via the GUI. If a custom option is available for a specific game, a new
tab called "Engine" will appear when adding or editing the configuration
of that game. If the custom options are not shown, the games in question
will need to be run once or readded in the ScummVM launcher's game list.
This will update the configuration of each entry, allowing the custom
options to be shown.

## 9.0) Screenshots (SDL backend only)

On systems using the SDL backend (for example Windows, Mac or Linux) you
can use alt+s to take snapshots (see section 5.4 - Hotkeys).

You can specify the directory in which you want the screenshots to be
created in the config file. To do so add a screenshotpath value under
the `[scummvm]` section:

    [scummvm]
    screenshotpath=/path/to/screenshots/

The default location, when no screenshot path is defined in the config
file, depends on the OS:

  - Windows: In `Users\username\My Pictures\ScummVM Screenshots`.
  - macOS X: On the Desktop.
  - Other unices: In the XDG Pictures user directory,
      e.g. `~/Pictures/ScummVM Screenshots`
  - Any other OS: In the current directory.

## 10.0) Compiling

For an up-to-date overview on how to compile ScummVM for various
platforms, please consult our Wiki, in particular this page:

<https://wiki.scummvm.org/index.php/Compiling_ScummVM>

If you are compiling for Windows, Linux or Mac OS X, you need SDL-1.2.2
or newer (older versions may work, but are unsupported), and a supported
compiler. Several compilers, including GCC, mingw and recent versions of
Microsoft Visual C++ are supported. If you wish to use MP3-compressed CD
tracks or .SOU files, you will need to install the MAD library; likewise
you will need the appropriate libraries for Ogg Vorbis and FLAC
compressed sound. For compressed save states, zlib is required.

Some parts of ScummVM, particularly scalers, have highly optimized
versions written in assembler. If you wish to use this option, you will
need to install nasm assembler (see <https://www.nasm.us/>). Note that
we currently only have x86 MMX optimized versions, and they will not
compile on other processors.

On Windows, you can define `USE_WINDBG` and attach WinDbg to browse
debug messages (see
<https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/index>).

  - Windows:

      - MinGW:

          - Please refer to:
            <https://wiki.scummvm.org/index.php/Compiling_ScummVM/MinGW>

      - Visual Studio (MSVC):

          - Please refer to:
            <https://wiki.scummvm.org/index.php/Compiling_ScummVM/Visual_Studio>

  - Linux:

      - GCC:

          - Please refer to:
            <https://wiki.scummvm.org/index.php/Compiling_ScummVM/GCC>

  - AmigaOS4:

      - Please refer to:
        <https://wiki.scummvm.org/index.php/Compiling_ScummVM/AmigaOS4>

  - Apple iPhone:

      - Please refer to:
        <https://wiki.scummvm.org/index.php/Compiling_ScummVM/iPhone>

  - Atari/FreeMiNT:

      - Please refer to:
        <https://wiki.scummvm.org/index.php/Compiling_ScummVM/Atari/FreeMiNT>

  - BeOS/ZETA/Haiku:

      - Please refer to:
        <https://wiki.scummvm.org/index.php/Compiling_ScummVM/BeOS/ZETA/Haiku>

  - Google Android:

      - Please refer to:
        <https://wiki.scummvm.org/index.php/Compiling_ScummVM/Android>

  - HP webOS:

      - Please refer to:
        <https://wiki.scummvm.org/index.php/Compiling_ScummVM/WebOS>

  - Mac OS:

      - Mac OS X:

          - Please refer to:
            <https://wiki.scummvm.org/index.php/Compiling_ScummVM/macOS>

      - Mac OS X 10.2.8:

          - Please refer to:
            <https://wiki.scummvm.org/index.php/Compiling_ScummVM/Mac_OS_X_10.2.8>

      - Mac OS X Crosscompiling:

          - Please refer to:
            <https://wiki.scummvm.org/index.php/Compiling_ScummVM/Mac_OS_X_Crosscompiling>

  - Maemo:

      - Please refer to:
        <https://wiki.scummvm.org/index.php/Compiling_ScummVM/Maemo>

  - Nintendo Wii and Gamecube:

      - Please refer to:
        <https://wiki.scummvm.org/index.php/Compiling_ScummVM/Wii>

  - Raspberry Pi:

      - Please refer to:
        <https://wiki.scummvm.org/index.php/Compiling_ScummVM/RPI>

  - Sega Dreamcast:

      - Please refer to:
        <https://wiki.scummvm.org/index.php/Compiling_ScummVM/Dreamcast>

  - Sony Playstation:

      - Sony PlayStation 2:

          - Please refer to:
            <https://wiki.scummvm.org/index.php/Compiling_ScummVM/PlayStation_2>

      - Sony PlayStation 3:
          - Please refer to:
            <https://wiki.scummvm.org/index.php/PlayStation_3#Building_from_source>

      - Sony PlayStation Portable:
          - Please refer to:
            <https://wiki.scummvm.org/index.php/Compiling_ScummVM/PlayStation_Portable>

  - Symbian:

      - Please refer to:
        <https://wiki.scummvm.org/index.php/Compiling_ScummVM/Symbian>

## 11.0) Changelog

Please refer to our extensive Changelog [here](NEWS.md).

## 12.0) Credits

Please refer to our extensive Credits list [here](AUTHORS).

-----

> Good Luck and Happy Adventuring\!
> The ScummVM team.
> <https://www.scummvm.org/>
