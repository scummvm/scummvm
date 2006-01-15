/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/md5.h"
#include "common/system.h"

#include "gui/message.h"
#include "gui/newgui.h"

#include "scumm/akos.h"
#include "scumm/charset.h"
#include "scumm/costume.h"
#include "scumm/debugger.h"
#include "scumm/dialogs.h"
#include "scumm/imuse.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/insane/insane.h"
#include "scumm/intern.h"
#include "scumm/logic_he.h"
#include "scumm/player_nes.h"
#include "scumm/player_v1.h"
#include "scumm/player_v2.h"
#include "scumm/player_v2a.h"
#include "scumm/player_v3a.h"
#include "scumm/resource_v7he.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/util.h"

#ifdef PALMOS_68K
#include "extras/palm-scumm-md5.h"
#else
#include "scumm/scumm-md5.h"
#endif
#include "scumm/verbs.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#ifdef MACOSX
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef _WIN32_WCE
extern bool isSmartphone(void);
#endif

#if (defined(PALMOS_ARM) || defined(PALMOS_DEBUG))
namespace Graphics {
	extern void initfonts();
}
#endif

static int generateSubstResFileName_(const char *filename, char *buf, int bufsize, int index);

using Common::File;

namespace Scumm {

// Use g_scumm from error() ONLY
ScummEngine *g_scumm = 0;

struct ScummGameSettings {
	const char *name;
	const char *description;
	byte id, version, heversion;
	int midi; // MidiDriverType values
	uint32 features;
	Common::Platform platform;

	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description, features };
		return dummy;
	}
};


enum {
	// We only compute the MD5 of the first megabyte of our data files.
	kMD5FileSizeLimit = 1024 * 1024
};


struct ObsoleteTargets {
	const char *from;
	const char *to;
	Common::Platform platform;

	GameSettings toGameSettings() const {
		GameSettings dummy = { from, "Obsolete Target", 0 };
		return dummy;
	}
};

static const Common::Platform UNK = Common::kPlatformUnknown;

/**
 * Conversion table mapping old obsolete target names to the
 * corresponding new target and platform combination.
 *
 * We use an ugly macro 'UNK' here to make the following table more readable.
 */
