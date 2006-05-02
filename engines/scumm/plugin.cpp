/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "backends/fs/fs.h"

#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/list.h"
#include "common/md5.h"
#include "common/system.h"	// Only needed for g_system

#include "scumm/plugin.h"
#include "scumm/scumm.h"
#include "scumm/intern.h"
#include "scumm/he/intern_he.h"

#ifdef PALMOS_68K
#include "extras/palm-scumm-md5.h"
#else
#include "scumm/scumm-md5.h"
#endif


namespace Scumm {

#pragma mark -
#pragma mark --- Data types & constants ---
#pragma mark -

enum {
	// We only compute the MD5 of the first megabyte of our data files.
	kMD5FileSizeLimit = 1024 * 1024
};

struct ObsoleteGameID {
	const char *from;
	const char *to;
	Common::Platform platform;
};

#define UNK Common::kPlatformUnknown



#pragma mark -
#pragma mark --- Tables ---
#pragma mark -


/**
 * This table contains all game IDs supported by the SCUMM engine, and maps
 * them to the full humand readable game name.
 */
static const PlainGameDescriptor gameDescriptions[] = {
	{ "atlantis", "Indiana Jones and the Fate of Atlantis" },
	{ "indy3", "Indiana Jones and the Last Crusade" },
	{ "loom", "Loom" },
	{ "maniac", "Maniac Mansion" },
	{ "monkey", "The Secret of Monkey Island" },
	{ "monkey2", "Monkey Island 2: LeChuck's Revenge" },
	{ "pass", "Passport to Adventure" },
	{ "samnmax", "Sam & Max Hit the Road" },
	{ "tentacle", "Day of the Tentacle" },
	{ "zak", "Zak McKracken and the Alien Mindbenders" },

#ifndef DISABLE_SCUMM_7_8
	{ "ft", "Full Throttle" },
	{ "dig", "The Dig" },
	{ "comi", "The Curse of Monkey Island" },
#endif

	{ "fbear", "Fatty Bear's Birthday Surprise" },
	{ "fbpack", "Fatty Bear's Fun Pack" },
	{ "funpack", "Putt-Putt's Fun Pack" },
	{ "puttmoon", "Putt-Putt Goes to the Moon" },
	{ "puttputt", "Putt-Putt Joins the Parade" },

#ifndef DISABLE_HE
	{ "activity", "Putt-Putt & Fatty Bear's Activity Pack" },
	{ "airport", "Let's Explore the Airport with Buzzy" },
	{ "artdemo", "Blue's Art Time Activities (Demo)" },
	{ "balloon", "Putt-Putt and Pep's Balloon-O-Rama" },
	{ "baseball", "Backyard Baseball" },
	{ "baseball2001", "Backyard Baseball 2001" },
	{ "Baseball2003", "Backyard Baseball 2003" },
	{ "basketball", "Backyard Basketball" },
	{ "BluesABCTime", "Blue's ABC Time" },
	{ "BluesBirthdayDemo", "Blue's Birthday Adventure (Demo)" },
	{ "catalog", "Humongous Interactive Catalog" },
	{ "chase", "SPY Fox in Cheese Chase" },
	{ "dog", "Putt-Putt and Pep's Dog on a Stick" },
	{ "farm", "Let's Explore the Farm with Buzzy" },
	{ "football", "Backyard Football" },
	{ "freddi", "Freddi Fish 1: The Case of the Missing Kelp Seeds" },
	{ "freddi2", "Freddi Fish 2: The Case of the Haunted Schoolhouse" },
	{ "freddi3", "Freddi Fish 3: The Case of the Stolen Conch Shell" },
	{ "freddi4", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch" },
	{ "freddicove", "Freddi Fish 5: The Case of the Creature of Coral Cave" },
	{ "FreddisFunShop", "Freddi Fish's One-Stop Fun Shop" },
	{ "jungle", "Let's Explore the Jungle with Buzzy" },
	{ "lost", "Pajama Sam's Lost & Found" },
	{ "maze", "Freddi Fish and Luther's Maze Madness" },
	{ "mustard", "SPY Fox in Hold the Mustard" },
	{ "pajama", "Pajama Sam 1: No Need to Hide When It's Dark Outside" },
	{ "pajama2", "Pajama Sam 2: Thunder and Lightning Aren't so Frightening" },
	{ "pajama3", "Pajama Sam 3: You Are What You Eat From Your Head to Your Feet" },
	{ "pjgames", "Pajama Sam: Games to Play On Any Day" },
	{ "puttcircus", "Putt-Putt Joins the Circus" },
	{ "puttrace", "Putt-Putt Enters the Race" },
	{ "PuttsFunShop", "Putt-Putt's One-Stop Fun Shop" },
	{ "putttime", "Putt-Putt Travels Through Time" },
	{ "puttzoo", "Putt-Putt Saves the Zoo" },
	{ "readdemo", "Blue's Reading Time Activities (Demo)" },
	{ "SamsFunShop", "Pajama Sam's One-Stop Fun Shop" },
	{ "soccer", "Backyard Soccer" },
	{ "Soccer2004", "Backyard Soccer 2004" },
	{ "SoccerMLS", "Backyard Soccer MLS Edition" },
	{ "socks", "Pajama Sam's Sock Works" },
	{ "spyfox", "SPY Fox 1: Dry Cereal" },
	{ "spyfox2", "SPY Fox 2: Some Assembly Required" },
	{ "spyozon", "SPY Fox 3: Operation Ozone" },
	{ "thinker1", "Big Thinkers First Grade" },
	{ "thinkerk", "Big Thinkers Kindergarten" },
	{ "water", "Freddi Fish and Luther's Water Worries" },
#endif
	{ 0, 0 }
};

/**
 * Conversion table mapping old obsolete game IDs to the
 * corresponding new game ID and platform combination.
 */
static const ObsoleteGameID obsoleteGameIDsTable[] = {
	{"bluesabctimedemo", "bluesabctime", UNK},
	{"comidemo", "comi", UNK},
	{"digdemo", "dig", UNK},
	{"digdemoMac", "dig", Common::kPlatformMacintosh},
	{"dottdemo", "tentacle", UNK},
	{"fate", "atlantis", UNK},
	{"ftMac", "ft",  Common::kPlatformMacintosh},
	{"ftpcdemo", "ft", UNK},
	{"ftdemo", "ft",  Common::kPlatformMacintosh},
	{"game", "monkey", UNK},
	{"indy3ega", "indy3", UNK},
	{"indy3towns", "indy3", Common::kPlatformFMTowns},
	{"indy4", "atlantis", Common::kPlatformFMTowns},
	{"indydemo", "atlantis", Common::kPlatformFMTowns},
	{"loomcd", "loom", UNK},
	{"loomTowns", "loom", Common::kPlatformFMTowns},
	{"mi2demo", "monkey2", UNK},
	{"monkey1", "monkey", UNK},
	{"monkeyEGA", "monkey", UNK},
	{"monkeyVGA", "monkey", UNK},
	{"playfate", "atlantis", UNK},
	{"samnmax-alt", "samnmax", UNK},
	{"samnmaxMac", "samnmax", Common::kPlatformMacintosh},
	{"samdemo", "samnmax", UNK},
	{"samdemoMac", "samnmax", Common::kPlatformMacintosh},
	{"snmdemo", "samnmax", UNK},
	{"snmidemo", "samnmax", UNK},
	{"tentacleMac", "tentacle", Common::kPlatformMacintosh},
	{"zakTowns", "zak", Common::kPlatformFMTowns},
	{NULL, NULL, UNK}
};

// The following table contains information about variants of our various
// games. We index into it with help of md5table (from scumm-md5.h), to find
// the correct GameSettings for a given game variant.
//
// The first listed variant is assumed to be the 'default' variant -- i.e. the
// variant that gets used when no explicit variant code has been specified.
//
// Note #1: Only set 'platform' to a value different from UNK if that game
// variant really *only* exists for that given platform. In all other cases,
// the correct platform will be determined via the MD5 table or derived from
// the filename.
//
// Note #2: Make sure that all variants for a given gameid are in sequence with
// no gaps. Some code may rely on this and stop searching the table early as
// soon as the gameid changes.
//
// Note #3: Use 0 (zero) for the variant field *if and only if* the game has
// only a single unique variant. This is used to help the detector quickly
// decide whether it has to worry about distinguishing multiple variants or not.
static const GameSettings gameVariantsTable[] = {
	{"maniac", "C64",  GID_MANIAC, 0, 0, MDT_PCSPK, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformC64},
	{"maniac", "V1",   GID_MANIAC, 1, 0, MDT_PCSPK, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformPC},
	{"maniac", "NES",  GID_MANIAC, 1, 0, MDT_NONE,  GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformNES},
	{"maniac", "V2",   GID_MANIAC, 2, 0, MDT_PCSPK, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, UNK},
	{"maniac", "Demo", GID_MANIAC, 2, 0, MDT_PCSPK, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE | GF_DEMO, Common::kPlatformPC},

	{"zak", "V1",       GID_ZAK, 1, 0, MDT_PCSPK, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, UNK},
	{"zak", "V2",       GID_ZAK, 2, 0, MDT_PCSPK, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, UNK},
	{"zak", "FM-TOWNS", GID_ZAK, 3, 0, MDT_TOWNS, GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},

	{"indy3", "EGA",      GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, UNK},
	{"indy3", "No Adlib", GID_INDY3, 3, 0, MDT_PCSPK,             GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, UNK},
	{"indy3", "VGA",      GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB, GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS, Common::kPlatformPC},
	{"indy3", "FM-TOWNS", GID_INDY3, 3, 0, MDT_TOWNS,             GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS | GF_AUDIOTRACKS, Common::kPlatformFMTowns},

	{"loom", "EGA",      GID_LOOM, 3, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, UNK},
	{"loom", "No Adlib", GID_LOOM, 3, 0, MDT_PCSPK,                        GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, UNK},
	{"loom", "FM-TOWNS", GID_LOOM, 3, 0, MDT_TOWNS,                        GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"loom", "VGA",      GID_LOOM, 4, 0, MDT_NONE,                         GF_SMALL_HEADER | GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},

	{"pass", 0, GID_PASS, 4, 0, MDT_PCSPK | MDT_ADLIB, GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},

	{"monkey", "VGA",      GID_MONKEY_VGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI, GF_SMALL_HEADER | GF_USE_KEY, Common::kPlatformPC},
	{"monkey", "EGA",      GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI, GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"monkey", "No Adlib", GID_MONKEY_EGA, 4, 0, MDT_PCSPK,                        GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformAtariST},
	{"monkey", "Demo",     GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB,            GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"monkey", "CD",       GID_MONKEY,     5, 0, MDT_ADLIB, GF_USE_KEY | GF_AUDIOTRACKS, UNK},
	{"monkey", "FM-TOWNS", GID_MONKEY,     5, 0, MDT_ADLIB, GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"monkey", "SEGA",     GID_MONKEY,     5, 0, MDT_NONE,  GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformSegaCD},

	{"monkey2",  0, GID_MONKEY2, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK},

	{"atlantis", 0, GID_INDY4, 5, 0, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK},

	{"tentacle", 0, GID_TENTACLE, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK},

	{"samnmax",  0, GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK},

#ifndef DISABLE_SCUMM_7_8
	{"ft", "",       GID_FT,  7, 0, MDT_NONE, GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, UNK},
	{"ft", "Demo",   GID_FT,  7, 0, MDT_NONE, GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, UNK},

	{"dig", "",      GID_DIG, 7, 0, MDT_NONE, GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, UNK},
	{"dig", "Demo",  GID_DIG, 7, 0, MDT_NONE, GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, UNK},

	{"comi", "",     GID_CMI, 8, 0, MDT_NONE, GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, Common::kPlatformWindows},
	{"comi", "Demo", GID_CMI, 8, 0, MDT_NONE, GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformWindows},
#endif

	// Humongous Entertainment Scumm Version 6
	{"puttmoon", "", GID_HEGAME,  6, 61, MDT_ADLIB | MDT_MIDI, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"funpack",  0, GID_FUNPACK, 6, 61, MDT_ADLIB | MDT_MIDI, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"fbpack",   0, GID_HEGAME,  6, 61, MDT_ADLIB | MDT_MIDI, GF_USE_KEY | GF_NEW_COSTUMES, UNK},

	{"puttputt", "HE 61", GID_HEGAME,   6, 61, MDT_ADLIB | MDT_MIDI, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"puttputt", "HE 60", GID_HEGAME,   6, 60, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, Common::kPlatformPC},
	{"puttputt", "Demo",  GID_PUTTDEMO, 6, 60, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK},

	{"fbear", "HE 61", GID_FBEAR, 6, 61, MDT_ADLIB | MDT_MIDI, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"fbear", "HE 70", GID_FBEAR, 6, 70, MDT_NONE,             GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

#ifndef DISABLE_HE
	{"activity", 0, GID_HEGAME, 6, 70, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},

	// Humongous Entertainment Scumm Version 7.1
	// The first version to use 640x480 resolution
	// There are also 7.1 versions of freddemo, airdemo and farmdemo
	{"catalog", "", GID_HEGAME, 6, 71, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"freddi", "", GID_HEGAME, 6, 71, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},

	// Humongous Entertainment Scumm Version 7.2
	{"airport", "", GID_HEGAME, 6, 72, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"puttzoo", "", GID_HEGAME, 6, 72, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},

	// Changed o_getResourceSize to cover all resource types
	{"farm", "", GID_HEGAME, 6, 73, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"jungle", "", GID_HEGAME, 6, 73, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},

	// Humongous Entertainment Scumm Version 8.0 ?  Scummsrc.80
	{"freddi2", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"pajama", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"putttime", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},

	{"balloon", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"dog", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"maze", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"socks", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},

	{"water", "",      GID_WATER, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"water", "HE 80", GID_WATER, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"water", "HE 99", GID_WATER, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},

	// Humongous Entertainment Scumm Version 9.0 ?  Scummsys.90
	{"baseball", "", GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"thinkerk", "", GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"thinker1", "", GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"freddi3", "", GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"spyfox", "", GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},

	// Humongous Entertainment Scumm Version 9.5 ?  Scummsys.95
	{"pajama2", "", GID_HEGAME, 6, 95, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"chase", "", GID_HEGAME, 6, 95, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},

	// Humongous Entertainment Scumm Version 9.8 ?  Scummsys.98
	// these and later games can easily be identified by the .(a) file instead of a .he1
	// and INIB chunk in the .he0
	{"lost", "", GID_HEGAME, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},

	{"puttrace", "HE 98",   GID_PUTTRACE, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"puttrace", "HE 98.5", GID_PUTTRACE, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, UNK},
	{"puttrace", "HE 99",   GID_PUTTRACE, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},

	{"bluesabctime", "", GID_HEGAME, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"soccer", "", GID_SOCCER, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},

	// Global scripts increased to 2048
	{"freddi4", "",       GID_HEGAME, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, UNK},
	{"freddi4", "unenc",  GID_HEGAME, 6, 98, MDT_NONE,              GF_NEW_COSTUMES | GF_HE_985, UNK},

	// Humongous Entertainment Scumm Version 9.9 ?  Scummsys.99
	{"football", 0, GID_FOOTBALL, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"pajama3", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, UNK},
	{"puttcircus", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, UNK},
	{"spyfox2", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, UNK},
	{"mustard", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, UNK},

	// Added the use of fonts
	{"FreddisFunShop", 0, GID_FUNSHOP, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, UNK},
	{"SamsFunShop", 0, GID_FUNSHOP, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, UNK},
	{"PuttsFunShop", 0, GID_FUNSHOP, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, UNK},

	// Added 16bit color
	{"baseball2001", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, UNK},
	{"SoccerMLS", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK},
	{"spyozon", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK},

	{"freddicove", 0,        GID_HEGAME, 6,  99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK},
	{"freddicove", "unenc",  GID_HEGAME, 6,  99, MDT_NONE,              GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK},
	{"freddicove", "HE 100", GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK},

	// Restructured the Scumm engine
	{"pjgames", 0, GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK},

	// Uses bink in external files for logos
	{"Baseball2003", 0, GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, UNK},
	{"basketball", 0, GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"Soccer2004", 0, GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, UNK},

	// Uses smacker in external files, for testing only
	{"BluesBirthdayDemo", 0, GID_HEGAME, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"artdemo", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, UNK},
	{"readdemo", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, UNK},


	// The following are meant to be generic HE game variants and as such do
	// not specify a game ID. Make sure that these are last in the table, else
	// they'll override more specific entries that follow later on.
	{"", "HE 70",   GID_HEGAME, 6,  70, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"", "HE 71",   GID_HEGAME, 6,  71, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"", "HE 72",   GID_HEGAME, 6,  72, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"", "HE 73",   GID_HEGAME, 6,  73, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"", "HE 80",   GID_HEGAME, 6,  80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"", "HE 90",   GID_HEGAME, 6,  90, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"", "HE 95",   GID_HEGAME, 6,  95, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"", "HE 98",   GID_HEGAME, 6,  98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"", "HE 98.5", GID_HEGAME, 6,  98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, UNK},
	{"", "HE 99",   GID_HEGAME, 6,  99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
	{"", "HE 100",  GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, UNK},
#endif
	{NULL, NULL, 0, 0, MDT_NONE, 0, 0, UNK}
};

using Common::UNK_LANG;

// The following describes how Fingolfin thinks this table might be used one day;
// this is work in progress, so read this with a salt of grain...
//
// The following table maps gameids to possible filename variants for that game.
// This information is used by the detector to determin possible "detect files".
// It is also later used by the engine creation code to verify the game to be
// launched is present. Finally, the correct GameFilenamePattern entry is passed on
// to the engine which uses it to locate the files for the game.
//
// The table is augmented by platform/language/variant information where applicable.
//
// Note: Setting variant to 0 means "don't care", while setting it to ""
// (i.e. an empty string) means "use the default variant".
static const GameFilenamePattern gameFilenamesTable[] = {
	{ "maniac", "%02d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "maniac", "%02d.MAN", kGenRoomNum, UNK_LANG, UNK, "Demo" },
	{ "maniac", "maniac1.d64", kGenUnchanged, UNK_LANG, Common::kPlatformC64, "C64" },   // ... and maniac2.d64
	{ "maniac", "Maniac Mansion (E).prg", kGenUnchanged, Common::EN_GRB, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (F).prg", kGenUnchanged, Common::FR_FRA, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (SW).prg", kGenUnchanged, Common::SE_SWE, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (U).prg", kGenUnchanged, Common::EN_USA, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (G).prg", kGenUnchanged, Common::DE_DEU, Common::kPlatformNES, "NES" },

	{ "zak", "%02d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "zak", "zak1.d64", kGenUnchanged, UNK_LANG, Common::kPlatformC64, 0 },         // ... and zak2.d64

	{ "indy3", "%02d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },

	{ "loom", "%02d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "loom", "%03d.LFL", kGenRoomNum, UNK_LANG, UNK, "VGA" },	// Loom CD

	{ "pass", "%03d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },

	{ "monkey", "%03d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },		// EGA & VGA versions
	{ "monkey", "monkey.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "monkey", "monkey1.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "monkey", "monkeyk.%03d", kGenDiskNum, Common::JA_JPN, Common::kPlatformFMTowns, "FM-TOWNS" }, // FM-TOWNS Jap
	{ "monkey", "game.%03d", kGenDiskNum, UNK_LANG, Common::kPlatformSegaCD, "SEGA" }, // SegaCD

	{ "monkey2", "monkey2.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "monkey2", "mi2demo.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },

	{ "atlantis", "atlantis.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "atlantis", "fate.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "atlantis", "playfate.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "atlantis", "indy4.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "atlantis", "indydemo.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "atlantis", "Fate of Atlantis Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "tentacle", "tentacle.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "tentacle", "dottdemo.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "tentacle", "Day of the Tentacle Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "tentacle", "Day of the Tentacle Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "samnmax", "samnmax.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "samnmax.sm%d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "Sam & Max Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "samnmax", "Sam & Max Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "samnmax", "ramnmax.%03d", kGenDiskNum, Common::RU_RUS, UNK, 0 }, // Used in some releases of Russian Sam'n'Max
	{ "samnmax", "samdemo.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "snmdemo.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "snmidemo.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "sdemo.sm%d", kGenDiskNum, UNK_LANG, UNK, 0 },

#ifndef DISABLE_SCUMM_7_8
	{ "dig", "dig.la%d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "dig", "The Dig Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "dig", "The Dig Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, "Demo" },

	{ "ft", "ft.la%d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "ft", "ft.%03d", kGenDiskNum, UNK_LANG, UNK, "Demo" },    // Used by PC version of Full Throttle demo
	{ "ft", "ftdemo.la%d", kGenDiskNum, UNK_LANG, UNK, "Demo" },
	{ "ft", "Full Throttle Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "ft", "Full Throttle Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, "Demo" },
	{ "ft", "Vollgas Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "ft", "Vollgas Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, "Demo" },

	{ "comi", "comi.la%d", kGenDiskNum, UNK_LANG, UNK, 0 },
#endif

	{ "fbear", "fbear", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "fbear", "fbdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "fbear", "Fatty Bear Demo", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "fbear", "Fatty Bear", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "fbpack", "fbpack", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "funpack", "funpack", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "puttmoon", "puttmoon", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttmoon", "moondemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttmoon", "Putt-Putt Moon Demo", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttmoon", "Putt-Putt Moon", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "puttputt", "puttputt", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttputt", "puttdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttputt", "Putt-Putt's Demo", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttputt", "Putt-Putt Parade", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },

#ifndef DISABLE_HE
	{ "airport", "airport", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "airport", "airdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "airport", "Airport Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "airport", "The AirPort", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "activity", "activity", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "balloon", "balloon", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "balloon", "Balloon-O-Rama", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "baseball", "baseball", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "baseball", "BaseBall", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "baseball2001", "baseball2001", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "baseball2001", "bb2demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "baseball2001", "Baseball 2001 Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "baseball2001", "Baseball 2001", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "baseball2001", "baseball 2001", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "Baseball2003", "Baseball2003", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "Baseball2003", "Baseball 2003", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "basketball", "basketball", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "basketball", "Basketball", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "bluesabctime", "bluesabctime", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "bluesabctime", "BluesABCTimeDemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "bluesabctime", "BluesABCTimeDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "catalog", "catalog", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "catalog", "catalog2", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "chase", "chase", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "chase", "Cheese Chase", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "dog", "dog", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "dog", "Dog on a Stick", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "farm", "farm", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "farm", "farmdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "farm", "Farm Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "football", "football", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "football", "FootBall", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "football", "FootBall Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "football", "FootBall2002", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "football", "footdemo", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "freddi", "freddi", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi", "freddemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi", "Freddi Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi", "Freddi Fish", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi", "FreddiD", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "freddi2", "freddi2", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi2", "ff2-demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi2", "FFHSDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi2", "FFHSDemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi2", "Freddi Fish 2 Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi2", "Freddi Fish 2", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi2", "FreddiCHSH", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi2", "Fritzi Fisch 2", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },

	{ "freddi3", "freddi3", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi3", "F3-mdemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "F3-Mdemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "f3-mdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi3", "FF3-DEMO", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi3", "Freddi Fish 3", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "FreddiFGT", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "freddi3", "FreddiFGT", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "FreddiSCS", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi3", "Fritzi3demo", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "Fritzi3demo", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "freddi3", "MM3-DEMO", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "freddi3", "MM3-Demo", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },

	{ "freddi4", "freddi4", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi4", "f4-demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi4", "ff4demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi4", "Ff4demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "Freddi 4", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "Freddi 4 Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "FreddiGS", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "freddi4", "FreddiGS", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "FreddiHRBG", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "freddicove", "freddicove", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "FreddiCCC", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "FreddiCove", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddicove", "FreddiDZZ", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "ff5demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "FFCoveDemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "FreddiCoveDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddicove", "FF5Demo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },

	{ "FreddisFunShop", "FreddisFunShop", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "FreddisFunShop", "Freddi's FunShop", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "jungle", "jungle", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "jungle", "The Jungle", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "lost", "lost", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "lost", "Lost and Found", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "lost", "smaller", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "maze", "maze", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "maze", "Maze Madness", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "mustard", "mustard", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "mustard", "Mustard", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "pajama", "pajama", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama", "Pyjama Pit", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama", "Pajama Sam", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama", "PajamaNHD", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama", "PJS-DEMO", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama", "PYJAMA", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama", "SAMDEMO", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "pajama", "SAMDEMO", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },

	{ "pajama2", "pajama2", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama2", "Pajama Sam 2", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama2", "PajamaTAL", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama2", "PyjamaDBMN", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama2", "PyjamaDBMN", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama2", "Pyjama Pit 2 Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama2", "PJP2DEMO", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama2", "PJ2Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama2", "pj2demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama2", "Pjs2demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama2", "PJ2 Demo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },

	{ "pajama3", "pajama3", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama3", "GPJ3Demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama3", "Pajama Sam 3", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "Pajama Sam 3-Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "pj3-demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama3", "pj3demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama3", "PJ3Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "Pajama Sam Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "PjSamDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "PjSamDemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama3", "PyjamaSKS", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "pajama3", "PyjamaSKS", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "UKPajamaEAT", kGenHEPC, Common::RU_RUS, UNK, 0 },

	{ "pjgames", "pjgames", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pjgames", "PJGames", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "puttcircus", "puttcircus", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttcircus", "circdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttcircus", "Putt Circus Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttcircus", "Putt Circus", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "puttrace", "puttrace", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttrace", "500demo", kGenHEPC, Common::NL_NLD, UNK, 0 },
	{ "puttrace", "racedemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttrace", "RaceDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttrace", "Rennen", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "puttrace", "Putt500 demo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "puttrace", "Putt Race", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttrace", "ToffRennen", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "puttrace", "ToffRennen", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "puttrace", "UKPuttRace", kGenHEPC, Common::RU_RUS, UNK, 0 }, // Russian

	{ "PuttsFunShop", "PuttsFunShop", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "PuttsFunShop", "Putt's FunShop", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "putttime", "putttime", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "putttime", "PuttPuttTTT", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "putttime", "PuttPuttTTT", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "putttime", "PuttTijd", kGenHEPC, UNK_LANG, UNK, 0 },	// FIXME: Is this a dutch version?
	{ "putttime", "Putt Time", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "putttime", "PuttTTT", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "putttime", "PuttTTT", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "putttime", "TIJDDEMO", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "putttime", "timedemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "putttime", "TimeDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "putttime", "TEMPDEMO", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "putttime", "Tempdemo", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "putttime", "toffzeit", kGenHEPC, Common::DE_DEU, UNK, 0 }, // German Toeff-Toeff: Reist durch die Zeit
	{ "putttime", "toffzeit", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 }, // German Toeff-Toeff: Reist durch die Zeit
	{ "putttime", "ZeitDemo", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "putttime", "ZEITDEMO", kGenHEPC, Common::DE_DEU, UNK, 0 },

	{ "puttzoo", "puttzoo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttzoo", "Puttzoo Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttzoo", "PuttZoo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 }, 
	{ "puttzoo", "T\xC3\xB6""ff-T\xC3\xB6""ff\xE2\x84\xA2 Zoo Demo", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },	// UTF-8 encoding
	{ "puttzoo", "T\xF6""ff-T""\xF6""ff\x99 Zoo Demo", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },	// Windows encoding
	{ "puttzoo", "zoodemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttzoo", "Zoo Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "SamsFunShop", "SamsFunShop", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "SamsFunShop", "Sam's FunShop", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "soccer", "soccer", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "soccer", "Soccer", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "Soccer2004", "Soccer2004", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "Soccer2004", "Soccer 2004", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "socks", "socks", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "socks", "SockWorks", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "spyfox", "spyfox", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "Fuchsdem", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "FUCHSDEM", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "FoxDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "foxdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "JAMESDEM", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "spyfox", "Spydemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "Spydemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "SPYFox", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "SPYFoxDC", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "SPYFoxDC", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "SpyFoxDMK", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "SpyFoxDMK", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "Spy Fox Demo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "JR-Demo", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },

	{ "spyfox2", "spyfox2", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox2", "sf2-demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox2", "sf2demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox2", "Sf2demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox2", "Spy Fox 2 - Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox2", "Spy Fox 2", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox2", "SpyFoxOR", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox2", "SpyFoxOR", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox2", "spyfoxsr", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "spyozon", "spyozon", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyozon", "sf3-demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyozon", "Spy Ozone Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyozon", "SPYFoxOZU", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyozon", "SpyOzon", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "thinker1", "thinker1", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "thinker1", "1grademo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "thinker1", "Thinker1", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "thinkerk", "thinkerk", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "thinkerk", "kinddemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "thinkerk", "KindDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "thinkerk", "ThinkerK", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "water", "water", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "water", "Water Worries", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
#endif
	{ NULL, NULL, kGenUnchanged, UNK_LANG, UNK, 0 }
};


#pragma mark -
#pragma mark --- Miscellaneous ---
#pragma mark -


static const char *findDescriptionFromGameID(const char *gameid) {
	const PlainGameDescriptor *g = gameDescriptions;
	while (g->gameid) {
		if (!scumm_stricmp(g->gameid, gameid)) {
			return g->description;
		}
		g++;
	}
	error("Unknown gameid '%s' encountered in findDescriptionFromGameID", gameid);
}

static int compareMD5Table(const void *a, const void *b) {
	const char *key = (const char *)a;
	const MD5Table *elem = (const MD5Table *)b;
	return strcmp(key, elem->md5);
}

const MD5Table *findInMD5Table(const char *md5) {
#ifdef PALMOS_68K
	uint32 arraySize = MemPtrSize((void *)md5table) / sizeof(MD5Table) - 1;
#else
	uint32 arraySize = ARRAYSIZE(md5table) - 1;
#endif
	return (const MD5Table *)bsearch(md5, md5table, arraySize, sizeof(MD5Table), compareMD5Table);
}

Common::String ScummEngine::generateFilename(const int room) const {
	const int diskNumber = (room > 0) ? res.roomno[rtRoom][room] : 0;
	char buf[128];

	if (_game.version == 4) {
		if (room == 0 || room >= 900) {
			snprintf(buf, sizeof(buf), "%03d.lfl", room);
		} else {
			snprintf(buf, sizeof(buf), "disk%02d.lec", diskNumber);
		}
	} else {
		char id = 0;

		switch (_filenamePattern.genMethod) {
		case kGenDiskNum:
			snprintf(buf, sizeof(buf), _filenamePattern.pattern, diskNumber);
			break;
	
		case kGenRoomNum:
			snprintf(buf, sizeof(buf), _filenamePattern.pattern, room);
			break;

		case kGenHEMac:
		case kGenHEMacNoParens:
		case kGenHEPC:
			if (room < 0) {
				id = '0' - room;
			} else if (_game.heversion >= 98) {
				int disk = 0;
				if (_heV7DiskOffsets)
					disk = _heV7DiskOffsets[room];
		
				switch(disk) {
				case 2:
					id = 'b';
					snprintf(buf, sizeof(buf), "%s.(b)", _filenamePattern.pattern);
					break;
				case 1:
					id = 'a';
					snprintf(buf, sizeof(buf), "%s.(a)", _filenamePattern.pattern);
					break;
				default:
					id = '0';
					snprintf(buf, sizeof(buf), "%s.he0", _filenamePattern.pattern);
				}
			} else if (_game.heversion >= 70) {
				id = (room == 0) ? '0' : '1';
			} else {
				id = diskNumber + '0';
			}
			
			if (_filenamePattern.genMethod == kGenHEPC) {
				// For HE >= 98, we already called snprintf above.
				if (_game.heversion < 98 || room < 0)
					snprintf(buf, sizeof(buf), "%s.he%c", _filenamePattern.pattern, id);
			} else {
				if (id == '3') { // special case for cursors
					// For mac they're stored in game binary
					strncpy(buf, _filenamePattern.pattern, sizeof(buf));
				} else {
					if (_filenamePattern.genMethod == kGenHEMac)
						snprintf(buf, sizeof(buf), "%s (%c)", _filenamePattern.pattern, id);
					else
						snprintf(buf, sizeof(buf), "%s %c", _filenamePattern.pattern, id);
				}
			}

			break;

		case kGenUnchanged:
			strncpy(buf, _filenamePattern.pattern, sizeof(buf));
			break;

		default:
			error("generateFilename: Unsupported genMethod");
		}
	}

	return buf;
}

Common::String generateFilenameForDetection(const GameFilenamePattern &gfp) {
	char buf[128];

	switch (gfp.genMethod) {
	case kGenDiskNum:
	case kGenRoomNum:
		snprintf(buf, sizeof(buf), gfp.pattern, 0);
		break;

	case kGenHEPC:
		snprintf(buf, sizeof(buf), "%s.he0", gfp.pattern);
		break;

	case kGenHEMac:
		snprintf(buf, sizeof(buf), "%s (0)", gfp.pattern);
		break;

	case kGenHEMacNoParens:
		snprintf(buf, sizeof(buf), "%s 0", gfp.pattern);
		break;

	case kGenUnchanged:
		strncpy(buf, gfp.pattern, sizeof(buf));
		break;

	default:
		error("generateFilenameForDetection: Unsupported genMethod");
	}

	return buf;
}

struct DetectorDesc {
	Common::String path;
	Common::String md5;
	uint8 md5sum[16];
	const MD5Table *md5Entry;	// Entry of the md5 table corresponding to this file, if any.
};

void detectGames(const FSList &fslist, Common::List<DetectorResult> &results, const char *gameid) {
	typedef Common::HashMap<Common::String, DetectorDesc, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> DescMap;
	DescMap fileMD5Map;
	const GameSettings *g;
	DetectorResult dr;
	
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			DetectorDesc d;
			d.path = file->path();
			d.md5Entry = 0;
			fileMD5Map[file->displayName()] = d;
		}
	}

	// Iterate over all filename patterns.
	for (const GameFilenamePattern *gfp = gameFilenamesTable; gfp->gameid; ++gfp) {
		// If a gameid was specified, we only try to detect that specific game,
		// so we can just skip over everything with a differing gameid.
		if (gameid && scumm_stricmp(gameid, gfp->gameid))
			continue;
	
		// Generate the detectname corresponding to the gfp. If the file doesn't
		// exist in the directory we are looking at, we can skip to the next
		// one immediately.
		Common::String file(generateFilenameForDetection(*gfp));
		if (!fileMD5Map.contains(file))
			continue;

		// Reset the DetectorResult variable
		dr.fp.pattern = gfp->pattern;
		dr.fp.genMethod = gfp->genMethod;
		dr.game.gameid = 0;
		dr.language = gfp->language;
		dr.md5.clear();
		memset(dr.md5sum, 0, 16);
		dr.extra = 0;
		
		//  ____            _     _ 
		// |  _ \ __ _ _ __| |_  / |
		// | |_) / _` | '__| __| | |
		// |  __/ (_| | |  | |_  | |
		// |_|   \__,_|_|   \__| |_|
		//
		// PART 1: Trying to find an exact match using MD5.
		//
		//
		// Background: We found a valid detection file. Check if its MD5
		// checksum occurs in our MD5 table. If it does, try to use that
		// to find an exact match.
		//
		// We only do that if the MD5 hadn't already been computed (since
		// we may look at some detection files multiple times).
		//
		DetectorDesc &d = fileMD5Map[file];
		if (d.md5.empty()) {
			uint8 md5sum[16];
			if (Common::md5_file(d.path.c_str(), md5sum, kMD5FileSizeLimit)) {
				char md5str[32+1];
				for (int j = 0; j < 16; j++) {
					sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
				}

				d.md5 = md5str;
				memcpy(d.md5sum, md5sum, 16);
				d.md5Entry = findInMD5Table(md5str);

				dr.md5 = d.md5;
				memcpy(dr.md5sum, d.md5sum, 16);

				if (d.md5Entry) {
					// Exact match found
					dr.language = d.md5Entry->language;
					dr.extra = d.md5Entry->extra;

					// Compute the precise game settings using gameVariantsTable.
					for (g = gameVariantsTable; g->gameid; ++g) {
						if (g->gameid[0] == 0 || !scumm_stricmp(d.md5Entry->gameid, g->gameid)) {
							// The gameid either matches, or is empty. The latter indicates
							// a generic entry, currently used for some generic HE settings.
							if (g->variant == 0 || !scumm_stricmp(d.md5Entry->variant, g->variant)) {
								// Perfect match found, use it and stop the loop
								dr.game = *g;
								dr.game.gameid = d.md5Entry->gameid;

								// Sanity check
								if (dr.game.platform != Common::kPlatformUnknown && dr.game.platform != d.md5Entry->platform)
									warning("SCUMM detectGames: Platform values differ for MD5 '%s': %d vs %d (please report to Fingolfin)",
													md5str, dr.game.platform, d.md5Entry->platform);
								dr.game.platform = d.md5Entry->platform;
								
								results.push_back(dr);
								break;
							}
						}
					}

					// Sanity check: We *should* have found a matching gameid / variant at this point.
					// If not, then there's a bug in our data tables...
					assert(dr.game.gameid != 0);
				}
			}
		}
		
		// If an exact match for this file has already been found, don't bother 
		// looking at it anymore.
		if (d.md5Entry)
			continue;



		//  ____            _     ____  
		// |  _ \ __ _ _ __| |_  |___ \ *
		// | |_) / _` | '__| __|   __) |
		// |  __/ (_| | |  | |_   / __/ 
		// |_|   \__,_|_|   \__| |_____|
		//
		// PART 2: Fuzzy matching for files with unknown MD5.
		//
		
		
		// We loop over the game variants matching the gameid associated to
		// the gfp record. We then try to decide for each whether it could be
		// appropriate or not.
		dr.md5 = d.md5;
		memcpy(dr.md5sum, d.md5sum, 16);
		for (g = gameVariantsTable; g->gameid; ++g) {
			// Skip over entries with a different gameid.
			if (g->gameid[0] == 0 || scumm_stricmp(gfp->gameid, g->gameid))
				continue;

			dr.game = *g;
			dr.extra = g->variant; // FIXME: We (ab)use 'variant' for the 'extra' description for now.
			if (gfp->platform != Common::kPlatformUnknown)
				dr.game.platform = gfp->platform;


			// If a variant has been specified, use that!
			if (gfp->variant) {
				if (!scumm_stricmp(gfp->variant, g->variant)) {
					// perfect match found
					results.push_back(dr);
					break;
				}
				continue;
			}
			
			// At this point, we know that the gameid matches, but no variant
			// was specified, yet there are multiple ones. So we try our best
			// to distinguish between the variants.
			// To do this, we take a close look at the detection file and
			// try to filter out some cases.

			Common::File tmp;
			if (!tmp.open(d.path.c_str())) {
				warning("SCUMM detectGames: failed to open '%s' for read access", d.path.c_str());
				continue;
			}
			
			if (file == "00.LFL") {
				// Used in V1, V2, V3 games.
				if (g->version > 3)
					continue;

				// Read a few bytes to narrow down the game.				
				byte buf[6];
				tmp.read(buf, 6);
				
				if (buf[0] == 0xbc && buf[1] == 0xb9) {
					// The NES version of MM
					if (g->id == GID_MANIAC && g->platform == Common::kPlatformNES) {
						// perfect match
						results.push_back(dr);
						break;
					}
				} else if (buf[0] == 0xCE && buf[1] == 0xF5) {
					// Looks like V1.
					// Candidates: maniac classic, zak classic
					
					if (g->version != 1)
						continue;

					// Zak has 58.LFL, Maniac doesn't have it.
					const bool has58LFL = fileMD5Map.contains("58.LFL");
					if (g->id == GID_MANIAC && !has58LFL) {
					} else if (g->id == GID_ZAK && has58LFL) {
					} else
						continue;
				} else if (buf[0] == 0xFF && buf[1] == 0xFE) {
					// GF_OLD_BUNDLE: could be V2 or old V3.
					// Candidates: maniac enhanced, zak enhanced, indy3ega, loom

					if (g->version != 2 && g->version != 3  || !(g->features & GF_OLD_BUNDLE))
						continue;

					/* We distinguish the games by the presence/absence of
					   certain files. In the following, '+' means the file
					   present, '-' means the file is absent.

					   maniac:    -58.LFL, -84.LFL,-86.LFL, -98.LFL

					   zak:       +58.LFL, -84.LFL,-86.LFL, -98.LFL
					   zakdemo:   +58.LFL, -84.LFL,-86.LFL, -98.LFL

					   loom:      +58.LFL, -84.LFL,+86.LFL, -98.LFL
					   loomdemo:  -58.LFL, +84.LFL,-86.LFL, -98.LFL

					   indy3:     +58.LFL, +84.LFL,+86.LFL, +98.LFL
					   indy3demo: -58.LFL, +84.LFL,-86.LFL, +98.LFL
					*/
					const bool has58LFL = fileMD5Map.contains("58.LFL");
					const bool has84LFL = fileMD5Map.contains("84.LFL");
					const bool has86LFL = fileMD5Map.contains("86.LFL");
					const bool has98LFL = fileMD5Map.contains("98.LFL");

					if (g->id == GID_INDY3         && has98LFL && has84LFL) {
					} else if (g->id == GID_ZAK    && !has98LFL && !has86LFL && !has84LFL && has58LFL) {
					} else if (g->id == GID_MANIAC && !has98LFL && !has86LFL && !has84LFL && !has58LFL) {
					} else if (g->id == GID_LOOM   && !has98LFL && (has86LFL != has84LFL)) {
					} else
						continue;
				} else if (buf[4] == '0' && buf[5] == 'R') {
					// newer V3 game
					// Candidates: indy3, indy3Towns, zakTowns, loomTowns

					if (g->version != 3 || (g->features & GF_OLD_BUNDLE))
						continue;

					/*
					Considering that we know about *all* TOWNS versions,
					and know their MD5s, we could simply rely on this and
					if we find something which has an unknown MD5, assume
					that it is an (so far unknown) version of Indy3.
		
					We can combine this with a look at the resource headers:
		
					Indy3:
					_numGlobalObjects 1000
					_numRooms 99
					_numCostumes 129
					_numScripts 139
					_numSounds 84
		
					Indy3Towns, ZakTowns, ZakLoom demo:
					_numGlobalObjects 1000
					_numRooms 99
					_numCostumes 199
					_numScripts 199
					_numSounds 199
		
					Assuming that all the town variants look like the latter, we can
					do the check like this:
					  if (numScripts == 139)
						assume Indy3
					  else if (numScripts == 199)
						assume towns game
					  else
						unknown, do not accept it
					*/
				} else {
					// TODO: Unknown file header, deal with it. Maybe an unencrypted
					// variant...
					// Anyway, we don't know to deal with the file, so we
					// just skip it.
				}
			} else if (file == "000.LFL") {
				// Used in V4
				// Candidates: monkeyEGA, pass, monkeyVGA, loomcd

				if (g->version != 4)
					continue;

				/*
				For all of them, we have:
				_numGlobalObjects 1000
				_numRooms 99
				_numCostumes 199
				_numScripts 199
				_numSounds 199
				
				Any good ideas to distinguish those? Maybe by the presence / absence
				of some files?
				At least PASS and the monkeyEGA demo differ by 903.LFL missing...
				And the count of DISK??.LEC files differs depending on what version
				you have (4 or 8 floppy versions). 
				loomcd of course shipped on only one "disc".
				
				pass: 000.LFL, 901.LFL, 902.LFL, 904.LFL, disk01.lec
				monkeyEGA:  000.LFL, 901-904.LFL, DISK01-09.LEC
				monkeyEGA DEMO: 000.LFL, 901.LFL, 902.LFL, 904.LFL, disk01.lec
				monkeyVGA: 000.LFL, 901-904.LFL, DISK01-04.LEC
				loomcd: 000.LFL, 901-904.LFL, DISK01.LEC
				*/

				const bool has903LFL = fileMD5Map.contains("903.LFL");
				const bool hasDisk02 = fileMD5Map.contains("DISK02.LEC");
				
				// There is not much we can do based on the presence / absence
				// of files. Only that if 903.LFL is present, it can't be PASS;
				// and if DISK02.LEC is present, it can't be LoomCD
				if (g->id == GID_PASS              && !has903LFL && !hasDisk02) {
				} else if (g->id == GID_LOOM       &&  has903LFL && !hasDisk02) {
				} else if (g->id == GID_MONKEY_VGA) {
				} else if (g->id == GID_MONKEY_EGA) {
				} else
					continue;
			} else {
				// Must be a V5+ game
				if (g->version < 5)
					continue;

				// So at this point the gameid is determined, but not necessarily
				// the variant!
				
				// TODO: Add code that handles this, at least for the non-HE games.
				// Note sure how realistic it is to correctly detect HE-game
				// variants, would require me to look at a sufficiently large
				// sample collection of HE games (assuming I had the time :).
				
				
				// TODO: For Mac versions in container file, we can sometimes
				// distinguish the demo from the regular version by looking
				// at the content of the container file and then looking for
				// the *.000 file in there.
			}
			
			// Add the file to the candidate list
			results.push_back(dr);
		}
	}
}

} // End of namespace Scumm

#pragma mark -
#pragma mark --- Plugin code ---
#pragma mark -


using namespace Scumm;


GameList Engine_SCUMM_gameIDList() {
	const PlainGameDescriptor *g = gameDescriptions;
	GameList games;
	while (g->gameid) {
		games.push_back(GameDescriptor(g->gameid, g->description));
		g++;
	}
	return games;
}

GameDescriptor Engine_SCUMM_findGameID(const char *gameid) {
	// First search the list of supported game IDs.
	const PlainGameDescriptor *g = gameDescriptions;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			return GameDescriptor(g->gameid, g->description);
		g++;
	}

	// If we didn't find the gameid in the main list, check if it
	// is an obsolete game id.
	GameDescriptor gs;
	const ObsoleteGameID *o = obsoleteGameIDsTable;
	while (o->from) {
		if (0 == scumm_stricmp(gameid, o->from)) {
			gs.gameid = gameid;
			gs.description = "Obsolete game ID";
			return gs;
		}
		o++;
	}
	return gs;
}


DetectedGameList Engine_SCUMM_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	Common::List<DetectorResult> results;

	detectGames(fslist, results, 0);
	
	
	for (Common::List<DetectorResult>::iterator x = results.begin(); x != results.end(); ++x) {
		DetectedGame dg(x->game.gameid, findDescriptionFromGameID(x->game.gameid),
				x->language, x->game.platform);
		dg.updateDesc(x->extra);	// Append additional information, if set, to the description.
		detectedGames.push_back(dg);
	}

	return detectedGames;
}

/**
 * Create a ScummEngine instance, based on the given detector data.
 *
 * This is heavily based on our MD5 detection scheme.
 */
PluginError Engine_SCUMM_create(OSystem *syst, Engine **engine) {
	assert(syst);
	assert(engine);
	const char *gameid = ConfMan.get("gameid").c_str();

	// We start by checking whether the specified game ID is obsolete.
	// If that is the case, we automatically upgrade the target to use
	// the correct new game ID (and platform, if specified).
	for (const ObsoleteGameID *o = obsoleteGameIDsTable; o->from; ++o) {
		if (!scumm_stricmp(gameid, o->from)) {
			// Match found, perform upgrade
			gameid = o->to;
			ConfMan.set("gameid", o->to);

			if (o->platform != Common::kPlatformUnknown)
				ConfMan.set("platform", Common::getPlatformCode(o->platform));

			warning("Target upgraded from game ID %s to %s", o->from, o->to);
			ConfMan.flushToDisk();
			break;
		}
	}


	FilesystemNode dir;
	if (ConfMan.hasKey("path") )
		dir = FilesystemNode(ConfMan.get("path"));
	FSList fslist = dir.listDir(FilesystemNode::kListFilesOnly);
	Common::List<DetectorResult> results;


	// Invoke the detector, but fixed to the specified gameid.
	detectGames(fslist, results, gameid);

	// Unable to locate game data
	if (results.empty()) {
		warning("ScummEngine: unable to locate game data at path '%s'", dir.path().c_str());
		return kNoGameDataFoundError;
	}
	
	// No unique match found. If a platform override is present, try to
	// narrow down the list a bit more.
	if (results.size() > 1 && ConfMan.hasKey("platform")) {
		Common::Platform platform = Common::parsePlatform(ConfMan.get("platform"));
		for (Common::List<DetectorResult>::iterator x = results.begin(); x != results.end(); ) {
			if (x->game.platform != platform) {
				x = results.erase(x); 
			} else {
				++x;
			}
		}
	}

	// Still no unique match found -> we just use the first one
	if (results.size() > 1) {
		warning("Engine_SCUMM_create: No unique game candidate found, using first one");
	}
	

	DetectorResult res(*(results.begin()));
	debug(1, "Using gameid %s, variant %s, extra %s", res.game.gameid, res.game.variant, res.extra);

	// Print the MD5 of the game; either verbose using printf, in case of an
	// unknown MD5, or with a medium debug level in case of a known MD5 (for
	// debugging purposes).
	if (!findInMD5Table(res.md5.c_str())) {
		printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", res.md5.c_str());
	} else {
		debug(1, "Using MD5 '%s'", res.md5.c_str());
	}

	// TODO: Do we really still need / want the platform override ?


	// Check for a user override of the platform. We allow the user to override
	// the platform, to make it possible to add games which are not yet in 
	// our MD5 database but require a specific platform setting.
	if (ConfMan.hasKey("platform"))
		res.game.platform = Common::parsePlatform(ConfMan.get("platform"));


	// Language override
	if (ConfMan.hasKey("language"))
		res.language = Common::parseLanguage(ConfMan.get("language"));


	// V3 FM-TOWNS games *always* should use the corresponding music driver,
	// anything else makes no sense for them.
	// TODO: Maybe allow the null driver, too?
	if (res.game.platform == Common::kPlatformFMTowns && res.game.version == 3) {
		res.game.midi = MDT_TOWNS;
	}

	// Finally, we have massaged the GameDescriptor to our satisfaction, and can
	// instantiate the appropriate game engine. Hooray!
	switch (res.game.version) {
	case 0:
		*engine = new ScummEngine_c64(syst, res);
		break;
	case 1:
	case 2:
		*engine = new ScummEngine_v2(syst, res);
		break;
	case 3:
		if (res.game.features & GF_OLD_BUNDLE)
			*engine = new ScummEngine_v3old(syst, res);
		else
			*engine = new ScummEngine_v3(syst, res);
		break;
	case 4:
		*engine = new ScummEngine_v4(syst, res);
		break;
	case 5:
		*engine = new ScummEngine_v5(syst, res);
		break;
	case 6:
		switch (res.game.heversion) {
#ifndef DISABLE_HE
		case 100:
			*engine = new ScummEngine_v100he(syst, res);
			break;
		case 99:
			*engine = new ScummEngine_v99he(syst, res);
			break;
		case 98:
		case 95:
		case 90:
			*engine = new ScummEngine_v90he(syst, res);
			break;
		case 80:
			*engine = new ScummEngine_v80he(syst, res);
			break;
		case 73:
		case 72:
			*engine = new ScummEngine_v72he(syst, res);
			break;
		case 71:
			*engine = new ScummEngine_v71he(syst, res);
			break;
		case 70:
			*engine = new ScummEngine_v70he(syst, res);
			break;
#endif
#ifndef PALMOS_68K
		case 61:
			*engine = new ScummEngine_v60he(syst, res);
			break;
#endif
		default:
			*engine = new ScummEngine_v6(syst, res);
		}
		break;
#ifndef DISABLE_SCUMM_7_8
	case 7:
		*engine = new ScummEngine_v7(syst, res);
		break;
	case 8:
		*engine = new ScummEngine_v8(syst, res);
		break;
#endif
	default:
		error("Engine_SCUMM_create(): Unknown version of game engine");
	}

	return kNoError;
}

REGISTER_PLUGIN(SCUMM, "Scumm Engine");

#ifdef PALMOS_68K
#include "scumm_globals.h"

_GINIT(Scumm_md5table)
_GSETPTR(md5table, GBVARS_MD5TABLE_INDEX, MD5Table, GBVARS_SCUMM)
_GEND

_GRELEASE(Scumm_md5table)
_GRELEASEPTR(GBVARS_MD5TABLE_INDEX, GBVARS_SCUMM)
_GEND

#endif
