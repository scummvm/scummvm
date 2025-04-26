ScummVM 2.10.0git
==============

This is a port of ScummVM (https://www.scummvm.org), a program which allows you
to run certain classic graphical adventure and role-playing games, provided you
already have their data files.

You can find a full list with details on which games are supported and how well
on the compatibility page: https://www.scummvm.org/compatibility.


New port?
---------

Keith Scroggins (aka KeithS) has been providing ScummVM (and many other) builds
for the Atari community for unbelievable 17 years. He put quite a lot of time
into testing each release, updating ScummVM dependencies to their latest
version and even regularly upgrading his compiler toolchain to get the best
possible performance.

However ScummVM (and SDL to some extent) is a beast, it requires quite a lot of
attention where the cycles go, e.g. an additional screen refresh can easily
drop frame rate to half.

After I had seen how snappy NovaCoder's ScummVM on the Amiga is (who coded his
own backend), I decided to check whether there isn't a way to get a better
performing port on our platform. And there is!

I have managed to create a "native" Atari port talking directly to hardware,
skipping the SDL layer altogether, which is in some cases usable even on plain
32 MHz Atari TT.


Differences between the versions
--------------------------------

After talking to Keith we have decided to provide three flavours of ScummVM.
Please refer to https://docs.scummvm.org/en/v2.10.0git/other_platforms/atari.html
for more details (TBD).

Atari Full package
~~~~~~~~~~~~~~~~~~

Minimum hardware requirements: Atari Falcon with 4 + 64 MB RAM, 68040 CPU.

- Because there's a limited horsepower available on our platform, features like
  16bpp graphics, software synthesisers, scalers, real-time software
  MP3/OGG/FLAC playback etc., are omitted. This saves CPU cycles, memory and
  disk space.

- Tailored video settings for the best possible performance and visual
  experience (Falcon RGB overscan, chunky modes with the SuperVidel, TT 640x480
  for the overlay, ...)

- Direct rendering and single/triple buffering support.

- Blitting routines optimised for 68040 / 68060 CPU.

- Custom (and optimal) surface drawing (especially for the cursor).

- Custom (hardware based) aspect ratio correction (!)

- Full support for the SuperVidel, incl. the SuperBlitter (!)

