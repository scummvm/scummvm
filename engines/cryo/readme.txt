Citadel of Mo, the last remaining place where humans can be safe from army
of vicious Tyrannosaurus led by allmighty Morkus Rex. What await young Adam,
prince of Mo, who just came of age and want to travel across the world?
Will he be able to restore long lost friendship between dinousaurus and humans?


This is SCUMMVM reimplementation of Cryo's Lost Eden game engine. In order to
stay as close as possible to original game and minimize number of bugs
introduced during code reconstruction, in its current state this project is
a straight reverse-engineered game code hooked up to SCUMMVM framework.
Because of that, this code in no way represent the quality or coding practices
of SCUMMVM itself. Essentially, this is how the game was originally written.

There are few Lost Eden game versions known to exists.

- Non-interactive PC demo version. Basically, a number of video files played
  in a loop with FM music in background. Google for "ANCIBUR2.HNM" file to
  find it.

- Interactive PC demo version. Allows to play through whole Citadel of Mo
  then shows "Order Now" banner.
  Can be found here: http://www.ag.ru/games/lost-eden/demos/2677
  Download is a self-extracting archive, unpack it with 7zip or other tool.

- PC version. Main version of the game. Written in assembly and partially based
  on Dune's game code.
  Runs in real-mode DOS environment, uses VGA 320x200 graphics and digitized
  sounds/music. Allows to select several languages for in-game subtitles. It is
  rumored that bootleg Russian translation also exists. Has 3 predefined slots
  for game save/load.

- MAC version. Almost identical to PC version. Has slightly modified UI. Such
  as exta spaces on the inventory bar, resized subtitles overlay and different
  implementation of mouse cursor (which is worse than the original one). Looks
  like screen transition effects are also changed / rearranged.
  This version have no limit on save game slots. Standard system file selection
  dialogs are used instead. All screen hot-spots coordinates loaded from
  resource file instead of hard-coded values in PC version.

- 3DO version. Uses completely different resource formats.

- CDI version. Uses completely different resource formats.

- CD32 version. Mentioned in PC demo version, but never released?


This reimplementation project is based on MAC version, since it's much easier
to work with than any other versions. As such, currently only MAC version
supported. Adding support for PC versions shouldn't be too difficult and
mainly involves implementing of proper resources indexing/loading. Game
engine loads resources by their indexes in game archive file and these indexes
are different between PC and MAC versions.

At this moment the game is fully playabe/completeable. List of currently
discovered bugs can be found in bugs.txt file. None of those are critical or
game-breaking. No sound support is present in this version, although all
sound front-end code is in place and should be operational. Only single
game save/load slot is supported and saves are not cross-platform compatible.
Also, no game restart feature work due to the way it's implemented.

Due to the way original engine is coded, it is not fully portable in its
current state. Game uses conditional dialog system (similar system is used in
Dune game as well) and this system involves accessing native engine
variables through predefined offsets within large structure. Any modification
of this structure will result in broken dialog system.

Because of limited development environment, this code is only tested on
MSVS2013 32-bit compiler. There may be some issues with GCC/LLVM compilers
or on 64-bit platforms. As mentioned above, this code is neither pretty or
bug-free (aka it's a can of worms). Several original bugs, various oddities
and problematic areas are marked with TODO comment in the source code. There
are number of variables with non-descripitve names like byte_1234, those
purpose is yet to be clearly understood. To make code debugging easier,
EDEN_DEBUG macro activates several extra features. Some parts, like image
drawing routines, can be simplified/generalized. Other parts, like CLView,
can be replaced with existing SCUMMVM classes.

Because parts of this code (mainly decompression and video playback) used
by other Cryo's games, it might be worthy to make them reusable by future
engines.
