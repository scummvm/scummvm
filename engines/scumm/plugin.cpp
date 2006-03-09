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
 * $Id: scumm.cpp 20883 2006-02-25 18:24:55Z fingolfin $
 *
 */

#include "common/stdafx.h"

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
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


using namespace Scumm;

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

static const Common::Platform UNK = Common::kPlatformUnknown;



#pragma mark -
#pragma mark --- Tables ---
#pragma mark -


/**
 * This table contains all game IDs supported by the SCUMM engine, and maps
 * them to the full humand readable game name.
 */
static const GameSettings gameDescriptions[] = {
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
	{ "BluesABCTimeDemo", "Blue's ABC Time (Demo)" },
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
 *
 * We use an ugly macro 'UNK' here to make the following table more readable.
 */
static const ObsoleteGameID obsoleteGameIDsTable[] = {
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

static const ScummGameSettings scumm_settings[] = {
	/* Scumm Version 1 */
	/* Scumm Version 2 */

	{"maniac", 0, GID_MANIAC, 2, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformPC},
	{"zak", 0, GID_ZAK, 2, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformPC},

	/* Scumm Version 3 */
	{"indy3", 0, GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformPC},
	{"loom", 0, GID_LOOM, 3, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformPC},

	/* Scumm Version 4 */
	{"pass", 0, GID_PASS, 4, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},

	/* Scumm version 5, small header -- we treat these as V4 games, since internally
	   they really are much closer to the V4 games than to all other V5 games. */
	{"monkey", 0, GID_MONKEY_VGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI,
	 GF_SMALL_HEADER | GF_USE_KEY, Common::kPlatformPC},

	/* Scumm version 5 */
	{"monkey2", 0, GID_MONKEY2, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_MIDI,
	 GF_USE_KEY, Common::kPlatformPC},

	{"atlantis", 0, GID_INDY4, 5, 0, MDT_ADLIB | MDT_MIDI,
	 GF_USE_KEY, Common::kPlatformPC},

	/* Scumm Version 6 */
	{"tentacle", 0, GID_TENTACLE, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_MIDI,
	 GF_USE_KEY, Common::kPlatformPC},

	{"samnmax", 0, GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_MIDI,
	 GF_USE_KEY, Common::kPlatformPC},

//	{"test", "Test demo game", GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_MIDI, GF_NEW_OPCODES, Common::kPlatformUnknown},

#ifndef DISABLE_SCUMM_7_8
	/* Scumm Version 7 */
	{"ft", 0, GID_FT, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, Common::kPlatformPC},

	{"dig", 0, GID_DIG, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, Common::kPlatformPC},

	/* Scumm Version 8 */
	{"comi", 0, GID_CMI, 8, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, Common::kPlatformWindows},

#endif

	// Humongous Entertainment Scumm Version 6
	{"puttputt", 0, GID_HEGAME, 6, 61, MDT_ADLIB | MDT_MIDI,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"puttmoon", 0, GID_HEGAME, 6, 61, MDT_ADLIB | MDT_MIDI,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"funpack", 0, GID_FUNPACK, 6, 61, MDT_ADLIB | MDT_MIDI,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"fbpack", 0, GID_HEGAME, 6, 61, MDT_ADLIB | MDT_MIDI,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"fbear", 0, GID_FBEAR, 6, 61, MDT_ADLIB | MDT_MIDI,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},

#ifndef DISABLE_HE
	{"activity", 0, GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 7.1
	// The first version to use 640x480 resolution
	// There are also 7.1 versions of freddemo, airdemo and farmdemo
	{"catalog", 0, GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"freddi", 0, GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 7.2
	{"airport", 0, GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"puttzoo", 0, GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Changed o_getResourceSize to cover all resource types
	{"farm", 0, GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"jungle", 0, GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 8.0 ?  Scummsrc.80
	{"freddi2", 0, GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"pajama", 0, GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"putttime", 0, GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"balloon", 0, GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"dog", 0, GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"maze", 0, GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"socks", 0, GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"water", 0, GID_WATER, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 9.0 ?  Scummsys.90
	{"baseball", 0, GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"thinkerk", 0, GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"thinker1", 0, GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"freddi3", 0, GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"spyfox", 0, GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 9.5 ?  Scummsys.95
	{"pajama2", 0, GID_HEGAME, 6, 95, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"chase", 0, GID_HEGAME, 6, 95, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 9.8 ?  Scummsys.98
	// these and later games can easily be identified by the .(a) file instead of a .he1
	// and INIB chunk in the .he0
	{"lost", 0, GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_CURSORLESS, Common::kPlatformWindows},
	{"puttrace", 0, GID_PUTTRACE, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"BluesABCTimeDemo", 0, GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"soccer", 0, GID_SOCCER, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Global scripts increased to 2048
	{"freddi4", 0, GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 9.9 ?  Scummsys.99
	{"football", 0, GID_FOOTBALL, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"pajama3", 0, GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"puttcircus", 0, GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"spyfox2", 0, GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"mustard", 0, GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},

	// Added the use of fonts
	{"FreddisFunShop", 0, GID_FUNSHOP, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"SamsFunShop", 0, GID_FUNSHOP, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"PuttsFunShop", 0, GID_FUNSHOP, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},

	// Added 16bit color
	{"baseball2001", 0, GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"SoccerMLS", 0, GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"freddicove", 0, GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"spyozon", 0, GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},

	// Restructured the Scumm engine
	{"pjgames", 0, GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},

	// Uses bink in external files for logos
	{"Baseball2003", 0, GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"basketball", 0, GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"Soccer2004", 0, GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},

	// Uses smacker in external files, for testing only
	{"BluesBirthdayDemo", 0, GID_HEGAME, 6, 98, MDT_NONE,
	GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"artdemo", 0, GID_HEGAME, 6, 99, MDT_NONE,
	GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"readdemo", 0, GID_HEGAME, 6, 99, MDT_NONE,
	GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},


#endif
	{NULL, NULL, 0, 0, 0, MDT_NONE, 0, Common::kPlatformUnknown}
};


#if 0
// The following table is currently *unused*. It's work in progress for a refined detector;
// the purpose of this particular table is to replace multiple_versions_md5_settings, 
// and maybe also scumm_settings. 
static const ScummGameSettings extra_versions[] = {
	// The C64 version of MM is detected via the platform field and hence has no seperate entry in this list
	{"maniac", "",    GID_MANIAC, 2, 0, MDT_PCSPK, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, UNK},
	{"maniac", "NES", GID_MANIAC, 1, 0, MDT_NONE,  GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES},
	{"maniac", "V1",  GID_MANIAC, 1, 0, MDT_PCSPK, GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformPC},

	// The C64 version of Zak is detected via the platform field and hence has no seperate entry in this list
	{"zak", "",         GID_ZAK, 2, 0, MDT_PCSPK, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, UNK},
	{"zak", "V1",       GID_ZAK, 1, 0, MDT_PCSPK, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformPC},
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
	{"monkey", "CD",       GID_MONKEY,     5, 0, MDT_ADLIB, GF_USE_KEY | GF_AUDIOTRACKS, UNK},
	{"monkey", "FM-TOWNS", GID_MONKEY,     5, 0, MDT_ADLIB, GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"monkey", "SEGA",     GID_MONKEY,     5, 0, MDT_NONE,  GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformSegaCD},


	/* Scumm Version 5 */
	{"monkey2", 0, GID_MONKEY2, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_MIDI, GF_USE_KEY, Common::kPlatformPC},

	{"atlantis", 0, GID_INDY4, 5, 0, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, Common::kPlatformPC},


	/* Scumm Version 6 */
	{"tentacle", 0, GID_TENTACLE, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_MIDI, GF_USE_KEY, Common::kPlatformPC},

	{"samnmax", 0, GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_MIDI, GF_USE_KEY, Common::kPlatformPC},


#ifndef DISABLE_SCUMM_7_8
	/* Scumm Version 7 */
	{"ft", "",     GID_FT, 7, 0, MDT_NONE, GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, UNK},
	{"ft", "Demo", GID_FT, 7, 0, MDT_NONE, GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, UNK},

	{"dig", "",     GID_DIG, 7, 0, MDT_NONE, GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, UNK},
	{"dig", "Demo", GID_DIG, 7, 0, MDT_NONE, GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, UNK},

	/* Scumm Version 8 */
	{"comi", "",     GID_CMI, 8, 0, MDT_NONE, GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, Common::kPlatformWindows},
	{"comi", "Demo", GID_CMI, 8, 0, MDT_NONE, GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformWindows},
#endif


	// TODO: Maybe the HE games (at least those which are using GID_HEGAME) should
	// be using a seperate table: Most of them in the existing scumm_settings table
	// could be realized by mapping to one of "HE NNN" entries found at the end
	// of this table. This would both slightly reduce the executable's size as
	// well as simplify maintanance.
	// To do this, we could add an "alias" table that maps a game ID to another
	// game/variant ID, but *only* internally (for purposes of setting up the
	// version/heversion/features etc). In particular, no auto-upgrading of
	// targets and no user visible differences to the current behavior.


	{"fbear", "HE 61", GID_FBEAR, 6, 61, MDT_ADLIB | MDT_MIDI, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"fbear", "HE 70", GID_FBEAR, 6, 70, MDT_NONE,             GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

#ifndef DISABLE_HE

	{"water", "HE 80", GID_WATER, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"water", "HE 99", GID_WATER, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},


	{"puttrace", "HE 98",   GID_PUTTRACE, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"puttrace", "HE 98.5", GID_PUTTRACE, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},
	{"puttrace", "HE 99",   GID_PUTTRACE, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},


	{"puttputt", "HE 61", GID_HEGAME,   6, 61, MDT_ADLIB | MDT_MIDI, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"puttputt", "HE 60", GID_HEGAME,   6, 60, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, Common::kPlatformWindows},
	{"puttputt", "Demo",  GID_PUTTDEMO, 6, 60, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK},


	{"freddi4", "Unencrypted Russian", GID_HEGAME, 6, 98, MDT_NONE, GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},


	{"freddicove", "Updated Dutch",       GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows}, // FreddiDZZ
	{"freddicove", "Updated Russian",     GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"freddicove", "Unencrypted Russian", GID_HEGAME, 6,  99, MDT_NONE,              GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"freddicove", "Updated Demo",        GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"freddicove", "Updated Dutch Demo",  GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows}, // FF5Demo


	{"lost", "Test",    GID_HEGAME, 6,  99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_CURSORLESS, Common::kPlatformWindows},
	{"lost", "Updated", GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_CURSORLESS, Common::kPlatformWindows},


	// The following are meant to be generic HE game variants and as such do not specify a game ID
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
#endif


// This additional table is used for MD5-based search
//
// Use main table to specify default flags and this table to override defaults.
//
// Please, add new entries sorted alpabetically by string name
static const ScummGameSettings multiple_versions_md5_settings[] = {
#ifndef PALMOS_68K
	{"2e85f7aa054930c692a5b1bed1dfc295", "Demo Updated", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown}, // Football2002

	{"037385a953789190298494d92b89b3d0", "Updated", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"8fec68383202d38c0d25e9e3b757c5df", "Demo", GID_CMI, 8, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformWindows},

	{"362c1d281fb9899254cda66ad246c66a", "Demo", GID_DIG, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformPC},
	{"cd9c05e755d7bf8e9b9590ad1ebe273e", "Demo Mac", GID_DIG, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformMacintosh},

	{"179879b6e35c1ead0d93aab26db0951b", "Windows", GID_FBEAR, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"22c9eb04455440131ffc157aeb8d40a8", "Windows Demo", GID_FBEAR, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"cf8ef3a1fb483c5c4b1c584d1167b2c4", "Updated German", GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"df047cc4792150f601290357566d36a6", "Updated", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"746e88c172a5b7a1ae89ac0ee3ee681a", "Updated Russian", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"507bb360688dc4180fdf0d7597352a69", "Updated Swedish", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"0855496dde35356b1a9691e22ba84cdc", "Demo", GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"566165a7338fa11029e7c14d94fa70d0", "Demo", GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"cf4ef315214c7d8cdab6302cdb7e50db", "German Demo", GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"c8aac5e3e701874e2fa4117896f9e1b1", "Macintosh Demo", GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformMacintosh},

	{"8ee63cafb1fe9d62aa0d5a23117e70e7", "Updated", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // FreddiCHSH
	{"51305e929e330e24a75a0351c8f9975e", "Updated", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"e41de1c2a15abbcdbf9977e2d7e8a340", "Updated Russian", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // FreddiCHSH
	{"d37c55388294b66e53e7ced3af88fa68", "Demo Updated", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // FFHSDemo

	{"83cedbe26aa8b58988e984e3d34cac8e", "Updated German", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"75bff95816b84672b877d22a911ab811", "Updated Russian", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"ed2b074bc3166087a747acb2a3c6abb0", "Updated German Demo", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows}, // Fritzi3demo

	{"07b810e37be7489263f7bc7627d4765d", "Unencrypted Russian", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},
	{"b5298a5c15ffbe8b381d51ea4e26d35c", "Updated German", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"78bd5f036ea35a878b74e4f47941f784", "Updated Russian", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"16effd200aa6b8abe9c569c3e578814d", "Updated Dutch Demo", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // ff4demo
	{"499c958affc394f2a3868f1eb568c3ee", "Updated Dutch Demo", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // ff4demo
	{"ebd324dcf06a4c49e1ba5c231eee1060", "Updated Demo", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"e03ed1474ec14de78359970e0457a820", "Updated Demo", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"688328c5bdc4c8ec4145688dfa077bf2", "German Demo", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // Ff4demo

	{"b8955d7d23b4972229060d1592489fef", "Updated Dutch", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows}, // FreddiDZZ
	{"4ce2d5b355964bbcb5e5ce73236ef868", "Updated Russian", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"21abe302e1b1e2b66d6f5c12e241ebfd", "Unencrypted Russian", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"45082a5c9f42ba14dacfe1fdeeba819d", "Updated Demo", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"6b257bb2827dd894b8109a50a1a18b5a", "Updated Dutch Demo", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows}, // FF5Demo

	{"4dbff3787aedcd96b0b325f2d92d7ad9", "Updated", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"f1b0e0d587b85052de5534a3847e68fe", "Updated", GID_WATER, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"9d7b67be003fea60be4dcbd193611936", "Mac Demo", GID_FT, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformMacintosh},
	{"32a433dea56b86a55b59e4ff7d755711", "PC Demo", GID_FT, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformPC},

	{"157367c3c21e0d03a0cba44361b4cf65", "AtariST", GID_INDY3, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformAtariST},
	{"0f9c7a76657f0840b8f7ccb5bffeb9f4", "AtariST Fr", GID_INDY3, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformAtariST},
	{"1dd7aa088e09f96d06818aa9a9deabe0", "Macintosh", GID_INDY3, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformMacintosh},

	{"1875b90fade138c9253a8e967007031a", "VGA", GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS, Common::kPlatformPC},
	{"399b217b0c8d65d0398076da486363a9", "VGA De", GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS, Common::kPlatformPC},
	{"17b5d5e6af4ae89d62631641d66d5a05", "VGA It", GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS, Common::kPlatformPC},
	{"3cce1913a3bc586b51a75c3892ff18dd", "VGA Ru", GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS, Common::kPlatformPC},

	{"04687cdf7f975a89d2474929f7b80946", "FM-TOWNS", GID_INDY3, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"3a0c35f3c147b98a2bdf8d400cfc4ab5", "FM-TOWNS Jp", GID_INDY3, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS | GF_AUDIOTRACKS, Common::kPlatformFMTowns},

	{"86c9902b7bec1a17926d4dae85beaa45", "Demo", GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"bf8b52fdd9a69c67f34e8e9fec72661c", "Demo", GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"8d479e36f35e80257dfc102cf4b8a912", "Updated Demo", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"0557df19f046a84c2fdc63507c6616cb", "Updated Dutch Demo", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"a0a7dea72003933b8b3f8b99b9f7ddeb", "AtariST", GID_LOOM, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformAtariST},
	{"c24c490373aeb48fbd54caa8e7ae376d", "AtariST De", GID_LOOM, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformAtariST},
	{"0e9b01430e31d9fcd94071d433bbc6bf", "AtariST Fr", GID_LOOM, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformAtariST},
	{"6f0be328c64d689bb606d22a389e1b0f", "Macintosh", GID_LOOM, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformMacintosh},

	{"5d88b9d6a88e6f8e90cded9d01b7f082", "256 color CD version", GID_LOOM, 4, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},
	{"c5d10e190d4b4d59114b824f2fdbd00e", "FM-TOWNS", GID_LOOM, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"31b8fda4c8c7413fa6b39997e776eba4", "FM-TOWNS Jp", GID_LOOM, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},

	{"3905799e081b80a61d4460b7b733c206", "NES E", GID_MANIAC, 1, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES},
	{"81bbfa181184cb494e7a81dcfa94fbd9", "NES F", GID_MANIAC, 1, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES},
	{"257f8c14d8c584f7ddd601bcb00920c7", "NES G", GID_MANIAC, 1, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES},
	{"22d07d6c386c9c25aca5dac2a0c0d94b", "NES SW", GID_MANIAC, 1, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES},
	{"d8d07efcb88f396bee0b402b10c3b1c9", "NES U", GID_MANIAC, 1, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES},
	{"7f45ddd6dbfbf8f80c0c0efea4c295bc", "v1", GID_MANIAC, 1, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformPC},

	{"898eaa21f79cf8d4f08db856244689ff", "Updated", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"a095e33061606d231ff37dca4c64c8ac", "Updated German", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // PYJAMA
	{"37aed3f91c1ef959e0bd265f9b13781f", "Updated", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // PajamaNHD
	{"d7ab7cd6105546016e6a0d46fb36b964", "Updated Demo", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // PJSamDemo

	{"30ba1e825d4ad2b448143ae8df18482a", "Updated Dutch Demo", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows}, // Pjs2demo
	{"32709cbeeb3044b34129950860a83f14", "Updated Russian", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // PajamaTAL
	{"c6907d44f1166941d982864cd42cdc89", "Updated German", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // PyjamaDBMN
	{"4521138d15d1fd7649c31fb981746231", "Updated German Demo", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows}, // PJP2DEMO

	{"a2bb6aa0537402c1b3c2ea899ccef64b", "Test", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_CURSORLESS, Common::kPlatformWindows},
	{"a561d2e2413cc1c71d5a1bf87bf493ea", "Updated", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_CURSORLESS, Common::kPlatformWindows},

	{"055ffe4f47753e47594ac67823220c54", "German", GID_PUTTRACE, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // ToffRennen
	{"6af2419fe3db5c2fdb091ae4e5833770", "Dutch Demo", GID_PUTTRACE, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows}, // 500demo
	{"aaa587701cde7e74692c68c1024b85eb", "Updated Dutch Demo", GID_PUTTRACE, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"663743c03ae0c007f3d665cf631c0e6b", "German Demo", GID_PUTTRACE, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // Rennen
	{"7c8100e360e8ef05f88069d4cfa0afd1", "UK Demo", GID_PUTTRACE, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"9c92eeaf517a31b7221ec2546ab669fd", "Windows", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"9c143c5905055d5df7a0f014ab379aee", "Windows Demo", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"0b3222aaa7efcf283eb621e0cefd26cc", "Russian", GID_HEGAME, 6, 60, MDT_ADLIB | MDT_MIDI,
	 GF_USE_KEY, Common::kPlatformWindows},
	{"31aa57f460a3d12429f0552a46a90b39", "Demo", GID_PUTTDEMO, 6, 60, MDT_ADLIB | MDT_MIDI,
	  GF_USE_KEY, Common::kPlatformPC},
	{"f40a7f495f59188ca57a9d1d50301bb6", "Macintosh Demo", GID_PUTTDEMO, 6, 60, MDT_ADLIB | MDT_MIDI,
	  GF_USE_KEY, Common::kPlatformMacintosh},
	{"6a30a07f353a75cdc602db27d73e1b42", "Windows", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"37ff1b308999c4cca7319edfcc1280a0", "Windows Demo", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"92e7727e67f5cd979d8a1070e4eb8cb3", "Updated", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},

	{"2108d83dcf09f8adb4bc524669c8cf51", "Updated", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"d4aac997e2f4e15341f0bfbf905419bd", "Updated", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"7c980a1b1596a93f26917318884f48f7", "Updated German", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"defb8cb9ec4b0f91acfb6b61c6129ad9", "Updated Russian", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"a525c1753c1db5011c00417da37887ef", "Updated", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"fcb78ebecab2757264c590890c319cc5", "Updated Dutch", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"4e5867848ee61bc30d157e2c94eee9b4", "Demo", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"6b19d0e25cbf720d05822379b8b90ed9", "Dutch Demo", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"59d5cfcc5e672a6e07baae01328b918b", "French Demo", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // TEMPDEMO
	{"fbb697d89d2beca87360a145f467bdae", "Updated German Demo", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // ZEITDEMO
	{"0ab19be9e2a3f6938226638b2a3744fe", "Updated Demo", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"d7b247c26bf1f01f8f7daf142be84de3", "Updated", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"8e3241ddd6c8dadf64305e8740d45e13", "Updated", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"d4b8ee426b1afd3e53bc0cf020418cf6", "Updated", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"1d05cd189e4908f79b57e78a4402f292", "EGA", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"49210e124e4c2b30f1290a9ef6306301", "EGA", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"e98b982ceaf9d253d730bde8903233d6", "EGA De", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"fc6b6148e80d67939d9a18697c0f626a", "EGA De", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"ce6a4cef315b20fef58a95bc40a2d8d3", "EGA Fr", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"aa7a07d94ae853f6460be4ce0a1bf530", "EGA Fr", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"1dd3c11ea4439adfe681e4e405b624e1", "EGA Fr", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"477dbafbd66a53c98416dc01aef019ad", "EGA It", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"910e31cffb28226bd68c569668a0d6b4", "EGA Sp", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"c666a998af90d81db447eccba9f72c8d", "Atari", GID_MONKEY_EGA, 4, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformAtariST},
	{"927a764615c7fcdd72f591355e089d8c", "Atari De", GID_MONKEY_EGA, 4, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformAtariST},
	{"9e5e0fb43bd22f4628719b7501adb717", "Atari Fr", GID_MONKEY_EGA, 4, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformAtariST},
	{"0a41311d462b6639fc45297b9044bf16", "Atari Sp", GID_MONKEY_EGA, 4, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformAtariST},
	{"71523b539491527d9860f4407faf0411", "Demo", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"771bc18ec6f93837b839c992b211904b", "Demo De", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},

	{"2d1e891fe52df707c30185e52c50cd92", "CD", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},
	{"305d3dd57c96c65b017bc70c8c7cfb5e", "CD De", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},
	{"f049e38c1f8302b5db6170f1872af89a", "CD Sp", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},
	{"da6269b18fcb08189c0aa9c95533cce2", "CD It", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},
	{"aa8a0cb65f3afbbe2c14c3f9f92775a3", "CD Fr", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},
	{"2ccd8891ce4d3f1a334d21bff6a88ca2", "Mac CD", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformMacintosh},

	{"8eb84cee9b429314c7f0bdcf560723eb", "FM-TOWNS", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformFMTowns},

	{"e17db1ddf91b39ca6bbc8ad3ed19e883", "FM-TOWNS Jp", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformFMTowns},

	{"c13225cb1bbd3bc9fe578301696d8021", "SegaCD", GID_MONKEY, 5, 0, MDT_NONE,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformSegaCD},

	{"3de99ef0523f8ca7958faa3afccd035a", "Updated", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"58436e634f4fae1d9973591c2ffa1fcb", "Updated", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"9bda5fee51d2fda5253d02c642016bf4", "Updated Dutch", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},
	{"a28135a7ade38cc0208b04507c46efd1", "Updated German", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"72ac6bc980d5101c2142189d746bd62f", "Updated Russian", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // SPYFoxDC
	{"9d4ab3e0e1d1ebc6ba8a6a4c470ed184", "Updated Demo", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"4edbf9d03550f7ba01e7f34d69b678dd", "Updated Dutch Demo", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},

	{"90c755e1c9b9b8a4129d37b2259d0655", "Updated", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"b23f7cd7c304d7dff08e92a96120d5b4", "v1", GID_ZAK, 1, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformPC},
	{"7020931d5a2be0a49d68e7a1882363e4", "v1", GID_ZAK, 1, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformPC},

	{"2d4536a56e01da4b02eb021e7770afa2", "FM-TOWNS", GID_ZAK, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"1ca86e2cf9aaa2068738a1e5ba477e60", "FM-TOWNS Jp", GID_ZAK, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},

	{"2d388339d6050d8ccaa757b64633954e", "FM-TOWNS", GID_ZAK, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"77f5c9cc0986eb729c1a6b4c8823bbae", "FM-TOWNS", GID_ZAK, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"3938ee1aa4433fca9d9308c9891172b1", "FM-TOWNS", GID_ZAK, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
#endif
	{NULL, NULL, 0, 0, MDT_NONE, 0, 0, Common::kPlatformUnknown}
};

#pragma mark -
#pragma mark --- Miscellaneous ---
#pragma mark -


static const char *findDescriptionFromGameID(const char *gameid) {
	const GameSettings *g = gameDescriptions;
	while (g->gameid) {
		if (!scumm_stricmp(g->gameid, gameid)) {
			return g->description;
		}
		g++;
	}
	error("Unknown gameid encountered in findDescriptionFromGameID");
}

static int compareMD5Table(const void *a, const void *b) {
	const char *key = (const char *)a;
	const MD5Table *elem = (const MD5Table *)b;
	return strcmp(key, elem->md5);
}


#pragma mark -
#pragma mark --- Plugin code ---
#pragma mark -


GameList Engine_SCUMM_gameIDList() {
	const GameSettings *g = gameDescriptions;
	GameList games;
	while (g->gameid) {
		games.push_back(*g);
		g++;
	}
	return games;
}

GameSettings Engine_SCUMM_findGameID(const char *gameid) {
	// First search the list of supported game IDs.
	const GameSettings *g = gameDescriptions;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			return *g;
		g++;
	}

	// If we didn't find the gameid in the main list, check if it
	// is an obsolete game id.
	GameSettings gs = { 0, 0 };
	const ObsoleteGameID *o = obsoleteGameIDsTable;
	while (o->from) {
		if (0 == scumm_stricmp(gameid, o->from)) {
			gs.gameid = gameid;
			gs.gameid = "Obsolete game ID";
			return gs;
		}
		o++;
	}
	return gs;
}


enum {
	kDetectNameMethodsCount = 8
};

static bool generateDetectName(const ScummGameSettings *g, int method, char *detectName) {
	detectName[0] = '\0';

	switch (method) {
	case 0:
		if (g->version > 3)
			return false;
		strcpy(detectName, "00.LFL");
		break;
	case 1:
		if (g->version < 3 || g->version > 5)
			return false;
		strcpy(detectName, "000.LFL");
		break;
	case 2:
		if (g->version < 4 || g->version > 7)
			return false;
		strcpy(detectName, g->gameid);
		strcat(detectName, ".000");
		break;
	case 3:
		if (g->version < 7)
			return false;
		strcpy(detectName, g->gameid);
		strcat(detectName, ".la0");
		break;
	case 4:
		if (g->heversion == 0)
			return false;
		strcpy(detectName, g->gameid);
		strcat(detectName, ".he0");
		break;
	case 5:
		// FIXME: Fingolfin asks: For which games is this case used? 
		// Please document this. Also: Why was this case missing in
		// Engine_SCUMM_create ? 
		strcpy(detectName, g->gameid);
		break;
	case 6:
		if (g->id != GID_SAMNMAX)
			return false;
		strcpy(detectName, g->gameid);
		strcat(detectName, ".sm0");
		break;
	case 7:
		if (g->id != GID_MANIAC)
			return false;
		strcpy(detectName, "00.MAN");
		break;
	}

	return true;
}


DetectedGameList Engine_SCUMM_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const ScummGameSettings *g;
	char detectName[128];
	char tempName[128];
	SubstResFileNames subst = { 0, 0, kGenAsIs };

	typedef Common::Map<Common::String, bool> StringSet;
	StringSet fileSet;

	for (g = scumm_settings; g->gameid; ++g) {
		// Determine the 'detectname' for this game, that is, the name of a
		// file that *must* be presented if the directory contains the data
		// for this game. For example, FOA requires atlantis.000

		// TODO: we need to add cache here
		for (int method = 0; method < kDetectNameMethodsCount; method++) {
			if (!generateDetectName(g, method, detectName))
				continue;

			strcpy(tempName, detectName);

			int substLastIndex = 0;

			do {
				// Iterate over all files in the given directory
				for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
					if (!file->isDirectory()) {
						const char *name = file->displayName().c_str();

						if (0 == scumm_stricmp(detectName, name)) {
							byte buf[6];

							if (g->version < 4) {
								// We take a look at the file now, to narrow
								// down the list of possible candidates a bit further.
								// E.g. it's trivial to distinguish V1 from V3 games.
								Common::File tmp;
								if (!tmp.open(file->path().c_str()))
									break;
								tmp.read(buf, 6);

								if (buf[0] == 0xCE && buf[1] == 0xF5) {
									// Looks like V1. However, we currently do not distinguish between V1 and V2
									// in the scumm_settings list.
									if (g->version != 1 && g->version != 2)
										break;

									// Candidates: maniac clasic, zak classic

									// TODO: Maybe we can use the filesize to distinguish these two?
									// English V1 Zak: 1896 bytes
									// English V1 MM:  1972 bytes
									// It would be interesting if those sizes are the same for other language
									// variants of these games, or for demos?
								} else if (buf[0] == 0xFF && buf[1] == 0xFE) {
									// GF_OLD_BUNDLE: could be V2 or old V3.
									if (!(g->features & GF_OLD_BUNDLE) || (g->version != 2 && g->version != 3))
										break;
									// Candidates: maniac enhanced, zak enhanced, indy3ega, loom
								/*
								TODO: Might be possible to distinguish those by the script count.
								Specifically, my versions of these games have this in their headers:

								Loom (en; de; en demo; en MAC):
								_numGlobalObjects 1000
								_numRooms 100
								_numCostumes 200
								_numScripts 200
								_numSounds 80

								Indy3EGA (en PC; en Mac; en demo):
								_numGlobalObjects 1000
								_numRooms 99
								_numCostumes 129
								_numScripts 139
								_numSounds 84

								MM (en; de):
								_numGlobalObjects 780
								_numRooms 61
								_numCostumes 40
								_numScripts 179
								_numSounds 120

								Zak (de; en demo):
								_numGlobalObjects 780
								_numRooms 61
								_numCostumes 40
								_numScripts 155
								_numSounds 120

								So, they all have a different number of scripts.
								*/
								} else if (buf[4] == '0' && buf[5] == 'R') {
									// newer V3 game
									if (g->version != 3)
										break;
									// Candidates: indy3, indy3Towns, zakTowns, loomTowns
								/*
								Considering that we know about *all* TOWNS versions,
								and know their MD5s, we could simply rely on this and
								if we find something which has an unknown MD5, assume
								that it is an (so far unknown) version of Indy3.

								We can combin this with a look at the resource headers:

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
								do the chceck like this:
								  if (numScripts == 139)
									assume Indy3
								  else if (numScripts == 199)
									assume towns game
								  else
									unknown, do not accept it
								*/
								} else if (buf[4] == 'R' && buf[5] == 'N') {
									// V4 game
									if (g->version != 4)
										break;
									// Candidates: monkeyEGA, pass, monkeyVGA, loomcd
								/*
								For all of them, we have:
								_numGlobalObjects 1000
								_numRooms 99
								_numCostumes 199
								_numScripts 199
								_numSounds 199
								*/
								} else if (buf[0] == 0xa0 && buf[1] == 0x07 && buf[2] == 0xa5 &&
										   buf[3] == 0xbc) {
									// MM NES .prg
									if (g->id != GID_MANIAC)
										break;
								} else if (buf[0] == 0xbc && buf[1] == 0xb9) {
									// MM NES 00.LFL
									if (g->id != GID_MANIAC)
										break;
								} else if (buf[0] == 0x31 && buf[1] == 0x0a) {
									// C64 MM & Zak disk1
									if (g->version != 2)
										break;
								} else if (buf[0] == 0xcd && buf[1] == 0xfe) {
									// C64 MM & Zak 00.LFL
									if (g->version != 2)
										break;
								} else {
									// This is not a V1-V4 game
									break;
								}
							}

							// Match found, add to list of candidates, then abort inner loop.
							const char *desc = findDescriptionFromGameID(g->gameid);
							if (substLastIndex > 0 && // HE Mac versions.
								(subst.genMethod == kGenMac ||
								 subst.genMethod == kGenMacNoParens)) {
								detectedGames.push_back(DetectedGame(g->gameid, desc,
																	 Common::UNK_LANG,
																	 Common::kPlatformMacintosh));
								fileSet[file->path()] = true;
							} else if (substLastIndex == 0 && g->id == GID_MANIAC &&
									   (buf[0] == 0xbc || buf[0] == 0xa0)) {
								detectedGames.push_back(DetectedGame(g->gameid, desc,
																	 Common::UNK_LANG,
																	 Common::kPlatformNES));
							} else if ((g->id == GID_MANIAC || g->id == GID_ZAK) &&
									   ((buf[0] == 0x31 && buf[1] == 0x0a) ||
										(buf[0] == 0xcd && buf[1] == 0xfe))) {
								detectedGames.push_back(DetectedGame(g->gameid, desc,
																	 Common::UNK_LANG,
																	 Common::kPlatformC64));
							} else {
								detectedGames.push_back(DetectedGame(g->gameid, desc));
								fileSet[file->path()] = false;
							}
							break;
						}
					}
				}

				substLastIndex = findSubstResFileName(subst, tempName, substLastIndex);
				applySubstResFileName(subst, tempName, detectName, sizeof(detectName));
			} while (subst.almostGameID != 0);
		}
	}

	// Now, we check the MD5 sums of the 'candidate' files. If we have an exact match,
	// only return that.
	bool exactMatch = false;
	for (StringSet::const_iterator iter = fileSet.begin(); iter != fileSet.end(); ++iter) {
		uint8 md5sum[16];
		const char *name = iter->_key.c_str();

		if (Common::md5_file(name, md5sum, 0, kMD5FileSizeLimit)) {
			char md5str[32+1];
			for (int j = 0; j < 16; j++) {
				sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
			}

			const MD5Table *elem;
			elem = (const MD5Table *)bsearch(md5str, md5table, ARRAYSIZE(md5table)-1, sizeof(MD5Table), compareMD5Table);
			if (elem) {
				if (!exactMatch)
					detectedGames.clear();	// Clear all the non-exact candidates

				const char *gameid = elem->gameid;

				// Find the GameSettings for that gameid
				for (g = scumm_settings; g->gameid; ++g) {
					if (0 == scumm_stricmp(g->gameid, gameid))
							break;
				}
				assert(g->gameid);
				// Insert the 'enhanced' game data into the candidate list
				const char *desc = findDescriptionFromGameID(g->gameid);
				if (iter->_value == true) // This was HE Mac game
					detectedGames.push_back(DetectedGame(g->gameid, desc, elem->language, Common::kPlatformMacintosh));
				else
					detectedGames.push_back(DetectedGame(g->gameid, desc, elem->language, elem->platform));
				exactMatch = true;
			}
		}
	}

	return detectedGames;
}

/**
 * Create a ScummEngine instance, based on the given detector data.
 *
 * This is heavily based on our MD5 detection scheme.
 */
Engine *Engine_SCUMM_create(GameDetector *detector, OSystem *syst) {
	Engine *engine;

	// We start by checking whether the specified game ID is obsolete.
	// If that is the case, we automaticlaly upgrade the target to use
	// the correct new game ID (and platform, if specified).
	const ObsoleteGameID *o = obsoleteGameIDsTable;
	while (o->from) {
		if (!scumm_stricmp(detector->_gameid.c_str(), o->from)) {
			// Match found, perform upgrade
			detector->_gameid = o->to;
			ConfMan.set("gameid", o->to);

			if (o->platform != Common::kPlatformUnknown)
				ConfMan.set("platform", Common::getPlatformCode(o->platform));

			warning("Target upgraded from game ID %s to %s", o->from, o->to);
			ConfMan.flushToDisk();
			break;
		}
		o++;
	}

	// Lookup the game ID in our database. If this lookup fails, then
	// the game ID is unknown, and we have to abort.
	const ScummGameSettings *g = scumm_settings;
	while (g->gameid) {
		if (!scumm_stricmp(detector->_gameid.c_str(), g->gameid))
			break;
		g++;
	}
	if (!g->gameid) {
		return 0;
	}

	// We now want to calculate the MD5 of the games detection file, so that we
	// can store it in savegames etc..
	const char *gameid = g->gameid;
	char detectName[256], tempName[256], gameMD5[32+1];
	uint8 md5sum[16];
	SubstResFileNames subst = { 0, 0, kGenAsIs };
	bool found = false;

	ScummGameSettings game = *g;

	// To this end, we first have to figure out what the proper detection file
	// is (00.LFL, 000.LFL, ...). So we iterate over all possible names,
	// and once we find a matching file, we assume that's it.
	for (int method = 0; method < kDetectNameMethodsCount && !found; method++) {
		if (!generateDetectName(g, method, detectName))
			continue;

		strcpy(tempName, detectName);

		int substLastIndex = 0;
		do {
			// FIXME: Repeatedly calling File::exists like this is a bad idea.
			// Instead, use the fs.h code to get a list of all files in that 
			// directory and simply check whether that filename is contained 
			// in it. 
			if (Common::File::exists(detectName, ConfMan.get("path").c_str())) {
				found = true;
				break;
			}

			substLastIndex = findSubstResFileName(subst, tempName, substLastIndex);
			applySubstResFileName(subst, tempName, detectName, sizeof(detectName));
		} while (subst.almostGameID != 0);

		if (found) {
			if (subst.almostGameID != 0)
				debug(5, "Generated filename substitute: %s -> %s", tempName, detectName);
			break;
		}
	}

	// Unable to locate game data
	if (!found) {
		return 0;
	}

	// Force game to have Mac platform if needed
	if (subst.almostGameID) {
		if (subst.genMethod == kGenMac ||
			subst.genMethod == kGenMacNoParens)
			game.platform = Common::kPlatformMacintosh;
	}

	// Check if the MD5 was overwritten, if so, use that in the subsequent search
	const char *md5;
	if (ConfMan.hasKey("target_md5")) {
		md5 = ConfMan.get("target_md5").c_str();
	} else {
		// Compute the MD5 of the file, and (if we succeeded) store a hex version
		// of it in gameMD5 (useful to print it to the user in messages).
		if (Common::md5_file(detectName, md5sum, ConfMan.get("path").c_str(), kMD5FileSizeLimit)) {
			for (int j = 0; j < 16; j++) {
				sprintf(gameMD5 + j*2, "%02x", (int)md5sum[j]);
			}
		}
		md5 = gameMD5;
	}

	// Now search our 'database' for the MD5; if a match is found, we use 
	// the information in the 'database' to correct the GameSettings.
	g = multiple_versions_md5_settings;
	while (g->gameid) {
		if (!scumm_stricmp(md5, g->gameid)) {
			// Match found. Copy the data and ensure that we use the correct
			// gameid (since we abused that field to store the MD5).
			game = *g;
			game.gameid = gameid;
			if (game.extra) {
				Common::String desc(findDescriptionFromGameID(gameid));
				desc += " (";
				desc += game.extra;
				desc += ")";

				// FIXME: Unconditionally setting the window caption here seems
				// quite wrong. In particular, we override whatever custom
				// description the user has set.
				g_system->setWindowCaption(desc.c_str());
			}
			break;
		}
		g++;
	}

	// Check for a user override of the platform. We allow the user to override
	// the platform, to make it possible to add games which are not yet in 
	// our MD5 database but require a specific platform setting.
	if (ConfMan.hasKey("platform"))
		game.platform = Common::parsePlatform(ConfMan.get("platform"));


	// V3 FM-TOWNS games *always* should use the corresponding music driver,
	// anything else makes no sense for them.
	if (game.platform == Common::kPlatformFMTowns && game.version == 3) {
		game.midi = MDT_TOWNS;
	}

	// Check for unknown MD5 checksums, and print a message if we encounter one.
	const MD5Table *elem;
#ifdef PALMOS_68K
	uint32 arraySize = MemPtrSize((void *)md5table) / sizeof(MD5Table) - 1;
#else
	uint32 arraySize = ARRAYSIZE(md5table) - 1;
#endif
	elem = (const MD5Table *)bsearch(md5, md5table, arraySize, sizeof(MD5Table), compareMD5Table);
	if (!elem)
		printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5);

	// Finally, we have massaged the GameSettings to our satisfaction, and can
	// instantiate the appropriate game engine. Hooray!
	switch (game.version) {
	case 1:
	case 2:
		if (game.id == GID_MANIAC && game.platform == Common::kPlatformC64)
			engine = new ScummEngine_c64(detector, syst, game, md5sum, subst);
		else
			engine = new ScummEngine_v2(detector, syst, game, md5sum, subst);
		break;
	case 3:
		if (game.features & GF_OLD_BUNDLE)
			engine = new ScummEngine_v3old(detector, syst, game, md5sum, subst);
		else
			engine = new ScummEngine_v3(detector, syst, game, md5sum, subst);
		break;
	case 4:
		engine = new ScummEngine_v4(detector, syst, game, md5sum, subst);
		break;
	case 5:
		engine = new ScummEngine_v5(detector, syst, game, md5sum, subst);
		break;
	case 6:
		switch (game.heversion) {
#ifndef DISABLE_HE
		case 100:
			engine = new ScummEngine_v100he(detector, syst, game, md5sum, subst);
			break;
		case 99:
			engine = new ScummEngine_v99he(detector, syst, game, md5sum, subst);
			break;
		case 98:
		case 95:
		case 90:
			engine = new ScummEngine_v90he(detector, syst, game, md5sum, subst);
			break;
		case 80:
			engine = new ScummEngine_v80he(detector, syst, game, md5sum, subst);
			break;
		case 73:
		case 72:
			engine = new ScummEngine_v72he(detector, syst, game, md5sum, subst);
			break;
		case 71:
			engine = new ScummEngine_v71he(detector, syst, game, md5sum, subst);
			break;
		case 70:
			engine = new ScummEngine_v70he(detector, syst, game, md5sum, subst);
			break;
#endif
#ifndef PALMOS_68K
		case 61:
			engine = new ScummEngine_v60he(detector, syst, game, md5sum, subst);
			break;
#endif
		default:
			engine = new ScummEngine_v6(detector, syst, game, md5sum, subst);
		}
		break;
#ifndef DISABLE_SCUMM_7_8
	case 7:
		engine = new ScummEngine_v7(detector, syst, game, md5sum, subst);
		break;
	case 8:
		engine = new ScummEngine_v8(detector, syst, game, md5sum, subst);
		break;
#endif
	default:
		error("Engine_SCUMM_create(): Unknown version of game engine");
	}

	return engine;
}

REGISTER_PLUGIN(SCUMM, "Scumm Engine")

#ifdef PALMOS_68K
#include "scumm_globals.h"

_GINIT(Scumm_md5table)
_GSETPTR(md5table, GBVARS_MD5TABLE_INDEX, MD5Table, GBVARS_SCUMM)
_GEND

_GRELEASE(Scumm_md5table)
_GRELEASEPTR(GBVARS_MD5TABLE_INDEX, GBVARS_SCUMM)
_GEND

#endif