- External DSP clock support for playing back samples at PC frequencies
  (Falcon only). Dual clock input frequency supported as well (Steinberg's FDI).

- Support for PC keys (page up, page down, pause, F11/F12, ...) and mouse wheel
  (Eiffel/Aranym only)

- Native MIDI output (if present).

- Runs also in Hatari and ARAnyM but in case of ARAnyM don't forget to disable
  fVDI to enable Videl output.

- FreeMiNT + memory protection friendly.

Atari Lite package
~~~~~~~~~~~~~~~~~~

Minimum hardware requirements: Atari TT / Falcon with 4 + 32 MB RAM.

As a further optimisation step, a 030-only version of ScummVM is provided, aimed
at less powerful TT and Falcon machines with the 68030 CPU. It further restricts
features but also improves performance and reduces executable size.

- Compiled with -m68030 => 68030/68882-specific optimisations enabled.

- Disabled 040+/SuperVidel code => faster code path for blitting.

- Doesn't support hires (640x480) games => smaller executable size.

- Overlay is rendered in 16 colours => faster redraw.

- Overlay during gameplay has no game background => even faster redraw.

- Overlay doesn't support alternative themes => faster loading time.

- "STMIDI" driver is automatically enabled (i.e. MIDI emulation is never used
  but still allows playing speech/sfx samples and/or CD audio)

FireBee package
~~~~~~~~~~~~~~~

Hardware requirements: MCF5475 Evaluation Board or FireBee.

This one is still built and provided by Keith.

- Based on most recent SDL

- Contains various optimisations discovered / implemented from the Atari
  backend.

- Works in GEM (in theory also in XBIOS but that seems to be still broken on
  FireBee).

- Support for all engines is included in the build, this does not mean all
  games work. For instance, support for OGG and MP3 audio is included but the
  system can not handle playback of compressed audio, not enough processing
  power for both gameplay and sound at the same time.  

  Scalers can be utilized to make the GEM window larger on the Firebee.
  Performance is best when not usimg AdLib sound, using STMIDI would be 
  optimal, but untested as of yet (I have been unable to get MIDI to work on my
  FireBee).

- Removed features: FLAC, MPEG2, Network/Cloud Support, HQ Scalers.


The rest of this document describes things specific to the Full / Lite package.
For the FireBee (SDL) build please refer to generic ScummVM documentation.


Platform-specific features outside the GUI
------------------------------------------

Keyboard shortcut "CONTROL+ALT+a": immediate aspect ratio correction on/off
toggle.

"output_rate" in scummvm.ini: sample rate for mixing. Allowed values depend on
the hardware connected:
  - TT030: 50066, 25033, 12517, 6258 Hz
  - Falcon030: as TT030 (except 6258) plus 49170, 32780, 24585, 19668, 16390,
    12292, 9834, 8195 Hz
  - External 22.5792 MHz DSP clock: as Falcon030 plus 44100, 29400, 22050,
    17640, 14700, 11025, 8820, 7350 Hz
  - External 24.576 MHz DSP clock: as Falcon030 plus 48000, 32000, 24000,
    19200, 16000, 12000, 9600, 8000 Hz
The lower the value, the faster the mixing but also worse quality. Default is
24585/25033 Hz (16-bit, stereo). Please note you don't have to enter the value
exactly, it will be rounded to the nearest sane value.

"output_channels" in scummvm.ini: mono (1) or stereo (2) mixing. Please note
that Falcon doesn't allow mixing in 16-bit mono, so this will have no effect on
this machine.

"print_rate" in scummvm.ini: used for optimising sample playback (where
available). It prints input and output sample format as well as name of the
converter used. See below for details.

"audio_buffer_size" in scummvm.ini: number of samples to preload. Default is
2048 which equals to about 83ms of audio lag and seems to be about right for
most games on my CT60@66 MHz.

If you want to play with "audio_buffer_size", the rule of thumb is: (lag in ms)
= (audio_buffer_size / output_rate) * 1000. But it's totally OK just to double
the samples value to get rid of stuttering in a heavier game.


Graphics modes
--------------

This topic is more complex than it looks. ScummVM renders game graphics using
rectangles and this port offers following options to render them:

Direct rendering
~~~~~~~~~~~~~~~~

This is direct writing of the pixels into the screen buffer. On SuperVidel it is
done natively, on Videl a chunky to planar conversion takes place beforehand.

Pros:

- on SuperVidel this offers fastest possible rendering (especially in 640x480
  with a lot of small rectangle updates where the buffer copying drags
  performance down)

- on Videl this _may_ offer fastest possible rendering if the rendering
  pipeline isn't flooded with too many small rectangles (C2P setup isn't for
  free). However with fullscreen intro sequences this is a no-brainer.

Cons:

- screen tearing in most cases

- on Videl, this may not work properly if a game engine uses its own buffers
  instead of surfaces (which are aligned on a 16pix boundary). Another source of
  danger is if an engine draws directly to the screen surface. Fortunately, each
  game can have its own graphics mode set separately so for games which do not
  work properly one can still leave the default graphics mode set.

- on Videl, overlay background isn't rendered (the gui code can't work with
  bitplanes)

SuperBlitter used: sometimes (when ScummVM allocates surface via its create()
function; custom/small buffers originating in the engine code are still copied
using the CPU).

Single buffering
~~~~~~~~~~~~~~~~

This is very similar to the previous mode with the difference that the engine
uses an intermediate buffer for storing the rectangles but yet it remembers
which ones they were.

Pros:

- second fastest possible rendering

Cons:

- screen tearing in most cases

- if there is too many smaller rectangles, it can be less efficient than
  updating the whole buffer at once

SuperBlitter used: yes, for rectangle blitting to screen and cursor restoration.
Sometimes also for generic copying between buffers (see above).

