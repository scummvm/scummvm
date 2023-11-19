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

#ifndef SCUMM_DETECTION_H
#define SCUMM_DETECTION_H

#include "common/language.h"
#include "common/platform.h"

namespace Scumm {


// GUI-options, primarily used by detection_tables.h
#define GUIO_TRIM_FMTOWNS_TO_200_PIXELS                GUIO_GAMEOPTIONS1
#define GUIO_ENHANCEMENTS                              GUIO_GAMEOPTIONS2
#define GUIO_AUDIO_OVERRIDE                            GUIO_GAMEOPTIONS3
#define GUIO_ORIGINALGUI                               GUIO_GAMEOPTIONS4
#define GUIO_LOWLATENCYAUDIO                           GUIO_GAMEOPTIONS5
#define GUIO_NETWORK                                   GUIO_GAMEOPTIONS6

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
	kEnhGameBreakingBugFixes  = 1 << 0, // Gamebreaking Bug Fixes
	kEnhMinorBugFixes         = 1 << 1, // Minor Bug Fixes
	kEnhTextLocFixes          = 1 << 2, // Text and Localization Fixes
	kEnhVisualChanges         = 1 << 3, // Visual Changes
	kEnhAudioChanges          = 1 << 4, // Audio Changes
	kEnhTimingChanges         = 1 << 5, // Timing Adjustments
	kEnhSubFmtCntChanges      = 1 << 6, // Subtitles Format/Content Changes
	kEnhRestoredContent       = 1 << 7, // Restored Cut Content
	kEnhUIUX                  = 1 << 8, // UI/UX Enhancements
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

/**
 * Descriptor of a specific SCUMM game. Used internally to store
 * information about the tons of game variants that exist.
 */
struct GameSettings {
	/**
	 * The gameid of this game.
	 */
	const char *gameid;

	/**
	 * An identifier which can be used to distinguish game variants.
	 * This string is also used to augment the description string
	 * generated by the detector, and to search the gameFilenamesTable.
	 * It is also used to search the MD5 table (it matches the "extra"
	 * data in scumm-md5.txt).
	 *
	 * Equal to 0 (zero) *if and only if* the game has precisely one
	 * variant. Failing to obey this rule can lead to odd bugs.
	 */
	const char *variant;

	/**
	 * An optional string that will be added to the 'preferredtarget'
	 * computed by the detector.
	 */
	const char *preferredTag;

	/**
	 * The numerical gameid of this game.
	 * This is not in one-to-one correspondence with the gameid above.
	 * But if two games settings have the same id (except for GID_HEGAME),
	 * then they also have the same gameid ; the converse does not hold
	 * in general.
	 */
	byte id;

	/** The SCUMM version. */
	byte version;

	/** The HE subversion. */
	byte heversion;

	/** MidiDriverFlags values */
	int midi;

	/**
	 * Bitmask obtained by ORing various GameFeatures enums, and used
	 * to en-/disable certain features of this game variant.
	 */
	uint32 features;

	/**
	 * Platform indicator, this is set to a value different from
	 * kPlatformUnknown if this game variant only existed for this
	 * specific platform.
	 */
	Common::Platform platform;

	/**
	 * Game GUI options. Used to enable/disable certain GUI widgets
	 */
	const char *guioptions;
};

enum FilenameGenMethod {
	kGenDiskNum,
	kGenDiskNumSteam,
	kGenRoomNum,
	kGenRoomNumSteam,
	kGenHEMac,
	kGenHEMacNoParens,
	kGenHEPC,
	kGenHEIOS,
	kGenUnchanged
};

struct FilenamePattern {
	const char *pattern;
	FilenameGenMethod genMethod;
};

struct GameFilenamePattern {
	const char *gameid;
	const char *pattern;
	FilenameGenMethod genMethod;
	Common::Language language;
	Common::Platform platform;
	const char *variant;
};

struct DetectorResult {
	FilenamePattern fp;
	GameSettings game;
	Common::Language language;
	Common::String md5;
	const char *extra;
};

/**
 * SCUMM feature flags define for every game which specific set of engine
 * features are used by that game.
 * Note that some of them could be replaced by checks for the SCUMM version.
 */
enum GameFeatures {
	/** A demo, not a full blown game. */
	GF_DEMO = 1 << 0,

