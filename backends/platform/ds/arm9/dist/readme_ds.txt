	


                    The official port of ScummVM
                 to the Nintendo DS handheld console
                     by Neil Millstone (agentq)
                   http://scummvm.drunkencoders.com
------------------------------------------------------------------------
Visit the main ScummVM website <http://www.scummvm.org>




     Contents
     ------------------------------------------------------------------------

	 

    * What's New?
    * What is ScummVM DS?
    * Features
    * Screenshots
    * How to get ScummVM DS onto your DS
          o Using a CF/SD/Mini SD/Micro SD card reader and a DLDI driver
          o Instructions for specific card readers
    * What to do if saving doesn't work or your card gets corruped -
      force SRAM saves
    * How to use ScummVM DS
    * Game Specific Controls
    * DS Options Screen
    * Which games are compatible with ScummVM DS
    * Predictive dictionary for Sierra AGI games
    * Converting your CD audio
    * Converting speech to MP3 format
    * Frequently Asked Questions
    * Downloads
    * Contributors
    * Donations and Getting Help
    * Building from Sources



      What's New?
      ------------------------------------------------------------------------

ScummVM DS 0.11.1

 * Bugfix release - No new DS port features


ScummVM DS 0.11.0

 * New games supported: Elvira 1 and 2, Waxworks (Amiga version)
 * Software scaler for improved image quality. Turn it on using the DS options
   screen (press select during the game). Thanks to Tramboi and Robin Watts for
   this feature!
 * Function keys added to virtual keyboard (used in AGI games)
 * Plenty of bug fixes 

      What is ScummVM DS?
      ------------------------------------------------------------------------

ScummVM <http://www.scummvm.org> is an interpreter that allows you to
play many point and click adventure games from the 80s and 90s on modern
computers. I didn't write ScummVM, that honour goes to the ScummVM team.
ScummVM runs games from Lucasarts that were created using the SCUMM
system (hence the name) but it has also been expanded to run games from
many other developers too. ScummVM is written in portable C++ and has
been ported to many differnet platforms, and I have made a port of it to
the DS. This means that you can play all these classic games on the DS
with full sound.



      Features
      ------------------------------------------------------------------------

    * Runs nearly all of Lucasarts' SCUMM games up to and including Sam
      & Max Hit the Road
    * Runs many non-Lucasrts point-and-click adventures too
    * Supports sound
    * Provides a GUI to change settings and choose games
    * Supports using the DS touch screen for controls
    * Suports saving games to compatible flash cards
    * All games run at pretty much full speed





      How to Get ScummVM DS Onto Your DS
      ------------------------------------------------------------------------

To use ScumMVM on your DS, you will need a way of getting code onto your
DS, and a copy of the game you want to run. Don't ask me how to get hold
of the games, they are still copyright works, so that means you have to
buy them. These games are all out of production though, so you'll have
to resort to buying a second hand copy from a place like eBay. You can
also use demo versions of the games which you can get from the ScummVM
website <http://www.scummvm.org/demos.php>.

The exception to this are the two games Beneath a Steel Sky and Flight
of the Amazon Queen. These two games have been given away for free by
their developer, Revolution Software, so you are free to download and
play these without paying for them. These can be download from the
ScummVM downloads page <http://www.scummvm.org/downloads.php>.

ScummVM DS is a homebrew application. This means that Nintendo haven't
approved it, as they do with commercial games. Therefore, in order to
run ScummVM you must rely on third party devices which are produced
without the approval or knowledge of Nintendo. The devices that can be
used to run homebrew on the DS vary widely. The easiest supported card
to use is probably the R4DS or M3 DS Real. These cards sits in your DS
slot. All that's required to be up and running on this card is to put
ScummVM DS and your games on the Micro SD card, stick the card reader
into your DS, and turn on. Nothing else is required. ScummVM DS can be
selected from a menu, and everything runs great. So if you're worried
about how hard it is to get this going, these are the ones to get.