static ObsoleteTargets obsoleteTargetsTable[] = {
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

	{"maniac", "Maniac Mansion", GID_MANIAC, 2, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformPC},
	{"zak",         "Zak McKracken and the Alien Mindbenders", GID_ZAK, 2, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformPC},

	/* Scumm Version 3 */
	{"indy3", "Indiana Jones and the Last Crusade", GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformPC},
	{"loom", "Loom", GID_LOOM, 3, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformPC},

	/* Scumm Version 4 */
	{"pass", "Passport to Adventure", GID_PASS, 4, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},

	/* Scumm version 5, small header -- we treat these as V4 games, since internally
	   they really are much closer to the V4 games than to all other V5 games. */
	{"monkey", "The Secret of Monkey Island", GID_MONKEY_VGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY, Common::kPlatformPC},

	/* Scumm version 5 */
	{"monkey2", "Monkey Island 2: LeChuck's Revenge", GID_MONKEY2, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC},

	{"atlantis", "Indiana Jones and the Fate of Atlantis", GID_INDY4, 5, 0, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC},

	/* Scumm Version 6 */
	{"tentacle", "Day of the Tentacle", GID_TENTACLE, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC},

	{"samnmax", "Sam & Max Hit the Road", GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC},

//	{"test", "Test demo game", GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE, GF_NEW_OPCODES, Common::kPlatformUnknown},

#ifndef DISABLE_SCUMM_7_8
	/* Scumm Version 7 */
	{"ft", "Full Throttle", GID_FT, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, Common::kPlatformPC},

	{"dig", "The Dig", GID_DIG, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, Common::kPlatformPC},

	/* Scumm Version 8 */
	{"comi", "The Curse of Monkey Island", GID_CMI, 8, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEFAULT_TO_1X_SCALER, Common::kPlatformWindows},

#endif

	// Humongous Entertainment Scumm Version 6
	{"puttputt", "Putt-Putt Joins The Parade", GID_HEGAME, 6, 61, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"puttmoon", "Putt-Putt Goes To The Moon", GID_HEGAME, 6, 61, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"funpack", "Putt-Putt's Fun Pack", GID_FUNPACK, 6, 61, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"fbpack", "Fatty Bear's Fun Pack", GID_HEGAME, 6, 61, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},
	{"fbear", "Fatty Bear's Birthday Surprise", GID_FBEAR, 6, 61, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC},

#ifndef DISABLE_HE
	{"activity", "Putt-Putt & Fatty Bear's Activity Pack", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 7.1
	// The first version to use 640x480 resolution
	// There are also 7.1 versions of freddemo, airdemo and farmdemo
	{"catalog", "Humongous Interactive Catalog", GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"freddi", "Freddi Fish 1: The Case of the Missing Kelp Seeds", GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 7.2
	{"airport", "Let's Explore the Airport with Buzzy", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"puttzoo", "Putt-Putt Saves the Zoo", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"farm", "Let's Explore the Farm with Buzzy", GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"jungle", "Let's Explore the Jungle with Buzzy", GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 8.0 ?  Scummsrc.80
	{"freddi2", "Freddi Fish 2: The Case of the Haunted Schoolhouse", GID_FREDDI2, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"pajama", "Pajama Sam 1: No Need to Hide When It's Dark Outside", GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"putttime", "Putt-Putt Travels Through Time", GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"balloon", "Putt-Putt and Pep's Balloon-O-Rama", GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"dog", "Putt-Putt and Pep's Dog on a Stick", GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"maze", "Freddi Fish and Luther's Maze Madness", GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"socks", "Pajama Sam's Sock Works", GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"water", "Freddi Fish and Luther's Water Worries", GID_WATER, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 9.0 ?  Scummsys.90
	{"baseball", "Backyard Baseball", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"thinkerk", "Big Thinkers Kindergarten", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"thinker1", "Big Thinkers First Grade", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"freddi3", "Freddi Fish 3: The Case of the Stolen Conch Shell", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"spyfox", "Spy Fox 1: Dry Cereal", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 9.5 ?  Scummsys.95
	{"pajama2", "Pajama Sam 2: Thunder and Lightning Aren't so Frightening", GID_HEGAME, 6, 95, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"chase", "Spy Fox in Cheese Chase Game", GID_HEGAME, 6, 95, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version 9.8 ?  Scummsys.98
	// these and later games can easily be identified by the .(a) file instead of a .he1
	// and INIB chunk in the .he0
	{"lost", "Pajama Sam's Lost & Found", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_CURSORLESS, Common::kPlatformWindows},
	{"puttrace", "Putt-Putt Enters the Race", GID_PUTTRACE, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"BluesABCTimeDemo", "Blue's ABC Time (Demo)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"soccer", "Backyard Soccer", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Global scripts increased to 2048
	{"freddi4", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},

	// Engine moved to c++
	// Humongous Entertainment Scumm Version 9.9 ?  Scummsys.99
	{"football", "Backyard Football", GID_FOOTBALL, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"puttcircus", "Putt-Putt Joins the Circus", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"spyfox2", "Spy Fox 2: Some Assembly Required", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"mustard", "Spy Fox in Hold the Mustard", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version ?
	{"freddicove", "Freddi Fish 5: The Case of the Creature of Coral Cave", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_HE_NOSUBTITLES | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"pajama3", "Pajama Sam 3: You Are What You Eat From Your Head to Your Feet", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"FreddisFunShop", "Freddi Fish's One-Stop Fun Shop", GID_FUNSHOP, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"SamsFunShop", "Pajama Sam's One-Stop Fun Shop", GID_FUNSHOP, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"PuttsFunShop", "Putt-Putt's One-Stop Fun Shop", GID_FUNSHOP, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows},
	{"spyozon", "Spy Fox 3: Operation Ozone", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},

	{"baseball2001", "Backyard Baseball 2001", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"SoccerMLS", "Backyard Soccer MLS Edition", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	// Humongous Entertainment Scumm Version ?
	{"basketball", "Backyard Basketball", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"pjgames", "Pajama Sam: Games to Play On Any Day", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},

	// Uses smacker in external files
	{"BluesBirthdayDemo", "Blue's Birthday Adventure (Demo)", GID_HEGAME, 6, 98, MDT_NONE,
	GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"artdemo", "Blue's Art Time Activities (Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"readdemo", "Blue's Reading Time Activities (Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},

	// Uses bink in external files for logos
	{"Baseball2003", "Backyard Baseball 2003", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"Soccer2004", "Backyard Soccer 2004", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows},

#endif
	{NULL, NULL, 0, 0, 0, MDT_NONE, 0, Common::kPlatformUnknown}
};

// This additional table is used for MD5-based search
//
// Use main table to specify default flags and this table to override defaults.
//
// Please, add new entries sorted alpabetically by string name
static const ScummGameSettings multiple_versions_md5_settings[] = {
#ifndef PALMOS_68K
	{"2e85f7aa054930c692a5b1bed1dfc295", "Backyard Football 2002 (Demo Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown}, // Football2002

	{"037385a953789190298494d92b89b3d0", "Humongous Interactive Catalog (Updated)", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"8fec68383202d38c0d25e9e3b757c5df", "The Curse of Monkey Island (Demo)", GID_CMI, 8, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEFAULT_TO_1X_SCALER | GF_DEMO, Common::kPlatformWindows},

	{"362c1d281fb9899254cda66ad246c66a", "The Dig (Demo)", GID_DIG, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformPC},
	{"cd9c05e755d7bf8e9b9590ad1ebe273e", "The Dig (Demo Mac)", GID_DIG, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformMacintosh},

	{"179879b6e35c1ead0d93aab26db0951b", "Fatty Bear's Birthday Surprise (Windows)", GID_FBEAR, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"22c9eb04455440131ffc157aeb8d40a8", "Fatty Bear's Birthday Surprise (Windows Demo)", GID_FBEAR, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"cf8ef3a1fb483c5c4b1c584d1167b2c4", "Freddi Fish 1: The Case of the Missing Kelp Seeds (Updated German)", GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"df047cc4792150f601290357566d36a6", "Freddi Fish 1: The Case of the Missing Kelp Seeds (Updated)", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"746e88c172a5b7a1ae89ac0ee3ee681a", "Freddi Fish 1: The Case of the Missing Kelp Seeds (Updated Russian)", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"507bb360688dc4180fdf0d7597352a69", "Freddi Fish 1: The Case of the Missing Kelp Seeds (Updated Swedish)", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"0855496dde35356b1a9691e22ba84cdc", "Freddi Fish 1: The Case of the Missing Kelp Seeds (Demo)", GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"566165a7338fa11029e7c14d94fa70d0", "Freddi Fish 1: The Case of the Missing Kelp Seeds (Demo)", GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"cf4ef315214c7d8cdab6302cdb7e50db", "Freddi Fish 1: The Case of the Missing Kelp Seeds (German Demo)", GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"c8aac5e3e701874e2fa4117896f9e1b1", "Freddi Fish 1: The Case of the Missing Kelp Seeds (Macintosh Demo)", GID_HEGAME, 6, 73, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"8ee63cafb1fe9d62aa0d5a23117e70e7", "Freddi Fish 2: The Case of the Haunted Schoolhouse (Updated)", GID_FREDDI2, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // FreddiCHSH
	{"51305e929e330e24a75a0351c8f9975e", "Freddi Fish 2: The Case of the Haunted Schoolhouse (Updated)", GID_FREDDI2, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"e41de1c2a15abbcdbf9977e2d7e8a340", "Freddi Fish 2: The Case of the Haunted Schoolhouse (Updated Russian)", GID_FREDDI2, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // FreddiCHSH
	{"d37c55388294b66e53e7ced3af88fa68", "Freddi Fish 2: The Case of the Haunted Schoolhouse (Demo Updated)", GID_FREDDI2, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // FFHSDemo

	{"83cedbe26aa8b58988e984e3d34cac8e", "Freddi Fish 3: The Case of the Stolen Conch Shell (Updated German)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"75bff95816b84672b877d22a911ab811", "Freddi Fish 3: The Case of the Stolen Conch Shell (Updated Russian)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"07b810e37be7489263f7bc7627d4765d", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Unencrypted Russian)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},
	{"b5298a5c15ffbe8b381d51ea4e26d35c", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Updated German)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"78bd5f036ea35a878b74e4f47941f784", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Updated Russian)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"16effd200aa6b8abe9c569c3e578814d", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Updated Dutch Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // ff4demo
	{"499c958affc394f2a3868f1eb568c3ee", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Updated Dutch Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // ff4demo
	{"ebd324dcf06a4c49e1ba5c231eee1060", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Updated Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"e03ed1474ec14de78359970e0457a820", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Updated Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"688328c5bdc4c8ec4145688dfa077bf2", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (German Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // Ff4demo

	{"b8955d7d23b4972229060d1592489fef", "Freddi Fish 5: The Case of the Creature of Coral Cave (Updated Dutch)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows}, // FreddiDZZ
	{"4ce2d5b355964bbcb5e5ce73236ef868", "Freddi Fish 5: The Case of the Creature of Coral Cave (Updated Russian)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"21abe302e1b1e2b66d6f5c12e241ebfd", "Freddi Fish 5: The Case of the Creature of Coral Cave (Unencrypted Russian)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"45082a5c9f42ba14dacfe1fdeeba819d", "Freddi Fish 5: The Case of the Creature of Coral Cave (Updated Demo)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},
	{"6b257bb2827dd894b8109a50a1a18b5a", "Freddi Fish 5: The Case of the Creature of Coral Cave (Updated Dutch Demo)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_NOSUBTITLES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows}, // FF5Demo

	{"4dbff3787aedcd96b0b325f2d92d7ad9", "Freddi Fish and Luther's Maze Madness (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"f1b0e0d587b85052de5534a3847e68fe", "Freddi Fish and Luther's Water Worries (Updated)", GID_WATER, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"9d7b67be003fea60be4dcbd193611936", "Full Throttle (Mac Demo)", GID_FT, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformMacintosh},
	{"32a433dea56b86a55b59e4ff7d755711", "Full Throttle (PC Demo)", GID_FT, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformPC},

	{"157367c3c21e0d03a0cba44361b4cf65", "Indiana Jones and the Last Crusade (AtariST)", GID_INDY3, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformAtariST},
	{"0f9c7a76657f0840b8f7ccb5bffeb9f4", "Indiana Jones and the Last Crusade (AtariST Fr)", GID_INDY3, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformAtariST},
	{"1dd7aa088e09f96d06818aa9a9deabe0", "Indiana Jones and the Last Crusade (Macintosh)", GID_INDY3, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformMacintosh},

	{"1875b90fade138c9253a8e967007031a", "Indiana Jones and the Last Crusade (VGA)", GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS, Common::kPlatformPC},
	{"399b217b0c8d65d0398076da486363a9", "Indiana Jones and the Last Crusade (VGA De)", GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS, Common::kPlatformPC},
	{"17b5d5e6af4ae89d62631641d66d5a05", "Indiana Jones and the Last Crusade (VGA It)", GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS, Common::kPlatformPC},
	{"3cce1913a3bc586b51a75c3892ff18dd", "Indiana Jones and the Last Crusade (VGA Ru)", GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS, Common::kPlatformPC},

	{"04687cdf7f975a89d2474929f7b80946", "Indiana Jones and the Last Crusade (FM-TOWNS)", GID_INDY3, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"3a0c35f3c147b98a2bdf8d400cfc4ab5", "Indiana Jones and the Last Crusade (FM-TOWNS Jp)", GID_INDY3, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS | GF_AUDIOTRACKS, Common::kPlatformFMTowns},

	{"86c9902b7bec1a17926d4dae85beaa45", "Let's Explore the Airport with Buzzy (Demo)", GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"bf8b52fdd9a69c67f34e8e9fec72661c", "Let's Explore the Farm with Buzzy (Demo)", GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"8d479e36f35e80257dfc102cf4b8a912", "Let's Explore the Farm with Buzzy (Updated Demo)", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"0557df19f046a84c2fdc63507c6616cb", "Let's Explore the Farm with Buzzy (Updated Dutch Demo)", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"a0a7dea72003933b8b3f8b99b9f7ddeb", "Loom (AtariST)", GID_LOOM, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformAtariST},
	{"c24c490373aeb48fbd54caa8e7ae376d", "Loom (AtariST De)", GID_LOOM, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformAtariST},
	{"0e9b01430e31d9fcd94071d433bbc6bf", "Loom (AtariST Fr)", GID_LOOM, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformAtariST},
	{"6f0be328c64d689bb606d22a389e1b0f", "Loom (Macintosh)", GID_LOOM, 3, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformMacintosh},

	{"5d88b9d6a88e6f8e90cded9d01b7f082", "Loom (256 color CD version)", GID_LOOM256, 4, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},
	{"c5d10e190d4b4d59114b824f2fdbd00e", "Loom (FM-TOWNS)", GID_LOOM, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"31b8fda4c8c7413fa6b39997e776eba4", "Loom (FM-TOWNS Jp)", GID_LOOM, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},

	{"d8d07efcb88f396bee0b402b10c3b1c9", "Maniac Mansion (NES E)", GID_MANIAC, 1, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES},
	{"81bbfa181184cb494e7a81dcfa94fbd9", "Maniac Mansion (NES F)", GID_MANIAC, 1, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES},
	{"22d07d6c386c9c25aca5dac2a0c0d94b", "Maniac Mansion (NES SW)", GID_MANIAC, 1, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES},
	{"3905799e081b80a61d4460b7b733c206", "Maniac Mansion (NES U)", GID_MANIAC, 1, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES},
	{"7f45ddd6dbfbf8f80c0c0efea4c295bc", "Maniac Mansion (v1)", GID_MANIAC, 1, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformPC},

	{"ef71a322b6530ac45b1a070f7c0795f7", "Moonbase Commander (Demo)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows},

	{"898eaa21f79cf8d4f08db856244689ff", "Pajama Sam: No Need To Hide When It's Dark Outside (Updated)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"37aed3f91c1ef959e0bd265f9b13781f", "Pajama Sam: No Need To Hide When It's Dark Outside (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // PajamaNHD
	{"d7ab7cd6105546016e6a0d46fb36b964", "Pajama Sam: No Need To Hide When It's Dark Outside (Updated Demo)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // PJSamDemo

	{"30ba1e825d4ad2b448143ae8df18482a", "Pajama Sam 2: Thunder and Lightning Aren't so Frightening (Updated Dutch Demo)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows}, // Pjs2demo
	{"32709cbeeb3044b34129950860a83f14", "Pajama Sam 2: Thunder and Lightning Aren't so Frightening (Updated Russian)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // PajamaTAL
	{"c6907d44f1166941d982864cd42cdc89", "Pajama Sam 2: Thunder and Lightning Aren't so Frightening (Updated German)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // PyjamaDBMN

	{"a2bb6aa0537402c1b3c2ea899ccef64b", "Pajama Sam's Lost & Found (Test)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_CURSORLESS, Common::kPlatformWindows},
	{"a561d2e2413cc1c71d5a1bf87bf493ea", "Pajama Sam's Lost & Found (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_CURSORLESS, Common::kPlatformWindows},

	{"055ffe4f47753e47594ac67823220c54", "Putt-Putt Enters the Race (German)", GID_PUTTRACE, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows}, // ToffRennen
	{"6af2419fe3db5c2fdb091ae4e5833770", "Putt-Putt Enters the Race (Dutch Demo)", GID_PUTTRACE, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows}, // 500demo
	{"aaa587701cde7e74692c68c1024b85eb", "Putt-Putt Enters the Race (Updated Dutch Demo)", GID_PUTTRACE, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},
	{"663743c03ae0c007f3d665cf631c0e6b", "Putt-Putt Enters the Race (German Demo)", GID_PUTTRACE, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows}, // Rennen
	{"7c8100e360e8ef05f88069d4cfa0afd1", "Putt-Putt Enters the Race (UK Demo)", GID_PUTTRACE, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},

	{"9c92eeaf517a31b7221ec2546ab669fd", "Putt-Putt Goes to the Moon (Windows)", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"9c143c5905055d5df7a0f014ab379aee", "Putt-Putt Goes to the Moon (Windows Demo)", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"0b3222aaa7efcf283eb621e0cefd26cc", "Putt-Putt Joins the Parade (Russian)", GID_HEGAME, 6, 60, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformWindows},
	{"31aa57f460a3d12429f0552a46a90b39", "Putt-Putt Joins the Parade (Demo)", GID_PUTTDEMO, 6, 60, MDT_ADLIB | MDT_NATIVE,
	  GF_USE_KEY, Common::kPlatformPC},
	{"f40a7f495f59188ca57a9d1d50301bb6", "Putt-Putt Joins the Parade (Macintosh Demo)", GID_PUTTDEMO, 6, 60, MDT_ADLIB | MDT_NATIVE,
	  GF_USE_KEY, Common::kPlatformPC},
	{"6a30a07f353a75cdc602db27d73e1b42", "Putt-Putt Joins the Parade (Windows)", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"37ff1b308999c4cca7319edfcc1280a0", "Putt-Putt Joins the Parade (Windows Demo)", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"92e7727e67f5cd979d8a1070e4eb8cb3", "Putt-Putt Saves the Zoo (Updated)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},

	{"2108d83dcf09f8adb4bc524669c8cf51", "Putt-Putt Travels Through Time (Updated)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"d4aac997e2f4e15341f0bfbf905419bd", "Putt-Putt Travels Through Time (Updated)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"7c980a1b1596a93f26917318884f48f7", "Putt-Putt Travels Through Time (Updated German)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"defb8cb9ec4b0f91acfb6b61c6129ad9", "Putt-Putt Travels Through Time (Updated Russian)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"a525c1753c1db5011c00417da37887ef", "Putt-Putt Travels Through Time (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"fcb78ebecab2757264c590890c319cc5", "Putt-Putt Travels Through Time (Updated Dutch)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"4e5867848ee61bc30d157e2c94eee9b4", "Putt-Putt Travels Through Time (Demo)", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"6b19d0e25cbf720d05822379b8b90ed9", "Putt-Putt Travels Through Time (Dutch Demo)", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"59d5cfcc5e672a6e07baae01328b918b", "Putt-Putt Travels Through Time (French Demo)", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // TEMPDEMO
	{"0ab19be9e2a3f6938226638b2a3744fe", "Putt-Putt Travels Through Time (Updated Demo)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"d7b247c26bf1f01f8f7daf142be84de3", "Putt-Putt and Pep's Balloon-O-Rama (Updated)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"8e3241ddd6c8dadf64305e8740d45e13", "Putt-Putt and Pep's Balloon-O-Rama (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"d4b8ee426b1afd3e53bc0cf020418cf6", "Putt-Putt and Pep's Dog on a Stick (Updated)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"1d05cd189e4908f79b57e78a4402f292", "The Secret of Monkey Island (EGA)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"49210e124e4c2b30f1290a9ef6306301", "The Secret of Monkey Island (EGA)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"e98b982ceaf9d253d730bde8903233d6", "The Secret of Monkey Island (EGA De)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"fc6b6148e80d67939d9a18697c0f626a", "The Secret of Monkey Island (EGA De)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"ce6a4cef315b20fef58a95bc40a2d8d3", "The Secret of Monkey Island (EGA Fr)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"aa7a07d94ae853f6460be4ce0a1bf530", "The Secret of Monkey Island (EGA Fr)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"1dd3c11ea4439adfe681e4e405b624e1", "The Secret of Monkey Island (EGA Fr)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"477dbafbd66a53c98416dc01aef019ad", "The Secret of Monkey Island (EGA It)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"910e31cffb28226bd68c569668a0d6b4", "The Secret of Monkey Island (EGA Sp)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"c666a998af90d81db447eccba9f72c8d", "The Secret of Monkey Island (Atari)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformAtariST},
	{"927a764615c7fcdd72f591355e089d8c", "The Secret of Monkey Island (Atari De)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformAtariST},
	{"9e5e0fb43bd22f4628719b7501adb717", "The Secret of Monkey Island (Atari Fr)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformAtariST},
	{"0a41311d462b6639fc45297b9044bf16", "The Secret of Monkey Island (Atari Sp)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformAtariST},
	{"71523b539491527d9860f4407faf0411", "The Secret of Monkey Island (Demo)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},
	{"771bc18ec6f93837b839c992b211904b", "The Secret of Monkey Island (Demo De)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC},

	{"2d1e891fe52df707c30185e52c50cd92", "The Secret of Monkey Island (CD)", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},
	{"305d3dd57c96c65b017bc70c8c7cfb5e", "The Secret of Monkey Island (CD De)", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},
	{"f049e38c1f8302b5db6170f1872af89a", "The Secret of Monkey Island (CD Sp)", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},
	{"da6269b18fcb08189c0aa9c95533cce2", "The Secret of Monkey Island (CD It)", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},
	{"aa8a0cb65f3afbbe2c14c3f9f92775a3", "The Secret of Monkey Island (CD Fr)", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},
	{"2ccd8891ce4d3f1a334d21bff6a88ca2", "The Secret of Monkey Island (Mac CD)", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformMacintosh},

	{"8eb84cee9b429314c7f0bdcf560723eb", "The Secret of Monkey Island (FM-TOWNS)", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformFMTowns},

	{"e17db1ddf91b39ca6bbc8ad3ed19e883", "The Secret of Monkey Island (FM-TOWNS Jp)", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformFMTowns},

	{"c13225cb1bbd3bc9fe578301696d8021", "The Secret of Monkey Island (SegaCD)", GID_MONKEY_SEGA, 5, 0, MDT_NONE,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC},

	{"3de99ef0523f8ca7958faa3afccd035a", "Spy Fox 1: Dry Cereal (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"58436e634f4fae1d9973591c2ffa1fcb", "Spy Fox 1: Dry Cereal (Updated)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"9bda5fee51d2fda5253d02c642016bf4", "Spy Fox 1: Dry Cereal (Updated Dutch)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},
	{"a28135a7ade38cc0208b04507c46efd1", "Spy Fox 1: Dry Cereal (Updated German)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"72ac6bc980d5101c2142189d746bd62f", "Spy Fox 1: Dry Cereal (Updated Russian)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows}, // SPYFoxDC
	{"9d4ab3e0e1d1ebc6ba8a6a4c470ed184", "Spy Fox 1: Dry Cereal (Updated Demo)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},
	{"4edbf9d03550f7ba01e7f34d69b678dd", "Spy Fox 1: Dry Cereal (Updated Dutch Demo)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformWindows},

	{"90c755e1c9b9b8a4129d37b2259d0655", "Spy Fox in Cheese Chase (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows},

	{"b23f7cd7c304d7dff08e92a96120d5b4", "Zak McKracken and the Alien Mindbenders (v1)", GID_ZAK, 1, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformPC},
	{"7020931d5a2be0a49d68e7a1882363e4", "Zak McKracken and the Alien Mindbenders (v1)", GID_ZAK, 1, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformPC},

	{"2d4536a56e01da4b02eb021e7770afa2", "Zak McKracken and the Alien Mindbenders (FM-TOWNS)", GID_ZAK256, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"ce3edc99cd4f478c5b37104d70c68ca5", "Zak McKracken and the Alien Mindbenders (FM-TOWNS Jp)", GID_ZAK256, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"1ca86e2cf9aaa2068738a1e5ba477e60", "Zak McKracken and the Alien Mindbenders (FM-TOWNS Jp)", GID_ZAK256, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},

	{"2d388339d6050d8ccaa757b64633954e", "Indy/Loom Demo (FM-TOWNS)", GID_ZAK256, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"77f5c9cc0986eb729c1a6b4c8823bbae", "Zak/Loom Demo (FM-TOWNS)", GID_ZAK256, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
	{"3938ee1aa4433fca9d9308c9891172b1", "Indy/Zak Demo (FM-TOWNS)", GID_ZAK256, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns},
#endif
	{NULL, NULL, 0, 0, MDT_NONE, 0, 0, Common::kPlatformUnknown}
};

enum genMethods {
	kGenMac,
	kGenMacNoParens,
	kGenPC,
	kGenAsIs
};

struct SubstResFileNames {
	const char *winName;
	const char *macName;
	int genMethod;
};

static SubstResFileNames substResFileNameTable[] = {
	{ "Intentionally/left/blank", "", kGenMacNoParens },
	{ "00.LFL", "Maniac Mansion (E).prg", kGenAsIs },
	{ "00.LFL", "Maniac Mansion (F).prg", kGenAsIs },
	{ "00.LFL", "Maniac Mansion (SW).prg", kGenAsIs },
	{ "00.LFL", "Maniac Mansion (U).prg", kGenAsIs },
	{ "00.LFL", "maniac1.d64", kGenAsIs }, // Do not
	{ "01.LFL", "maniac2.d64", kGenAsIs }, // swap
	{ "00.LFL", "zak1.d64", kGenAsIs },    // these
	{ "01.LFL", "zak2.d64", kGenAsIs },    // lines
	{ "atlantis.000", "Fate of Atlantis Data", kGenAsIs },
	{ "atlantis", "fate", kGenPC },
	{ "atlantis", "playfate", kGenPC },
	{ "atlantis", "indy4", kGenPC },
	{ "atlantis", "indydemo", kGenPC },
	{ "tentacle.000", "Day Of The Tentacle Data", kGenAsIs },
	{ "tentacle", "dottdemo", kGenPC },
	{ "tentacle.000", "Day of the Tentacle Demo Data", kGenAsIs },
	{ "monkey", "monkey1", kGenPC },
	{ "monkey", "monkeyk", kGenPC }, // FM-TOWNS Jap
	{ "monkey", "game", kGenPC }, // SegaCD
	{ "monkey2", "mi2demo", kGenPC },
	{ "samnmax.000", "Sam & Max Data", kGenAsIs },
	{ "samnmax.000", "Sam & Max Demo Data", kGenAsIs },
	{ "samnmax", "ramnmax", kGenPC }, // Used in some releases of Russian Sam'n'Max
	{ "samnmax", "samdemo", kGenPC },
	{ "samnmax", "snmdemo", kGenPC },
	{ "samnmax", "snmidemo", kGenPC },
	{ "samnmax", "sdemo", kGenPC },
#ifndef DISABLE_SCUMM_7_8
	{ "dig.la0", "The Dig Data", kGenAsIs },
	{ "dig.la0", "The Dig Demo Data", kGenAsIs },
	{ "ft.la0", "Full Throttle Data", kGenAsIs },
	{ "ft.la0", "Full Throttle Demo Data", kGenAsIs },
	{ "ft.la0", "Vollgas Data", kGenAsIs },
	{ "ft.la0", "Vollgas Demo Data", kGenAsIs },
	{ "ft", "ftdemo", kGenPC },
#endif
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
	{ "BluesABCTimeDemo", "BluesABCTimeDemo", kGenMac },
	{ "catalog", "catalog2", kGenPC },
	{ "chase", "Cheese Chase", kGenMac },
	{ "dog", "Dog on a Stick", kGenMac },
	{ "farm", "farmdemo", kGenPC },
	{ "farm", "Farm Demo", kGenMac },
	{ "fbear", "fbdemo", kGenPC },
	{ "fbear", "Fatty Bear Demo", kGenMacNoParens },
	{ "fbear", "Fatty Bear", kGenMacNoParens },
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
	{ "pjgames", "PJGames", kGenMac },
	{ "puttcircus", "circdemo", kGenPC },
	{ "puttcircus", "Putt Circus Demo", kGenMac },
	{ "puttcircus", "Putt Circus", kGenMac },
	{ "puttmoon", "moondemo", kGenPC },
	{ "puttmoon", "Putt-Putt Moon Demo", kGenMacNoParens },
	{ "puttmoon", "Putt-Putt Moon", kGenMacNoParens },
	{ "puttputt", "puttdemo", kGenPC },
	{ "puttputt", "Putt-Putt's Demo", kGenMacNoParens },
	{ "puttputt", "Putt-Putt Parade", kGenMacNoParens },
	{ "puttrace", "500demo", kGenPC },
	{ "puttrace", "racedemo", kGenPC },
	{ "puttrace", "RaceDemo", kGenMac },
	{ "puttrace", "Rennen", kGenPC },
	{ "puttrace", "Putt500 demo", kGenMac }, // NL Mac demo
	{ "puttrace", "Putt Race", kGenMac },
	{ "puttrace", "ToffRennen", kGenPC },
	{ "puttrace", "ToffRennen", kGenMac },
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
	{ "putttime", "toffzeit", kGenPC }, // German T’¨¢öff-T’¨¢öff: Reist durch die Zeit
	{ "putttime", "toffzeit", kGenMac }, // German T’¨¢öff-T’¨¢öff: Reist durch die Zeit
	{ "putttime", "ZeitDemo", kGenMac },
	{ "putttime", "ZEITDEMO", kGenPC },
	{ "puttzoo", "Puttzoo Demo", kGenMac },
	{ "puttzoo", "PuttZoo", kGenMac },
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
	{ NULL, NULL, 0 }
};

static int compareMD5Table(const void *a, const void *b) {
	const char *key = (const char *)a;
	const MD5Table *elem = (const MD5Table *)b;
	return strcmp(key, elem->md5);
}

ScummEngine::ScummEngine(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: Engine(syst),
	  _gameId(gs.id),
	  _version(gs.version),
	  _heversion(gs.heversion),
	  _features(gs.features),
	  _platform(gs.platform),
	  _substResFileNameIndex(substResFileNameIndex),
	  _substResFileNameIndexBundle(0),
	  _debugger(0),
	  _currentScript(0xFF), // Let debug() work on init stage
	  gdi(this),
	  res(this),
	  _pauseDialog(0), _mainMenuDialog(0), _versionDialog(0),
	  _targetName(detector->_targetName) {

	// Copy MD5 checksum
	memcpy(_gameMD5, md5sum, 16);

	// Check for unknown MD5
	char md5str[32+1];
	for (int j = 0; j < 16; j++) {
		sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
	}
	const MD5Table *elem;
#ifdef PALMOS_68K
	uint32 arraySize = MemPtrSize((void *)md5table) / sizeof(MD5Table) - 1;
#else
	uint32 arraySize = ARRAYSIZE(md5table) - 1;
#endif
	elem = (const MD5Table *)bsearch(md5str, md5table, arraySize, sizeof(MD5Table), compareMD5Table);
	if (!elem)
		printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);

	// Add default file directories.
	if (((_platform == Common::kPlatformAmiga) || (_platform == Common::kPlatformAtariST)) && (_version <= 4)) {
		// This is for the Amiga version of Indy3/Loom/Maniac/Zak
		File::addDefaultDirectory(_gameDataPath + "ROOMS/");
		File::addDefaultDirectory(_gameDataPath + "rooms/");
	}

	if ((_platform == Common::kPlatformMacintosh) && (_version == 3)) {
		// This is for the Mac version of Indy3/Loom
		File::addDefaultDirectory(_gameDataPath + "Rooms 1/");
		File::addDefaultDirectory(_gameDataPath + "Rooms 2/");
		File::addDefaultDirectory(_gameDataPath + "Rooms 3/");
	}

#ifndef DISABLE_SCUMM_7_8
#ifdef MACOSX
	if (_version == 8 && !memcmp(_gameDataPath.c_str(), "/Volumes/MONKEY3_", 17)) {
		// Special case for COMI on Mac OS X. The mount points on OS X depend
		// on the volume name. Hence if playing from CD, we'd get a problem.
		// So if loading of a resource file fails, we fall back to the (fixed)
		// CD mount points (/Volumes/MONKEY3_1 and /Volumes/MONKEY3_2).
		//
		// This check for whether we play from CD is very crude, though.

		File::addDefaultDirectory("/Volumes/MONKEY3_1/RESOURCE/");
		File::addDefaultDirectory("/Volumes/MONKEY3_1/resource/");
		File::addDefaultDirectory("/Volumes/MONKEY3_2/");
		File::addDefaultDirectory("/Volumes/MONKEY3_2/RESOURCE/");
		File::addDefaultDirectory("/Volumes/MONKEY3_2/resource/");
	} else
#endif
	if (_version == 8) {
		// This is for COMI
		File::addDefaultDirectory(_gameDataPath + "RESOURCE/");
		File::addDefaultDirectory(_gameDataPath + "resource/");
	}

	if (_version == 7) {
		// This is for Full Throttle & The Dig
		File::addDefaultDirectory(_gameDataPath + "VIDEO/");
		File::addDefaultDirectory(_gameDataPath + "video/");
		File::addDefaultDirectory(_gameDataPath + "DATA/");
		File::addDefaultDirectory(_gameDataPath + "data/");
	}
#endif

	// We read data directly from NES ROM instead of extracting it with
	// external tool
	if ((_platform == Common::kPlatformNES) && _substResFileNameIndex) {
		char tmpBuf[128];
		generateSubstResFileName("00.LFL", tmpBuf, sizeof(tmpBuf));
		_fileHandle = new ScummNESFile();
		_containerFile = tmpBuf;
	} else if ((_platform == Common::kPlatformC64) && _substResFileNameIndex) {
		char tmpBuf1[128], tmpBuf2[128];
		generateSubstResFileName("00.LFL", tmpBuf1, sizeof(tmpBuf1));
		generateSubstResFileName("01.LFL", tmpBuf2, sizeof(tmpBuf2));

		_fileHandle = new ScummC64File(tmpBuf1, tmpBuf2, _gameId == GID_MANIAC);

		_containerFile = tmpBuf1;
	} else
		_fileHandle = new ScummFile();

	// The mac versions of Indy4, Sam&Max, DOTT, FT and The Dig used a
	// special meta (container) file format to store the actual SCUMM data
	// files. The rescumm utility used to be used to extract those files. 
	// While that is still possible, we now support reading those files 
	// directly. The first step is to check whether one of them is present
	// (we do that here); the rest is handled by the  ScummFile class and 
	// code in openResourceFile() (and in the Sound class, for MONSTER.SOU
	// handling).
	if (_version >= 5 && _heversion == 0 && _substResFileNameIndex &&
		_platform == Common::kPlatformMacintosh && 
		substResFileNameTable[_substResFileNameIndex].genMethod == kGenAsIs) {
		if (_fileHandle->open(substResFileNameTable[_substResFileNameIndex].macName)) {
			_containerFile = substResFileNameTable[_substResFileNameIndex].macName;
			_substResFileNameIndex = 0;
		}
	}

	// Init all vars
	_imuse = NULL;
	_imuseDigital = NULL;
	_musicEngine = NULL;
	_verbs = NULL;
	_objs = NULL;
	_storedFlObjects = NULL;
	_debugFlags = 0;
	_sound = NULL;
	memset(&vm, 0, sizeof(vm));
	_smushVideoShouldFinish = false;
	_smushPaused = false;
	_insaneRunning = false;
	_quit = false;
	_pauseDialog = NULL;
	_mainMenuDialog = NULL;
	_versionDialog = NULL;
	_fastMode = 0;
	_actors = NULL;
	_arraySlot = NULL;
	_inventory = NULL;
	_newNames = NULL;
	_scummVars = NULL;
	_roomVars = NULL;
	_varwatch = 0;
	_bitVars = NULL;
	_numVariables = 0;
	_numBitVariables = 0;
	_numRoomVariables = 0;
	_numLocalObjects = 0;
	_numGlobalObjects = 0;
	_numStoredFlObjects = 0;
	_numArray = 0;
	_numVerbs = 0;
	_numFlObject = 0;
	_numInventory = 0;
	_numRooms = 0;
	_numScripts = 0;
	_numSounds = 0;
	_numCharsets = 0;
	_numNewNames = 0;
	_numGlobalScripts = 0;
	_numCostumes = 0;
	_numImages = 0;
	_numLocalScripts = 60;
	_numSprites = 0;
	_numTalkies = 0;
	_numPalettes = 0;
	_numUnk = 0;
	_curActor = 0;
	_curVerb = 0;
	_curVerbSlot = 0;
	_curPalIndex = 0;
	_currentRoom = 0;
	_egoPositioned = false;
	_keyPressed = 0;
	_lastKeyHit = 0;
	_mouseAndKeyboardStat = 0;
	_leftBtnPressed = 0;
	_rightBtnPressed = 0;
	_bootParam = 0;
	_dumpScripts = false;
	_debugMode = 0;
	_heV7DiskOffsets = NULL;
	_heV7RoomIntOffsets = NULL;
	_objectOwnerTable = NULL;
	_objectRoomTable = NULL;
	_objectStateTable = NULL;
	_numObjectsInRoom = 0;
	_userPut = 0;
	_userState = 0;
	_activeObject = 0;
	_resourceHeaderSize = 8;
	_saveLoadFlag = 0;
	_saveLoadSlot = 0;
	_lastSaveTime = 0;
	_saveTemporaryState = false;
	memset(_saveLoadName, 0, sizeof(_saveLoadName));
	memset(_localScriptOffsets, 0, sizeof(_localScriptOffsets));
	_scriptPointer = NULL;
	_scriptOrgPointer = NULL;
	_opcode = 0;
	vm.numNestedScripts = 0;
	_curExecScript = 0;
	_lastCodePtr = NULL;
	_resultVarNumber = 0;
	_scummStackPos = 0;
	memset(_vmStack, 0, sizeof(_vmStack));
	_keyScriptKey = 0;
	_keyScriptNo = 0;
	_fileOffset = 0;
	memset(_resourceMapper, 0, sizeof(_resourceMapper));
	_lastLoadedRoom = 0;
	_roomResource = 0;
	OF_OWNER_ROOM = 0;
	_verbMouseOver = 0;
	_inventoryOffset = 0;
	_classData = NULL;
	_actorToPrintStrFor = 0;
	_sentenceNum = 0;
	memset(_sentence, 0, sizeof(_sentence));
	memset(_string, 0, sizeof(_string));
	_screenB = 0;
	_screenH = 0;
	_roomHeight = 0;
	_roomWidth = 0;
	_screenHeight = 0;
	_screenWidth = 0;
	memset(virtscr, 0, sizeof(virtscr));
	memset(&camera, 0, sizeof(CameraData));
	memset(_colorCycle, 0, sizeof(_colorCycle));
	_ENCD_offs = 0;
	_EXCD_offs = 0;
	_CLUT_offs = 0;
	_EPAL_offs = 0;
	_IM00_offs = 0;
	_PALS_offs = 0;
	_fullRedraw = false;
	_bgNeedsRedraw = false;
	_screenEffectFlag = false;
	_completeScreenRedraw = false;
	memset(&_cursor, 0, sizeof(_cursor));
	memset(_grabbedCursor, 0, sizeof(_grabbedCursor));
	_currentCursor = 0;
	_newEffect = 0;
	_switchRoomEffect2 = 0;
	_switchRoomEffect = 0;
	_scrollBuffer = NULL;

	_doEffect = false;
	memset(&_flashlight, 0, sizeof(_flashlight));
	_bompActorPalettePtr = NULL;
	_shakeEnabled = false;
	_shakeFrame = 0;
	_screenStartStrip = 0;
	_screenEndStrip = 0;
	_screenTop = 0;
	_drawObjectQueNr = 0;
	memset(_drawObjectQue, 0, sizeof(_drawObjectQue));
	_palManipStart = 0;
	_palManipEnd = 0;
	_palManipCounter = 0;
	_palManipPalette = NULL;
	_palManipIntermediatePal = NULL;
	memset(gfxUsageBits, 0, sizeof(gfxUsageBits));
	_hePalettes = NULL;
	_shadowPalette = NULL;
	_shadowPaletteSize = 0;
	memset(_currentPalette, 0, sizeof(_currentPalette));
	memset(_darkenPalette, 0, sizeof(_darkenPalette));
	memset(_HEV7ActorPalette, 0, sizeof(_HEV7ActorPalette));
	_palDirtyMin = 0;
	_palDirtyMax = 0;
	_haveMsg = 0;
	_useTalkAnims = false;
	_defaultTalkDelay = 0;
	_midiDriver = MD_NULL;
	_tempMusic = 0;
	_saveSound = 0;
	memset(_extraBoxFlags, 0, sizeof(_extraBoxFlags));
	memset(_scaleSlots, 0, sizeof(_scaleSlots));
	_charset = NULL;
	_charsetColor = 0;
	memset(_charsetColorMap, 0, sizeof(_charsetColorMap));
	memset(_charsetData, 0, sizeof(_charsetData));
	_charsetBufPos = 0;
	memset(_charsetBuffer, 0, sizeof(_charsetBuffer));
	_copyProtection = false;
	_demoMode = false;
	_confirmExit = false;
	_voiceMode = 0;
	_talkDelay = 0;
	_NES_lastTalkingActor = 0;
	_NES_talkColor = 0;
	_keepText = false;
	_costumeLoader = NULL;
	_costumeRenderer = NULL;
	_2byteFontPtr = 0;
	_V1TalkingActor = 0;
	_NESStartStrip = 0;

	_actorClipOverride.top = 0;
	_actorClipOverride.bottom = 480;
	_actorClipOverride.left = 0;
	_actorClipOverride.right = 640;

	_skipDrawObject = 0;
	memset(_heTimers, 0, sizeof(_heTimers));

	memset(_akosQueue, 0, sizeof(_akosQueue));
	_akosQueuePos = 0;

	//
	// Init all VARS to 0xFF
	//
	VAR_LANGUAGE = 0xFF;
	VAR_KEYPRESS = 0xFF;
	VAR_SYNC = 0xFF;
	VAR_EGO = 0xFF;
	VAR_CAMERA_POS_X = 0xFF;
	VAR_HAVE_MSG = 0xFF;
	VAR_ROOM = 0xFF;
	VAR_OVERRIDE = 0xFF;
	VAR_MACHINE_SPEED = 0xFF;
	VAR_ME = 0xFF;
	VAR_NUM_ACTOR = 0xFF;
	VAR_CURRENT_LIGHTS = 0xFF;
	VAR_CURRENTDRIVE = 0xFF;	// How about merging this with VAR_CURRENTDISK?
	VAR_CURRENTDISK = 0xFF;
	VAR_TMR_1 = 0xFF;
	VAR_TMR_2 = 0xFF;
	VAR_TMR_3 = 0xFF;
	VAR_MUSIC_TIMER = 0xFF;
	VAR_ACTOR_RANGE_MIN = 0xFF;
	VAR_ACTOR_RANGE_MAX = 0xFF;
	VAR_CAMERA_MIN_X = 0xFF;
	VAR_CAMERA_MAX_X = 0xFF;
	VAR_TIMER_NEXT = 0xFF;
	VAR_VIRT_MOUSE_X = 0xFF;
	VAR_VIRT_MOUSE_Y = 0xFF;
	VAR_ROOM_RESOURCE = 0xFF;
	VAR_LAST_SOUND = 0xFF;
	VAR_CUTSCENEEXIT_KEY = 0xFF;
	VAR_OPTIONS_KEY = 0xFF;
	VAR_TALK_ACTOR = 0xFF;
	VAR_CAMERA_FAST_X = 0xFF;
	VAR_SCROLL_SCRIPT = 0xFF;
	VAR_ENTRY_SCRIPT = 0xFF;
	VAR_ENTRY_SCRIPT2 = 0xFF;
	VAR_EXIT_SCRIPT = 0xFF;
	VAR_EXIT_SCRIPT2 = 0xFF;
	VAR_VERB_SCRIPT = 0xFF;
	VAR_SENTENCE_SCRIPT = 0xFF;
	VAR_INVENTORY_SCRIPT = 0xFF;
	VAR_CUTSCENE_START_SCRIPT = 0xFF;
	VAR_CUTSCENE_END_SCRIPT = 0xFF;
	VAR_CHARINC = 0xFF;
	VAR_CHARCOUNT = 0xFF;
	VAR_WALKTO_OBJ = 0xFF;
	VAR_DEBUGMODE = 0xFF;
	VAR_HEAPSPACE = 0xFF;
	VAR_RESTART_KEY = 0xFF;
	VAR_PAUSE_KEY = 0xFF;
	VAR_MOUSE_X = 0xFF;
	VAR_MOUSE_Y = 0xFF;
	VAR_TIMER = 0xFF;
	VAR_TMR_4 = 0xFF;
	VAR_SOUNDCARD = 0xFF;
	VAR_VIDEOMODE = 0xFF;
	VAR_MAINMENU_KEY = 0xFF;
	VAR_FIXEDDISK = 0xFF;
	VAR_CURSORSTATE = 0xFF;
	VAR_USERPUT = 0xFF;
	VAR_SOUNDRESULT = 0xFF;
	VAR_TALKSTOP_KEY = 0xFF;
	VAR_FADE_DELAY = 0xFF;
	VAR_NOSUBTITLES = 0xFF;

	VAR_SOUNDPARAM = 0xFF;
	VAR_SOUNDPARAM2 = 0xFF;
	VAR_SOUNDPARAM3 = 0xFF;
	VAR_MOUSEPRESENT = 0xFF;
	VAR_MEMORY_PERFORMANCE = 0xFF;
	VAR_VIDEO_PERFORMANCE = 0xFF;
	VAR_ROOM_FLAG = 0xFF;
	VAR_GAME_LOADED = 0xFF;
	VAR_NEW_ROOM = 0xFF;
	VAR_VERSION_KEY = 0xFF;

	VAR_V5_TALK_STRING_Y = 0xFF;

	VAR_ROOM_WIDTH = 0xFF;
	VAR_ROOM_HEIGHT = 0xFF;
	VAR_SUBTITLES = 0xFF;
	VAR_V6_EMSSPACE = 0xFF;

	VAR_CAMERA_POS_Y = 0xFF;
	VAR_CAMERA_MIN_Y = 0xFF;
	VAR_CAMERA_MAX_Y = 0xFF;
	VAR_CAMERA_THRESHOLD_X = 0xFF;
	VAR_CAMERA_THRESHOLD_Y = 0xFF;
	VAR_CAMERA_SPEED_X = 0xFF;
	VAR_CAMERA_SPEED_Y = 0xFF;
	VAR_CAMERA_ACCEL_X = 0xFF;
	VAR_CAMERA_ACCEL_Y = 0xFF;
	VAR_CAMERA_DEST_X = 0xFF;
	VAR_CAMERA_DEST_Y = 0xFF;
	VAR_CAMERA_FOLLOWED_ACTOR = 0xFF;

	VAR_LEFTBTN_DOWN = 0xFF;
	VAR_RIGHTBTN_DOWN = 0xFF;
	VAR_LEFTBTN_HOLD = 0xFF;
	VAR_RIGHTBTN_HOLD = 0xFF;

	VAR_SAVELOAD_SCRIPT = 0xFF;
	VAR_SAVELOAD_SCRIPT2 = 0xFF;

	VAR_DEFAULT_TALK_DELAY = 0xFF;
	VAR_CHARSET_MASK = 0xFF;

	VAR_CUSTOMSCALETABLE = 0xFF;
	VAR_V6_SOUNDMODE = 0xFF;

	VAR_ACTIVE_VERB = 0xFF;
	VAR_ACTIVE_OBJECT1 = 0xFF;
	VAR_ACTIVE_OBJECT2 = 0xFF;
	VAR_VERB_ALLOWED = 0xFF;
	VAR_CLICK_AREA = 0xFF;

	VAR_BLAST_ABOVE_TEXT = 0xFF;
	VAR_VOICE_MODE = 0xFF;
	VAR_MUSIC_BUNDLE_LOADED = 0xFF;
	VAR_VOICE_BUNDLE_LOADED = 0xFF;

	VAR_REDRAW_ALL_ACTORS = 0xFF;
	VAR_SKIP_RESET_TALK_ACTOR = 0xFF;

	VAR_MUSIC_CHANNEL = 0xFF;
	VAR_SOUND_CHANNEL = 0xFF;
	VAR_SOUNDCODE_TMR = 0xFF;
	VAR_DEFAULT_SOUND_CHANNEL = 0xFF;

	VAR_MAIN_SCRIPT = 0xFF;

	VAR_NUM_SCRIPT_CYCLES = 0xFF;
	VAR_SCRIPT_CYCLE = 0xFF;

	VAR_NUM_GLOBAL_OBJS = 0xFF;
	VAR_KEY_STATE = 0xFF;
	VAR_MOUSE_STATE = 0xFF;

	// Use g_scumm from error() ONLY
	g_scumm = this;

	// Read settings from the detector & config manager
	_debugMode = (gDebugLevel >= 0);
	_dumpScripts = detector->_dumpScripts;
	_bootParam = ConfMan.getInt("boot_param");
	// Boot params often need debugging switched on to work
	if (_bootParam)
		_debugMode = true;

	// Allow the user to override the game name with a custom string.
	// This allows some game versions to work which use filenames
	// differing from the regular version(s) of that game.
	_gameName = ConfMan.hasKey("basename") ? ConfMan.get("basename") : gs.name;

	_copyProtection = ConfMan.getBool("copy_protection");
	_demoMode = ConfMan.getBool("demo_mode");
	if (ConfMan.hasKey("nosubtitles")) {
		printf("Configuration key 'nosubtitles' is deprecated. Use 'subtitles' instead\n");
		if (!ConfMan.hasKey("subtitles"))
			ConfMan.set("subtitles", !ConfMan.getBool("nosubtitles"));
	}

	// Make sure that at least subtitles are enabled
	if (ConfMan.getBool("speech_mute") && !ConfMan.getBool("subtitles"))
		ConfMan.set("subtitles", 1);

	// TODO Detect subtitle only versions of scumm6 games
	if (ConfMan.getBool("speech_mute"))
		_voiceMode = 2;
	else
		_voiceMode = ConfMan.getBool("subtitles");

	_confirmExit = ConfMan.getBool("confirm_exit");

	if (ConfMan.hasKey("render_mode")) {
		_renderMode = Common::parseRenderMode(ConfMan.get("render_mode").c_str());
	} else {
		_renderMode = Common::kRenderDefault;
	}

	// Do some render mode restirctions
	if (_version == 1)
		_renderMode = Common::kRenderDefault;

	switch (_renderMode) {
	case Common::kRenderHercA:
	case Common::kRenderHercG:
		if (_version > 2 && _gameId != GID_MONKEY_EGA)
			_renderMode = Common::kRenderDefault;
		break;

	case Common::kRenderCGA:
		if (!(_features & GF_16COLOR))
			_renderMode = Common::kRenderDefault;
		break;

	case Common::kRenderEGA:
		if (!(_features & GF_16COLOR))
			_renderMode = Common::kRenderDefault;
		break;

	case Common::kRenderAmiga:
		if (!(_features & GF_16COLOR))
			_renderMode = Common::kRenderDefault;
		break;

	default:
		break;
	}

	_hexdumpScripts = false;
	_showStack = false;

	if (_platform == Common::kPlatformFMTowns && _version == 3) {	// FM-TOWNS V3 games use 320x240
		_screenWidth = 320;
		_screenHeight = 240;
	} else if (_features & GF_DEFAULT_TO_1X_SCALER) {
		_screenWidth = 640;
		_screenHeight = 480;
	} else if (_platform == Common::kPlatformNES) {
		_screenWidth = 256;
		_screenHeight = 240;
	} else if (_renderMode == Common::kRenderHercA || _renderMode == Common::kRenderHercG) {
		_features |= GF_DEFAULT_TO_1X_SCALER;
		_screenWidth = 320;
		_screenHeight = 200;
	} else {
		_screenWidth = 320;
		_screenHeight = 200;
	}

	_compositeBuf = (byte *)malloc(_screenWidth * _screenHeight);

	_herculesBuf = 0;
	if (_renderMode == Common::kRenderHercA || _renderMode == Common::kRenderHercG) {
		_herculesBuf = (byte *)malloc(Common::kHercW * Common::kHercH);
	}

	_midi = gs.midi;
}

ScummEngine::~ScummEngine() {
	if (_musicEngine) {
		_musicEngine->terminate();
		delete _musicEngine;
	}

	_mixer->stopAll();

	delete [] _actors;
	delete [] _sortedActors;

	delete _2byteFontPtr;
	delete _charset;
	delete _pauseDialog;
	delete _mainMenuDialog;
	delete _versionDialog;
	delete _fileHandle;

	delete _sound;

	delete _costumeLoader;
	delete _costumeRenderer;

	free(_shadowPalette);

	free(_palManipPalette);
	free(_palManipIntermediatePal);

	res.freeResources();

	free(_objectStateTable);
	free(_objectRoomTable);
	free(_objectOwnerTable);
	free(_inventory);
	free(_verbs);
	free(_objs);
	free(_roomVars);
	free(_scummVars);
	free(_bitVars);
	free(_newNames);
	free(_classData);
	free(_arraySlot);

	free(_compositeBuf);
	free(_herculesBuf);

	delete _debugger;
}

ScummEngine_v4::ScummEngine_v4(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: ScummEngine_v5(detector, syst, gs, md5sum, substResFileNameIndex) {
	_resourceHeaderSize = 6;
}

ScummEngine_v3::ScummEngine_v3(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: ScummEngine_v4(detector, syst, gs, md5sum, substResFileNameIndex) {
}

ScummEngine_v3old::ScummEngine_v3old(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: ScummEngine_v3(detector, syst, gs, md5sum, substResFileNameIndex) {
	_resourceHeaderSize = 4;
}

ScummEngine_v2::ScummEngine_v2(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: ScummEngine_v3old(detector, syst, gs, md5sum, substResFileNameIndex) {
}

ScummEngine_c64::ScummEngine_c64(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: ScummEngine_v2(detector, syst, gs, md5sum, substResFileNameIndex) {

	_currentAction = 0;
	_currentMode = 0;
}

ScummEngine_v6::ScummEngine_v6(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: ScummEngine(detector, syst, gs, md5sum, substResFileNameIndex) {
	_blastObjectQueuePos = 0;
	memset(_blastObjectQueue, 0, sizeof(_blastObjectQueue));
	_blastTextQueuePos = 0;
	memset(_blastTextQueue, 0, sizeof(_blastTextQueue));

	_smushFrameRate = 0;

	VAR_VIDEONAME = 0xFF;
	VAR_RANDOM_NR = 0xFF;
	VAR_STRING2DRAW = 0xFF;

	VAR_TIMEDATE_YEAR = 0xFF;
	VAR_TIMEDATE_MONTH = 0xFF;
	VAR_TIMEDATE_DAY = 0xFF;
	VAR_TIMEDATE_HOUR = 0xFF;
	VAR_TIMEDATE_MINUTE = 0xFF;
	VAR_TIMEDATE_SECOND = 0xFF;
}

#ifndef DISABLE_HE
ScummEngine_v70he::ScummEngine_v70he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: ScummEngine_v60he(detector, syst, gs, md5sum, substResFileNameIndex) {
	if (_platform == Common::kPlatformMacintosh && (_heversion >= 72 && _heversion <= 73))
		_resExtractor = new MacResExtractor(this);
	else
		_resExtractor = new Win32ResExtractor(this);

	_wiz = new Wiz(this);

	_heV7RoomOffsets = NULL;

	_heSndSoundId = 0;
	_heSndOffset = 0;
	_heSndChannel = 0;
	_heSndFlags = 0;
	_heSndSoundFreq = 0;

	_skipProcessActors = 0;

	VAR_NUM_SOUND_CHANNELS = 0xFF;
	VAR_WIZ_TCOLOR = 0xFF;
}

ScummEngine_v70he::~ScummEngine_v70he() {
	delete _resExtractor;
	delete _wiz;
	free(_heV7DiskOffsets);
	free(_heV7RoomIntOffsets);
	free(_heV7RoomOffsets);
}

ScummEngine_v71he::ScummEngine_v71he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: ScummEngine_v70he(detector, syst, gs, md5sum, substResFileNameIndex) {
	_auxBlocksNum = 0;
	memset(_auxBlocks, 0, sizeof(_auxBlocks));
	_auxEntriesNum = 0;
	memset(_auxEntries, 0, sizeof(_auxEntries));
}

ScummEngine_v72he::ScummEngine_v72he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: ScummEngine_v71he(detector, syst, gs, md5sum, substResFileNameIndex) {
	VAR_NUM_ROOMS = 0xFF;
	VAR_NUM_SCRIPTS = 0xFF;
	VAR_NUM_SOUNDS = 0xFF;
	VAR_NUM_COSTUMES = 0xFF;
	VAR_NUM_IMAGES = 0xFF;
	VAR_NUM_CHARSETS = 0xFF;
	VAR_NUM_SPRITE_GROUPS = 0xFF;
	VAR_NUM_SPRITES = 0xFF;
	VAR_NUM_PALETTES = 0xFF;
	VAR_NUM_UNK = 0xFF;
	VAR_POLYGONS_ONLY = 0xFF;
	VAR_PLATFORM = 0xFF;
	VAR_WINDOWS_VERSION = 0xFF;
	VAR_CURRENT_CHARSET = 0xFF;
	VAR_U32_VERSION = 0xFF;
}

ScummEngine_v80he::ScummEngine_v80he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: ScummEngine_v72he(detector, syst, gs, md5sum, substResFileNameIndex) {
	_heSBNGId = 0;
}

ScummEngine_v90he::ScummEngine_v90he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: ScummEngine_v80he(detector, syst, gs, md5sum, substResFileNameIndex) {
	_sprite = new Sprite(this);
}

ScummEngine_v90he::~ScummEngine_v90he() {
	delete _sprite;
	if (_heversion >= 98) {
		delete _logicHE;
	}
}
#endif

#ifndef DISABLE_SCUMM_7_8
ScummEngine_v7::ScummEngine_v7(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: ScummEngine_v6(detector, syst, gs, md5sum, substResFileNameIndex) {
	_existLanguageFile = false;
	_languageBuffer = NULL;
	_languageIndex = NULL;
}

ScummEngine_v7::~ScummEngine_v7() {
	free(_languageBuffer);
	free(_languageIndex);
}

ScummEngine_v8::ScummEngine_v8(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex)
	: ScummEngine_v7(detector, syst, gs, md5sum, substResFileNameIndex) {
	_objectIDMap = 0;
}

ScummEngine_v8::~ScummEngine_v8() {
	delete [] _objectIDMap;
}
#endif

#pragma mark -
#pragma mark --- Initialization ---
#pragma mark -

int ScummEngine::init(GameDetector &detector) {

	// Initialize backend
	_system->beginGFXTransaction();
		initCommonGFX(detector);
		if (_renderMode == Common::kRenderHercA || _renderMode == Common::kRenderHercG) {
			_system->initSize(Common::kHercW, Common::kHercH, 1);
			_features |= GF_DEFAULT_TO_1X_SCALER;
			_system->setGraphicsMode("1x");
		} else {
			_system->initSize(_screenWidth, _screenHeight, (detector._force1xOverlay ? 1 : 2));
			if (_features & GF_DEFAULT_TO_1X_SCALER)
				_system->setGraphicsMode("1x");
		}
	_system->endGFXTransaction();

	// On some systems it's not safe to run CD audio games from the CD.
	if (_features & GF_AUDIOTRACKS)
		checkCD();

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0 && (_features & GF_AUDIOTRACKS))
		_system->openCD(cd_num);

	// Create the sound manager
	_sound = new Sound(this);

	// Setup the music engine
	setupMusic(_midi);

	// TODO: We shouldn't rely on the global Language values matching those COMI etc. expect.
	// Rather we should explicitly translate them.
	_language = Common::parseLanguage(ConfMan.get("language"));

	// Load localization data, if present
	loadLanguageBundle();

	// Load CJK font, if present
	loadCJKFont();

	// Create the charset renderer
	if (_platform == Common::kPlatformNES)
		_charset = new CharsetRendererNES(this);
	else if (_version <= 2)
		_charset = new CharsetRendererV2(this, _language);
	else if (_version == 3)
		_charset = new CharsetRendererV3(this);
#ifndef DISABLE_SCUMM_7_8
	else if (_version == 8)
		_charset = new CharsetRendererNut(this);
#endif
	else
		_charset = new CharsetRendererClassic(this);

	// Create the costume renderer
	if (_features & GF_NEW_COSTUMES) {
		_costumeRenderer = new AkosRenderer(this);
		_costumeLoader = new AkosCostumeLoader(this);
	} else if (_platform == Common::kPlatformC64 && _gameId == GID_MANIAC) {
		_costumeRenderer = new C64CostumeRenderer(this);
		_costumeLoader = new C64CostumeLoader(this);
	} else if (_platform == Common::kPlatformNES) {
		_costumeRenderer = new NESCostumeRenderer(this);
		_costumeLoader = new NESCostumeLoader(this);
	} else {
		_costumeRenderer = new ClassicCostumeRenderer(this);
		_costumeLoader = new ClassicCostumeLoader(this);
	}

#ifndef DISABLE_SCUMM_7_8
	// Create FT INSANE object
	if (_gameId == GID_FT)
		_insane = new Insane((ScummEngine_v6 *)this);
	else
#endif
		_insane = 0;

	// Load game from specified slot, if any
	if (ConfMan.hasKey("save_slot")) {
		requestLoad(ConfMan.getInt("save_slot"));
	}

	allocResTypeData(rtBuffer, MKID('NONE'), 10, "buffer", 0);

	setupScummVars();

	setupOpcodes();

	if (_version == 8)
		_numActors = 80;
	else if (_version == 7)
		_numActors = 30;
	else if (_gameId == GID_SAMNMAX)
		_numActors = 30;
	else if (_gameId == GID_MANIAC)
		_numActors = 25;
	else if (_heversion >= 80)
		_numActors = 62;
	else if (_heversion >= 72)
		_numActors = 30;
	else
		_numActors = 13;

	if (_version >= 7)
		OF_OWNER_ROOM = 0xFF;
	else
		OF_OWNER_ROOM = 0x0F;

	// if (_gameId==GID_MONKEY2 && _bootParam == 0)
	//	_bootParam = 10001;

	if (!_copyProtection && _gameId == GID_INDY4 && _bootParam == 0) {
		_bootParam = -7873;
	}

	if (!_copyProtection && _gameId == GID_SAMNMAX && _bootParam == 0) {
		_bootParam = -1;
	}

	readIndexFile();

#ifdef PALMOS_68K
	if (_features & GF_NEW_COSTUMES)
		res._maxHeapThreshold = gVars->memory[kMemScummNewCostGames];
	else
		res._maxHeapThreshold = gVars->memory[kMemScummOldCostGames];
#else
	if (_heversion >= 90 && _HEHeapSize) {
		res._maxHeapThreshold = _HEHeapSize * 1024;
	} else if (_features & GF_NEW_COSTUMES) {
		// Since the new costumes are very big, we increase the heap limit, to avoid having
		// to constantly reload stuff from the data files.
		res._maxHeapThreshold = 2500000;
#ifdef __PLAYSTATION2__
		res._maxHeapThreshold = 6 * 1024 * 1024;
#endif
	} else {
		res._maxHeapThreshold = 550000;
	}
#endif
	res._minHeapThreshold = 400000;

	scummInit();
	initScummVars();

	if (VAR_DEBUGMODE != 0xFF) {
		VAR(VAR_DEBUGMODE) = _debugMode;
		if (_heversion >= 80 && _debugMode)
			VAR(85) = 1;
	}

	if (_gameId == GID_MONKEY || _gameId == GID_MONKEY_SEGA)
		_scummVars[74] = 1225;

	if (_imuse) {
		_imuse->setBase(res.address[rtSound]);
	}

	if (_version >= 5)
		_sound->setupSound();

#if (defined(PALMOS_ARM) || defined(PALMOS_DEBUG))
	Graphics::initfonts();
#endif

	// Create debugger
	if (!_debugger)
		_debugger = new ScummDebugger(this);

	return 0;
}

void ScummEngine::scummInit() {
	int i;

	_tempMusic = 0;
	debug(9, "scummInit");

	if ((_gameId == GID_MANIAC) && (_version == 1) && !(_platform == Common::kPlatformNES)) {
		if (_platform == Common::kPlatformC64)
			initScreens(8, 144);
		else
			initScreens(16, 152);
	} else if (_version >= 7 || _heversion >= 71) {
		initScreens(0, _screenHeight);
	} else {
		initScreens(16, 144);
	}

	_palManipCounter = 0;

	for (i = 0; i < 256; i++)
		_roomPalette[i] = i;
	if (_version == 1) {
		// Use 17 color table for v1 games to allow
		// correct color for inventory and sentence
		// line
		// Original games used some kind of dynamic
		// color table remapping between rooms
		if (_platform == Common::kPlatformC64)
			setupC64Palette();
		else {
			if (_gameId == GID_MANIAC) {
				if (_platform == Common::kPlatformNES)
					setupNESPalette();
				else
					setupV1ManiacPalette();
			} else
				setupV1ZakPalette();
		}
	} else if (_features & GF_16COLOR) {
		for (i = 0; i < 16; i++)
			_shadowPalette[i] = i;

		switch (_renderMode) {
		case Common::kRenderEGA:
			setupEGAPalette();
			break;

		case Common::kRenderAmiga:
			setupAmigaPalette();
			break;

		case Common::kRenderCGA:
			setupCGAPalette();
			break;

		case Common::kRenderHercA:
		case Common::kRenderHercG:
			setupHercPalette();
			break;

		default:
			if ((_platform == Common::kPlatformAmiga) || (_platform == Common::kPlatformAtariST))
				setupAmigaPalette();
			else
				setupEGAPalette();
		}
	}

	if (_version > 3 && _version < 8)
		loadCharset(1);

	if (_features & GF_OLD_BUNDLE)
		loadCharset(0);	// FIXME - HACK ?

	setShake(0);
	setupCursor();

	// Allocate and Initialize actors
	Actor::initActorClass(this);
	_actors = new Actor[_numActors];
	_sortedActors = new Actor * [_numActors];
	for (i = 0; i < _numActors; i++) {
		_actors[i]._number = i;
		_actors[i].initActor(1);

		// this is from IDB
		if ((_version == 1) || (_gameId == GID_MANIAC && _demoMode))
			_actors[i].setActorCostume(i);
	}

	if (_gameId == GID_MANIAC && _version == 1) {
		setupV1ActorTalkColor();
	} else if (_gameId == GID_MANIAC && _version == 2 && _demoMode) {
		// HACK Some palette changes needed for demo script
		// in Maniac Mansion (Enhanced)
		_actors[3].setPalette(3, 1);
		_actors[9]._talkColor = 15;
		_actors[10]._talkColor = 7;
		_actors[11]._talkColor = 2;
		_actors[13]._talkColor = 5;
		_actors[23]._talkColor = 14;
	}

	vm.numNestedScripts = 0;
	vm.cutSceneStackPointer = 0;

	memset(vm.cutScenePtr, 0, sizeof(vm.cutScenePtr));
	memset(vm.cutSceneData, 0, sizeof(vm.cutSceneData));

	for (i = 0; i < _numVerbs; i++) {
		_verbs[i].verbid = 0;
		_verbs[i].curRect.right = _screenWidth - 1;
		_verbs[i].oldRect.left = -1;
		_verbs[i].type = 0;
		_verbs[i].color = 2;
		_verbs[i].hicolor = 0;
		_verbs[i].charset_nr = 1;
		_verbs[i].curmode = 0;
		_verbs[i].saveid = 0;
		_verbs[i].center = 0;
		_verbs[i].key = 0;
	}

	if (_version == 7) {
		VAR(VAR_CAMERA_THRESHOLD_X) = 100;
		VAR(VAR_CAMERA_THRESHOLD_Y) = 70;
		VAR(VAR_CAMERA_ACCEL_X) = 100;
		VAR(VAR_CAMERA_ACCEL_Y) = 100;
	} else if (!(_features & GF_NEW_CAMERA)) {
		if (_platform == Common::kPlatformNES) {
			camera._leftTrigger = 6;	// 6
			camera._rightTrigger = 21;	// 25
		} else {
			camera._leftTrigger = 10;
			camera._rightTrigger = (_heversion >= 71) ? 70 : 30;
		}
		camera._mode = 0;
	}
	camera._follows = 0;

	virtscr[0].xstart = 0;

	if (VAR_CURRENT_LIGHTS != 0xFF) {
		// Setup light
		_flashlight.xStrips = 7;
		_flashlight.yStrips = 7;
		_flashlight.buffer = NULL;
	}

	_mouse.x = 104;
	_mouse.y = 56;

	_ENCD_offs = 0;
	_EXCD_offs = 0;

	_currentScript = 0xFF;
	_sentenceNum = 0;

	_currentRoom = 0;
	_numObjectsInRoom = 0;
	_actorToPrintStrFor = 0;

	_charsetBufPos = 0;
	_haveMsg = 0;

	_varwatch = -1;
	_screenStartStrip = 0;

	_defaultTalkDelay = 3;
	_talkDelay = 0;
	_keepText = false;

	_currentCursor = 0;
	_cursor.state = 0;
	_userPut = 0;

	_newEffect = 129;
	_fullRedraw = true;

	clearDrawObjectQueue();

	if (_platform == Common::kPlatformNES)
		decodeNESBaseTiles();

	for (i = 0; i < 6; i++) {
		if (_version == 3) { // FIXME - what is this?
			_string[i]._default.xpos = 0;
			_string[i]._default.ypos = 0;
		} else {
			_string[i]._default.xpos = 2;
			_string[i]._default.ypos = 5;
		}
		_string[i]._default.right = _screenWidth - 1;
		_string[i]._default.height = 0;
		_string[i]._default.color = 0xF;
		_string[i]._default.center = 0;
		_string[i]._default.charset = 0;
	}

	// all keys are released
	for (i = 0; i < 512; i++)
		_keyDownMap[i] = false;

	_lastSaveTime = _system->getMillis();
}

void ScummEngine_c64::scummInit() {
	ScummEngine::scummInit();
	initC64Verbs();
}

void ScummEngine_v2::scummInit() {
	ScummEngine::scummInit();

	if (_platform == Common::kPlatformNES) {
		initNESMouseOver();
		_switchRoomEffect2 = _switchRoomEffect = 6;
	} else {
		initV2MouseOver();
		// Seems in V2 there was only a single room effect (iris),
		// so we set that here.
		_switchRoomEffect2 = 1;
		_switchRoomEffect = 5;
	}

	_inventoryOffset = 0;
}

void ScummEngine_v6::scummInit() {
	ScummEngine::scummInit();
	setDefaultCursor();
}

void ScummEngine_v60he::scummInit() {
	ScummEngine::scummInit();

	// HACK cursor hotspot is wrong
	// Original games used
	// setCursorHotspot(8, 7);
	if (_gameId == GID_FUNPACK)
		setCursorHotspot(16, 16);
}

#ifndef DISABLE_HE
void ScummEngine_v72he::scummInit() {
	ScummEngine_v60he::scummInit();

	_stringLength = 1;
	memset(_stringBuffer, 0, sizeof(_stringBuffer));
}

void ScummEngine_v90he::scummInit() {
	ScummEngine_v72he::scummInit();

	_heObject = 0;
	_heObjectNum = 0;
	_hePaletteNum = 0;

	_sprite->resetTables(0);
	memset(&_wizParams, 0, sizeof(_wizParams));

	if (_features & GF_HE_CURSORLESS)
		setDefaultCursor();

	if (_heversion >= 98) {
		switch (_gameId) {
		case GID_PUTTRACE:
			_logicHE = new LogicHErace(this);
			break;

		case GID_FUNSHOP:
			_logicHE = new LogicHEfunshop(this);
			break;

		case GID_FOOTBALL:
			_logicHE = new LogicHEfootball(this);
			break;

		default:
			_logicHE = new LogicHE(this);
			break;
		}
	}
}

void ScummEngine_v99he::scummInit() {
	ScummEngine_v90he::scummInit();

	_hePalettes = (uint8 *)malloc((_numPalettes + 1) * 1024);
	memset(_hePalettes, 0, (_numPalettes + 1) * 1024);

	// Array 129 is set to base name
	int len = resStrLen((const byte *)_gameName.c_str()) + 1;
	ArrayHeader *ah = defineArray(129, kStringArray, 0, 0, 0, len);
	memcpy(ah->data, _gameName.c_str(), len);

}
#endif

void ScummEngine::setupMusic(int midi) {
	_midiDriver = MidiDriver::detectMusicDriver(midi);
	_native_mt32 = (ConfMan.getBool("native_mt32") || (_midiDriver == MD_MT32));
	// FIXME: MD_TOWNS should not be _midi_native in the first place!! iMuse code needs to be restructured.
	if ((_gameId == GID_TENTACLE) || (_gameId == GID_SAMNMAX) || (_midiDriver == MD_TOWNS))
		_enable_gs = false;
	else
		_enable_gs = ConfMan.getBool("enable_gs");

	/* Bind the mixer to the system => mixer will be invoked
	 * automatically when samples need to be generated */
	if (!_mixer->isReady()) {
		warning("Sound mixer initialization failed\n");
		if (_midiDriver == MD_ADLIB ||
				_midiDriver == MD_PCSPK ||
				_midiDriver == MD_PCJR)	{
			_midiDriver = MD_NULL;
			warning("MIDI driver depends on sound mixer, switching to null MIDI driver\n");
		}
	}

	// Init iMuse
	if (_features & GF_DIGI_IMUSE) {
#ifndef DISABLE_SCUMM_7_8
		_musicEngine = _imuseDigital = new IMuseDigital(this, 10);
#endif
	} else if (_platform == Common::kPlatformC64) {
		// TODO
		_musicEngine = NULL;
	} else if (_platform == Common::kPlatformNES) {
		_musicEngine = new Player_NES(this);
	} else if ((_platform == Common::kPlatformAmiga) && (_version == 2)) {
		_musicEngine = new Player_V2A(this);
	} else if ((_platform == Common::kPlatformAmiga) && (_version == 3)) {
		_musicEngine = new Player_V3A(this);
	} else if ((_platform == Common::kPlatformAmiga) && (_version < 5)) {
		_musicEngine = NULL;
	} else if (_gameId == GID_MANIAC && (_version == 1)) {
		_musicEngine = new Player_V1(this, _midiDriver != MD_PCSPK);
	} else if (_version <= 2) {
		_musicEngine = new Player_V2(this, _midiDriver != MD_PCSPK);;
	} else if (((_midiDriver == MD_PCJR) || (_midiDriver == MD_PCSPK)) && ((_version > 2) && (_version < 5))) {
		_musicEngine = new Player_V2(this, _midiDriver != MD_PCSPK);
	} else if (_version > 2 && _heversion <= 61) {
		MidiDriver *nativeMidiDriver = MidiDriver::createMidi(_midiDriver);
		if (nativeMidiDriver != NULL && _native_mt32)
			nativeMidiDriver->property (MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
		bool multi_midi = ConfMan.getBool("multi_midi") && _midiDriver != MD_NULL && (midi & MDT_ADLIB);
		MidiDriver *adlibMidiDriver;
		if (nativeMidiDriver == NULL || multi_midi) {
			adlibMidiDriver = MidiDriver_ADLIB_create(_mixer);
			adlibMidiDriver->property(MidiDriver::PROP_OLD_ADLIB, (_features & GF_SMALL_HEADER) ? 1 : 0);
		} else
			adlibMidiDriver = NULL;

		_musicEngine = _imuse = IMuse::create(_system, nativeMidiDriver, adlibMidiDriver);
		if (_imuse) {
			if (ConfMan.hasKey("tempo"))
				_imuse->property(IMuse::PROP_TEMPO_BASE, ConfMan.getInt("tempo"));
			_imuse->property(IMuse::PROP_NATIVE_MT32, _native_mt32);
			_imuse->property(IMuse::PROP_GS, _enable_gs);
			if (_heversion >= 60 || midi == MDT_TOWNS) {
				_imuse->property(IMuse::PROP_LIMIT_PLAYERS, 1);
				_imuse->property(IMuse::PROP_RECYCLE_PLAYERS, 1);
			}
			if (midi == MDT_TOWNS)
				_imuse->property(IMuse::PROP_DIRECT_PASSTHROUGH, 1);
		}
	}

	setupVolumes();
}

void ScummEngine::setupVolumes() {

	// Sync the engine with the config manager
	int soundVolumeMusic = ConfMan.getInt("music_volume");
	int soundVolumeSfx = ConfMan.getInt("sfx_volume");
	int soundVolumeSpeech = ConfMan.getInt("speech_volume");

	if (_musicEngine) {
		_musicEngine->setMusicVolume(soundVolumeMusic);
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, soundVolumeSfx);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, soundVolumeSpeech);
}



#pragma mark -
#pragma mark --- Main loop ---
#pragma mark -

int ScummEngine::go() {
	_engineStartTime = _system->getMillis() / 1000;

	// If requested, load a save game instead of running the boot script
	if (_saveLoadFlag != 2 || !loadState(_saveLoadSlot, _saveTemporaryState)) {
		int args[16];
		memset(args, 0, sizeof(args));
		args[0] = _bootParam;

		_saveLoadFlag = 0;
#ifndef DISABLE_HE
		if (_heversion >= 98) {
			((ScummEngine_v90he *)this)->_logicHE->initOnce();
			((ScummEngine_v90he *)this)->_logicHE->beforeBootScript();
		}
#endif
		if (_gameId == GID_MANIAC && _demoMode)
			runScript(9, 0, 0, args);
		else
			runScript(1, 0, 0, args);
	} else {
		_saveLoadFlag = 0;
	}

	int delta = 0;
	int diff = _system->getMillis();

	while (!_quit) {

		updatePalette();
		_system->updateScreen();

		diff -= _system->getMillis();
		waitForTimer(delta * 15 + diff);
		diff = _system->getMillis();
		delta = scummLoop(delta);

		if (delta < 1)	// Ensure we don't get into a loop
			delta = 1;  // by not decreasing sleepers.

		if (_quit) {
			// TODO: Maybe perform an autosave on exit?
		}
	}

	return 0;
}

void ScummEngine::waitForTimer(int msec_delay) {
	uint32 start_time;

	if (_fastMode & 2)
		msec_delay = 0;
	else if (_fastMode & 1)
		msec_delay = 10;

	start_time = _system->getMillis();

	while (!_quit) {
		parseEvents();

		_sound->updateCD(); // Loop CD Audio if needed
		if (_system->getMillis() >= start_time + msec_delay)
			break;
		_system->delayMillis(10);
	}
}

int ScummEngine::scummLoop(int delta) {
	if (_debugger->isAttached())
		_debugger->onFrame();

	// Randomize the PRNG by calling it at regular intervals. This ensures
	// that it will be in a different state each time you run the program.
	_rnd.getRandomNumber(2);

#ifndef DISABLE_HE
	if (_heversion >= 98) {
		((ScummEngine_v90he *)this)->_logicHE->startOfFrame();
	}
#endif
	if (_version > 2) {
		VAR(VAR_TMR_1) += delta;
		VAR(VAR_TMR_2) += delta;
		VAR(VAR_TMR_3) += delta;
		if (_gameId == GID_ZAK256 || _gameId == GID_INDY3) {
			// All versions of Indy3 set three extra timers
			// FM-TOWNS version of Zak sets three extra timers
			VAR(39) += delta;
			VAR(40) += delta;
			VAR(41) += delta;
		}
	}
	if (VAR_TMR_4 != 0xFF)
		VAR(VAR_TMR_4) += delta;

	if (delta > 15)
		delta = 15;

	decreaseScriptDelay(delta);

	_talkDelay -= delta;
	if (_talkDelay < 0)
		_talkDelay = 0;

	// Record the current ego actor before any scripts (including input scripts)
	// get a chance to run.
	int oldEgo = 0;
	if (VAR_EGO != 0xFF)
		oldEgo = VAR(VAR_EGO);

	// In V1-V3 games, CHARSET_1 is called much earlier than in newer games.
	// See also bug #770042 for a case were this makes a difference.
	if (_version <= 3)
		CHARSET_1();

	processKbd(false);

	if (_features & GF_NEW_CAMERA) {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x;
		VAR(VAR_CAMERA_POS_Y) = camera._cur.y;
	} else if (_version <= 2) {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x / 8;
	} else {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x;
	}
	if (_version <= 7)
		VAR(VAR_HAVE_MSG) = (_haveMsg == 0xFE) ? 0xFF : _haveMsg;

	if (_platform == Common::kPlatformC64 && _gameId == GID_MANIAC) {
		// TODO
	} else if (_version <= 2) {
		VAR(VAR_VIRT_MOUSE_X) = _virtualMouse.x / 8;
		VAR(VAR_VIRT_MOUSE_Y) = _virtualMouse.y / 2;

		// Adjust mouse coordinates as narrow rooms in NES are centered
		if (_platform == Common::kPlatformNES && _NESStartStrip > 0) {
			VAR(VAR_VIRT_MOUSE_X) -= 2;
			if (VAR(VAR_VIRT_MOUSE_X) < 0)
				VAR(VAR_VIRT_MOUSE_X) = 0;
		}
	} else {
		VAR(VAR_VIRT_MOUSE_X) = _virtualMouse.x;
		VAR(VAR_VIRT_MOUSE_Y) = _virtualMouse.y;
		VAR(VAR_MOUSE_X) = _mouse.x;
		VAR(VAR_MOUSE_Y) = _mouse.y;
		if (VAR_DEBUGMODE != 0xFF) {
			// This is NOT for the Mac version of Indy3/Loom
			VAR(VAR_DEBUGMODE) = _debugMode;
		}
	}

	if (_features & GF_AUDIOTRACKS) {
		// Covered automatically by the Sound class
	} else if (VAR_MUSIC_TIMER != 0xFF) {
		if (_musicEngine) {
			// The music engine generates the timer data for us.
			VAR(VAR_MUSIC_TIMER) = _musicEngine->getMusicTimer();
		} else {
			// Used for Money Island 1 (Amiga)
			// TODO: The music delay (given in milliseconds) might have to be tuned a little
			// to get it correct for all games. Without the ability to watch/listen to the
			// original games, I can't do that myself.
			const int MUSIC_DELAY = 350;
			_tempMusic += delta * 15;	// Convert delta to milliseconds
			if (_tempMusic >= MUSIC_DELAY) {
				_tempMusic -= MUSIC_DELAY;
				VAR(VAR_MUSIC_TIMER) += 1;
			}
		}
	}

	// Trigger autosave all 5 minutes.
	if (!_saveLoadFlag && _system->getMillis() > _lastSaveTime + 5 * 60 * 1000) {
		_saveLoadSlot = 0;
		sprintf(_saveLoadName, "Autosave %d", _saveLoadSlot);
		_saveLoadFlag = 1;
		_saveTemporaryState = false;
	}

	if (VAR_GAME_LOADED != 0xFF)
		VAR(VAR_GAME_LOADED) = 0;
	if (_saveLoadFlag) {
load_game:
		bool success;
		const char *errMsg = 0;
		char filename[256];

		if (_saveLoadFlag == 1) {
			success = saveState(_saveLoadSlot, _saveTemporaryState);
			if (!success)
				errMsg = "Failed to save game state to file:\n\n%s";

			// Ender: Disabled for small_header games, as can overwrite game
			//  variables (eg, Zak256 cashcard values). Temp disabled for V8
			// because of odd timing issue with scripts and the variable reset
			if (success && _saveTemporaryState && !(_features & GF_SMALL_HEADER) && _version < 8)
				VAR(VAR_GAME_LOADED) = 201;
		} else {
			success = loadState(_saveLoadSlot, _saveTemporaryState);
			if (!success)
				errMsg = "Failed to load game state from file:\n\n%s";

			// Ender: Disabled for small_header games, as can overwrite game
			//  variables (eg, Zak256 cashcard values).
			if (success && _saveTemporaryState && !(_features & GF_SMALL_HEADER))
				VAR(VAR_GAME_LOADED) = 203;
		}

		makeSavegameName(filename, _saveLoadSlot, _saveTemporaryState);
		if (!success) {
			displayMessage(0, errMsg, filename);
		} else if (_saveLoadFlag == 1 && _saveLoadSlot != 0 && !_saveTemporaryState) {
			// Display "Save successful" message, except for auto saves
			char buf[256];
			snprintf(buf, sizeof(buf), "Successfully saved game state in file:\n\n%s", filename);

			GUI::TimedMessageDialog dialog(buf, 1500);
			runDialog(dialog);
		}
		if (success && _saveLoadFlag != 1)
			clearClickedStatus();

		_saveLoadFlag = 0;
		_lastSaveTime = _system->getMillis();
	}

	if (_completeScreenRedraw) {
		_charset->clearCharsetMask();
		_charset->_hasMask = false;

		// HACK as in game save stuff isn't supported currently
		if (_gameId == GID_LOOM || _gameId == GID_LOOM256) {
			int args[16];
			uint value;
			memset(args, 0, sizeof(args));
			args[0] = 2;

			if (_platform == Common::kPlatformMacintosh)
				value = 105;
			else
 				value = (_gameId == GID_LOOM256) ? 150 : 100;
			byte restoreScript = (_platform == Common::kPlatformFMTowns) ? 17 : 18;
			// if verbs should be shown restore them
			if (VAR(value) == 2)
				runScript(restoreScript, 0, 0, args);
		} else if (_version > 3) {
			for (int i = 0; i < _numVerbs; i++)
				drawVerb(i, 0);
		} else {
			redrawVerbs();
		}

		handleMouseOver(false);

		_completeScreenRedraw = false;
		_fullRedraw = true;
	}

	if (_heversion >= 80) {
		_sound->processSoundCode();
	}
	runAllScripts();
	checkExecVerbs();
	checkAndRunSentenceScript();

	if (_quit)
		return 0;

	// HACK: If a load was requested, immediately perform it. This avoids
	// drawing the current room right after the load is request but before
	// it is performed. That was annoying esp. if you loaded while a SMUSH
	// cutscene was playing.
	if (_saveLoadFlag && _saveLoadFlag != 1) {
		goto load_game;
	}

	if (_currentRoom == 0) {
		if (_version > 3)
			CHARSET_1();
		drawDirtyScreenParts();
	} else {
		walkActors();
		moveCamera();
		updateObjectStates();
		if (_version > 3)
			CHARSET_1();

		if (camera._cur.x != camera._last.x || _bgNeedsRedraw || _fullRedraw
				|| ((_features & GF_NEW_CAMERA) && camera._cur.y != camera._last.y)) {
			redrawBGAreas();
		}

		processDrawQue();

		if (_heversion >= 99)
			_fullRedraw = false;

		// Full Throttle always redraws verbs and draws verbs before actors
		if (_version >= 7)
			redrawVerbs();

#ifndef DISABLE_HE
		if (_heversion >= 90) {
			((ScummEngine_v90he *)this)->_sprite->resetBackground();
			((ScummEngine_v90he *)this)->_sprite->sortActiveSprites();
		}
#endif

		setActorRedrawFlags();
		resetActorBgs();

		if (VAR_CURRENT_LIGHTS != 0xFF &&
		    !(VAR(VAR_CURRENT_LIGHTS) & LIGHTMODE_screen) &&
		      VAR(VAR_CURRENT_LIGHTS) & LIGHTMODE_flashlight) {
			drawFlashlight();
			setActorRedrawFlags();
		}

		processActors();

		_fullRedraw = false;

		if (_version >= 4 && _heversion <= 61)
			cyclePalette();
		palManipulate();
		if (_doEffect) {
			_doEffect = false;
			fadeIn(_newEffect);
			clearClickedStatus();
		}

		if (VAR_MAIN_SCRIPT != 0xFF && VAR(VAR_MAIN_SCRIPT) != 0) {
			runScript(VAR(VAR_MAIN_SCRIPT), 0, 0, 0);
		}

		// Handle mouse over effects (for verbs).
		handleMouseOver(oldEgo != VAR(VAR_EGO));

		// Render everything to the screen.
		drawDirtyScreenParts();

		if (_version <= 5)
			playActorSounds();
	}

	_sound->processSound();

#ifndef DISABLE_SCUMM_7_8
	if (_imuseDigital) {
		_imuseDigital->flushTracks();
		if ( ((_gameId == GID_DIG) && (!(_features & GF_DEMO))) || (_gameId == GID_CMI) )
			_imuseDigital->refreshScripts();
	}
#endif

	camera._last = camera._cur;

	if (!(++res._expireCounter)) {
		res.increaseResourceCounter();
	}

	animateCursor();

	/* show or hide mouse */
	_system->showMouse(_cursor.state > 0);

#ifndef DISABLE_HE
	if (_heversion >= 90) {
		((ScummEngine_v90he *)this)->_sprite->updateImages();
	}
	if (_heversion >= 98) {
		((ScummEngine_v90he *)this)->_logicHE->endOfFrame();
	}
#endif

	if (VAR_TIMER != 0xFF)
		VAR(VAR_TIMER) = 0;
	return VAR(VAR_TIMER_NEXT);

}

#pragma mark -
#pragma mark --- SCUMM ---
#pragma mark -

int ScummEngine::getHETimer(int timer) {
	checkRange(15, 1, timer, "getHETimer: Timer %d out of range(%d)");
	int time = _system->getMillis() - _heTimers[timer];
	return time;
}

void ScummEngine::setHETimer(int timer) {
	checkRange(15, 1, timer, "setHETimer: Timer %d out of range(%d)");
	_heTimers[timer] = _system->getMillis();
}

void ScummEngine::pauseGame() {
	pauseDialog();
}

void ScummEngine::shutDown() {
	_quit = true;
}

void ScummEngine::restart() {
// TODO: Check this function - we should probably be reinitting a lot more stuff, and I suspect
//	 this leaks memory like a sieve

// Fingolfing seez: An alternate way to implement restarting would be to create
// a save state right after startup ... to this end we could introduce a SaveFile
// subclass which is implemented using a memory buffer (i.e. no actual file is
// created). Then to restart we just have to load that pseudo save state.


	int i;

	// Reset some stuff
	_currentRoom = 0;
	_currentScript = 0xFF;
	killAllScriptsExceptCurrent();
	setShake(0);
	_sound->stopAllSounds();

	// Clear the script variables
	for (i = 0; i < _numVariables; i++)
		_scummVars[i] = 0;

	// Empty inventory
	for (i = 0; i < _numGlobalObjects; i++)
		clearOwnerOf(i);

	// Reinit things
	allocateArrays();                   // Reallocate arrays
	readIndexFile();                    // Reread index (reset objectstate etc)
	scummInit();                       // Reinit scumm variables
	initScummVars();
	if (_imuse) {
		_imuse->setBase(res.address[rtSound]);
	}
	_sound->setupSound();               // Reinit sound engine

	// Re-run bootscript
	int args[16];
	memset(args, 0, sizeof(args));
	args[0] = _bootParam;
	if (_gameId == GID_MANIAC && _demoMode)
		runScript(9, 0, 0, args);
	else
		runScript(1, 0, 0, args);
}

void ScummEngine::startManiac() {
	debug(0, "stub startManiac()");
	displayMessage(0, "Usually, Maniac Mansion would start now. But ScummVM doesn't do that yet. To play it, go to 'Add Game' in the ScummVM start menu and select the 'Maniac' directory inside the Tentacle game directory.");
}

#pragma mark -
#pragma mark --- GUI ---
#pragma mark -

int ScummEngine::runDialog(Dialog &dialog) {
	_dialogStartTime = _system->getMillis() / 1000;

	// Pause sound & video
	bool old_soundsPaused = _sound->_soundsPaused;
	_sound->pauseSounds(true);
	bool oldSmushPaused = _smushPaused;
	_smushPaused = true;

	// Open & run the dialog
	int result = dialog.runModal();

	// Restore old cursor
	updateCursor();

	// Resume sound & video
	_sound->pauseSounds(old_soundsPaused);
	_smushPaused = oldSmushPaused;

	_engineStartTime += (_system->getMillis() / 1000) - _dialogStartTime;
	_dialogStartTime = 0;

	// Return the result
	return result;
}

void ScummEngine::pauseDialog() {
	if (!_pauseDialog)
		_pauseDialog = new PauseDialog(this, 10);
	runDialog(*_pauseDialog);
}

void ScummEngine::versionDialog() {
	if (!_versionDialog)
		_versionDialog = new PauseDialog(this, 11);
	runDialog(*_versionDialog);
}

void ScummEngine::mainMenuDialog() {
	if (!_mainMenuDialog)
		_mainMenuDialog = new MainMenuDialog(this);
	runDialog(*_mainMenuDialog);
}

void ScummEngine::confirmExitDialog() {
	ConfirmDialog d(this, "Do you really want to quit (y/n)?");

	if (runDialog(d)) {
		_quit = true;
	}
}

void ScummEngine::confirmRestartDialog() {
	ConfirmDialog d(this, "Do you really want to restart (y/n)?");

	if (runDialog(d)) {
		restart();
	}
}

char ScummEngine::displayMessage(const char *altButton, const char *message, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, message);
	vsnprintf(buf, STRINGBUFLEN, message, va);
	va_end(va);

	GUI::MessageDialog dialog(buf, "OK", altButton);
	return runDialog(dialog);
}

#pragma mark -
#pragma mark --- Miscellaneous ---
#pragma mark -


uint32 ScummEngine::fileReadDword() {
#if defined(SCUMM_LITTLE_ENDIAN)
	return _fileHandle->readUint32LE();
#elif defined(SCUMM_BIG_ENDIAN)
	return _fileHandle->readUint32BE();
#endif
}

void ScummEngine::errorString(const char *buf1, char *buf2) {
	if (_currentScript != 0xFF) {
		ScriptSlot *ss = &vm.slot[_currentScript];
		sprintf(buf2, "(%d:%d:0x%X): %s", _roomResource,
			ss->number, _scriptPointer - _scriptOrgPointer, buf1);
	} else {
		strcpy(buf2, buf1);
	}

#ifdef _WIN32_WCE
	if (isSmartphone())
		return;
#endif

	// Unless an error -originated- within the debugger, spawn the debugger. Otherwise
	// exit out normally.
	if (_debugger && !_debugger->isAttached()) {
		printf("%s\n", buf2);	// (Print it again in case debugger segfaults)
		_debugger->attach(buf2);
		_debugger->onFrame();
	}
}

int ScummEngine::generateSubstResFileName(const char *filename, char *buf, int bufsize, int index) {
	if (index == -3)
		index = _substResFileNameIndex;

	return generateSubstResFileName_(filename, buf, bufsize, index);
}


} // End of namespace Scumm

using namespace Scumm;

GameList Engine_SCUMM_gameList() {
	const ScummGameSettings *g = scumm_settings;
	const ObsoleteTargets *o = obsoleteTargetsTable;
	GameList games;
	while (g->name) {
		games.push_back(g->toGameSettings());
		g++;
	}

	while (o->from) {
		games.push_back(o->toGameSettings());
		o++;
	}
	return games;
}

DetectedGameList Engine_SCUMM_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const ScummGameSettings *g;
	char detectName[128];
	char tempName[128];
	int substLastIndex = 0;

	typedef Common::Map<Common::String, bool> StringSet;
	StringSet fileSet;

	for (g = scumm_settings; g->name; ++g) {
		// Determine the 'detectname' for this game, that is, the name of a
		// file that *must* be presented if the directory contains the data
		// for this game. For example, FOA requires atlantis.000
		const char *base = g->name;
		detectName[0] = '\0';

		// TODO: we need to add cache here
		for (int method = 0; method < 8; method++) {
			switch (method) {
			case 0:
				strcpy(detectName, "00.LFL");
				break;
			case 1:
				strcpy(detectName, "000.LFL");
				break;
			case 2:
				strcpy(detectName, base);
				strcat(detectName, ".la0");
				break;
			case 3:
				strcpy(detectName, base);
				strcat(detectName, ".he0");
				break;
			case 4:
				strcpy(detectName, base);
				break;
			case 5:
				strcpy(detectName, base);
				strcat(detectName, ".000");
				break;
			case 6:
				strcpy(detectName, base);
				strcat(detectName, ".sm0");
				break;
			case 7:
				strcpy(detectName, "00.MAN");
				break;
			}
			strcpy(tempName, detectName);

			substLastIndex = 0;

			while (substLastIndex != -1) {
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
								File tmp;
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
								TODO: MIght be possible to distinguish those by the script count.
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
									if (g->version != 3 || (g->features & GF_OLD_BUNDLE))
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
							if (substLastIndex > 0 && // HE Mac versions.
								(substResFileNameTable[substLastIndex].genMethod == kGenMac ||
								 substResFileNameTable[substLastIndex].genMethod == kGenMacNoParens)) {
								detectedGames.push_back(DetectedGame(g->toGameSettings(),
																	 Common::UNK_LANG,
																	 Common::kPlatformMacintosh));
								fileSet[file->path()] = true;
							} else if (substLastIndex == 0 && g->id == GID_MANIAC &&
									   (buf[0] == 0xbc || buf[0] == 0xa0)) {
								detectedGames.push_back(DetectedGame(g->toGameSettings(),
																	 Common::UNK_LANG,
																	 Common::kPlatformNES));
							} else if ((g->id == GID_MANIAC || g->id == GID_ZAK) &&
									   ((buf[0] == 0x31 && buf[1] == 0x0a) ||
										(buf[0] == 0xcd && buf[1] == 0xfe))) {
								detectedGames.push_back(DetectedGame(g->toGameSettings(),
																	 Common::UNK_LANG,
																	 Common::kPlatformC64));
							} else {
								detectedGames.push_back(g->toGameSettings());
								fileSet[file->path()] = false;
							}
							break;
						}
					}
				}

				substLastIndex = generateSubstResFileName_(tempName, detectName, sizeof(detectName), substLastIndex+1);
			}
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

				const char *target = elem->target;

				// Find the GameSettings for that target
				for (g = scumm_settings; g->name; ++g) {
					if (0 == scumm_stricmp(g->name, target))
							break;
				}
				assert(g->name);
				// Insert the 'enhanced' game data into the candidate list
				if (iter->_value == true) // This was HE Mac game
					detectedGames.push_back(DetectedGame(g->toGameSettings(), elem->language, Common::kPlatformMacintosh));
				else
					detectedGames.push_back(DetectedGame(g->toGameSettings(), elem->language, elem->platform));
				exactMatch = true;
			}
		}
	}

	return detectedGames;
}

static int generateSubstResFileName_(const char *filename, char *buf, int bufsize, int index) {
	if (index <= 0)
		return -1;

	char num = filename[strlen(filename) - 1];

	// In some cases we have .(a) and .(b) extensions
	if (num == ')')
		num = filename[strlen(filename) - 2];

	const char *ext = NULL;

	ext = strrchr(filename, '.');
	size_t len = (ext != NULL) ? ext - filename : strlen(filename);

	for (int i = index; substResFileNameTable[i].winName; i++) {
		if (!scumm_strnicmp(filename, substResFileNameTable[i].winName, len)) {
			switch (substResFileNameTable[i].genMethod) {
			case kGenMac:
			case kGenMacNoParens:
				if (num == '3') { // special case for cursors
					// For mac they're stored in game binary
					strncpy(buf, substResFileNameTable[i].macName, bufsize);
				} else {
					if (substResFileNameTable[i].genMethod == kGenMac)
						snprintf(buf, bufsize, "%s (%c)", substResFileNameTable[i].macName, num);
					else
						snprintf(buf, bufsize, "%s %c", substResFileNameTable[i].macName, num);
				}
				break;

			case kGenPC:
				if (ext)
					snprintf(buf, bufsize, "%s%s", substResFileNameTable[i].macName, ext);
				else
					strncpy(buf, substResFileNameTable[i].macName, bufsize);
				break;

			case kGenAsIs:
				strncpy(buf, substResFileNameTable[i].macName, bufsize);
				break;

			default:
				*buf = 0;
				break;
			}

			return i;
		}
	}
	return -1;
}

Engine *Engine_SCUMM_create(GameDetector *detector, OSystem *syst) {
	Engine *engine;

	const ObsoleteTargets *o = obsoleteTargetsTable;
	while (o->from) {
		if (!scumm_stricmp(detector->_game.name, o->from)) {
			detector->_game.name = o->to;

			ConfMan.set("gameid", o->to);

			if (o->platform != Common::kPlatformUnknown)
				ConfMan.set("platform", Common::getPlatformCode(o->platform));

			warning("Target upgraded from %s to %s", o->from, o->to);
			ConfMan.flushToDisk();
			break;
		}
		o++;
	}

	const ScummGameSettings *g = scumm_settings;
	while (g->name) {
		if (!scumm_stricmp(detector->_game.name, g->name))
			break;
		g++;
	}
	if (!g->name) {
		error("Invalid game '%s'\n", detector->_game.name);
		return 0;
	}

	ScummGameSettings game = *g;

	// Calculate MD5 of the games detection file, for savegames etc.
	const char *name = g->name;
	char detectName[256], tempName[256], gameMD5[32+1];
	uint8 md5sum[16];
	int substLastIndex = 0;
	bool found = false;

	for (int method = 0; method < 7 && !found; method++) {
		switch (method) {
		case 0:
			strcpy(detectName, "00.LFL");
			break;
		case 1:
			strcpy(detectName, "000.LFL");
			break;
		case 2:
			strcpy(detectName, name);
			strcat(detectName, ".la0");
			break;
		case 3:
			strcpy(detectName, name);
			strcat(detectName, ".he0");
			break;
		case 4:
			strcpy(detectName, name);
			strcat(detectName, ".000");
			break;
		case 5:
			strcpy(detectName, name);
			strcat(detectName, ".sm0");
			break;
		case 6:
			strcpy(detectName, "00.MAN");
			break;
		}
		strcpy(tempName, detectName);

		substLastIndex = 0;
		while (substLastIndex != -1) {
			if (File::exists(detectName, ConfMan.get("path").c_str())) {
				found = true;
				break;
			}

			substLastIndex = generateSubstResFileName_(tempName, detectName, sizeof(detectName), substLastIndex + 1);
		}
		if (found) {
			if (substLastIndex != 0)
				debug(5, "Generated filename substitute: %s -> %s", tempName, detectName);
			break;
		}
	}

	// Unable to locate game data
	if (!found) {
		return 0;
	}

	// Force game to have Mac platform if needed
	if (substLastIndex > 0) {
		if (substResFileNameTable[substLastIndex].genMethod == kGenMac ||
			substResFileNameTable[substLastIndex].genMethod == kGenMacNoParens)
			game.platform = Common::kPlatformMacintosh;
	}

	if (Common::md5_file(detectName, md5sum, ConfMan.get("path").c_str(), kMD5FileSizeLimit)) {
		for (int j = 0; j < 16; j++) {
			sprintf(gameMD5 + j*2, "%02x", (int)md5sum[j]);
		}
	}

	// Check if MD5 was overwritten
	if (ConfMan.hasKey("target_md5")) {
		const char *md5 = ConfMan.get("target_md5").c_str();

		g = multiple_versions_md5_settings;
		while (g->name) {
			if (!scumm_stricmp(md5, g->name))
				break;
			g++;
		}
		if (g->name) {
			game = *g;
			game.name = name;
			if (game.description)
				g_system->setWindowCaption(game.description);
		}
	} else {
		// Perform MD5-based search
		g = multiple_versions_md5_settings;
		while (g->name) {
			if (!scumm_stricmp(gameMD5, g->name))
				break;
			g++;
		}
		if (g->name) {
			game = *g;
			game.name = name;
			if (game.description)
				g_system->setWindowCaption(game.description);
		}
	}
	// Starting from version 7.1, HE games use 640x480. We check this here since multiple
	// versions _could_ use different resolutions (I haven't verified this, though).
	if (game.heversion >= 71) {
		game.features |= GF_DEFAULT_TO_1X_SCALER;
	}

	// TODO: REMOVE DEPRECATED OPTION
	// (Perhaps GUI should display a messagebox on encountering an unknown key?)

	// Check for a user override of the platform
	// TODO: Is it a good idea to allow for such an override? Ideally, shouldn't
	// we simply fully rely on the auto detection of the platform?
	// -> potentially might cause troubles with FM-Towns/amiga versions?
	if (ConfMan.hasKey("platform"))
		game.platform = Common::parsePlatform(ConfMan.get("platform"));

	if (game.platform == Common::kPlatformFMTowns && game.version == 3) {
		game.midi = MDT_TOWNS;
	}

	switch (game.version) {
	case 1:
	case 2:
		if (game.id == GID_MANIAC && game.platform == Common::kPlatformC64)
			engine = new ScummEngine_c64(detector, syst, game, md5sum, substLastIndex);
		else
			engine = new ScummEngine_v2(detector, syst, game, md5sum, substLastIndex);
		break;
	case 3:
		if (game.features & GF_OLD_BUNDLE)
			engine = new ScummEngine_v3old(detector, syst, game, md5sum, substLastIndex);
		else
			engine = new ScummEngine_v3(detector, syst, game, md5sum, substLastIndex);
		break;
	case 4:
		engine = new ScummEngine_v4(detector, syst, game, md5sum, substLastIndex);
		break;
	case 5:
		engine = new ScummEngine_v5(detector, syst, game, md5sum, substLastIndex);
		break;
	case 6:
		switch (game.heversion) {
#ifndef DISABLE_HE
		case 100:
			engine = new ScummEngine_v100he(detector, syst, game, md5sum, substLastIndex);
			break;
		case 99:
			engine = new ScummEngine_v99he(detector, syst, game, md5sum, substLastIndex);
			break;
		case 98:
		case 95:
		case 90:
			engine = new ScummEngine_v90he(detector, syst, game, md5sum, substLastIndex);
			break;
		case 80:
			engine = new ScummEngine_v80he(detector, syst, game, md5sum, substLastIndex);
			break;
		case 73:
		case 72:
			engine = new ScummEngine_v72he(detector, syst, game, md5sum, substLastIndex);
			break;
		case 71:
			engine = new ScummEngine_v71he(detector, syst, game, md5sum, substLastIndex);
			break;
		case 70:
			engine = new ScummEngine_v70he(detector, syst, game, md5sum, substLastIndex);
			break;
#endif
#ifndef PALMOS_68K
		case 61:
			engine = new ScummEngine_v60he(detector, syst, game, md5sum, substLastIndex);
			break;
#endif
		default:
			engine = new ScummEngine_v6(detector, syst, game, md5sum, substLastIndex);
		}
		break;
#ifndef DISABLE_SCUMM_7_8
	case 7:
		engine = new ScummEngine_v7(detector, syst, game, md5sum, substLastIndex);
		break;
	case 8:
		engine = new ScummEngine_v8(detector, syst, game, md5sum, substLastIndex);
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
