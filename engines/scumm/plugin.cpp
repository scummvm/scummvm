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

// The following table contains information about variants of our various games.
// We index into it with help of md5table (from scumm-md5.h), to find the correct
// GameSettings for a given game variant.
//
// The first listed variant is assumed to be the 'default' variant -- i.e. the
// variant that gets used when no explicit variant code has been specified.
static const GameSettings gameVariantsTable[] = {
	// The C64 version of MM is detected via the platform field and hence has no seperate entry in this list
	{"maniac", "V2",  GID_MANIAC, 2, 0, MDT_PCSPK, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, UNK},
	{"maniac", "NES", GID_MANIAC, 1, 0, MDT_NONE,  GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformNES},
	{"maniac", "V1",  GID_MANIAC, 1, 0, MDT_PCSPK, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformPC},

	// The C64 version of Zak is detected via the platform field and hence has no seperate entry in this list
	{"zak", "V2",       GID_ZAK, 2, 0, MDT_PCSPK, GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, UNK},
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
	{"puttmoon", 0, GID_HEGAME,  6, 61, MDT_ADLIB | MDT_MIDI, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"funpack",  0, GID_FUNPACK, 6, 61, MDT_ADLIB | MDT_MIDI, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"fbpack",   0, GID_HEGAME,  6, 61, MDT_ADLIB | MDT_MIDI, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},

	{"puttputt", "HE 61", GID_HEGAME,   6, 61, MDT_ADLIB | MDT_MIDI, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"puttputt", "HE 60", GID_HEGAME,   6, 60, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, Common::kPlatformWindows},
	{"puttputt", "Demo",  GID_PUTTDEMO, 6, 60, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK},

	{"fbear", "HE 61", GID_FBEAR, 6, 61, MDT_ADLIB | MDT_MIDI, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"fbear", "HE 70", GID_FBEAR, 6, 70, MDT_NONE,             GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

#ifndef DISABLE_HE
	{"activity", "", GID_HEGAME, 6, 70, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 7.1
	// The first version to use 640x480 resolution
	// There are also 7.1 versions of freddemo, airdemo and farmdemo
	{"catalog", "", GID_HEGAME, 6, 71, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"freddi", "", GID_HEGAME, 6, 71, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 7.2
	{"airport", "", GID_HEGAME, 6, 72, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"puttzoo", "", GID_HEGAME, 6, 72, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Changed o_getResourceSize to cover all resource types
	{"farm", "", GID_HEGAME, 6, 73, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"jungle", "", GID_HEGAME, 6, 73, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 8.0 ?  Scummsrc.80
	{"freddi2", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"pajama", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"putttime", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"balloon", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"dog", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"maze", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"socks", "", GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"water", "",       GID_WATER, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"water", "HE 80", GID_WATER, 6, 80, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"water", "HE 99", GID_WATER, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 9.0 ?  Scummsys.90
	{"baseball", "", GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"thinkerk", "", GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"thinker1", "", GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"freddi3", "", GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"spyfox", "", GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 9.5 ?  Scummsys.95
	{"pajama2", "", GID_HEGAME, 6, 95, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"chase", "", GID_HEGAME, 6, 95, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 9.8 ?  Scummsys.98
	// these and later games can easily be identified by the .(a) file instead of a .he1
	// and INIB chunk in the .he0
	{"lost", "", GID_HEGAME, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"puttrace", "HE 98",   GID_PUTTRACE, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"puttrace", "HE 98.5", GID_PUTTRACE, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},
	{"puttrace", "HE 99",   GID_PUTTRACE, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"bluesabctime", "", GID_HEGAME, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"soccer", "", GID_SOCCER, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Global scripts increased to 2048
	{"freddi4", "",       GID_HEGAME, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},
	{"freddi4", "unenc", GID_HEGAME, 6, 98, MDT_NONE,              GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 9.9 ?  Scummsys.99
	{"football", 0, GID_FOOTBALL, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"pajama3", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"puttcircus", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"spyfox2", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"mustard", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},

	// Added the use of fonts
	{"FreddisFunShop", 0, GID_FUNSHOP, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"SamsFunShop", 0, GID_FUNSHOP, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"PuttsFunShop", 0, GID_FUNSHOP, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},

	// Added 16bit color
	{"baseball2001", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"SoccerMLS", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"spyozon", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},

	{"freddicove", 0,        GID_HEGAME, 6,  99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"freddicove", "unenc",  GID_HEGAME, 6,  99, MDT_NONE,              GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"freddicove", "HE 100", GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},

	// Restructured the Scumm engine
	{"pjgames", 0, GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},

	// Uses bink in external files for logos
	{"Baseball2003", 0, GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"basketball", 0, GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"Soccer2004", 0, GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},

	// Uses smacker in external files, for testing only
	{"BluesBirthdayDemo", 0, GID_HEGAME, 6, 98, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"artdemo", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"readdemo", 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},


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

static const SubstResFileNames substResFileNameTable[] = {
	// The first few entries for 00.LFL/01.LFL files are here for two reasons:
	// 1) For the detector to detect these games
	// 2) Because the ScummEngine constructor distinguishes between extracted
	//    and non-extracted variants of these C64/NES games by the presence
	//    or absence of a SubstResFileNames record.
	// Use 1 will go away with the new detector code. Use 2 needs some more
	// attention, but most likely should be solved by passing the name of the
	// "detect file" to the contructor.
	{ "00.LFL", "Maniac Mansion (E).prg", kGenAsIs },
	{ "00.LFL", "Maniac Mansion (F).prg", kGenAsIs },
	{ "00.LFL", "Maniac Mansion (SW).prg", kGenAsIs },
	{ "00.LFL", "Maniac Mansion (U).prg", kGenAsIs },
	{ "00.LFL", "Maniac Mansion (G).prg", kGenAsIs },
	{ "00.LFL", "maniac1.d64", kGenAsIs }, // Do not
	{ "01.LFL", "maniac2.d64", kGenAsIs }, // swap
	{ "00.LFL", "zak1.d64", kGenAsIs },    // these
	{ "01.LFL", "zak2.d64", kGenAsIs },    // lines

	{ "atlantis", "Fate of Atlantis Data", kGenAsIs },
	{ "atlantis", "fate", kGenPC },
	{ "atlantis", "playfate", kGenPC },
	{ "atlantis", "indy4", kGenPC },
	{ "atlantis", "indydemo", kGenPC },
	{ "tentacle", "Day of the Tentacle Data", kGenAsIs },
	{ "tentacle", "dottdemo", kGenPC },
	{ "tentacle", "Day of the Tentacle Demo Data", kGenAsIs },
	{ "monkey", "monkey1", kGenPC },
	{ "monkey", "monkeyk", kGenPC }, // FM-TOWNS Jap
	{ "monkey", "game", kGenPC }, // SegaCD
	{ "monkey2", "mi2demo", kGenPC },
	{ "samnmax", "Sam & Max Data", kGenAsIs },
	{ "samnmax", "Sam & Max Demo Data", kGenAsIs },
	{ "samnmax", "ramnmax", kGenPC }, // Used in some releases of Russian Sam'n'Max
	{ "samnmax", "samdemo", kGenPC },
	{ "samnmax", "snmdemo", kGenPC },
	{ "samnmax", "snmidemo", kGenPC },
	{ "samnmax", "sdemo", kGenPC },
#ifndef DISABLE_SCUMM_7_8
	{ "dig", "The Dig Data", kGenAsIs },
	{ "dig", "The Dig Demo Data", kGenAsIs },
	{ "ft", "Full Throttle Data", kGenAsIs },
	{ "ft", "Full Throttle Demo Data", kGenAsIs },
	{ "ft", "Vollgas Data", kGenAsIs },
	{ "ft", "Vollgas Demo Data", kGenAsIs },
	{ "ft", "ftdemo", kGenPC },
#endif
	{ "fbear", "fbdemo", kGenPC },
	{ "fbear", "Fatty Bear Demo", kGenMacNoParens },
	{ "fbear", "Fatty Bear", kGenMacNoParens },
	{ "puttmoon", "moondemo", kGenPC },
	{ "puttmoon", "Putt-Putt Moon Demo", kGenMacNoParens },
	{ "puttmoon", "Putt-Putt Moon", kGenMacNoParens },
	{ "puttputt", "puttdemo", kGenPC },
	{ "puttputt", "Putt-Putt's Demo", kGenMacNoParens },
	{ "puttputt", "Putt-Putt Parade", kGenMacNoParens },
#ifndef DISABLE_HE
	{ "airport", "airdemo", kGenPC },
	{ "airport", "Airport Demo", kGenMac },
	{ "airport", "The AirPort", kGenMac },
	{ "balloon", "Balloon-O-Rama", kGenMac },
	{ "baseball", "BaseBall", kGenMac },
	{ "baseball2001", "bb2demo", kGenPC },
	{ "baseball2001", "Baseball 2001 Demo", kGenMac },
	{ "baseball2001", "Baseball 2001", kGenMac },
	{ "baseball2001", "baseball 2001", kGenPC },
	{ "Baseball2003", "Baseball 2003", kGenMac },
	{ "basketball", "Basketball", kGenMac },
	{ "bluesabctime", "BluesABCTimeDemo", kGenPC },
	{ "bluesabctime", "BluesABCTimeDemo", kGenMac },
	{ "catalog", "catalog2", kGenPC },
	{ "chase", "Cheese Chase", kGenMac },
	{ "dog", "Dog on a Stick", kGenMac },
	{ "farm", "farmdemo", kGenPC },
	{ "farm", "Farm Demo", kGenMac },
	{ "football", "FootBall", kGenMac },
	{ "football", "FootBall Demo", kGenMac },
	{ "football", "FootBall2002", kGenPC },
	{ "football", "footdemo", kGenPC },
	{ "freddi", "freddemo", kGenPC },
	{ "freddi", "Freddi Demo", kGenMac },
	{ "freddi", "Freddi Fish", kGenMac },
	{ "freddi", "FreddiD", kGenPC },
	{ "freddi2", "ff2-demo", kGenPC },
	{ "freddi2", "FFHSDemo", kGenMac },
	{ "freddi2", "FFHSDemo", kGenPC },
	{ "freddi2", "Freddi Fish 2 Demo", kGenMac },
	{ "freddi2", "Freddi Fish 2", kGenMac },
	{ "freddi2", "FreddiCHSH", kGenPC },
	{ "freddi2", "Fritzi Fisch 2", kGenMac },
	{ "freddi3", "F3-mdemo", kGenMac },
	{ "freddi3", "F3-Mdemo", kGenMac },
	{ "freddi3", "f3-mdemo", kGenPC },
	{ "freddi3", "FF3-DEMO", kGenPC },
	{ "freddi3", "Freddi Fish 3", kGenMac },
	{ "freddi3", "FreddiFGT", kGenPC },
	{ "freddi3", "FreddiFGT", kGenMac },
	{ "freddi3", "FreddiSCS", kGenPC },
	{ "freddi3", "Fritzi3demo", kGenMac },
	{ "freddi3", "Fritzi3demo", kGenPC },
	{ "freddi3", "MM3-DEMO", kGenPC },
	{ "freddi3", "MM3-Demo", kGenMac }, // FR Mac demo
	{ "freddi4", "f4-demo", kGenPC },
	{ "freddi4", "ff4demo", kGenPC },
	{ "freddi4", "Ff4demo", kGenMac },
	{ "freddi4", "Freddi 4", kGenMac },
	{ "freddi4", "Freddi 4 Demo", kGenMac },
	{ "freddi4", "FreddiGS", kGenPC },
	{ "freddi4", "FreddiGS", kGenMac },
	{ "freddi4", "FreddiHRBG", kGenPC },
	{ "freddicove", "FreddiCCC", kGenPC },
	{ "freddicove", "FreddiCove", kGenMac },
	{ "freddicove", "FreddiDZZ", kGenPC },
	{ "freddicove", "ff5demo", kGenPC },
	{ "freddicove", "FFCoveDemo", kGenPC },
	{ "freddicove", "FreddiCoveDemo", kGenMac },
	{ "freddicove", "FF5Demo", kGenMac }, // NL Mac demo
	{ "FreddisFunShop", "Freddi's FunShop", kGenMac },
	{ "jungle", "The Jungle", kGenMac },
	{ "lost", "Lost and Found", kGenMac },
	{ "lost", "smaller", kGenPC },
	{ "maze", "Maze Madness", kGenMac},
	{ "mustard", "Mustard", kGenMac },
	{ "pajama", "Pyjama Pit", kGenMac },
	{ "pajama", "Pajama Sam", kGenMac },
	{ "pajama", "PajamaNHD", kGenPC },
	{ "pajama", "PJS-DEMO", kGenPC },
	{ "pajama", "PYJAMA", kGenPC },
	{ "pajama", "SAMDEMO", kGenPC },
	{ "pajama", "SAMDEMO", kGenMac }, // FR Mac demo
	{ "pajama2", "Pajama Sam 2", kGenMac },
	{ "pajama2", "PajamaTAL", kGenPC },
	{ "pajama2", "PyjamaDBMN", kGenPC },
	{ "pajama2", "PyjamaDBMN", kGenMac },
	{ "pajama2", "Pyjama Pit 2 Demo", kGenMac },
	{ "pajama2", "PJP2DEMO", kGenPC },
	{ "pajama2", "PJ2Demo", kGenMac },
	{ "pajama2", "pj2demo", kGenPC },
	{ "pajama2", "Pjs2demo", kGenPC },
	{ "pajama2", "PJ2 Demo", kGenMac }, // NL Mac demo
	{ "pajama3", "GPJ3Demo", kGenPC },
	{ "pajama3", "Pajama Sam 3", kGenMac },
	{ "pajama3", "Pajama Sam 3-Demo", kGenMac },
	{ "pajama3", "pj3-demo", kGenPC },
	{ "pajama3", "pj3demo", kGenPC },
	{ "pajama3", "PJ3Demo", kGenMac },
	{ "pajama3", "Pajama Sam Demo", kGenMac },
	{ "pajama3", "PjSamDemo", kGenMac },
	{ "pajama3", "PjSamDemo", kGenPC },
	{ "pajama3", "PyjamaSKS", kGenPC },
	{ "pajama3", "PyjamaSKS", kGenMac },
	{ "pajama3", "UKPajamaEAT", kGenPC }, // Russian
	{ "pjgames", "PJGames", kGenMac },
	{ "puttcircus", "circdemo", kGenPC },
	{ "puttcircus", "Putt Circus Demo", kGenMac },
	{ "puttcircus", "Putt Circus", kGenMac },
	{ "puttrace", "500demo", kGenPC },
	{ "puttrace", "racedemo", kGenPC },
	{ "puttrace", "RaceDemo", kGenMac },
	{ "puttrace", "Rennen", kGenPC },
	{ "puttrace", "Putt500 demo", kGenMac }, // NL Mac demo
	{ "puttrace", "Putt Race", kGenMac },
	{ "puttrace", "ToffRennen", kGenPC },
	{ "puttrace", "ToffRennen", kGenMac },
	{ "puttrace", "UKPuttRace", kGenPC }, // Russian
	{ "PuttsFunShop", "Putt's FunShop", kGenMac },
	{ "putttime", "PuttPuttTTT", kGenPC },
	{ "putttime", "PuttPuttTTT", kGenMac },
	{ "putttime", "PuttTijd", kGenPC },
	{ "putttime", "Putt Time", kGenMac },
	{ "putttime", "PuttTTT", kGenMac },
	{ "putttime", "PuttTTT", kGenPC },
	{ "putttime", "TIJDDEMO", kGenPC },
	{ "putttime", "timedemo", kGenPC },
	{ "putttime", "TimeDemo", kGenMac },
	{ "putttime", "TEMPDEMO", kGenPC },
	{ "putttime", "Tempdemo", kGenMac }, // FR Mac demo
	{ "putttime", "toffzeit", kGenPC }, // German Toeff-Toeff: Reist durch die Zeit
	{ "putttime", "toffzeit", kGenMac }, // German Toeff-Toeff: Reist durch die Zeit
	{ "putttime", "ZeitDemo", kGenMac },
	{ "putttime", "ZEITDEMO", kGenPC },
	{ "puttzoo", "Puttzoo Demo", kGenMac },
	{ "puttzoo", "PuttZoo", kGenMac }, 

	{ "puttzoo", "T\xC3\xB6""ff-T\xC3\xB6""ff\xE2\x84\xA2 Zoo Demo", kGenMac },	// German Toeff-Toeff, UTF-8 encoding
	{ "puttzoo", "T\xF6""ff-T""\xF6""ff\x99 Zoo Demo", kGenMac },	// German Toeff-Toeff, Windows encoding

	{ "puttzoo", "zoodemo", kGenPC },
	{ "puttzoo", "Zoo Demo", kGenMac },
	{ "SamsFunShop", "Sam's FunShop", kGenMac },
	{ "soccer", "Soccer", kGenMac },
	{ "Soccer2004", "Soccer 2004", kGenMac },
	{ "socks", "SockWorks", kGenMac },
	{ "spyfox", "Fuchsdem", kGenMac },
	{ "spyfox", "FUCHSDEM", kGenPC},
	{ "spyfox", "FoxDemo", kGenMac },
	{ "spyfox", "foxdemo", kGenPC},
	{ "spyfox", "JAMESDEM", kGenPC },
	{ "spyfox", "Spydemo", kGenMac},
	{ "spyfox", "Spydemo", kGenPC},
	{ "spyfox", "SPYFox", kGenMac },
	{ "spyfox", "SPYFoxDC", kGenPC },
	{ "spyfox", "SPYFoxDC", kGenMac },
	{ "spyfox", "SpyFoxDMK", kGenPC },
	{ "spyfox", "SpyFoxDMK", kGenMac },
	{ "spyfox", "Spy Fox Demo", kGenMac }, // NL Mac demo
	{ "spyfox", "JR-Demo", kGenMac }, // FR Mac demo
	{ "spyfox2", "sf2-demo", kGenPC },
	{ "spyfox2", "sf2demo", kGenPC },
	{ "spyfox2", "Sf2demo", kGenMac },
	{ "spyfox2", "Spy Fox 2 - Demo", kGenMac },
	{ "spyfox2", "Spy Fox 2", kGenMac },
	{ "spyfox2", "SpyFoxOR", kGenPC },
	{ "spyfox2", "SpyFoxOR", kGenMac },
	{ "spyfox2", "spyfoxsr", kGenPC },
	{ "spyozon", "sf3-demo", kGenPC },
	{ "spyozon", "Spy Ozone Demo", kGenMac },
	{ "spyozon", "SPYFoxOZU", kGenPC },
	{ "spyozon", "SpyOzon", kGenMac },
	{ "thinker1", "1grademo", kGenPC },
	{ "thinker1", "Thinker1", kGenMac },
	{ "thinkerk", "kinddemo", kGenPC },
	{ "thinkerk", "KindDemo", kGenMac },
	{ "thinkerk", "ThinkerK", kGenMac },
	{ "water", "Water Worries", kGenMac },
#endif
	{ NULL, NULL, kGenAsIs }
};


#if 0

enum FilenameGenMethod {
	kGenDiskNum,
	kGenRoomNum,
	kGenHEMac,
	kGenHEMacNoParens,
	kGenHEPC,
	kGenUnchanged
};

struct GameFilenamePattern {
	const char *gameid;
	const char *pattern;
	FilenameGenMethod genMethod;
	Common::Language language;
	Common::Platform platform;
	const char *variant;
};

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

	default:
		error("generateFilenameForDetection: Unhandled genMethod");
	}

	return buf;
}

#if 0
Common::String ScummEngine::generateFilename(int room, int diskNumber) {
	char buf[128];

	if (_game.version == 4) {
		if (room == 0 || room >= 900) {
			snprintf(buf, sizeof(buf), "%.3d.lfl", room);
		} else {
			snprintf(buf, sizeof(buf), "disk%.2d.lec", diskNumber);
		}
	} else {
		char id = 0;

		switch (_substEntry.genMethod) {
		case kGenDiskNum:
			snprintf(buf, sizeof(buf), _substEntry.pattern, diskNumber);
			break;
	
		case kGenRoomNum:
			snprintf(buf, sizeof(buf), _substEntry.pattern, room);
			break;

		case kGenHEMac:
		case kGenHEMacNoParens:
		case kGenHEPC:
			// FIXME: Not sure if the following HE section is quite correct...
			if (_game.heversion >= 98) {
				int disk = 0;
				if (_heV7DiskOffsets)
					disk = _heV7DiskOffsets[room];
		
				switch(disk) {
				case 2:
					id = 'b';
					snprintf(buf, sizeof(buf), "%s.(b)", pattern);
					break;
				case 1:
					id = 'a';
					snprintf(buf, sizeof(buf), "%s.(a)", pattern);
					break;
				default:
					id = '0';
					snprintf(buf, sizeof(buf), "%s.he0", pattern);
				}
			} else if (_game.heversion >= 70) {
				id = (room == 0) ? '0' : '1';
			} else {
				id = diskNumber + '0';
			}
			
			if (_substEntry.genMethod == kGenHEPC) {
				// For HE >= 98, we already called snprintf above.
				if (_game.heversion < 98)
					snprintf(buf, sizeof(buf), "%s.he%c", pattern, id);
			} else {
				if (id == '3') { // special case for cursors
					// For mac they're stored in game binary
					strncpy(buf, _substEntry.pattern, bufsize);
				} else {
					if (subst.genMethod == kGenMac)
						snprintf(buf, sizeof(buf), "%s (%c)", _substEntry.pattern, id);
					else
						snprintf(buf, sizeof(buf), "%s %c", _substEntry.pattern, id);
				}
			}

			break;

		default:
			error("FOO");
		}
	}

	return buf;
}
#endif

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
	{ "maniac", "%.2d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "maniac", "%.2d.MAN", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "maniac", "maniac1.d64", kGenUnchanged, UNK_LANG, Common::kPlatformC64, 0 },   // ... and maniac2.d64
	{ "maniac", "Maniac Mansion (E).prg", kGenUnchanged, Common::EN_GRB, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (F).prg", kGenUnchanged, Common::FR_FRA, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (SW).prg", kGenUnchanged, Common::SE_SWE, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (U).prg", kGenUnchanged, Common::EN_USA, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (G).prg", kGenUnchanged, Common::DE_DEU, Common::kPlatformNES, "NES" },

	{ "zak", "%.2d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "zak", "zak1.d64", kGenUnchanged, UNK_LANG, Common::kPlatformC64, 0 },         // ... and zak2.d64

	{ "indy3", "%.2d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },

	{ "loom", "%.2d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "loom", "%.3d.LFL", kGenRoomNum, UNK_LANG, UNK, "VGA" },	// Loom CD

	{ "pass", "%.3d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },

	{ "monkey", "%.3d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },		// EGA & VGA versions
	{ "monkey", "monkey.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "monkey", "monkey1.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "monkey", "monkeyk.%.3d", kGenRoomNum, Common::JA_JPN, Common::kPlatformFMTowns, "FM-TOWNS" }, // FM-TOWNS Jap
	{ "monkey", "game.%.3d", kGenRoomNum, UNK_LANG, Common::kPlatformSegaCD, "SEGA" }, // SegaCD

	{ "monkey2", "monkey2.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "monkey2", "mi2demo.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },

	{ "atlantis", "atlantis.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "atlantis", "fate.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "atlantis", "playfate.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "atlantis", "indy4.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "atlantis", "indydemo.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "atlantis", "Fate of Atlantis Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "tentacle", "tentacle.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "tentacle", "dottdemo.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "tentacle", "Day of the Tentacle Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "tentacle", "Day of the Tentacle Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "samnmax", "samnmax.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "samnmax.sm%d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "Sam & Max Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "samnmax", "Sam & Max Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "samnmax", "ramnmax.%.3d", kGenRoomNum, Common::RU_RUS, UNK, 0 }, // Used in some releases of Russian Sam'n'Max
	{ "samnmax", "samdemo.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "snmdemo.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "snmidemo.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "sdemo.%.3d", kGenRoomNum, UNK_LANG, UNK, 0 },

#ifndef DISABLE_SCUMM_7_8
	{ "dig", "dig.la%d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "dig", "The Dig Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "dig", "The Dig Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "ft", "ft.la%d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "ft", "ft.%.3d", kGenDiskNum, UNK_LANG, UNK, 0 },    // Used by PC version of Full Throttle demo
	{ "ft", "ftdemo.la%d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "ft", "Full Throttle Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "ft", "Full Throttle Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "ft", "Vollgas Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "ft", "Vollgas Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "comi", "comi.la%d", kGenDiskNum, UNK_LANG, UNK, 0 },
#endif

	{ "fbear", "fbear", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "fbear", "fbdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "fbear", "Fatty Bear Demo", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "fbear", "Fatty Bear", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },

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
	{ "freddi2", "Fritzi Fisch 2", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },	// FIXME: Is this a german version?

	{ "freddi3", "freddi3", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi3", "F3-mdemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "F3-Mdemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "f3-mdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi3", "FF3-DEMO", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi3", "Freddi Fish 3", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "FreddiFGT", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi3", "FreddiFGT", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "FreddiSCS", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi3", "Fritzi3demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },	// FIXME: Is this a german version?
	{ "freddi3", "Fritzi3demo", kGenHEPC, UNK_LANG, UNK, 0 },	// FIXME: Is this a german version?
	{ "freddi3", "MM3-DEMO", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi3", "MM3-Demo", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 }, // FR Mac demo

	{ "freddi4", "freddi4", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi4", "f4-demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi4", "ff4demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi4", "Ff4demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "Freddi 4", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "Freddi 4 Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "FreddiGS", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi4", "FreddiGS", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "FreddiHRBG", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "freddicove", "freddicove", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "FreddiCCC", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "FreddiCove", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddicove", "FreddiDZZ", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "ff5demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "FFCoveDemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "FreddiCoveDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddicove", "FF5Demo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 }, // NL Mac demo

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
	{ "pajama", "SAMDEMO", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama", "SAMDEMO", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 }, // FR Mac demo

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
	{ "pajama2", "PJ2 Demo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 }, // NL Mac demo

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
	{ "pajama3", "PyjamaSKS", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama3", "PyjamaSKS", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "UKPajamaEAT", kGenHEPC, Common::RU_RUS, UNK, 0 }, // Russian

	{ "pjgames", "pjgames", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pjgames", "PJGames", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "puttcircus", "puttcircus", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttcircus", "circdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttcircus", "Putt Circus Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttcircus", "Putt Circus", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "puttrace", "puttrace", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttrace", "500demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttrace", "racedemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttrace", "RaceDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttrace", "Rennen", kGenHEPC, UNK_LANG, UNK, 0 },	// FIXME: Is this a german version?
	{ "puttrace", "Putt500 demo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 }, // NL Mac demo
	{ "puttrace", "Putt Race", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttrace", "ToffRennen", kGenHEPC, UNK_LANG, UNK, 0 },	// FIXME: Is this a german version?
	{ "puttrace", "ToffRennen", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },	// FIXME: Is this a german version?
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
	{ "putttime", "TEMPDEMO", kGenHEPC, UNK_LANG, UNK, 0 },	// FIXME: Is this a french version?
	{ "putttime", "Tempdemo", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 }, // FR Mac demo
	{ "putttime", "toffzeit", kGenHEPC, Common::DE_DEU, UNK, 0 }, // German Toeff-Toeff: Reist durch die Zeit
	{ "putttime", "toffzeit", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 }, // German Toeff-Toeff: Reist durch die Zeit
	{ "putttime", "ZeitDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },	// FIXME: Is this a german version?
	{ "putttime", "ZEITDEMO", kGenHEPC, UNK_LANG, UNK, 0 },	// FIXME: Is this a german version?

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
	{ "spyfox", "Fuchsdem", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },	// FIXME: Is this a german version?
	{ "spyfox", "FUCHSDEM", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "FoxDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "foxdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "JAMESDEM", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "Spydemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "Spydemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "SPYFox", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "SPYFoxDC", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "SPYFoxDC", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "SpyFoxDMK", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "SpyFoxDMK", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "Spy Fox Demo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 }, // NL Mac demo
	{ "spyfox", "JR-Demo", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 }, // FR Mac demo

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


#endif


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

#pragma mark -
#pragma mark --- Filename substitution ---
#pragma mark -


static void applySubstResFileName(const SubstResFileNames &subst, char *buf, int bufsize, const char *ext, char num) {
	switch (subst.genMethod) {
	case kGenMac:
	case kGenMacNoParens:
		if (num == '3') { // special case for cursors
			// For mac they're stored in game binary
			strncpy(buf, subst.expandedName, bufsize);
		} else {
			if (subst.genMethod == kGenMac)
				snprintf(buf, bufsize, "%s (%c)", subst.expandedName, num);
			else
				snprintf(buf, bufsize, "%s %c", subst.expandedName, num);
		}
		break;

	case kGenPC:
		if (ext)
			snprintf(buf, bufsize, "%s%s", subst.expandedName, ext);
		else
			strncpy(buf, subst.expandedName, bufsize);
		break;

	case kGenAsIs:
		strncpy(buf, subst.expandedName, bufsize);
		break;

	default:
		*buf = 0;
		break;
	}
}

bool applySubstResFileName(const SubstResFileNames &subst, const char *filename, char *buf, int bufsize) {
	if (subst.almostGameID == 0)
		return false;

	size_t len = strlen(filename);
	assert(len > 0);

	char num = filename[len - 1];

	// In some cases we have .(a) and .(b) extensions
	if (num == ')')
		num = filename[len - 2];

	const char *ext = strrchr(filename, '.');
	if (ext)
		len = ext - filename;

	if (!scumm_strnicmp(filename, subst.almostGameID, len)) {
		applySubstResFileName(subst, buf, bufsize, ext, num);
		return true;
	}

	return false;
}

int findSubstResFileName(SubstResFileNames &subst, const char *filename, int index) {
	if (index < 0)
		return -1;

	size_t len = strlen(filename);
	assert(len > 0);

	char num = filename[len - 1];

	// In some cases we have .(a) and .(b) extensions
	if (num == ')')
		num = filename[len - 2];

	const char *ext = strrchr(filename, '.');
	if (ext)
		len = ext - filename;

	int i;
	for (i = index; substResFileNameTable[i].almostGameID; i++) {
		if (!scumm_strnicmp(filename, substResFileNameTable[i].almostGameID, len)) {
			subst = substResFileNameTable[i];
			return i+1;
		}
	}
	subst = substResFileNameTable[i];
	return -1;
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


enum {
	kDetectNameMethodsCount = 8
};

static bool generateDetectName(const GameSettings &g, int method, char *detectName) {
	detectName[0] = '\0';

	switch (method) {
	case 0:
		if (g.version <= 3)
			strcpy(detectName, "00.LFL");
		break;
	case 1:
		// FIXME: The following would normally only allow for V4 games. But 
		// for loom, there is both a v3 and a v4 version, but due to the way
		// the detector works at this time, we'll only ever see the v3 variant
		// here.
		if (g.version == 3 || g.version == 4)
			strcpy(detectName, "000.LFL");
		break;
	case 2:
		if (g.version < 4 || g.version > 7)
			return false;
		strcpy(detectName, g.gameid);
		strcat(detectName, ".000");
		break;
	case 3:
		if (g.version >= 7) {
			strcpy(detectName, g.gameid);
			strcat(detectName, ".la0");
		}
		break;
	case 4:
		if (g.heversion != 0) {
			strcpy(detectName, g.gameid);
			strcat(detectName, ".he0");
		}
		break;
	case 5:
		// FIXME: Fingolfin asks: For which games is this case used? 
		// Please document this. Also: Why was this case missing in
		// Engine_SCUMM_create ?
		strcpy(detectName, g.gameid);
		break;
	case 6:
		if (g.id == GID_SAMNMAX) {
			strcpy(detectName, g.gameid);
			strcat(detectName, ".sm0");
		}
		break;
	case 7:
		if (g.id == GID_MANIAC)
			strcpy(detectName, "00.MAN");
		break;
	default:
		return false;
	}

	if (!detectName[0])
		return false;

	return true;
}

DetectedGameList Engine_SCUMM_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const GameSettings *g;
	char detectName[128];
	char tempName[128];
	SubstResFileNames subst = { 0, 0, kGenAsIs };

	typedef Common::HashMap<Common::String, bool> StringSet;
	StringSet fileSet;

	const char *lastGameid = "";

	for (g = gameVariantsTable; g->gameid; ++g) {
		// HACK: For now we only consider the first ("default") variant for
		// gameids that have multiple variants. In a future version of the
		// detector code, this may change.
		if (0 == strcmp(lastGameid, g->gameid))
			continue;
		lastGameid = g->gameid;
	
		// Determine the 'detectname' for this game, that is, the name of a
		// file that *must* be presented if the directory contains the data
		// for this game. For example, FOA requires atlantis.000

		// TODO: we need to add cache here
		for (int method = 0; method < kDetectNameMethodsCount; method++) {
			if (!generateDetectName(*g, method, detectName))
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
							DetectedGame dg(g->gameid, findDescriptionFromGameID(g->gameid));
							if (substLastIndex > 0 && // HE Mac versions.
								(subst.genMethod == kGenMac ||
								 subst.genMethod == kGenMacNoParens)) {
								dg.platform = Common::kPlatformMacintosh;
								fileSet[file->path()] = true;
							} else if (substLastIndex == 0 && g->id == GID_MANIAC &&
									   (buf[0] == 0xbc || buf[0] == 0xa0)) {
								dg.platform = Common::kPlatformNES;
							} else if ((g->id == GID_MANIAC || g->id == GID_ZAK) &&
									   ((buf[0] == 0x31 && buf[1] == 0x0a) ||
										(buf[0] == 0xcd && buf[1] == 0xfe))) {
								dg.platform = Common::kPlatformC64;
							} else {
								fileSet[file->path()] = false;
							}
							
							dg.updateDesc();	// Append the platform, if set, to the description.

							detectedGames.push_back(dg);
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

			const MD5Table *elem = findInMD5Table(md5str);
			if (elem) {
				if (!exactMatch)
					detectedGames.clear();	// Clear all the non-exact candidates

				DetectedGame dg(elem->gameid, findDescriptionFromGameID(elem->gameid), elem->language);
				if (iter->_value == true) // This was HE Mac game
					dg.platform = Common::kPlatformMacintosh;
				else
					dg.platform = elem->platform;
				dg.updateDesc(elem->extra);	// Append extra information to the description.
				
				// Insert the 'enhanced' game data into the candidate list
				detectedGames.push_back(dg);

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
	// If that is the case, we automatically upgrade the target to use
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
	const GameSettings *g = gameVariantsTable;
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
	char detectName[256], tempName[256];
	uint8 md5sum[16];
	SubstResFileNames subst = { 0, 0, kGenAsIs };
	bool found = false;

	GameSettings game = *g;

	// To this end, we first have to figure out what the proper detection file
	// is (00.LFL, 000.LFL, ...). So we iterate over all possible names,
	// and once we find a matching file, we assume that's it.
	for (int method = 0; method < kDetectNameMethodsCount && !found; method++) {
		if (!generateDetectName(game, method, detectName))
			continue;

		strcpy(tempName, detectName);

		int substLastIndex = 0;
		do {
			// FIXME: Repeatedly calling File::exists like this is a bad idea.
			// Instead, use the fs.h code to get a list of all files in that 
			// directory and simply check whether that filename is contained 
			// in it. 
			if (Common::File::exists(detectName)) {
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

	// Determine a MD5 checksum which then is used to narrow down the choice
	// of game variants. 
	const char *md5 = NULL;
	char md5buf[33];

	// First, check if the MD5 was overridden with a config file entry.
	if (ConfMan.hasKey("target_md5")) {
		assert(ConfMan.get("target_md5").size() == 32);
		md5 = ConfMan.get("target_md5").c_str();
	} else {
		// Next, check if the MD5 was overridden via a target_md5.txt file.
		Common::File target_md5F;
		target_md5F.open("target_md5.txt");
	
		if (target_md5F.isOpen()) {
			bool valid = true;
	
			target_md5F.readLine(md5buf, 33);
			for (int j = 0; j < 32 && valid; ++j)
				if (!((md5buf[j] >= '0' && md5buf[j] <= '9') || 
					  (md5buf[j] >= 'A' && md5buf[j] <= 'F') ||
					  (md5buf[j] >= 'a' && md5buf[j] <= 'f')))
					valid = false;
	
			if (valid)
				md5 = md5buf;
		}
	}
	
	// Finally, if no MD5 value has been determined so far, compute it from the
	// detect file.
	if (!md5) {
		// Compute the MD5 of the file, and (if we succeeded) store a hex version
		// of it in gameMD5 (useful to print it to the user in messages).
		if (Common::md5_file(detectName, md5sum, NULL, kMD5FileSizeLimit)) {
			for (int j = 0; j < 16; j++) {
				sprintf(md5buf + j*2, "%02x", (int)md5sum[j]);
			}
		}
		md5 = md5buf;
	}



	// Now look up the MD5 in our lookup table (md5table).
	const MD5Table *elem = findInMD5Table(md5);


	// If a match was found, we use the information obtained from the md5table
	// to walk through the gameVariantsTable array and find a match there.
	// Otherwise, we print a warning about the MD5 being unknwon.
	if (elem) {
		// The MD5 is known and was found in our md5table.
		debug(5, "Using MD5 '%s'", md5);
		
		// Sanity check: Make sure the gameids match!
		if (scumm_stricmp(elem->gameid, gameid)) {
			error("MD5 %s implies gameid '%s', but gameid '%s' was used",
					md5, elem->gameid, gameid);
		}
	
		// Compute the precise game settings using 'gameVariantsTable'.
		for (g = gameVariantsTable; g->gameid; ++g) {
			if (g->gameid[0] == 0 || !scumm_stricmp(elem->gameid, g->gameid)) {
				// The gameid either matches, or is empty (the latter indicates
				// a generic entry, used currently for generic HE specifies.

				if (g->variant == 0 || !scumm_stricmp(elem->variant, g->variant)) {
					// Perfect match found, use it and stop the loop
					game = *g;
					game.gameid = gameid;
					if (elem->platform != Common::kPlatformUnknown) {
						if (game.platform != Common::kPlatformUnknown && game.platform != elem->platform)
							warning("Platform values differ for MD5 '%s': %d vs %d (please report to Fingolfin)",
										md5, game.platform, elem->platform);
						game.platform = elem->platform;
					}
					break;
				}
			}
		}
	} else {
		printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5);
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

	// Finally, we have massaged the GameDescriptor to our satisfaction, and can
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