Other devices use the GBA slot, the slot normally used for a Game Boy
Advance cartridge for backwards compatibility, and these need a NoPass
in order to boot the GBA slot in DS mode. There are several types of
these which all work in the same way. One of the more popular is the
Datel Media Launcher, but the PassKey 3 and many others should work
well. Then you need a card reader for your GBA slot. There are various
versions of the Supercard and M3 Adaptor.

The important thing to remember is that your card reader must have a
DLDI driver. This is essential, as without it ScummVM will not be able
to read from your card. If you're card reader doesn't have a DLDI
driver, you should ask the card manufacturer to make one. I can't help
you with this.

You'll find plenty of information on how to use these various methods by
searching on Google.

NOTE: This site used to detail how to use FlashMe, WifiMe, PassMe or a
PassMe2, but have now been made obsolete by the NoPass. There are other
places on the net where you can get information on such methods.

Previous version of ScummVM DS supported a method which used a zip file
to run games on unsupported flash card readers.  This method is no longer
supported.



      How to Get ScummVM DS Onto Your DS - Using a CF/SD/Mini SD/Micro
      SD card reader and a DLDI driver
      ------------------------------------------------------------------------

All DS card readers are different in the way that they work. In order to
support many different card readers, ScummVM DS doesn't contain support
for any of them, but instead relies on a DLDI driver to be installed
into the ScummVM DS code. This is done using a program called DLDITool
which you can download and run on your computer. Each DLDI driver is
designed to tell ScummVM DS how to use a specific type of card reader.
These drivers can be used with any homebrew program which supports the
DLDI interface.

While each card reader should work with these instructions, there are
some exceptions. Please read the card reader notes
section to see if there is any specific information about your card reader.

Here is what you need to do:

    * Visit the DLDI page <http://dldi.drunkencoders.com/> and
      download the executable for DLDITool for your operating system
      (versions are available for Windows, Linux, and MacOS)
    * Download the DLDI for your card reader. This is the big table at
      the top of the page. The first column marked DLDI is the one you
      want. You should get a single file with a .dldi extension.
    * Extract DLDITool into a folder, and put the DLDI of your choice in
      the same folder.
    * If you're using the command line version of DLDITool enter the
      following at a command prompt:

          dlditool <dldiname> <scummvm nds name>


      If you're using the Windows GUI version, double click on
      dlditool32.exe, select your card reader from the box, drag your
      ScummVM binaries (either the .nds, or the .ds.gba version
      depending on your card reader. I think only Supercards use the
      .ds.gba files) into the lower box, then click patch.

      Either way, you should see 'Patched Successfully'. If you don't,
      you're doing something wrong.

      You need to patch one of the builds labeled A - F depending on
      which game you want to run. See the table on the ScummVM DS
      website to see which games are supported by which build.

    * Put the patched .nds or .ds.gba files on your flash card. If
      you're using the Supercard, you will need to use the .ds.gba
      files, but rename them to .nds.
    * Put your game data in any folder on the card. Do NOT use a zip file.
    * Boot up your DS and run ScummVM.
    * Click 'Add Game', browse to the folder with your game data, click
      'Choose', then 'OK'. Click 'Start' to run the game. 

If your copy of ScummVM DS has been successfully patched, you will get a
message on the top screen that looks like this:

	DLDI Device:
	GBA Movie Player (Compact Flash)

The message should show the name of your card reader. If it is wrong,
you have used the wrong DLDI file.

If you haven't patched your .nds file, you will get the following message

	DLDI Driver not patched!
	DLDI Initialise failed.

In this case, you've made a mistake following the above instructions, or
have patched the wrong file.

You may also see the following message:

	DLDI Device:
	GBA Movie Player (Compact Flash)
	DLDI Initialise failed.

In this case, the driver did not start up correctly. The driver is
probably faulty, or incompatible with your card reader.

In the case of the Supercard, M3 Lite and DS Link, there are several
drivers available. You might want to try one of the others.

This version of ScummVM DS will run on any card reader that has a DLDI
driver available. If yours doesn't, you need to pressure your card
reader manufacturer to release one.

