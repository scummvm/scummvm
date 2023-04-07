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
years (https://docs.scummvm.org/en/v2.6.1/other_platforms/atari.html). It is
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


Main features
-------------

- Optimized for the Atari Falcon (ideally with the CT60/CT63/CT60e but for the
less hungry games even a CT2/DFB@50 MHz or the AfterBurner040 could be enough).

- Full support for the SuperVidel, incl. the SuperBlitter (!)

- Removed features found too demanding for our platform; the most visible
  change is the exclusion of the 16bpp games (those are mostly hi-res anyway)
  but games in 640x480@8bpp work nicely.

- Direct rendering and single/double/triple buffering support.

- Custom (and optimal) drawing routines (especially for the cursor).

- Custom (Super)Videl resolutions for the best possible performance and visual
  experience (320x240 in RGB, chunky modes with SuperVidel, 640x480@8bpp for
  the overlay, ...)

- Custom (hardware based) aspect ratio correction (!)

- Support for PC keys (page up, page down, pause, F11/F12, ...) and mouse wheel
  (Eiffel/Aranym only)

This makes such games as The Curse of Monkey Island better playable (on
SuperVidel nearly always also with CD (WAV) music and speech). Also, AdLib
emulation works nicely with many games without noticeable slow downs.


Platform-specific features outside the GUI
------------------------------------------

Keyboard shortcut "CONTROL+u": immediate mute on/off toggle (disables also
sample mixing, contrary to what "Mute all" in the options does!)

Keyboard shortcut "CONTROL+ALT+a": immediate aspect ratio correction on/off
toggle.

"output_rate" in scummvm.ini: sample rate for mixing, can be 49170, 32780,
24585, 19668, 16390, 12292, 9834, 8195 (the lower the value, the faster the
mixing but also in worse quality). Default is 24585 Hz (16-bit, stereo).

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

Direct rendering (vsync on/off) - present only with the SuperVidel
Single buffering (vsync on/off)
Double buffering (vsync always on, the checkbox is ignored)
Triple buffering (vsync always off, the checkbox is ignored)

Direct rendering:
~~~~~~~~~~~~~~~~~

This is direct writing of the pixels into (SuperVidel's) screen buffer. Since
the updates are supplied as rectangles and not the whole screen there's no way
to implement direct writing *and* double/triple buffering. Vsync() only
synchronizes the point when the rendering process begins - if it takes more
than the time reserved for the vertical blank interrupt (what happens
with most of the games), you'll see screen tearing.

Pros:

- fastest possible rendering (especially in 640x480 with a lot of small
  rectangle updates where the buffer copying drags performance down)

Cons:

- screen tearing in most cases

- SuperVidel only: using C2P would be not only suboptimal (every rectangle
  would be C2P'ed instead of just copy and C2P of the final screen) but poses an
  additional problem as C2P requires data aligned on a 16px boundary and
  ScummVM supplies arbitrarily-sized rectangles (this is solvable by custom
  Surface allocation but it's not bullet-proof). In theory I could implement
  direct rendering for the Falcon hicolor (320x240@16bpp) but this creates
  another set of issues like when palette would be updated but not the whole
  screen - so some rectangles would be rendered in old palette and some in new.

SuperBlitter used: sometimes (when ScummVM allocates surface via its create()
function; custom/small buffers originating in the engine code are still copied
using the CPU).

Single buffering:
~~~~~~~~~~~~~~~~~

This is very similar to the previous mode with the difference that the engine
uses an intermediate buffer for storing the rectangles but yet it remembers
which ones they were. It works also on plain Videl and applies the chunky to
planar process to each one of the rectangles separately, avoiding fullscreen
updates (but if such is needed, there is an optimized code path for it). Vsync()
is used the same way as in the previous mode, i.e. screen tearing is still
possible.

Pros:

- second fastest possible rendering

Cons:

- screen tearing in most cases

- if there is too many smaller rectangles, it can be less efficient than
  updating the whole buffer at once

SuperBlitter used: yes, for rectangle blitting to screen and cursor restoration.
Sometimes also for generic copying between buffers (see above).

Double buffering:
~~~~~~~~~~~~~~~~~

The most common rendering mode. It extends the idea of single buffering - it
renders into two buffers, one is visible while the other one is used for
updating. At the end of the update process the two buffers are swapped, so the
newly updated one is displayed. By definition, Vsync() must be always enabled
(the buffers are swapped in the vertical blank handler) otherwise you'd see
screen tearing.

Pros:

- stable frame rate, leading to fixed e.g. 30 FPS rendering for the whole time
  if game takes, say, 1.7 - 1.9 frames per update

- no screen tearing in any situation

Cons:

- if there is too many smaller rectangles, it can be less efficient than
  single buffering

- frame rate is set to 60/30/15/etc FPS so you can see big irregular jumps
  between 30 and 15 FPS for example; this is happening when screen updates take
  variable amount of time but since Vsync() is always called, the rendering
  pipeline has to wait until the next frame even if only 1% of the frame time
  has been used.

SuperBlitter used: yes, for rectangle blitting to screen and cursor restoration.
Sometimes also for generic copying between buffers (see above).

Triple buffering:
~~~~~~~~~~~~~~~~~

Best of both worlds - screen tearing is avoided thanks to using of multiple
buffers and the rendering pipeline doesn't have to wait until Vsync() (therefore
this flag is ignored).

Please note that Atari backend uses "true" triple buffering as described in
https://en.wikipedia.org/wiki/Multiple_buffering#Triple_buffering and not "swap
chain" as described in https://en.wikipedia.org/wiki/Swap_chain. The latter
would be slightly slower as three buffers would need to be updated instead of
two.

Pros:

- best compromise between performance and visual experience

- works well with both higher and lower frame rates

Cons:

- if there is too many smaller rectangles, it can be less efficient than
  single buffering

- slightly irregular frame rate (depends solely on the game's complexity)

- in case of extremely fast rendering, one or more frames are dropped in favor
  of showing only the most recent one (unlikely; double buffer guaranties that
  every frame is shown, no matter how insignificant)

SuperBlitter used: yes, for rectangle blitting to screen and cursor restoration.
Sometimes also for generic copying between buffers (see above).

Triple buffering is the default mode for this port.


SuperVidel and SuperBlitter
---------------------------

As mentioned, this port uses SuperVidel and its SuperBlitter heavily. That
means that if the SuperVidel is detected, it does the following:

- patches all 8bpp VGA resolutions to chunky ones, rendering all C2P routines
  useless

- patches all surface addresses with OR'ing 0xA0000000, i.e. using SV RAM
  instead of slow ST RAM (and even instead of TT RAM for allowing pure
  SuperBlitter copying)

- when SuperVidel FW version >= 9 is detected, the async FIFO buffer is used
  instead of the slower sync blitting (where one has to wait for every
  rectangle blit to finish), this sometimes leads to nearly zero-cost rendering
  and makes a *huge* difference for 640x480 fullscreen updates.


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

It could seem that sample music replay must be the most demanding one but on the
contrary! _Always_ choose a CD version of a game (with *.wav tracks) to any
other version. With one exception: if you have a native MIDI device able to
replay the given game's MIDI notes (using the STMIDI plugin). MIDI emulation
(synthesis) can easily eat as much as 50% of all used CPU time.

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

"Mute" vs. "Mute all" in GUI vs. "No music" in GUI
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Not the same thing. "Mute" (available only via the shortcut CONTROL+u) generates
an event to which the sample mixer can react (i.e. stop mixing the silence...).

"Mute all" doesn't generate anything, it basically just lowers the volume of the
music to zero.

"No music" means using the null audio plugin which prevents generating any MIDI
music (and therefore avoiding the expensive synthesis emulation) but beware, it
doesn't affect CD (*.wav) playback at all!

So for the best performance, always choose "No music" in the GUI options when
the game contains MIDI tracks and "Mute" when the game contains a sampled
soundtrack.

Please note that it is not that bad, you surely can play The Secret of Monkey
Island with AdLib enabled (but the CD/talkie versions sound better and
are cheaper to play ;)).

Vsync in GUI
~~~~~~~~~~~~

Carefully with the vsync option. It can easily cripple direct/single buffer
rendering by 10-15 FPS if not used with caution. That happens if a game takes,
say, 1.2 frames per update (so causing screen tearing anyway and rendering the
option useless) but Vsync() forces it to wait 2 full frames instead.

Slow GUI
~~~~~~~~

Themes handling is quite slow - each theme must be depacked, each one contains
quite a few XML files to parse and quite a few images to load/convert. That's
the reason why the built-in one is used as default, it dramatically speeds up
loading time. A compromise solution is to depack the theme in an equally named
directory (i.e. avoiding the depacking phase) but you need a filesystem with
long name support for that to work.


Known issues
------------

- aspect ratio correction works on RGB only (yet)

- SuperVidel's DVI output is stretched when in 320x200 or 640x400; I'll  wait
  for other people's experiences, maybe only my LCD is so lame.

- adding a game in TOS and loading it in FreeMiNT (and vice versa) generates
  incompatible paths. Either use only one system or edit scummvm.ini and set
  there only relative paths (mintlib bug/limitation).

- the talkie version of MI1 needs to be merged from two sources: first generate
  the DOS version and then additionally also the flac version. Then convert all
  *.flac files into *.wav and replace monkey.sof (flac) with monster.sou (DOS).
  And of course, don't forget to set the extra path in Game options to the
  folder where *.wav files are located! For MI2 just use the DOS version,
  there are no CD tracks available. :(


Future plans
------------

- aspect ratio correction for VGA/SuperVidel

- unified file paths in scummvm.ini

- DSP-based sample mixer

- avoid loading music/speech files (and thus slowing down everything) if muted

- cached audio/video streams (i.e. don't load only "audio_buffer_size" number
  of samples but cache, say, 1 second so disk i/o wont be so stressed)

- using LDG or Thorsten Otto's sharedlibs: https://tho-otto.de/sharedlibs.php
  for game engine plugins to relieve the huge binary size

- add support for the TT030; this would be easily possible when I rewrite the
  renderer with a more flexible resolution switching

- ignore (queue) updateScreen() calls to avoid aggressive drawing / buffer
  switching from some engines; update every X ms instead

- don't hardcode some of the buffers for cacheing purposes, determine the size
  based on amount of free RAM

- true audio CD support via MetaDOS API

- OPL2LPT and Retrowave support (if I manage to purchase it somewhere)

- engines based on Graphics::Screen don't have to use my chunky buffer (however
  it may be tricky to detect this situation)

- C2P could support 4- and 6-bit depth

- increase file buffer size with setvbuf or by using
  Common::wrapBufferedWriteStream and disabling stdio buffering


Closing words
-------------

This backend is part of ScummVM 2.8.0 onwards. Let's see whether we can make it
to the official website. :-)

MiKRO / Mystic Bytes, XX.XX.2023
Kosice / Slovakia
miro.kropacek@gmail.com
http://mikro.atari.org