Triple buffering
~~~~~~~~~~~~~~~~

This is the "true" triple buffering as described in
https://en.wikipedia.org/wiki/Multiple_buffering#Triple_buffering and not "swap
chain" as described in https://en.wikipedia.org/wiki/Swap_chain. The latter
would be slightly slower as three buffers would need to be updated instead of
two.

Pros:

- no screen tearing

- best compromise between performance and visual experience

- works well with both higher and lower frame rates

Cons:

- if there is too many smaller rectangles, it can be less efficient than
  single buffering

- slightly irregular frame rate (depends solely on the game's complexity)

- in case of extremely fast rendering, one or more frames are dropped in favour
  of showing only the most recent one

SuperBlitter used: yes, for rectangle blitting to screen and cursor restoration.
Sometimes also for generic copying between buffers (see above).

Triple buffering is the default mode.


SuperVidel and SuperBlitter
---------------------------

As mentioned, this port uses SuperVidel and its SuperBlitter heavily. That means
that if the SuperVidel is detected, it does the following:

- uses 8bpp chunky resolutions

- patches all surface addresses with OR'ing 0xA0000000, i.e. using SV RAM
  instead of slow ST RAM (and even instead of TT RAM for allowing pure
  SuperBlitter copying)

- when SuperVidel FW version >= 9 is detected, the async FIFO buffer is used
  instead of the slower sync blitting (where one has to wait for every rectangle
  blit to finish), this sometimes leads to nearly zero-cost rendering and makes
  a *huge* difference for 640x480 fullscreen updates.


Aspect ratio correction
-----------------------

Please refer to the official documentation about its usage. Normally ScummVM
implements this functionality using yet another fullscreen transformation of
320x200 surface into a 320x240 one (there is even a selection of algorithms for
this task, varying in performance and quality).

Naturally, this would pose a terrible performance anchor on our backend so some
cheating has been used:

- on RGB, the vertical refresh rate frequency is set to 60 Hz, creating an
  illusion of creating non-square pixels. Works best on CRT monitors.

- on VGA, the vertical refresh rate frequency is set to 70 Hz, with more or
  less the same effect as on RGB. Works best on CRT monitors.

- on SuperVidel, video output is modified in such way that the DVI/HDMI monitor
  receives a 320x200 image and if properly set/supported, it will automatically
  stretch the image to 320x240 (this is usually a setting called "picture
  expansion" or "picture stretch" -- make sure it isn't set to something like
  "1:1" or "dot by dot")

Yes, it's a hack. :) Owners of a CRT monitor can achieve the same effect by the
analog knobs -- stretch and move the 320x200 picture unless black borders are no
longer visible. This hack provides a more elegant and per-game functionality.


Audio mixing
------------

ScummVM works internally with 16-bit samples so on the TT a simple downsampling
to 8-bit resolution is used. However there's still one piece missing - an XBIOS
emulator (so ScummVM doesn't have to access hardware directly). There are two
options (both available from https://mikrosk.github.io/xbios): STFA and X-SOUND,
any of these will do. Or executing ScummVM in EmuTOS which contains the same
routines as X-SOUND.


Performance considerations/pitfalls
-----------------------------------

It's important to understand what affects performance on our limited platform to
avoid unpleasant playing experiences.

Game engines with unexpected performance hit
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A typical example from this category is Gobliiins (and its sequels), some SCI
engine games (Gabriel Knight, Larry 2/7, ...) or Sherlock engine (The Case of
the Rose Tattoo). At first it looks like our machine or Atari backend is doing
something terribly wrong but the truth is that it is the engine itself which is
doing a lot of redraws, sometimes even before reaching the backend. The only
solution is to profile and fix those engines.

Too many fullscreen updates
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Somewhat related to the previous point - sometimes the engine authors didn't
realise the impact of every update on the overall performance and instead of
updating only the rectangles that really had changed, they ask for a fullscreen
update. Not a problem on a >1 GHz machine but very visible on Atari! Also, this
is (by definition) the case of animated intros, especially those in 640x480.

MIDI vs. AdLib vs. sampled music
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It could seem that sampled music replay must be the most demanding one but on
the contrary! Always choose a CD version of a game (with *.wav tracks) to any
other version. With one exception: if you have a native MIDI device able to
replay the given game's MIDI notes (using the STMIDI plugin).

MIDI emulation (synthesis) can easily eat as much as 50% of all used CPU time
(on the CT60). By default, this port uses the MAME OPL emulation (which is said
to be fastest but also least accurate) but some engines require the DOSBOX one
which is even more demanding. By the way, you can put "FM_high_quality=true" or
"FM_medium_quality=true" into scummvm.ini if you want to experiment with a
better quality synthesis, otherwise the lowest quality will be used (applies for
MAME OPL only).

On TT, in most cases it makes sense to use ScummVM only if you own a native MIDI
synthesiser (like mt32-pi: https://github.com/dwhinham/mt32-pi). MIDI emulation
is out of question and downsampling to 8-bit resolution takes a good chunk of
CPU time which could be utilised elsewhere. However there are games which are
fine with sampled music/speech even on plain TT (e.g. Lands of Lore).

CD music slows everything down
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Some games use separate audio *and* video streams (files). Even if the CPU is
able to handle both, the bottleneck becomes ... disk access. This is visible in
The Curse Of Monkey Island for example -- there's audible stuttering during the
intro sequence (and during the game as well). Increasing "audio_buffer_size"
makes the rendering literally crawling! Why? Because disk I/O is busy with
loading even *more* sample data so there's less time for video loading and
rendering. Try to put "musdisk1.bun" and "musdisk2.bun" into a ramdisk (i.e.
symlink to u:/ram in FreeMiNT), you'll be pleasantly surprised with the
performance boost gained.

Mute vs. "No music"
~~~~~~~~~~~~~~~~~~~

Currently ScummVM requires each backend to mix samples, even though they may
contain muted output (i.e. zeroes). This is because the progression of sample
playback tells ScummVM how much time has passed in e.g. an animation.

"No music" means using the null audio plugin which prevents generating any MIDI
music (and therefore avoiding the expensive synthesis emulation) but beware, it
doesn't affect CD (*.wav) playback at all! Same applies for speech and sfx.

The least amount of cycles is spent when:
- "No music" as "Preferred device": this prevents MIDI synthesis of any kind
- "Subtitles" as "Text and speech": this prevents any sampled speech to be
  mixed
- all external audio files are deleted (typically *.wav); that way the mixer
  won't have anything to mix. However beware, this is not allowed in every game!

Sample rate
~~~~~~~~~~~

It's important to realise the impact the sample rate has on the given game. The
most obvious setting is its value: the bigger, the more demanding audio mixing
becomes. However if you inspect many games' samples, you will notice that most
of them (esp. the ones from the 80s/90s) use simple samples like mono 11025 Hz
(sometimes even less).

Obviously, setting "output_channels" to "1" is the easiest improvement
(unfortunately only on TT). Next best thing you can do is to buy an external DSP
clock for your Falcon: nearly all games use sample frequencies which are
multiplies of 44100 Hz: 22050, 11025, ... so with the external clock there won't
be the need to resample them.

There's one caveat, though: it is important whether your replay frequency is
equal, multiply of or other than the desired one. Let's consider 44100 and 22050
frequencies as an example (also applies to all the other frequencies):

- if you set 44100 Hz and a game requests 44100 Hz => so called "copyConvert"
  method will be used (fastest)
- if you set 22050 Hz and a game requests 44100 Hz => so called "simpleConvert"
  method will be used (skipping every second sample, second fastest)
- if you set 44100 Hz and a game requests 22050 Hz => so called
  "interpolateConvert" method will be used (slowest!)
- any other combination: "interpolateConvert" (slowest)

So how do you know which frequency to set as "output_rate" ? This is where
"print_rate" comes to rescue. Enabling this option in scummvm.ini will tell you
for each game which sample converters are being used and for what input/values.
So you can easily verify whether the given game's demands match your setting.

Unfortunately, currently per-game "output_rate" / "output_channels" is not
possible but this may change in the future.

Slow GUI
~~~~~~~~

Themes handling is quite slow - each theme must be depacked, each one contains
quite a few XML files to parse and quite a few images to load/convert. That's
the reason why the built-in one is used as default, it dramatically speeds up
loading time. To speed things up in other cases, the full version is
distributed with repackaged theme files with compression level zero.


Changes to upstream
-------------------

There is a few features that have been disabled or changed and are not possible
/ plausible to merge into upstream:

- the aforementioned "print_rate" feature, too invasive for other platforms

- this port contains an implementation of much faster tooltips in the overlay.
  However there is a minor rendering bug which sometimes corrupts the
  background. But since its impact is huge, I left it in.


Known issues
------------

- adding a game in TOS and loading it in FreeMiNT (and vice versa) generates
  incompatible paths. Either use only one system or edit scummvm.ini and set
  there only relative paths (mintlib bug/limitation).

- when run on TT, screen contains horizontal black lines. That is due to the
  fact that TT offers only 320x480 in 256 colours. Possibly fixable by a Timer B
  interrupt.

- horizontal screen shaking doesn't work on TT because TT Shifter doesn't
  support fine scrolling. However it is "emulated" via vertical shaking.

- aspect ratio correction has no effect on TT because is not possible to alter
  its vertical screen refresh frequency.

- the talkie version of SOMI needs to be merged from two sources:
  - the DOS version (install.bat) to obtain file "monster.sou"
  - the FLAC version (install_flac.bat) to obtain folders "cd_music_flac" and
    "se_music_flac" (these *.flac files then have to be converted to *.wav
    manually)
  - files "monkey.000" and "monkey.001" can be taken from either version
  - point the extra path to the folder with *.wav files (or copy its content
    where monkey.00? files are located)

- following engines have been explicitly disabled:
  - Cine (2 games)
    - incompatible with other engines / prone to freezes
    - https://wiki.scummvm.org/index.php?title=Cine
  - Director (many games)
    - huge game list slows detection for other games, and would require
      (currently missing) localization support
    - only small subset of games actually supported by upstream, but none of
      them detected on TOS 8+3 file system
    - https://wiki.scummvm.org/index.php?title=Director
  - Hugo (3 games)
    - uses (lot of) overlay dialogs which are problematic for Atari backend
    - engine GUI (for save/load/etc) does not support 8-bit screens
    - https://wiki.scummvm.org/index.php?title=Hugo
  - Ultima (many games)
    - the only non-hires ultima engine is ultima1; see
      https://bugs.scummvm.org/ticket/14790
    - this prevents adding the 15 MB ultima.dat to the release archive
    - https://wiki.scummvm.org/index.php?title=Ultima

- When using FreeMiNT, ScummVM requires a recent kernel (>= 2021), otherwise
  keyboard handling won't work properly.

- When using EmuTOS, ScummVM requires a recent release (>= 1.3), otherwise
  various screen- and sound-related issues may appear.

Future plans
------------

- unified file paths in scummvm.ini

- DSP-based sample mixer (WAV, FLAC, MP2)

- avoid loading music/speech files (and thus slowing down everything) if muted

- cached audio/video streams (i.e. don't load only "audio_buffer_size" number
  of samples but cache, say, 1 second so disk i/o won't be so stressed)

- using Thorsten Otto's sharedlibs: https://tho-otto.de/sharedlibs.php for game
  engine plugins to relieve the huge binary size

- true audio CD support via MetaDOS API

- OPL2LPT and Retrowave support (if I manage to purchase it somewhere)


Closing words
-------------

Many optimisations and improvements wouldn't be possible without help of Eero
Tamminen, so thank you for all the help with profiling in Hatari.

Miro Kropacek aka MiKRO
Kosice / Slovakia
miro.kropacek@gmail.com
http://mikro.atari.org