DO NOT EMAIL ME TO ASK ME TO CREATE A DRIVER FOR YOUR CARD READER, I
CANNOT DO THIS.


      How to Get ScummVM DS Onto Your DS - Instructions for specific
      card readers
      ------------------------------------------------------------------------

    * *GBAMP CF:* You need to upload replacement firmware to your card
      reader before it will work. You can download the firmware program
      here <http://chishm.drunkencoders.com/NDSMP/index.html>. Name your
      .nds file _BOOT_MP.nds.
    * *M3 CF/SD:* Copy the .nds file to your card with the M3 Game
      Manager in order to avoid an annoying message when you boot your
      M3. Use the default options to copy the file. Be sure to press 'A'
      in the M3 browser to start the .nds file, and not 'Start', or it
      won't work.
    * *M3 CF/SD:* Copy the .nds file to your card with the M3 Game
      Manager in order to avoid an annoying message when you boot your
      M3. Use the default options to copy the file. Be sure to press 'A'
      in the M3 browser to start the .nds file, and not 'Start', or it
      won't work.
    * *Supercard CF/SD (slot-2):* Use the .ds.gba files to run ScummVM
      on the Supercard. Other than that, just follow the instructions as
      normal.
    * *Supercard Lite (slot-2):* It has been reported that only the
      standard Supercard driver and the Moonshell version work with
      ScummVM DS.
    * *Datel Max Media Dock: * If you haven't already, upgrade your
      firmware to the latest version. The firmware that came with my Max
      Media Dock was unable to run ScummVM DS at all. Click here to
      visit Datel's support page and download the latest firmware
      <http://us.codejunkies.com/mpds/support.htm>
    * *NinjaDS*: There are firmware upgrades for this device, but for
      me, ScummVM DS ran straight out of the box. Visit this page
      <http://www.ninjads.com/news.html> to download the latest firmware
      if you want. If you have installed FlashMe on your DS, it will
      make your DS crash on boot when the NinjaDS is inserted. You can
      hold the 'select' button during boot to disable FlashMe, which
      will allow the NinjaDS to work. Due to this, it is not recommended
      to install FlashMe if you use a NinjaDS.
    * *EZ-Flash*: This card reader uses .ds.gba files from the ScummVM
      archive. Rename them to .nds before patching them with the DLDI
      patcher.
    * *R4DS*: If you upgrade the firmware for your R4DS to version 1.10
      or later, the card will autmatically DLDI patch the game, meaning
      you don't have to use dlditool to patch the .NDS file. This makes
      things a lot easier! 
    * *M3DS Real*: This card autmatically DLDI patches the game, meaning
      that you do not need to do this yourself.



      Which games are compatible with ScummVM DS?
      ------------------------------------------------------------------------

I'm glad you asked. Here is a list of the compatible games in version
0.11.0. Demo versions of the games listed should work too.

Flight of the Amazon Queen and Beneath a Steel Sky have generously been
released as freeware by the original authors, Revolution Software
<http://www.revolution.co.uk/>. This is a great thing and we should
support Revolution for being so kind to us. You can download the game
data from the official ScummVM download page
<http://www.scummvm.org/downloads.php>.

The other games on this list are commercial, and still under copyright,
which means downloading them without paying for it is illegal. You can
probably find a second-hand copy on eBay. Please don't email me to ask
for a copy, as I am unable to send it to you.

Game                                      Build   Notes

Manic Mansion                             A	

Zak McKracken and the Alien Mindbenders   A	

Indiana Jones and the Last Crusade        A	

Loom                                      A	

Passport to Adventure                     A	

The Secret of Monkey Island               A	

Monkey Island 2: LeChuck's Revenge        A	

Indiana Jones and the Fate of Atlantis    A	

Day of the Tentacle	                  A	

Sam & Max Hit the Road	                  A     Some slowdown in a few scenes
                                                when MP3 audio is enabled

Bear Stormin' (DOS)                       A

Fatty Bear's Birthday Surprise (DOS)      A

Fatty Bear's Fun Pack (DOS)               A

Putt-Putt's Fun Pack (DOS)                A

Putt-Putt Goes to the Moon (DOS)          A

Putt-Putt Joins the Parade (DOS)          A