	/** Games with the AKOS costume system (ScummEngine_v7 and subclasses, HE games). */
	GF_NEW_COSTUMES = 1 << 2,

	/** Games using XOR encrypted data files. */
	GF_USE_KEY = 1 << 4,

	/** Small header games (ScummEngine_v4 and subclasses). */
	GF_SMALL_HEADER = 1 << 5,

	/** Old bundle games (ScummEngine_v3old and subclasses). */
	GF_OLD_BUNDLE = 1 << 6,

	/** EGA games. */
	GF_16COLOR = 1 << 7,

	/** VGA versions of V3 games.  Equivalent to (version == 3 && not GF_16COLOR) */
	GF_OLD256 = 1 << 8,

	/** Games which have Audio CD tracks. */
	GF_AUDIOTRACKS = 1 << 9,

	/**
	 * Games using only very few local variables in scripts.
	 * Apparently that is only the case for 256 color version of Indy3.
	 */
	GF_FEW_LOCALS = 1 << 11,

	/** HE games for which localized versions exist */
	GF_HE_LOCALIZED = 1 << 13,

	/**
	 *  HE games with more global scripts and different sprite handling
	 *  i.e. read it as HE version 9.85. Used for HE98 only.
	 */
	GF_HE_985 = 1 << 14,

	/** HE games with 16 bit color */
	GF_16BIT_COLOR = 1 << 15,

	/**
	 * SCUMM v5-v7 Mac games stored in a container file
	 * Used to differentiate between m68k and PPC versions of Indy4
	 */
	GF_MAC_CONTAINER = 1 << 16,

	/**
	 * SCUMM HE Official Hebrew translations were audio only
	 * but used reversed string for credits etc.
	 * Used to disable BiDi in those games.
	 */
	GF_HE_NO_BIDI = 1 << 17,

	/**
	 * The "Ultimate Talkie" versions of Monkey Island, which have been
	 * patched so that most workarounds/bugfixes no longer apply to them.
	 */
	GF_ULTIMATE_TALKIE = 1 << 18
};

enum ScummGameId {
	GID_CMI,
	GID_DIG,
	GID_FT,
	GID_INDY3,
	GID_INDY4,
	GID_LOOM,
	GID_MANIAC,
	GID_MONKEY_EGA,
	GID_MONKEY_VGA,
	GID_MONKEY,
	GID_MONKEY2,
	GID_PASS,
	GID_SAMNMAX,
	GID_TENTACLE,
	GID_ZAK,

	GID_HEGAME,      // Generic name for all HE games with default behavior
	GID_PUTTDEMO,
	GID_FBEAR,
	GID_PUTTMOON,
	GID_FUNPACK,
	GID_PUTTZOO,
	GID_FREDDI,
	GID_FREDDI3,
	GID_FREDDI4,
	GID_BIRTHDAYRED,
	GID_BIRTHDAYYELLOW,
	GID_TREASUREHUNT,
	GID_PUTTRACE,
	GID_FUNSHOP,	// Used for all three funshops
	GID_FOOTBALL,
	GID_FOOTBALL2002,
	GID_SOCCER,
	GID_SOCCERMLS,
	GID_SOCCER2004,
	GID_BASEBALL2001,
	GID_BASEBALL2003,
	GID_BASKETBALL,
	GID_MOONBASE,
	GID_PJGAMES,
	GID_HECUP		// CUP demos
};

struct RuScummPatcher {
	ScummGameId gameid;
	const char *variant;
	const char *patcherName;
};
} // End of namespace Scumm


#endif
