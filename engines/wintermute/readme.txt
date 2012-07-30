Wintermute Engine - Copyright (c) 2011 Jan Nedoma
ScummVM port by Einar Johan Trøan Sømåen (somaen)

***************************
*** General information ***
***************************
The Wintermute Engine is a game engine mainly aimed at creating adventure
games, it supports both 3D, 2.5D, 2D and First-Person games, and has numerous
games both free, open-source and commercial created with it. This port was
created by somaen as part of Google Summer of Code 2012.

****************
*** Features ***
****************
This port of the Wintermute Engine (WME) is based on WME Lite, which lacks the following functionality
originally found in WME: (from http://res.dead-code.org/doku.php/wmelite:start)

The following features of WME 1.x are NOT supported by WME Lite:
* 3D characters. WME Lite only supports 2D games.
* Sprite frame mirroring.
* Sprite rotations.
* Sprite blending modes.
* Video playback.
* Plugins.
* Calling external functions from DLL libraries from scripts.
* Game Explorer support.
* 'Directory' script object.

This port does reimplement a few of these features, currently:
* Sprite frame mirroring - WORKS.
* Video playback - Theora PARTIALLY WORKING. (Slow, and doesn't support seeking)

In addition, this port removes a few additional features that were never/rarely used:
* 'File' script object - ScummVM doesn't have any easy way to write/read arbitrary files.
* Debugger/Compiler - weren't properly accessible in WME Lite anyhow.
* CD-numbering support in .dcp-files - was never used.
* 'SaveDirectory'-property of 'Game' will not return anything usefull to the game-scripts (saving is handled through SaveFileMan)

*******************************
*** Additional limitations: ***
*******************************
* Only .OGG and RAW-.WAV sounds are supported at this point
* TTF-fonts might behave a bit differently, owing to both the change to FreeType in WME Lite
    and the change in dpi in this port of WME.
* The window-caption-setting in-game will be ignored, for the sake of concistency with ScummVM.
* Most VKey-combinations might still be missing (as they already were in WME Lite)
* Since we don't use FreeImage, some games might use odd files that weren't expected when the 
    image-decoders in ScummVM were written. One example here is interlaced-PNGs.
* UTF8-support is not ported, which means only games with western charsets will work for now.
* Games that select language by moving .dcp-files around still need a bit more handling on detection/load
    adding support for those languages on a language-by-language basis.
* Most games assume the availability of the Windows-fonts (particularly arial.ttf)
    at this point no fallback has been put in place for using FreeFonts as replacements,
	simply for lack of having them easily accessible to the engines at this point. So, at least
	arial.ttf should be put in either the game-folder or made available through the extras-folder
	for now, otherwise kGUIBigFont will be used as a replacement.
	

*********************************
*** Advanced engine-features: ***
*********************************
At this point the engine implements the following "advanced engine features":
* RTL ("Return to Launcher") support
* Global options dialog support
* Listing savestates via command line or Launcher
* Loading savestates via command line or Launcher
* Deleting savestates via the Launcher and GMM
* Savestate metadata support
* Loading/Saving during run time

and NOT the following:
* Enhanced debug/error messages

*****************
*** Detection ***
*****************
Since Wintermute has authoring tools available, there will at any point in
time be atleast a few games that are works-in-progress, and as the authors
of these games might want to test their games in ScummVM, the engine has
to be able to detect arbitrary Wintermute-games, to this end the detector
code in this engine will check any folder containing "data.dcp", and try to
read "startup.settings" and "default.game" (or optionally any other .game-file
defined in startup.settings), the Name/Caption fields in the .game-file will
be used as gameid/title (prefixing the gameid with "wmefan-" to avoid confusion
with any other WME game that might happen to have taken that id.

All COMPLETED games should have their md5s and gameid's properly added, IFF
they don't require 3D. 

3D games may also be added, for the purpose of giving the user feedback
as to why their game won't run, but at this point, any such MD5 should
be added as a comment only, to avoid confusion, as no mechanism for giving
the user feedback about 3D-games not being supported is currently added.

*************************************
*** Games targeted by the engine: ***
*************************************
This engine potentially targets a very large amount of games:
http://res.dead-code.org/doku.php/games:start

Since the feature-set of WME Lite differs from that of the full Wintermute Engine,
games will need to be targeted on a case-by-case, feature-by-feature basis, this is
a list of the games that are currently known to work (although perhaps with minor
issues) through to completion:

* Dirty Split (dirtysplit)
* the white chamber (twc)
* Chivalry is NOT dead (chivalry)
* Rosemary (rosemary)
* The Box (thebox)
* J.U.L.I.A. (Demo) (julia)
* Pigeons in the park (pigeons)

Untested, but starts:
* East Side Story (Demo) (eastside)
* Actual Destination (actualdest)
* Ghost in the sheet (ghostsheet)

********************************
*** Games with known issues: ***
********************************
Certain games will work mostly fine with this engine, but can still
be impossible to complete for various reasons, this is a list of games
that technically qualify (as in they do not require the 3D-parts of the engine)
but have issues that make them problematic or not completable:

Won't start:
* Five Lethal Demons (5ld) - Requires support for interlaced PNGs
* Five Magical Amulets (5ma) - Requires support for interlaced PNGs
* Kulivoeko - Requires support for interlaced PNGs
* Reversion (reversion) - Requires support for Non-V1.1 JPEGs and interlaced PNGs
* Mirage (mirage) - Tries to seek in a vorbis-stream inside a ZipStream
* Hamlet or the last game without MMORPS features, shaders and product placement (hamlet)
    - Requires support for interlaced PNGs

Gameplay broken:
* J.U.L.I.A. (Full game) (julia) - Requires sprite-rotation for a puzzle.

Non-critical:
* Ghost in the sheet (ghostsheet) - uses Non-V1.1-JPEGs
* East Side Story (eastside) - wants "framd.ttf"

*****************************
*** General known issues: ***
*****************************

Mostly a TODO-section, to not forget fixing outstanding general issues:
* Save/Load-screens are not shown during save/load
    this is probably a result of reducing the amount of redrawing done
	during save/load, and I'm not sure it should be put back, if that means
	making saves slower again.
* Font-sizes are wrong enough to allow Dirty Split to draw text that is hidden in
    the original game (most visible on the coin-interface)
* Alpha-masks for Theora-videos are broken on big-endian platforms