Beneath a Steel Sky                       B

Flight of the Amazon Queen                B

Simon the Sorcerer 1                      C     Zoomed view does not follow the
                                                speaking character
Simon the Sorcerer 2                      C     Zoomed view does not follow the
                                                speaking character
Elvira 1                                  C

Elvira 2                                  C

Waxworks (Amiga version)                  C

Gobliiins                                 D	

Gobliins 2                                D	

Goblins 3                                 D

Ween: The Prophecy                        D	

Bargon Attack                             D	

Future Wars                               D	

All Sierra AGI games.
For a complete list, see this page
<http://wiki.scummvm.org/index.php/AGI>   D	

Inherit the Earth                         E	

The Legend of Kyrandia                    F     Zoomed view does not follow the
                                                speaking character

There is no support for Full Throttle, The Dig, or The Curse of Monkey
Island because of memory issues. There simply is not enough RAM on the
DS to run these games. Sorry. Also there is no support for Windows Humongous
Entertainment games. The extra code required to make this work uses up
too much RAM.


      What to do when saving doesn't work or your card gets corrupted -
      forcing SRAM Saves
      ------------------------------------------------------------------------

If ScummVM DS cannot save games to your SD card, or it causes corruption
when it does, you can force it to use GBA SRAM to save the game. This
uses your flash cart reader's GBA features to save the game to a .sav or
.dat file (depending on the reader). Only slot-2 devices can use SRAM saves, 
and only ones with support for GBA games.

If you want to use SRAM save, just create a text file called scummvm.ini
(or scummvmb.ini, scummvmc.ini for builds B or C) in the root of your
card which contains the following:

[ds]
forcesramsave=true

When you boot your game, ScummVM DS will not save games to your SD card
directly, instead it will save to GBA SRAM. On most cards, you need to
transfer the data to your SD card by rebooting and using your card
reader's boot-up menu. Using this method, around four saves can me made.

One disadvantage of forcing SRAM saves is that your settings won't be
saved. You can add games manually to the ini file so that you don't have
to select them on each boot. Just add a section like the following on
for each game on your card.

[monkey2]
description=Monkey Island 2: LeChuck's Revenge (English/DOS)
path=mp:/MONKEY2


      How to Use ScummVM
      ------------------------------------------------------------------------

Once you've booted up ScummVM, you'll see the start up screen. Tap the
'Add' button with the pen, then browse to the folder containing your
game data. If you didn't use folders, the data will be in the root, and
you just need to tap the 'Choose' button. You will get some options for
the game. You can usually just click 'Ok' to this. Now click on the name
of the game you want to play from the list and click 'Start'. Your game
will start!

You can use the B button to skip cutscenes, and the select button to
show an options menu which will let you tweak the DS contols, including
switch between scaled and unscaled video modes. The text is clearer in
the unscaled mode, but the whole game doesn't fit on the screen. To
scroll around, hold either shoulder button and use the D-pad or drag the
screen around with the stylus. Even in scaled mode, a small amount is
missing from the top and bottom of the screen. You can scroll around to
see those areas. The top screen shows a zoomed-in view. This scrolls
around to focus on the character who's speaking, and also follows where
the pen touches the screen. You can change the zoom level by holding one
of the shoulder buttons and pressing B to zoom in and A to zoom out.
There is also a fixed 200% zoom option which can be selected on the DS
Options screen (press select during the game).

Press the start button for the in-game menu where you can load or save
your game (this works in Lucasarts games, other games vary). Saves will
write directly to your flash card. You can choose the folder where they
are stored using the GUI that appears when you boot up. If you're using
a GBA Flash Cartridge, or an unsupported flash card adaptor, you will be
using GBA SRAM to save your game. Four or five save game will fit in
save RAM. If you save more games than will fit, a warning will appear on
the top screen. When you turn your DS off, the new save will be lost,
and only the first ones you saved will be present.

