ScummVM
=======

This is a new port of ScummVM (https://www.scummvm.org), a program which allows
you to run certain classic graphical adventure and role-playing games, provided
you already have their data files.

You can find a full list with details on which games are supported and how well
on the compatibility page: https://www.scummvm.org/compatibility.


Yet another port?
-----------------

Yes, I am aware of the official Atari/FreeMiNT port done by KeithS over the
years (https://docs.scummvm.org/en/v2.7.0/other_platforms/atari.html). It is
even updated every release and put on the official ScummVM website. That port
is basically just a recompiled SDL backend for our platform - that certainly
has some advantages (works in GEM, can be easily compiled for the FireBee etc.)
but I have decided to take a different route:

- Reduced executable size, basically whatever is not essential or plausible on
  our platform is left out. That reduces the file size to half. See also the
  next point.

- Because there's a limited horsepower available on our platform, features like
  hi-res 16bpp graphics, software synthesizers, scalers, real-time software
  MP3/OGG/FLAC playback etc., are omitted. This saves memory and disk space,
  making the whole port more lightweight.

- This port natively talks to the hardware, avoiding intermediate layers like
  SDL. Thus, it has more optimisations, fewer redraws, fewer data copying and
  is less crash-prone.

- Because we limit scope only to 8bpp games, it opens a door to more thorough
  testing and there is a certain interest in this in the community. 16bpp games
  could be played only in ARAnyM or similar, limiting the test audience a lot.

After I had seen how snappy NovaCoder's ScummVM on the Amiga is (who coded
his own backend), I decided to do the same and see whether I could do better.
And I could!


Hardware requirements
---------------------

This port requires an Atari computer with TT or Falcon compatible video modes.
Ideally accelerated with at least 4+32 MB of RAM. It runs fine also in Hatari
and ARAnyM but in case of ARAnyM don't forget to disable fVDI to show Videl
output.


Main features
-------------

- Optimized for the Atari TT/Falcon: ideally the CT60/CT63/CT60e but some games
  run fine on the AfterBurner040, CT2/DFB@50 MHz, Speedy@48 MHz or even less!

- Full support for the SuperVidel, incl. the SuperBlitter (!)

- Removed features found too demanding for our platform; the most visible
  change is the exclusion of the 16bpp games (those are mostly hi-res anyway)
  but games in 640x480@8bpp work nicely (Falcon only, unfortunately).

- Direct rendering and single/triple buffering support.

- Custom (and optimal) drawing routines (especially for the cursor).

- Tailored video settings for the best possible performance and visual
  experience (Falcon RGB overscan, chunky modes with the SuperVidel, TT 640x480
  for the overlay, ...)

- Custom (hardware based) aspect ratio correction (!)

- Support for PC keys (page up, page down, pause, F11/F12, ...) and mouse wheel
  (Eiffel/Aranym only)

- AdLib emulation works nicely with many games without noticeable slow downs.


Platform-specific features outside the GUI
------------------------------------------

Keyboard shortcut "CONTROL+ALT+a": immediate aspect ratio correction on/off
toggle.

"output_rate" in scummvm.ini: sample rate for mixing, can be 49170, 32780,
24585, 19668, 16390, 12292, 9834, 8195 on the Falcon and 50066, 25033, 12517,
6258 on the TT (the lower the value, the faster the mixing but also worse
quality). Default is 24585/25033 Hz (16-bit, stereo).

"audio_buffer_size" in scummvm.ini: number of samples to preload. Default is
2048 which equals to about 83ms of audio lag and seems to be about right for
most games on my CT60@66 MHz.

If you want to play with those two values, the rule of thumb is: (lag in ms) =
(audio_buffer_size / output_rate) * 1000. But it's totally OK just to double
the samples value to get rid of stuttering in a heavier game.


Graphics modes
--------------

This topic is more complex than it looks. ScummVM renders game graphics using
rectangles and this port offers following options to render them:

Direct rendering (present only with the SuperVidel)
Single buffering
Triple buffering

Direct rendering:
~~~~~~~~~~~~~~~~~

This is direct writing of the pixels into (SuperVidel's) screen buffer.

Pros:

- fastest possible rendering (especially in 640x480 with a lot of small
  rectangle updates where the buffer copying drags performance down)

Cons:

- screen tearing in most cases

- SuperVidel only: using C2P would be not only suboptimal (every rectangle
  would be C2P'ed instead of multiple copying and just one C2P of the final
  screen) but poses an additional problem as C2P requires data aligned on a
  16px boundary and ScummVM supplies arbitrarily-sized rectangles (this is
  solvable by custom Surface allocation but it's not bullet-proof). In theory I
  could implement direct rendering for the Falcon hicolor (320x240@16bpp) but
  this creates another set of issues like when palette would be updated but not
  the whole screen - so some rectangles would be rendered in old palette and
  some in new.

SuperBlitter used: sometimes (when ScummVM allocates surface via its create()
function; custom/small buffers originating in the engine code are still copied
using the CPU).

Single buffering:
~~~~~~~~~~~~~~~~~

This is very similar to the previous mode with the difference that the engine
uses an intermediate buffer for storing the rectangles but yet it remembers
which ones they were. It works also on plain Videl and applies the chunky to
planar process to each one of the rectangles separately, avoiding fullscreen
updates (but if such is needed, there is an optimized code path for it).

Pros:

- second fastest possible rendering

Cons:

- screen tearing in most cases

- if there is too many smaller rectangles, it can be less efficient than
  updating the whole buffer at once

SuperBlitter used: yes, for rectangle blitting to screen and cursor restoration.
Sometimes also for generic copying between buffers (see above).

Triple buffering:
~~~~~~~~~~~~~~~~~

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

- in case of extremely fast rendering, one or more frames are dropped in favor
  of showing only the most recent one

SuperBlitter used: yes, for rectangle blitting to screen and cursor restoration.
Sometimes also for generic copying between buffers (see above).

Triple buffering is the default mode for this port.


SuperVidel and SuperBlitter
---------------------------

As mentioned, this port uses SuperVidel and its SuperBlitter heavily. That
means that if the SuperVidel is detected, it does the following:

- uses 8bpp chunky resolutions

- patches all surface addresses with OR'ing 0xA0000000, i.e. using SV RAM
  instead of slow ST RAM (and even instead of TT RAM for allowing pure
  SuperBlitter copying)

- when SuperVidel FW version >= 9 is detected, the async FIFO buffer is used
  instead of the slower sync blitting (where one has to wait for every
  rectangle blit to finish), this sometimes leads to nearly zero-cost rendering
  and makes a *huge* difference for 640x480 fullscreen updates.


Audio mixing
------------

ScummVM works internally with 16-bit stereo samples. This mode is not available
on the TT so a substitute solution must be used. This solution is called STFA
by The Removers: http://removers.free.fr/softs/stfa.php. Install, activate STFA
BIOS in the CPX, done. Now you have 16-bit DMA available, too but beware, it is
also quite CPU demanding so very few games can actually make use of it (see the
chapter about audio performance considerations below).


Performance considerations/pitfalls
-----------------------------------

It's important to understand what affects performance on our limited platform
to avoid unpleasant playing experiences.

Game engines with unexpected performance hit
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A typical example from this category is Gobliiins (and its sequels) and SCI
engine games (Gabriel Knight, Larry 2/7, ...). At first it looks like our
machine or Atari backend is doing something terribly wrong but the truth is
that it is the engine itself which is doing a lot of unnecessary redraws and
updates, sometimes even before reaching the backend. The only solution is to
profile and fix those engines.

Too many fullscreen updates
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Somewhat related to the previous point - sometimes the engine authors didn't
realize the impact of every update on the overall performance and instead of
updating only the rectangles that really had changed, they ask for a fullscreen
update. Not a problem on a >1 GHz machine but very visible on Atari! Also, this
is (by definition) the case of animated intros, especially those in 640x480.

MIDI vs. AdLib vs. sampled music
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It could seem that sample music replay must be the most demanding one but on
the contrary! Always choose a CD version of a game (with *.wav tracks) to any
other version. With one exception: if you have a native MIDI device able to
replay the given game's MIDI notes (using the STMIDI plugin).

MIDI emulation (synthesis) can easily eat as much as 50% of all used CPU time
(on the CT60). By default, this port uses the MAME OPL emulation (which is said
to be fastest but also least accurate) but some engines require the DOSBOX one
which is even more demanding. By the way, you can put "FM_high_quality=true"
or "FM_medium_quality=true" into scummvm.ini if you want to experiment with a
better quality synthesis, otherwise the lowest quality will be used (applies
for MAME OPL only).

On the TT, in most cases it makes sense to use ScummVM only if you own a
native MIDI synthesizer (like mt32-pi: https://github.com/dwhinham/mt32-pi).
MIDI emulation is out of question and STFA takes a good chunk of CPU time for
downsampling to 8-bit resolution which could be utilized elsewhere.

CD music slows everything down
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Some games use separate audio *and* video streams (files). Even if the CPU is
able to handle both, the bottleneck becomes ... disk access. This is visible in
The Curse Of Monkey Island for example -- there's audible stuttering during the
intro sequence (and during the game as well). Increasing "audio_buffer_size"
makes the rendering literally crawling! Why? Because disk I/O is busy with
loading even *more* sample data so there's less time for video loading and
rendering. Try to put "musdisk1.bun" and "musdisk2.bun" into a ramdisk (i.e.
u:/ram in FreeMiNT), you'll be pleasantly surprised with the performance boost
gained.

Mute vs. "No music"
~~~~~~~~~~~~~~~~~~~

Currently ScummVM requires each backend to mix samples, even though they may
contain muted output (i.e. zeroes). This is because the progression of sample
playback tells ScummVM how much time has passed in e.g. an animation.

"No music" means using the null audio plugin which prevents generating any MIDI
music (and therefore avoiding the expensive synthesis emulation) but beware, it
doesn't affect CD (*.wav) playback at all! Same applies for speech and sfx.

The least amount of cycles is spent when:
- "No music" (or keep it default and choose a native MIDI device) is set in the
   GUI options; this prevents MIDI sythesis of any kind
- all external audio files are deleted (typically *.wav); that way the mixer
  wont have anything to mix. However beware, this is not allowed in every game!

Slow GUI
~~~~~~~~

Themes handling is quite slow - each theme must be depacked, each one contains
quite a few XML files to parse and quite a few images to load/convert. That's
the reason why the built-in one is used as default, it dramatically speeds up
loading time. To speed things up in other cases, the "fat" version is
distributed with repackaged theme files with compression level zero.


"Slim" vs. "Fat" version
------------------------

As a further optimisation step, a 030-only version of ScummVM is provided,
aimed at accelerated TT and Falcon machines with the 68030 CPU. It further
restricts features but also improves performance:

- compiled with -m68030 => 68030/68882-specific optimisations enabled

- disabled 040+/SuperVidel code => faster code path for blitting

- doesn't support hires (640x480) games => smaller executable size

- overlay is rendered in 16 colours => faster redraw

- overlay during gameplay has no game backround => ever faster redraw

- overlay doesn't support alternative themes => faster loading time

- "STMIDI" driver is automatically enabled (i.e. MIDI emulation is never used
  but still allows playing speech/sfx samples and/or CD audio)


Known issues
------------

- aspect ratio correction works on RGB only (yet)

- adding a game in TOS and loading it in FreeMiNT (and vice versa) generates
  incompatible paths. Either use only one system or edit scummvm.ini and set
  there only relative paths (mintlib bug/limitation).

- when run on TT, screen contains horizontal black lines. That is due to the
  fact that TT offers only 320x480 in 256 colours. Possibly fixable by a Timer
  B interrupt.

- horizontal screen shaking doesn't work on TT because TT Shifter doesn't
  support fine scrolling.

- tooltips in overlay are sometimes drawn with corrupted background.

- the talkie version of MI1 needs to be merged from two sources: first generate
  the DOS version and then additionally also the flac version. Then convert all
  *.flac files into *.wav and replace monkey.sof (flac) with monster.sou (DOS).
  And of course, don't forget to set the extra path in Game options to the
  folder where *.wav files are located! For MI2 just use the DOS version,
  there are no CD tracks available. :(


Future plans
------------

- aspect ratio correction for TT/VGA/SuperVidel

- unified file paths in scummvm.ini

- DSP-based sample mixer (WAV, FLAC, MP2)

- avoid loading music/speech files (and thus slowing down everything) if muted

- cached audio/video streams (i.e. don't load only "audio_buffer_size" number
  of samples but cache, say, 1 second so disk i/o wont be so stressed)

- using LDG or Thorsten Otto's sharedlibs: https://tho-otto.de/sharedlibs.php
  for game engine plugins to relieve the huge binary size

- true audio CD support via MetaDOS API

- OPL2LPT and Retrowave support (if I manage to purchase it somewhere)


Closing words
-------------

This backend is part of ScummVM 2.8.0 onwards. Let's see whether we can make it
to the official website. :-)

MiKRO / Mystic Bytes, XX.XX.2023
Kosice / Slovakia
miro.kropacek@gmail.com
http://mikro.atari.org
