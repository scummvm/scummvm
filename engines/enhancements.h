/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ENGINES_ENHANCEMENTS_H
#define ENGINES_ENHANCEMENTS_H

/* Game enhancements */

/* "How should I mark an enhancement?" - A practical guide for the developer:
 *
 *  Hi! If you're here it means that you are probably trying to make up
 *  your mind about how to correctly mark your brand new game enhancement:
 *  if that's the case... congratulations, you've come to the right place! :-)
 *
 *  Marking a piece of code as an enhancement is as simple as guarding it with
 *  a conditional check using the enhancementEnabled(<class>) function.
 *  For example:
 *
 *      if (enhancementEnabled(<kMyBeautifulEnhancementClass>)) {
 *          // Piece of code which makes Guybrush hair white
 *      }
 *
 *  That's it! :-)
 *
 *  You've probably noticed that the function above needs a class in order to work.
 *  Of course there is no one kind of enhancement: each of them might tackle
 *  different aspect of the game, from the graphics to the gameplay itself.
 *  So it all comes to identifying the correct class for your enhancement.
 *
 *  We identify nine different enhancement classes:
 *
 *  --- Gamebreaking Bug Fixes ---
 *
 *  This is a class of enhancements which is ALWAYS active; it encapsulates
 *  code changes which were not in the original game but which are absolutely
 *  necessary in order to avoid deadlocks or even crashes! Being able to differentiate
 *  between original code and these enhancements can be quite useful for future
 *  developers and their research (e.g. "why is this code different from the disassembly?",
 *  "why did they change it like that?").
 *
 *  --- Minor Bug Fixes ---
 *
 *  Did the original developers blit a green pixel on a blue tinted background
 *  and YOU'RE ABSOLUTELY SURE that they didn't do that on purpose? Is one of the
 *  voice files playing as garbled noise instead of sounding like normal speech?
 *  Is the game looking like there is something which is clearly wrong with it?
 *  This is the class you're looking for, then!
 *
 *  --- Text and Localization Fixes ---
 *
 *  If you spot any issues which pertain texts (accents not being rendered properly
 *  on localizations, placeholder lines forgotten by the developers, obvious typos), and
 *  which are NOT about the format of the subtitle (color, position) or the content,
 *  then this is the class you should be using.
 *  Do not use this class when changing an already grammatically correct line to another
 *  line for the purpose of matching the text to the speech line!
 *  Use "Subtitle Format/Content Changes" instead.
 *
 *  --- Visual Changes ---
 *
 *  Any graphical change which is not classifiable as a "fix" but is, as a matter of fact,
 *  a deliberate change which strays away from the original intentions, should be marked
 *  with this class. Some examples of this are enhancements which modify palettes and add
 *  or edit graphical elements in order to better match a particular "reference" version.
 *
 *  --- Audio Changes ---
 *
 *  Like above, but for anything sound related.
 *
 *  --- Timing Adjustments ---
 *
 *  Are you making a scene slower or faster for any reason? Are you changing the framerate
 *  of an in-game situation? Choose this class!
 *
 *  --- Subtitles Format/Content Changes ---
 *
 *  Any changes to the subtitles format should be classified under this class.
 *  This also includes changes to the subtitles content when not under the "fix" umbrella,
 *  for example when you are changing an already grammatically and graphically correct line
 *  to match it with the corresponding speech file.
 *
 *  --- Restored Cut Content ---
 *
 *  Have you found any line of dialog, a graphical element or even a piece of music which
 *  is in the game data but is not being used? Go nuts with this enhancement class then! :-)
 *
 *  --- UI/UX Enhancements ---
 *
 *  These old games are beautiful. But sometimes they can be so clunky... :-)
 *  If you make any changes in order to yield a slightly-less-clunky user experience
 *  you should classify them under this class. Here's a couple of real use cases:
 *  - SAMNMAX:     the CD version of the game begins with what seems to be a fake loading
 *                 screen for the sounds. We have an enhancement which just skips that :-)
 *  - Early games: some early titles use a save menu screen which is piloted by SCUMM scripts,
 *                 and therefore run at an in-game framerate. This causes lag and lost keypresses
 *                 from your keyboard when attempting to write the names of your savegames.
 *                 We remove the framerate cap so that writing is not painful anymore... :-P
 *
 */

enum {
	kEnhGameBreakingBugFixes = 1 << 0, // Gamebreaking Bug Fixes
	kEnhMinorBugFixes = 1 << 1,        // Minor Bug Fixes
	kEnhTextLocFixes = 1 << 2,         // Text and Localization Fixes
	kEnhVisualChanges = 1 << 3,        // Visual Changes
	kEnhAudioChanges = 1 << 4,         // Audio Changes
	kEnhTimingChanges = 1 << 5,        // Timing Adjustments
	kEnhSubFmtCntChanges = 1 << 6,     // Subtitles Format/Content Changes
	kEnhRestoredContent = 1 << 7,      // Restored Cut Content
	kEnhUIUX = 1 << 8,                 // UI/UX Enhancements
};

/* "How are the enhancements grouped?" - A practical guide to follow if you're lost:
 *
 *  GROUP 1: Fix original bugs
 *
 *  This category includes both game-breaking bugs which cause the game to crash/deadlock and
 *  minor bug fixes (e.g. text and localization issues). Enhancements in this category should
 *  pertain stuff which is very clearly a bug (for example a badly shaped walkbox, a wrong accent
 *  in a word from the subtitles, a strip of pixels which is very clearly out of place/with the
 *  wrong palette, AND NOT include things like subtitles and boxes color changes, enhancements
 *  which make a version similar to another, etc.). Basically when this and only this is active,
 *  the game should not have deadlock situations and the immersiveness should not be broken by
 *  very evident graphical glitches, charset issues, etc.
 *
 *  GROUP 2: Audio-visual improvements
 *
 *  This category comprises visual and audio changes as well as timing adjustments. This is the
 *  category in which we can basically put everything which I said not to put in the previous
 *  category. This includes: changing the spacing of the font from the original, changing colors
 *  of subtitles for consistency, changes to the subtitles content in order to match the speech
 *  or to fix the localization, music changes (like the ones in COMI and FT), graphic changes
 *  which are not as essential as the ones from the previous category, etc.
 *
 *  GROUP 3: Restored content
 *
 *  This category reintroduces content cut or unused which was not in the original. This
 *  can include content which was somehow masked by mistake by the scripts.
 *
 *  GROUP 4: Modern UI/UX adjustments
 *
 *  This category pertains to all enhancements to the user interface and user experience:
 *  e.g. the artificial loading screen at the beginning of Sam&Max, speeding up the framerate
 *  in old original menus to have a decent keyboard polling rate.
 *
 */

enum {
	kEnhGrp1 = (kEnhMinorBugFixes | kEnhTextLocFixes),
	kEnhGrp2 = (kEnhVisualChanges | kEnhAudioChanges | kEnhTimingChanges | kEnhSubFmtCntChanges),
	kEnhGrp3 = (kEnhRestoredContent),
	kEnhGrp4 = (kEnhUIUX)
};

#endif