Many of the games use both mouse buttons. Usually the right button
performs the default action on any object you click on. To simulate this
with the DS pen, you can switch the input into one of three modes. Press
left on the D-pad to enable the left mouse button. Press right on the
D-pad to enable the right mouse button. Press up on the D-pad to enable
hover mode. In this mode, you won't click on anything, just hover the
mouse cursor over it. This lets you pick out active objects in the scene.

An icon on the top screen will show you which mode you're in.


In hover mode, there are some additional controls. While holding the pen
on the screen, tapping D-pad left or D-pad right (or A/Y in left handed
mode) will click the left or right mouse button.

Here is a complete list of controls in right-handed mode (the default
setting):
Key                Usage
Pad Left           Left mouse button
Pad Right          Right mouse button
Pad Up             Hover mouse (no mouse button)
Pad Down           Skip dialogue line (for some Lucasarts games), Show inventory
                   (for Beneath a Steel Sky), Show active objects (for Simon the Sorceror)
Start              Pause/game menu (works in some games)
Select             DS Options
B                  Skip cutscenes
A                  Swap main screen and zoomed screen
Y                  Show/Hide debug console
X                  Show/Hide on-screen keyboard
L + D-pad or L + Pen	Scroll touch screen view
L + B              Zoom in
L + A	           Zoom out



And here's left-handed mode:
Key                Usage
Y	           Left mouse button
A	           Right mouse button
X	           Hover mouse (no mouse button)
B	           Skip dialogue line (for some Lucasarts games), Show inventory (for
                   Beneath a Steel Sky), Show active objects (for Simon the Sorceror)
Start              Pause/game menu (works in some games)
Select             DS Options
D-pad down         Skip cutscenes
D-pad up           Swap main screen and zoomed screen
D-pad left         Show/Hide debug console
D-pad right        Show/Hide on-screen keyboard
R + D-pad or R + Pen	Scroll touch screen view
R + D-pad down     Zoom in
R + d-pad right    Zoom out



      Game-specific controls
      ------------------------------------------------------------------------

    * Sam and Max Hit the Road: The current cursor mode is displayed on
      the top screen. Use d-pad right to switch mode.
    * Indiana Jones games: If you get into a fight, press Select, and
      check the box marked 'Use Indy Fighting Controls'.
      Return to the game, then use the following controls to fight:

      D-pad left: move left
      D-pad right: move right
      D-pad up: guard up
      D-pad down: guard down
      Y: guard middle
      X: Punch high
      A: Punch middle
      B: Punch low
      Left shoulder: Fight towards the left
      Right shoulder: Fight towards the right

      The icon on the top screen shows which way you're currently
      facing. Remember to turn the option off when the fight ends, or
      the normal controls won't work!
    * Beneath a Steel Sky: Press D-pad down to show your inventory.
    * Simon the Sorcerer 1/2: Press D-pad down to show active objects.
    * AGI games: Press Start to show the menu bar.
    * Bargon Attack: Press Start to hit F1 when you need to start the
      game. Use shift with the number keys on the on-screen keyboard to
      press other function keys. 


      DS Options Screen
      ------------------------------------------------------------------------

Pressing the 'select' button during any game to show the DS options
screen.  This screen shows options specific to the Nintendo DS version
of ScummVM.

High Quality Audio - Enhance the sound quality, at the expense of some
slowdown during some games.

Indy Fighting Controls - Enable fighting controls for the Indiana Jones
games.  See 'Game Specific Controls' for more information.

Zoomed Screen at fixed 200% zoom - disable the zoom in/out feature on the
top screen and force it to show the graphics at double size.

Left handed Mode - Switch the controls on the D-pad with the controls
on the A/B/X/Y buttons.

Disable power off - ScummVM DS turns the power off when the game quits.
This option disables that feature.

Show mouse cursor - Shows the game's mouse cursor on the bottom screen.

Snap to border - makes it easier for the mouse controls to reach the edges
of the screen.  Useful for Beneath a Steel Sky and Goblins 3.

Scaling options:

Three scaling options are available for the main screen.

Harware Scale - Scales using the DS hardware scaler using a flicker method.
Produces lower quality graphics but doesn't slow the game down.

Software Scale - Scales using the CPU.  A much higher quality image is
produced, but at the expense of speed in some games.

Unscaled - Allows you to see the graphics as originaly displayed.  This
doesn't fit on the DS screen, but you can scroll the screen around by holding
the left shoulder button and using the D-pad or touch screen.




      Auto completion dictionary for Sierra AGI games
      ------------------------------------------------------------------------

If you are playing a Sierra AGI game, you will be using the on-screen
keyboard quite a lot (press X to show it). To reduce the amount you have
to type, the game can automatically complete long words for you. To use
this feature, simply copy the PRED.DIC file from the ScummVM DS archive
into your game folder on your card. Now, when you use the keyboard,
possible words will be shown underneith it. To type one of those words,
simply double click on it with your stylus.


      Converting your CD audio
      ------------------------------------------------------------------------

ScummVM supports playing CD audio for specific games which came with
music stored as standard music CD tracks. To use this music in ScummVM
DS, they need to be ripped from the CD and stored in a specific format.
This can only be done for the CD versions of certain games, such as
Monkey Island 1, Loom, and Gobliiins. All the floppy games and CD games
that didn't have CD audio tracks for music don't require any conversion,
and will work unmodified on ScummVM DS. MP3 audio files for CD music are
not supported.

Cdex can do the conversion very well and I recommend using it to convert
your audio files, although any CD ripping software can be used, so feel
free to use your favourite program. The format you need to use is
IMA-ADPCM 4-bit Mono. You may use any sample rate. All other formats
will be rejected, including uncompressed WAV files.

Now I will to describe how to rip your CD tracks with Cdex, which can be
found here: Cdex Homepage <http://sourceforge.net/projects/cdexos/>

To set this up in Cdex, select Settings from the Options menu. On the
Encoder tab, select 'WAV Output Encoder'. Under 'Encoder Options',
choose the following:

      Format: WAV
      Compression: IMA ADPCM
      Samplerate: 22050 Hz
      Channels: Mono
      On the fly encoding: On

Next, go to the 'Filenames' tab and select the folder you want to save
your Wav files to. Under 'Filename format', enter 'track%3'. This should
name your WAV files in the correct way. Click OK.

Now select all the tracks on your CD, and click 'Extract CD tracks to a
compressed audio file'. Cdex should rip all the audio off your CD.

Now all you have to do is copy the newly created WAV files into the same
directory that your other game data is stored on your CompactFlash card.
Next time your run ScummVM DS, it should play with music!

*Important Note:* Do not select 'Extract CD tracks to a WAV file'. This
creates uncompressed WAVs only. You want 'Extract CD tracks to a
compressed audio file'.


      Converting Speech files to MP3 format
      ------------------------------------------------------------------------

ScummVM supports playing back speech for talkie games in MP3 format.
Unfortunately, the DS CPU is not quite up to the task, and MP3 audio
will sometimes cause slowdown in your game. However, if your flash card
isn't big enough to fit the audio files on, you will have no choice!

To convert your audio you will need a copy of the ScummVM Tools package
<http://sourceforge.net/project/showfiles.php?group_id=37116&package_id=67433>.
You will also need a copy of the LAME MP3 encoder
<http://www.free-codecs.com/Lame_Encoder_download.htm>.

Once this is all installed and set up, the process to encode your audio
varies from game to game, but the Lucasarts games can all be compressed
using the following command line:

compress_scumm_sou --mp3 monster.sou

This produces a monster.so3 file which you can copy to your flash card
and replaces the original monster.sou. Ogg format (monster.sog) and flac
format files are not currently supported by ScummVM DS, and it is
unlikely they will ever be supported. There is no way to convert .sog or
.so3 files back to .sou files. Just dig out your original CD and copy
the file from that.


      Frequently Asked Questions
      ------------------------------------------------------------------------

I get a lot of email about ScummVM DS. Nearly all of them are exactly
the same. Here I'm going to try and answer the questions that everybody
asks me in the hope that I will spend less time answering questions that
are clearly in the documentation and more time helping people who have a
real problem or have discovered a real bug.

*Q:* I can't see the bottom line of inventory items in Day of the
Tentacle, Monkey Island 2, or a few other games! What do I do?
*A:* Hold down the left shoulder button and use D-pad (or the touch
screen) to scroll the screen around.

*Q:* I dont see a menu when I press Start in Flight of the Amazon Queen
or Simon the Sorcerer. Is ScummVM broken?
*A:* No. To save in Simon the Sorcerer, click 'use', then click on the
postcard in your inventory. In Flight of the Amazon Queen, click 'use',
then click on the journal in your inventory.

*Q:* Why does ScummVM crash when I play Monkey Island 1?
*A:* This happens when MP3 audio tracks are present from the PC version
of ScummVM. Delete the MP3 tracks and reencode them to ADPCM WAV files
as described in the CD audio section.

*Q:* When will you support my Mini/Micro SD card reader? I want it!
Pretty please?
*A:* ScummVM uses DLDI drivers. If your card reader manufacturer doesn't
provide a driver, there is nothing I can do about it. The people to ask
are the card reader manufacturers themselves.

*Q:* ScummVM won't save/load, or crashes in the newspaper scene in DOTT,
or during the intro in Fate of Atlantis. What do I do?
*A:* This is down to a buggy DLDI driver which doesn't support unaligned
data reads properly. Contact the author of the DLDI and point them to
this FAQ, and they should know what to do. If the author of a DLDI want
to contact me, feel free, and I will tell them what the issue is.

*Q:* Can't you use the extra RAM in the M3/Supercard or the official
Opera Expansion Pack to support more games like The Dig and Full
Throttle? DS Linux has done it, so why can't you?
*A:* Not at the moment. The extra RAM has certain differences to the
build in RAM which makes it difficult to use for general programs. As
ScummVM DS is an official port, the changes to the ScummVM code base
must be minimal to avoid making the code difficult to read for other
users. I do have plans to work on this some time in the future, but
don't nag me about when it'll be done. If and when there's progress with
this, I will post on the ScummVM forums about it.


*Q:* ScummVM DS turns off my DS when I hit 'Quit' in the game or quit
from the frontend. Why doesn't it return to the menu?
*A:* Due to bugs in the ScummVM codebase, many of the ScummVM games
cannot quit cleanly leaving the machine in the same state as when it
started. You will notice that no other versions of ScummVM can quit back
to the menu either. This will be fixed at some time in the future.




      Contributors
      ------------------------------------------------------------------------

ScummVM DS uses chishm's GBA Movie Player FAT driver.
The CPU scaler is by Tramboi and Robin Watts
The ARM code was optimised by Robin Watts
Thanks to highpass for the ScummVM DS icons.
Thanks to zhevon for the Sam & Max cursor code.
Thanks to theNinjaBunny for the M3 Adaptor guide on this site.
Thanks also to everyone on the GBADev Forums.

This program was brought to you by caffiene, sugar, and late nights.


      Donations and Getting Help
      ------------------------------------------------------------------------

If you have problems getting ScummVM to work on your hardware, please
read the FAQ first. /Please/ don't ask me questions which are
answered in the FAQ, I get many emails about this program each day, and
I can't help the people who really need help if I'm answering the same
question all the time which is already answered on this page. Other than
that, feel free to post on the ScummVM DS forum <http://forums.scummvm.org>
for help. Please do your research first though. There is no way of
running this on an out-of-the box DS without extra hardware. Most of
these things are fairly inexpensive though.

If you want to contact me, please email me on scummvm at millstone dot
demon dot co dot uk.

If you want to help with the development of ScummVM DS, great! Download
the source code and get building. There are plenty of things left to do.

You can also help by making a donation if you've particularly enjoyed
ScummVM DS. This uses Paypal, and is completely secure. There's no
pressure though, ScummVM DS is completely free. This is just for those
who would like to make a contribution to further development.



      Building from Sources
      ------------------------------------------------------------------------

To build ScummVM DS from source, it's probably better to checkout the
latest version of the code from the ScummVM SVN repository. The ScummVM
Sourceforge.net homepage <http://sourceforge.net/projects/scummvm> has
all the information about how to do this.


