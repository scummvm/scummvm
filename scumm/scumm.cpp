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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"

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
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse.h"
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
#ifdef __PALM_OS__
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
	const char *baseFilename;
	const char *detectFilename;

	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description, features };
		return dummy;
	}
};


enum {
	// We only compute the MD5 of the first megabyte of our data files.
	kMD5FileSizeLimit = 1024 * 1024
};


static const ScummGameSettings scumm_settings[] = {
	/* Scumm Version 1 */
	/* Scumm Version 2 */

	{"maniac", "Maniac Mansion", GID_MANIAC, 2, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE | GF_MULTIPLE_VERSIONS, Common::kPlatformPC, 0, 0},
	{"zak",         "Zak McKracken and the Alien Mindbenders", GID_ZAK, 2, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE | GF_MULTIPLE_VERSIONS, Common::kPlatformPC, 0, 0},

	/* Scumm Version 3 */
	{"indy3EGA", "Indiana Jones and the Last Crusade", GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformPC, 0, 0},
	{"indy3Towns", "Indiana Jones and the Last Crusade (FM-TOWNS)", GID_INDY3, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS | GF_AUDIOTRACKS, Common::kPlatformFMTowns, 0, 0},
	{"indy3", "Indiana Jones and the Last Crusade (256)", GID_INDY3, 3, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS, Common::kPlatformPC, 0, 0},

	{"zakTowns", "Zak McKracken and the Alien Mindbenders (FM-TOWNS)", GID_ZAK256, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns, 0, 0},
	{"loom", "Loom", GID_LOOM, 3, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_16COLOR | GF_USE_KEY | GF_OLD_BUNDLE, Common::kPlatformPC, 0, 0},
	{"loomTowns", "Loom (FM Towns)", GID_LOOM, 3, 0, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_NO_SCALING | GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns, 0, 0},

	/* Scumm Version 4 */
	{"monkeyEGA", "Monkey Island 1 (EGA)", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC, 0, 0},
	{"pass", "Passport to Adventure", GID_PASS, 4, 0, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, Common::kPlatformPC, 0, 0},

	/* Scumm version 5, small header -- we treat these as V4 games, since internally
	   they really are much closer to the V4 games than to all other V5 games. */
	{"monkeyVGA", "Monkey Island 1 (256 color Floppy version)", GID_MONKEY_VGA, 4, 0, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY, Common::kPlatformPC, 0, 0},
	{"loomcd", "Loom (256 color CD version)", GID_LOOM256, 4, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC, 0, 0},

	/* Scumm version 5 */
	{"monkey", "Monkey Island 1", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC, 0, 0},
	{"monkey1", "Monkey Island 1 (alt)", GID_MONKEY, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC, 0, 0},
	{"game", "Monkey Island 1 (SegaCD version)", GID_MONKEY_SEGA, 5, 0, MDT_NONE,
	 GF_USE_KEY | GF_AUDIOTRACKS, Common::kPlatformPC, 0, 0},
	{"monkey2", "Monkey Island 2: LeChuck's revenge", GID_MONKEY2, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, 0, 0},
	{"mi2demo", "Monkey Island 2: LeChuck's revenge (Demo)", GID_MONKEY2, 5, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, 0, 0},

	{"atlantis", "Indiana Jones and the Fate of Atlantis", GID_INDY4, 5, 0, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, 0, 0},
	{"playfate", "Indiana Jones and the Fate of Atlantis (Demo)", GID_INDY4, 5, 0, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, 0, 0},
	{"fate", "Indiana Jones and the Fate of Atlantis (Demo)", GID_INDY4, 5, 0, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, 0, 0},
	{"indy4", "Indiana Jones and the Fate of Atlantis (FM-TOWNS)", GID_INDY4, 5, 0, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, 0, 0},
	{"indydemo", "Indiana Jones and the Fate of Atlantis (FM-TOWNS Demo)", GID_INDY4, 5, 0, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, 0, 0},

	/* Scumm Version 6 */
	{"tentacle", "Day Of The Tentacle", GID_TENTACLE, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, 0, 0},
	{"tentacleMac", "Day Of The Tentacle", GID_TENTACLE, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformMacintosh, "tentacle", "Day Of The Tentacle Data"},
	{"dottdemo", "Day Of The Tentacle (Demo)", GID_TENTACLE, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, 0, 0},

	{"samnmax", "Sam & Max", GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, 0, 0},
	{"samnmax-alt", "Sam & Max (alt)", GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, "samnmax", "samnmax.sm0"},
	{"samnmaxMac", "Sam & Max (Mac)", GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformMacintosh, "samnmax", "Sam & Max Data"},
	{"samdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, 0, 0},
	{"samdemoMac", "Sam & Max (Mac Demo)", GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformMacintosh, "samdemo", "Sam & Max Demo Data"},
	{"snmdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, 0, "snmdemo.sm0"},
	{"snmidemo", "Sam & Max (Interactive WIP Demo)", GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformPC, 0, 0},

//	{"test", "Test demo game", GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE, GF_NEW_OPCODES, Common::kPlatformUnknown, 0, 0},

	/* Scumm Version 7 */
	{"ft", "Full Throttle", GID_FT, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, Common::kPlatformPC, 0, 0},
	{"ftMac", "Full Throttle (Mac)", GID_FT, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, Common::kPlatformMacintosh, "ft", "Full Throttle Data"},
	{"ftdemo", "Full Throttle (Mac Demo)", GID_FT, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformMacintosh, 0, "Full Throttle Demo Data"},
	{"ftpcdemo", "Full Throttle (PC Demo)", GID_FT, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformPC, "ft", "ft.000"},


	{"dig", "The Dig", GID_DIG, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, Common::kPlatformPC, 0, 0},
	{"digMac", "The Dig (Mac)", GID_DIG, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, Common::kPlatformMacintosh, "dig", "The Dig Data"},
	{"digdemo", "The Dig (Demo)", GID_DIG, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformPC, "dig", "dig.la0"},
	{"digdemoMac", "The Dig (Mac Demo)", GID_DIG, 7, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, Common::kPlatformMacintosh, "dig", "The Dig Demo Data"},

#ifndef __PALM_OS__
	/* Scumm Version 8 */
	{"comi", "The Curse of Monkey Island", GID_CMI, 8, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEFAULT_TO_1X_SCALER, Common::kPlatformWindows, 0, 0},
	{"comidemo", "The Curse of Monkey Island (Demo)", GID_CMI, 8, 0, MDT_NONE,
	 GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEFAULT_TO_1X_SCALER | GF_DEMO, Common::kPlatformWindows, "comi", "COMI.LA0"},
#endif

	// Humongous Entertainment Scumm Version 6
	{"puttputt", "Putt-Putt Joins The Parade", GID_HEGAME, 6, 61, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformPC, 0, 0},
	{"puttdemo", "Putt-Putt Joins The Parade (Demo)", GID_HEGAME, 6, 60, MDT_ADLIB | MDT_NATIVE,
	  GF_USE_KEY | GF_MULTIPLE_VERSIONS, Common::kPlatformPC, 0, 0},
	{"moondemo", "Putt-Putt Goes To The Moon (Demo)", GID_HEGAME, 6, 61, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformPC, 0, 0},
	{"puttmoon", "Putt-Putt Goes To The Moon", GID_HEGAME, 6, 61, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformPC, 0, 0},
	{"funpack", "Putt-Putt's Fun Pack", GID_FUNPACK, 6, 61, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC, 0, 0},
	{"fbpack", "Fatty Bear's Fun Pack", GID_HEGAME, 6, 61, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformPC, 0, 0},
	{"fbear", "Fatty Bear's Birthday Surprise", GID_FBEAR, 6, 61, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformPC, 0, 0},
	{"fbdemo", "Fatty Bear's Birthday Surprise (DOS Demo)", GID_FBEAR, 6, 61, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformPC, 0, 0},

#ifndef __PALM_OS__
	{"activity", "Putt-Putt & Fatty Bear's Activity Pack", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},

	// Humongous Entertainment Scumm Version 7.1
	// The first version to use 640x480 resolution
	// There are also 7.1 versions of freddi, freddemo, airdemo and farmdemo
	{"catalog", "Humongous Interactive Catalog", GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},

	// Humongous Entertainment Scumm Version 7.2
	{"catalog2", "Humongous Interactive Catalog 2", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"freddemo", "Freddi Fish 1: The Case of the Missing Kelp Seeds (Demo)", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"farmdemo", "Let's Explore the Farm with Buzzy (Demo)", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"farm", "Let's Explore the Farm with Buzzy", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"airdemo", "Let's Explore the Airport with Buzzy (Demo)", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"airport", "Let's Explore the Airport with Buzzy", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"jungle", "Let's Explore the Jungle with Buzzy", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},

	{"puttzoo", "Putt-Putt Saves the Zoo", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"zoodemo", "Putt-Putt Saves the Zoo (Demo)", GID_HEGAME, 6, 72, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},

	// Humongous Entertainment Scumm Version 8.0 ?  Scummsrc.80 
	{"ff2-demo", "Freddi Fish 2: The Case of the Haunted Schoolhouse (Demo)", GID_FREDDI2, 6, 80, MDT_NONE,
	GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"freddi2", "Freddi Fish 2: The Case of the Haunted Schoolhouse", GID_FREDDI2, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"pjs-demo", "Pajama Sam 1: No Need to Hide When It's Dark Outside (Demo)", GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"pajama", "Pajama Sam 1: No Need to Hide When It's Dark Outside", GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},

	{"dog", "Putt-Putt and Pep's Dog on a Stick", GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"balloon", "Putt-Putt and Pep's Balloon-O-Rama", GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"maze", "Freddi Fish and Luther's Maze Madness", GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"socks", "Pajama Sam's Sock Works", GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"water", "Freddi Fish and Luther's Water Worries", GID_HEGAME, 6, 80, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},

	// Humongous Entertainment Scumm Version 9.0 ?  Scummsys.90
	{"freddi", "Freddi Fish 1: The Case of the Missing Kelp Seeds", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"kinddemo", "Big Thinkers Kindergarten (Demo)", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"thinkerk", "Big Thinkers Kindergarten", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"1grademo", "Big Thinkers First Grade (Demo)", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"freddi3", "Freddi Fish 3: The Case of the Stolen Conch Shell", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"f3-mdemo", "Freddi Fish 3: The Case of the Stolen Conch Shell (Demo)", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	// there is also a C++ engine based version of timedemo
	{"TimeDemo", "Putt-Putt Travels Through Time (Demo)", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"spyfox", "Spyfox 1: Dry Cereal", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	// there is also a C++ engine version of spydemo
	{"Spydemo", "Spyfox 1: Dry Cereal (Demo)", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"baseball", "Backyard Baseball", GID_HEGAME, 6, 90, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},

	// Humongous Entertainment Scumm Version 9.5 ?  Scummsys.95
	{"pj2demo", "Pajama Sam 2: Thunder and Lightning Aren't so Frightening (Demo)", GID_HEGAME, 6, 95, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"pajama2", "Pajama Sam 2: Thunder and Lightning Aren't so Frightening", GID_HEGAME, 6, 95, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"chase", "Spy Fox in Cheese Chase Game", GID_HEGAME, 6, 95, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},

	// Humongous Entertainment Scumm Version 9.8 ?  Scummsys.98
	// these and later games can easily be identified by the .(a) file instead of a .he1
	// and INIB chunk in the .he0
	{"lost", "Pajama Sam's Lost & Found", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_CURSORLESS | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"racedemo", "Putt-Putt Enters the Race (Demo)", GID_PUTTRACE, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"puttrace", "Putt-Putt Enters the Race", GID_PUTTRACE, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"BluesABCTimeDemo", "Blue's ABC Time (Demo)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"soccer", "Backyard Soccer", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},

	// Global scripts increased to 2048
	{"freddi4", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985 | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"f4-demo", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Demo)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985 | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},

	// Engine moved to c++ 
	// Humongous Entertainment Scumm Version 9.9 ?  Scummsys.99
	{"smaller", "Pajama Sam's Lost & Found (Test)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_CURSORLESS, Common::kPlatformWindows, 0, 0},
	{"puttcircus", "Putt-Putt Joins the Circus", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows, 0, 0},
	{"circdemo", "Putt-Putt Joins the Circus (Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"sf2-demo", "Spyfox 2: Some Assembly Required (Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"spyfox2", "Spyfox 2: Some Assembly Required", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows, 0, 0},
	{"mustard", "Spy Fox in Hold the Mustard", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows, 0, 0},
	{"football", "Backyard Football", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},

	// Humongous Entertainment Scumm Version ?
	{"freddicove", "Freddi Fish 5: The Case of the Creature of Coral Cave", GID_FREDDICOVE, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_16BIT_COLOR | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"pj3-demo", "Pajama Sam 3: You Are What You Eat From Your Head to Your Feet (Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"pajama3", "Pajama Sam 3: You Are What You Eat From Your Head to Your Feet", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows, 0, 0},
	{"FreddisFunShop", "Freddi Fish's One-Stop Fun Shop", GID_FUNSHOP, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows, 0, 0},
	{"SamsFunShop", "Pajama Sam's One-Stop Fun Shop", GID_FUNSHOP, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows, 0, 0},
	{"PuttsFunShop", "Putt-Putt's One-Stop Fun Shop", GID_FUNSHOP, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED, Common::kPlatformWindows, 0, 0},
	{"putttime", "Putt-Putt Travels Through Time", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"spyozon", "Spyfox 3: Operation Ozone", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows, 0, 0},
	{"sf3-demo", "Spyfox 3: Operation Ozone (Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows, 0, 0},

	{"bb2demo", "Backyard Baseball 2001 (Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows, 0, 0},
	{"footdemo", "Backyard Football 2002 (Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, Common::kPlatformWindows, 0, 0},
	{"SoccerMLS", "Backyard Soccer MLS Edition", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},

	// Humongous Entertainment Scumm Version ?
	{"ff5demo", "Freddi Fish 5: The Case of the Creature of Coral Cave (Demo)", GID_FREDDICOVE, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows, 0, 0},
	{"pjgames", "Pajama Sam: Games to Play On Any Day", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformWindows, 0, 0},

	// Uses smacker in external files
	{"BluesBirthdayDemo", "Blue's Birthday Adventure (Demo)", GID_HEGAME, 6, 98, MDT_NONE,
	GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"artdemo", "Blue's Art Time Activities (Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows, 0, 0},
	{"readdemo", "Blue's Reading Time Activities (Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows, 0, 0},

	// Uses bink in external files for logos
	{"Baseball2003", "Backyard Baseball 2003", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows, 0, 0},
	{"Soccer2004", "Backyard Soccer 2004", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_16BIT_COLOR, Common::kPlatformWindows, 0, 0},

#endif
	{NULL, NULL, 0, 0, 0, MDT_NONE, 0, Common::kPlatformUnknown, 0, 0}
};

// This additional table is used for titles where GF_MULTIPLE_VERSIONS is specified.
// Now these are HE games. Some of them were released for different versions of SPUTM,
// and instead of multiplying GIDs this table is used.
//
// Use main table to specify default flags and this table to override defaults.
//
// Please, add new entries sorted alpabetically by string name
static const ScummGameSettings multiple_versions_md5_settings[] = {
#ifndef __PALM_OS__
	{"2e85f7aa054930c692a5b1bed1dfc295", "Backyard Football 2002 (Demo Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0}, // Football2002

	{"179879b6e35c1ead0d93aab26db0951b", "Fatty Bear's Birthday Surprise (Windows)", GID_FBEAR, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"22c9eb04455440131ffc157aeb8d40a8", "Fatty Bear's Birthday Surprise (Windows Demo)", GID_FBEAR, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},

	{"c0039ad982999c92d0de81910d640fa0", "Freddi Fish 1: The Case of the Missing Kelp Seeds", GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},
	{"d4cccb5af88f3e77f370896e9ba8c5f9", "Freddi Fish 1: The Case of the Missing Kelp Seeds", GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},
	{"084ed0fa98a6d1e9368d67fe9cfbd417", "Freddi Fish 1: The Case of the Missing Kelp Seeds (Demo) (puttputt cd)", GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},

	{"8ee63cafb1fe9d62aa0d5a23117e70e7", "Freddi Fish 2: The Case of the Haunted Schoolhouse (Updated)", GID_FREDDI2, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0}, // FreddiCHSH
	{"51305e929e330e24a75a0351c8f9975e", "Freddi Fish 2: The Case of the Haunted Schoolhouse (Updated)", GID_FREDDI2, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},
	{"e41de1c2a15abbcdbf9977e2d7e8a340", "Freddi Fish 2: The Case of the Haunted Schoolhouse (Updated Ru)", GID_FREDDI2, 6, 100, MDT_NONE, // FIXME: number of actors
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0}, // FreddiCHSH
	{"d37c55388294b66e53e7ced3af88fa68", "Freddi Fish 2: The Case of the Haunted Schoolhouse (Demo Updated)", GID_FREDDI2, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0}, // FFHSDemo

	{"75bff95816b84672b877d22a911ab811", "Freddi Fish 3: The Case of the Stolen Conch Shell (Updated Ru)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0}, // FIXME: number of actors

	{"07b810e37be7489263f7bc7627d4765d", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Unencrypted Ru)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformUnknown, 0, 0},
	{"78bd5f036ea35a878b74e4f47941f784", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Updated Ru)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0}, // FIXME: number of actors
	{"16effd200aa6b8abe9c569c3e578814d", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Updated Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0}, // ff4demo
	{"499c958affc394f2a3868f1eb568c3ee", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Updated Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0}, // ff4demo
	{"ebd324dcf06a4c49e1ba5c231eee1060", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Updated Demo)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},

	{"b8955d7d23b4972229060d1592489fef", "Freddi Fish 5: The Case of the Creature of Coral Cave (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformUnknown, 0, 0}, // FreddiDZZ
	{"4ce2d5b355964bbcb5e5ce73236ef868", "Freddi Fish 5: The Case of the Creature of Coral Cave (Updated Ru)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformUnknown, 0, 0},
	{"21abe302e1b1e2b66d6f5c12e241ebfd", "Freddi Fish 5: The Case of the Creature of Coral Cave (Unencrypted)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformUnknown, 0, 0},

	{"4dbff3787aedcd96b0b325f2d92d7ad9", "Freddi Fish and Luther's Maze Madness (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},

	{"86c9902b7bec1a17926d4dae85beaa45", "Let's Explore the Airport with Buzzy (Demo) (puttputt cd)", GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},
	{"bf8b52fdd9a69c67f34e8e9fec72661c", "Let's Explore the Farm with Buzzy (Demo) (puttputt cd)", GID_HEGAME, 6, 71, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},

	{"d8d07efcb88f396bee0b402b10c3b1c9", "Maniac Mansion (NES E)", GID_MANIAC, 1, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES, 0, "Maniac Mansion (E).prg"},
	{"81bbfa181184cb494e7a81dcfa94fbd9", "Maniac Mansion (NES F)", GID_MANIAC, 1, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES, 0, "Maniac Mansion (F).prg"},
	{"22d07d6c386c9c25aca5dac2a0c0d94b", "Maniac Mansion (NES SW)", GID_MANIAC, 1, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES, 0, "Maniac Mansion (SW).prg"},
	{"3905799e081b80a61d4460b7b733c206", "Maniac Mansion (NES U)", GID_MANIAC, 1, 0, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformNES, 0, "Maniac Mansion (U).prg"},
	{"7f45ddd6dbfbf8f80c0c0efea4c295bc", "Maniac Mansion (v1)", GID_MANIAC, 1, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformUnknown, 0, 0},

	{"ef71a322b6530ac45b1a070f7c0795f7", "Moonbase Commander (Demo)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_LOCALIZED | GF_16BIT_COLOR, Common::kPlatformUnknown, 0, 0},

	{"37aed3f91c1ef959e0bd265f9b13781f", "Pajama Sam: No Need To Hide When It's Dark Outside (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0}, // PajamaNHD
	{"d7ab7cd6105546016e6a0d46fb36b964", "Pajama Sam: No Need To Hide When It's Dark Outside (Demo Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0}, // PJSamDemo

	{"30ba1e825d4ad2b448143ae8df18482a", "Pajama Sam 2: Thunder and Lightning Aren't so Frightening (Updated Demo)", GID_HEGAME, 6, 98, MDT_NONE, // FIXME: number of actors
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformUnknown, 0, 0}, // Pjs2demo
	{"32709cbeeb3044b34129950860a83f14", "Pajama Sam 2: Thunder and Lightning Aren't so Frightening (Updated)", GID_HEGAME, 6, 99, MDT_NONE, // FIXME: number of actors
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0}, // PajamaTAL

	{"a561d2e2413cc1c71d5a1bf87bf493ea", "Pajama Sam's Lost & Found (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_CURSORLESS, Common::kPlatformUnknown, 0, 0},

	{"6af2419fe3db5c2fdb091ae4e5833770", "Putt-Putt Enters the Race (Updated Demo)", GID_PUTTRACE, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformUnknown, 0, 0}, // 500demo
	{"aaa587701cde7e74692c68c1024b85eb", "Putt-Putt Enters the Race (Updated Demo)", GID_PUTTRACE, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformUnknown, 0, 0},

	{"9c92eeaf517a31b7221ec2546ab669fd", "Putt-Putt Goes To The Moon (Windows)", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"9c143c5905055d5df7a0f014ab379aee", "Putt-Putt Goes To The Moon (Windows Demo)", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},

	{"0b3222aaa7efcf283eb621e0cefd26cc", "Putt-Putt Joins The Parade (early version)", GID_HEGAME, 6, 0, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, Common::kPlatformUnknown, 0, 0},
	{"6a30a07f353a75cdc602db27d73e1b42", "Putt-Putt Joins The Parade (Windows)", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformWindows, 0, 0},
	{"37ff1b308999c4cca7319edfcc1280a0", "Putt-Putt Joins The Parade (Windows Demo)", GID_HEGAME, 6, 70, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},

	{"92e7727e67f5cd979d8a1070e4eb8cb3", "Putt-Putt Saves the Zoo (Updated)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformUnknown, 0, 0},

	{"a525c1753c1db5011c00417da37887ef", "Putt-Putt Travels Through Time (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},
	{"fcb78ebecab2757264c590890c319cc5", "Putt-Putt Travels Through Time (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},
	{"0ab19be9e2a3f6938226638b2a3744fe", "Putt-Putt Travels Through Time (Updated Demo)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},

	{"8e3241ddd6c8dadf64305e8740d45e13", "Putt-Putt and Pep's Balloon-O-Rama (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},

	{"d4b8ee426b1afd3e53bc0cf020418cf6", "Putt-Putt and Pep's Dog on a Stick (Updated)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},

	{"3de99ef0523f8ca7958faa3afccd035a", "Spyfox 1: Dry Cereal (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},
	{"9bda5fee51d2fda5253d02c642016bf4", "Spyfox 1: Dry Cereal (Updated)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformUnknown, 0, 0},
	{"72ac6bc980d5101c2142189d746bd62f", "Spyfox 1: Dry Cereal (Updated Ru)", GID_HEGAME, 6, 99, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0}, // SPYFoxDC
	{"9d4ab3e0e1d1ebc6ba8a6a4c470ed184", "Spyfox 1: Dry Cereal (Updated Demo)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},
	{"4edbf9d03550f7ba01e7f34d69b678dd", "Spyfox 1: Dry Cereal (Updated Demo)", GID_HEGAME, 6, 98, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES | GF_HE_985, Common::kPlatformUnknown, 0, 0},

	{"90c755e1c9b9b8a4129d37b2259d0655", "Spy Fox in Cheese Chase Game (Updated)", GID_HEGAME, 6, 100, MDT_NONE,
	 GF_USE_KEY | GF_NEW_COSTUMES, Common::kPlatformUnknown, 0, 0},

	{"b23f7cd7c304d7dff08e92a96120d5b4", "Zak McKracken and the Alien Mindbenders (v1)", GID_ZAK, 1, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformUnknown, 0, 0},
	{"7020931d5a2be0a49d68e7a1882363e4", "Zak McKracken and the Alien Mindbenders (v1)", GID_ZAK, 1, 0, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, Common::kPlatformUnknown, 0, 0},
#endif
	{NULL, NULL, 0, 0, MDT_NONE, 0, 0, Common::kPlatformUnknown, 0, 0}
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
	{ "Intentionally/left/blank", "", kGenMacNoParens},
	{ "00.LFL", "Maniac Mansion (E).prg", kGenAsIs },
	{ "00.LFL", "Maniac Mansion (F).prg", kGenAsIs },
	{ "00.LFL", "Maniac Mansion (SW).prg", kGenAsIs },
	{ "00.LFL", "Maniac Mansion (U).prg", kGenAsIs },
	{ "airdemo", "Airport Demo", kGenMac},
	{ "balloon", "Balloon-O-Rama", kGenMac},
	{ "baseball", "BaseBall", kGenMac},
	{ "BluesABCTimeDemo", "BluesABCTimeDemo", kGenMac},
	{ "chase", "Cheese Chase", kGenMac},
	{ "circdemo", "Putt Circus Demo", kGenMac},
	{ "dog", "Dog on a Stick", kGenMac},
	{ "f3-mdemo", "F3-Mdemo", kGenMac },
	{ "f3-mdemo", "F3-mdemo", kGenMac },
	{ "f3-mdemo", "FF3-DEMO", kGenPC },
	{ "f4-demo", "Freddi 4 Demo", kGenMac },
	{ "f4-demo", "ff4demo", kGenPC },
	{ "farmdemo", "Farm Demo", kGenMac},
	{ "fbear", "Fatty Bear", kGenMacNoParens },
	{ "fbdemo", "Fatty Bear Demo", kGenMacNoParens },
	{ "ff2-demo", "Freddi Fish 2 Demo", kGenMac },
	{ "ff2-demo", "FFHSDemo", kGenPC },
	{ "ff2-demo", "FFHSDemo", kGenMac },
	{ "footdemo", "FootBall Demo", kGenMac },
	{ "football", "FootBall", kGenMac },
	{ "footdemo", "FootBall2002", kGenPC },
	{ "freddemo", "Freddi Demo", kGenMac },
	{ "freddi", "Freddi Fish", kGenMac },
	{ "freddi", "FreddiD", kGenPC },
	{ "freddi2", "Freddi Fish 2", kGenMac },
	{ "freddi2", "FreddiCHSH", kGenPC },
	{ "freddi3", "Freddi Fish 3", kGenMac },
	{ "freddi3", "FreddiSCS", kGenPC },
	{ "freddi4", "Freddi 4", kGenMac },
	{ "freddi4", "FreddiHRBG", kGenPC },
	{ "freddicove", "FreddiCove", kGenMac },
	{ "freddicove", "FreddiCCC", kGenPC },
	{ "freddicove", "FreddiDZZ", kGenPC },
	{ "FreddisFunShop", "Freddi's FunShop", kGenMac },
	{ "ff5demo", "FFCoveDemo", kGenPC },
	{ "kinddemo", "Kinddemo", kGenMac },
	{ "kinddemo", "KindDemo", kGenMac },
	{ "lost", "Lost and Found", kGenMac},
	{ "maze", "Maze Madness", kGenMac},
	{ "monkey", "monkey1", kGenPC},
	{ "moondemo", "Putt-Putt Moon Demo", kGenMacNoParens },
	{ "mustard", "Mustard", kGenMac},
	{ "pajama", "Pajama Sam", kGenMac},
	{ "pajama", "PajamaNHD", kGenPC},
	{ "pajama2", "Pajama Sam 2", kGenMac},
	{ "pajama2", "PajamaTAL", kGenPC},
	{ "pj2demo", "PJ2Demo", kGenMac },
	{ "pj2demo", "Pjs2demo", kGenPC },
	{ "pj3-demo", "Pajama Sam 3-Demo", kGenMac },
	{ "pj3-demo", "pj3demo", kGenPC },
	{ "pjs-demo", "Pajama Sam Demo", kGenMac },
	{ "pjs-demo", "PjSamDemo", kGenPC },
	{ "pjs-demo", "PjSamDemo", kGenMac },
	{ "circdemo", "Putt Circus Demo", kGenMac },
	{ "puttcircus", "Putt Circus", kGenMac },
	{ "puttdemo", "Putt-Putt's Demo", kGenMacNoParens },
	{ "puttmoon", "Putt-Putt Moon", kGenMacNoParens },
	{ "puttputt", "Putt-Putt Parade", kGenMacNoParens },
	{ "puttrace", "Putt Race", kGenMac },
	{ "PuttsFunShop", "Putt's FunShop", kGenMac },
	{ "putttime", "PuttTijd", kGenPC },
	{ "putttime", "PuttTTT", kGenMac },
	{ "putttime", "PuttTTT", kGenPC },
	{ "putttime", "PuttPuttTTT", kGenPC },
	{ "puttzoo", "PuttZoo", kGenMac },
	{ "racedemo", "500demo", kGenPC},
	{ "racedemo", "RaceDemo", kGenMac },
	{ "samnmax", "ramnmax", kGenPC }, // Used in some releases of Russian Sam'n'Max
	{ "SamsFunShop", "Sam's FunShop", kGenMac },
	{ "sf2-demo", "Spy Fox 2 - Demo", kGenMac },
	{ "sf2-demo", "sf2demo", kGenPC },
	{ "sf3-demo", "Spy Ozone Demo", kGenMac },
	{ "soccer", "Soccer", kGenMac },
	{ "socks", "SockWorks", kGenMac },
	{ "Spydemo", "foxdemo", kGenPC},
	{ "Spydemo", "FoxDemo", kGenMac },
	{ "spyfox", "SPYFox", kGenMac },
	{ "spyfox", "SPYFoxDC", kGenPC },
	{ "spyfox2", "Spy Fox 2", kGenMac },
	{ "spyfox2", "spyfoxsr", kGenPC },
	{ "spyozon", "SpyOzon", kGenMac },
	{ "spyozon", "SPYFoxOZU", kGenPC },
	{ "thinkerk", "ThinkerK", kGenMac },
	{ "timedemo", "TIJDDEMO", kGenPC },
	{ "timedemo", "TimeDemo", kGenMac },
	{ "water", "Water Worries", kGenMac },
	{ "zoodemo", "Puttzoo Demo", kGenMac },
	{ "zoodemo", "Zoo Demo", kGenMac}
};

static int compareMD5Table(const void *a, const void *b) {
	const char *key = (const char *)a;
	const MD5Table *elem = (const MD5Table *)b;
	return strcmp(key, elem->md5);
}

ScummEngine::ScummEngine(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16])
	: Engine(syst),
	  _gameId(gs.id),
	  _version(gs.version),
	  _heversion(gs.heversion),
	  _features(gs.features),
	  _platform(gs.platform),
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
#ifdef __PALM_OS__
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

	// We read data directly from NES ROM instead of extracting it with
	// external tool
	if ((_platform == Common::kPlatformNES) && (_substResFileNameIndex || gs.detectFilename))
		_fileHandle = new ScummNESFile();
	else
		_fileHandle = new ScummFile();

	// The mac versions of Sam&Max, DOTT, FT and The Dig used a special meta
	// (container) file format to store the actual SCUMM data files. The
	// rescumm utility used to be used to extract those files. While that is
	// still possible, we now support reading those files directly.
	// The first step is to check whether one of them is present (we do that
	// here); the rest is handled by the  ScummFile class and code in
	// openResourceFile() (and in the Sound class, for MONSTER.SOU handling).
	if (gs.detectFilename) {
		if (_fileHandle->open(gs.detectFilename)) {
			_containerFile = gs.detectFilename;
		}
	}

	// Init all vars
	_imuse = NULL;
	_imuseDigital = NULL;
	_musicEngine = NULL;
	_verbs = NULL;
	_objs = NULL;
	_debugger = NULL;
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
	_currentScript = 0;
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
	_substResFileNameIndex = 0;
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
	_auxBlocksNum = 0;
	memset(_auxBlocks, 0, sizeof(_auxBlocks));
	_auxEntriesNum = 0;
	memset(_auxEntries, 0, sizeof(_auxEntries));
	memset(_timers, 0, sizeof(_timers));

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
	VAR_MOUSE_BUTTONS = 0xFF;
	VAR_MOUSE_HOLD = 0xFF;
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

	VAR_SKIP_RESET_TALK_ACTOR = 0xFF;
	VAR_MUSIC_CHANNEL = 0xFF;
	VAR_SOUND_CHANNEL = 0xFF;

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

	// Allow the user to override the game name with a custom string.
	// This allows some game versions to work which use filenames
	// differing from the regular version(s) of that game.
	_gameName = ConfMan.hasKey("basename") ? ConfMan.get("basename") : gs.baseFilename ? gs.baseFilename : gs.name;

	_copyProtection = ConfMan.getBool("copy_protection");
	_demoMode = ConfMan.getBool("demo_mode");
	if (ConfMan.hasKey("nosubtitles")) {
		warning("Configuration key 'nosubtitles' is deprecated. Use 'subtitles' instead");
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

	// FIXME: a dirty hack. Currently this is checked before engine
	//  creation.
	if (_heversion >= 71) {
		_features |= GF_DEFAULT_TO_1X_SCALER;
	}

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

ScummEngine_v4::ScummEngine_v4(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) 
 : ScummEngine_v5(detector, syst, gs, md5sum) {
	_resourceHeaderSize = 6;
}

ScummEngine_v3::ScummEngine_v3(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) 
 : ScummEngine_v4(detector, syst, gs, md5sum) {
}

ScummEngine_v3old::ScummEngine_v3old(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) 
 : ScummEngine_v3(detector, syst, gs, md5sum) {
	_resourceHeaderSize = 4;
}

ScummEngine_v2::ScummEngine_v2(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) 
 : ScummEngine_v3old(detector, syst, gs, md5sum) {
}

ScummEngine_v6::ScummEngine_v6(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) 
 : ScummEngine(detector, syst, gs, md5sum) {
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

ScummEngine_v70he::ScummEngine_v70he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16])
 : ScummEngine_v60he(detector, syst, gs, md5sum) {
	 _win32ResExtractor = new Win32ResExtractor(this);
	 _macResExtractor = new MacResExtractor(this);

	_heV7RoomOffsets = NULL;

	_heSndSoundId = 0;
	_heSndOffset = 0;
	_heSndChannel = 0;
	_heSndFlags = 0;
	_heSndSoundFreq = 0;

	_skipProcessActors = 0;

	VAR_NUM_SOUND_CHANNELS = 0xFF;
}

ScummEngine_v70he::~ScummEngine_v70he() {
	free(_heV7DiskOffsets);
	free(_heV7RoomIntOffsets);
	free(_heV7RoomOffsets);
}

ScummEngine_v72he::ScummEngine_v72he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16])
 : ScummEngine_v71he(detector, syst, gs, md5sum) {
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
	VAR_WIZ_TCOLOR = 0xFF;
}

ScummEngine_v80he::ScummEngine_v80he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16])
 : ScummEngine_v72he(detector, syst, gs, md5sum) {
	_heSBNGId = 0;
}

ScummEngine_v90he::~ScummEngine_v90he() {
	if (_heversion >= 98) {
		delete _logicHE;
	}
}


ScummEngine_v7::ScummEngine_v7(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16])
 : ScummEngine_v6(detector, syst, gs, md5sum) {
	_existLanguageFile = false;
	_languageBuffer = NULL;
	_languageIndex = NULL;
}

ScummEngine_v7::~ScummEngine_v7() {
	free(_languageBuffer);
	free(_languageIndex);
}

ScummEngine_v8::ScummEngine_v8(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16])
 : ScummEngine_v7(detector, syst, gs, md5sum) {
	_objectIDMap = 0;
}

ScummEngine_v8::~ScummEngine_v8() {
	delete [] _objectIDMap;
}


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

		// FIXME: All this seems a dirty hack to me. We already
		// have this check in constructor
		if (_heversion >= 71) {
			_features |= GF_DEFAULT_TO_1X_SCALER;
		}

	_system->endGFXTransaction();

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
		_charset = new CharsetRendererNES(this, _language);
	else if (_version <= 2)
		_charset = new CharsetRendererV2(this, _language);
	else if (_version == 3)
		_charset = new CharsetRendererV3(this);
	else if (_version == 8)
		_charset = new CharsetRendererNut(this);
	else
		_charset = new CharsetRendererClassic(this);

	// Create the costume renderer
	if (_features & GF_NEW_COSTUMES) {
		_costumeRenderer = new AkosRenderer(this);
		_costumeLoader = new AkosCostumeLoader(this);
	} else if (_platform == Common::kPlatformNES) {
		_costumeRenderer = new NESCostumeRenderer(this);
		_costumeLoader = new NESCostumeLoader(this);
	} else {
		_costumeRenderer = new ClassicCostumeRenderer(this);
		_costumeLoader = new ClassicCostumeLoader(this);
	}

	// Create FT INSANE object
	if (_gameId == GID_FT)
		_insane = new Insane((ScummEngine_v6 *)this);
	else
		_insane = 0;

	// Load game from specified slot, if any
	if (ConfMan.hasKey("save_slot")) {
		requestLoad(ConfMan.getInt("save_slot"));
	}

#ifdef __PALM_OS__
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
	} else {
		res._maxHeapThreshold = 550000;
	}
#endif
	res._minHeapThreshold = 400000;

	allocResTypeData(rtBuffer, MKID('NONE'), 10, "buffer", 0);

	setupScummVars();

	setupOpcodes();

	if (_version == 8)
		_numActors = 80;
	else if ((_version == 7) || (_heversion == 72))
		_numActors = 30;
	else if (_gameId == GID_SAMNMAX)
		_numActors = 20;
	else if (_gameId == GID_MANIAC)
		_numActors = 25;
	else if (_heversion >= 80)
		_numActors = 63;
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

	// Since MM NES substitutes whole file class we get monster.sou file
	// name badly generated, so avoid even attempts to open it
	if (!(_platform == Common::kPlatformNES))
		_sound->setupSound();

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
		if (_gameId == GID_MANIAC) {
			if (_platform == Common::kPlatformNES)
				setupNESPalette();
			else
				setupV1ManiacPalette();
		} else
			setupV1ZakPalette();
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
}

void ScummEngine_v6::scummInit() {
	ScummEngine::scummInit();

	if (_gameId == GID_TENTACLE && res.roomno[rtRoom][60]) {
		// HACK: For DOTT we manually set the default cursor. See also bug #786994
		setCursorFromImg(697, 60, 1);
		setCursorTransparency(1);
	}
}

void ScummEngine_v60he::scummInit() {
	ScummEngine::scummInit();

	// HACK cursor hotspot is wrong
	// Original games used 
	// setCursorHotspot(8, 7);
	if (_gameId == GID_FUNPACK)
		setCursorHotspot(16, 16);
}

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

	spritesResetTables(0);
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

		default:
			_logicHE = new LogicHE(this);
			break;
		}
	}
}

void ScummEngine_v99he::scummInit() {
	ScummEngine_v90he::scummInit();

	_hePalettes = (uint8 *)malloc((_numPalettes + 1) * 1024);

	// Array 129 is set to base name
	int len = resStrLen((const byte *)_gameName.c_str()) + 1;
	ArrayHeader *ah = defineArray(129, kStringArray, 0, 0, 0, len);
	memcpy(ah->data, _gameName.c_str(), len);

}

void ScummEngine::setupMusic(int midi) {
	_midiDriver = MidiDriver::detectMusicDriver(midi);
	_native_mt32 = (ConfMan.getBool("native_mt32") || (_midiDriver == MD_MT32));
	// FIXME: MD_TOWNS should not be _midi_native in the first place!! iMuse code needs to be restructured.
	if ((_gameId == GID_TENTACLE) || (_gameId == GID_SAMNMAX) || (_midiDriver == MD_TOWNS))
		_enable_gs = false;
	else 
		_enable_gs = ConfMan.getBool("enable_gs");

#ifndef __GP32__ //ph0x FIXME, "quick dirty hack"
	/* Bind the mixer to the system => mixer will be invoked
	 * automatically when samples need to be generated */
	if (!_mixer->isReady()) {
		warning("Sound mixer initialization failed");
		if (_midiDriver == MD_ADLIB ||
				_midiDriver == MD_PCSPK ||
				_midiDriver == MD_PCJR)	{
			_midiDriver = MD_NULL;
			warning("MIDI driver depends on sound mixer, switching to null MIDI driver");
		}
	}

	// Init iMuse
	if (_features & GF_DIGI_IMUSE) {
		_musicEngine = _imuseDigital = new IMuseDigital(this, 10);
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

#endif // ph0x-hack
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
	// If requested, load a save game instead of running the boot script
	if (_saveLoadFlag != 2 || !loadState(_saveLoadSlot, _saveTemporaryState)) {
		int args[16];
		memset(args, 0, sizeof(args));
		args[0] = _bootParam;	

		_saveLoadFlag = 0;
		if (_heversion >= 98) {
			((ScummEngine_v90he *)this)->_logicHE->initOnce();
			((ScummEngine_v90he *)this)->_logicHE->beforeBootScript();
		}
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

	if (_heversion >= 98) {
		((ScummEngine_v90he *)this)->_logicHE->startOfFrame();
	}

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
	if (_version <= 2) {
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
	} else if (_musicEngine && VAR_MUSIC_TIMER != 0xFF) {
		// The music engine generates the timer data for us.
		VAR(VAR_MUSIC_TIMER) = _musicEngine->getMusicTimer();
	} else if (_features & GF_SMALL_HEADER) {
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
		_completeScreenRedraw = false;
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

		_fullRedraw = true;
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

		if (_heversion >= 90) {
			((ScummEngine_v90he *)this)->spritesBlitToScreen();
			((ScummEngine_v90he *)this)->spritesSortActiveSprites();
		}
	
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

		// Handle mouse over effects (for verbs).
		handleMouseOver(oldEgo != VAR(VAR_EGO));

		// Render everything to the screen.
		drawDirtyScreenParts();

		if (_version <= 5)
			playActorSounds();
	}

	_sound->processSoundQues();

	if (_imuseDigital) {
		_imuseDigital->flushTracks();
		if ( ((_gameId == GID_DIG) && (!(_features & GF_DEMO))) || (_gameId == GID_CMI) )
			_imuseDigital->refreshScripts();
	}

	camera._last = camera._cur;

	if (!(++res._expireCounter)) {
		res.increaseResourceCounter();
	}

	animateCursor();
	
	/* show or hide mouse */
	_system->showMouse(_cursor.state > 0);

	if (_heversion >= 90) {
		((ScummEngine_v90he *)this)->spritesUpdateImages();
	}
	if (_heversion >= 98) {
		((ScummEngine_v90he *)this)->_logicHE->endOfFrame();
	}

	if (VAR_TIMER != 0xFF)
		VAR(VAR_TIMER) = 0;
	return VAR(VAR_TIMER_NEXT);

}

#pragma mark -
#pragma mark --- SCUMM ---
#pragma mark -

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
	warning("stub startManiac()");
	displayMessage(0, "Usually, Maniac Mansion would start now. But ScummVM doesn't do that yet. To play it, go to 'Add Game' in the ScummVM start menu and select the 'Maniac' directory inside the Tentacle game directory.");
}

#pragma mark -
#pragma mark --- GUI ---
#pragma mark -

int ScummEngine::runDialog(Dialog &dialog) {
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

int ScummEngine::generateSubstResFileName(const char *filename, char *buf, int bufsize) {
	return generateSubstResFileName_(filename, buf, bufsize, _substResFileNameIndex);
}


} // End of namespace Scumm

using namespace Scumm;

GameList Engine_SCUMM_gameList() {
	const ScummGameSettings *g = scumm_settings;
	GameList games;
	while (g->name) {
		games.push_back(g->toGameSettings());
		g++;
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
		const char *base = g->baseFilename ? g->baseFilename : g->name;
		detectName[0] = '\0';

		if (g->detectFilename) {
			strcpy(detectName, g->detectFilename);
		} else if (g->version <= 3) {
			strcpy(detectName, "00.LFL");
		} else if (g->version == 4) {
			strcpy(detectName, "000.LFL");
		} else if (g->version >= 7) {
			strcpy(detectName, base);
			strcat(detectName, ".la0");
		} else if (g->heversion >= 60) {
			strcpy(detectName, base);
			strcat(detectName, ".he0");
		} else if (g->platform == Common::kPlatformNES) {
			strcpy(detectName, base);
		} else {
			strcpy(detectName, base);
			strcat(detectName, ".000");
		}
		strcpy(tempName, detectName);

		substLastIndex = 0;

		while (substLastIndex != -1) {
			// Iterate over all files in the given directory
			for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
				if (!file->isDirectory()) {
					const char *name = file->displayName().c_str();

					if (0 == scumm_stricmp(detectName, name)) {
						if (g->version <= 4) {
							// We take a look at the file now, to narrow
							// down the list of possible candidates a bit further.
							// E.g. it's trivial to distinguish V1 from V3 games.
							File tmp;
							byte buf[6];
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

	const char *ext = strrchr(filename, '.');
	int len = ext - filename;

	for (int i = index; i < ARRAYSIZE(substResFileNameTable); i++) {
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
				snprintf(buf, bufsize, "%s%s", substResFileNameTable[i].macName, ext);
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

	if (g->detectFilename) {
		strcpy(detectName, game.detectFilename);
	} else if (g->version <= 3) {
		strcpy(detectName, "00.LFL");
	} else if (g->version == 4) {
		strcpy(detectName, "000.LFL");
	} else if (g->version >= 7) {
		strcpy(detectName, name);
		strcat(detectName, ".la0");
	} else if (g->heversion >= 60) {
		strcpy(detectName, name);
		strcat(detectName, ".he0");
	} else {
		strcpy(detectName, name);
		strcat(detectName, ".000");
	}
	strcpy(tempName, detectName);

	bool found = false;
	while (substLastIndex != -1) {
		if (File::exists(detectName, ConfMan.get("path").c_str())) {
			found = true;
			break;
		}

		substLastIndex = generateSubstResFileName_(tempName, detectName, sizeof(detectName), substLastIndex + 1);
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

	// Use MD5 to determine specific game version, if required.
	if (game.features & GF_MULTIPLE_VERSIONS) {	
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

	// Special cases for HE games
	// Games starting freddi use 640x480
	if (game.heversion >= 71) {
		game.features |= GF_DEFAULT_TO_1X_SCALER;
	}

	switch (game.version) {
	case 1:
	case 2:
		engine = new ScummEngine_v2(detector, syst, game, md5sum);
		break;
	case 3:
		if (game.features & GF_OLD_BUNDLE)
			engine = new ScummEngine_v3old(detector, syst, game, md5sum);
		else
			engine = new ScummEngine_v3(detector, syst, game, md5sum);
		break;
	case 4:
		engine = new ScummEngine_v4(detector, syst, game, md5sum);
		break;
	case 5:
		engine = new ScummEngine_v5(detector, syst, game, md5sum);
		break;
	case 6:
		switch (game.heversion) {
#ifndef __PALM_OS__
		case 100:
			engine = new ScummEngine_v100he(detector, syst, game, md5sum);
			break;
		case 99:
			engine = new ScummEngine_v99he(detector, syst, game, md5sum);
			break;
		case 98:
		case 95:
		case 90:
			engine = new ScummEngine_v90he(detector, syst, game, md5sum);
			break;
		case 80:
			engine = new ScummEngine_v80he(detector, syst, game, md5sum);
			break;
		case 72:
			engine = new ScummEngine_v72he(detector, syst, game, md5sum);
			break;
		case 71:
			engine = new ScummEngine_v71he(detector, syst, game, md5sum);
			break;
		case 70:
			engine = new ScummEngine_v70he(detector, syst, game, md5sum);
			break;
#endif
		case 61:
			engine = new ScummEngine_v60he(detector, syst, game, md5sum);
			break;
		default:
			engine = new ScummEngine_v6(detector, syst, game, md5sum);
		}
		break;
	case 7:
		engine = new ScummEngine_v7(detector, syst, game, md5sum);
		break;
#ifndef __PALM_OS__
	case 8:
		engine = new ScummEngine_v8(detector, syst, game, md5sum);
		break;
#endif
	default:
		error("Engine_SCUMM_create(): Unknown version of game engine");
	}

	// FIXME: dirty HACK. Should we introduce another parameter to constructor
	// instead?
	((ScummEngine *)engine)->_substResFileNameIndex = substLastIndex;

	return engine;
}

REGISTER_PLUGIN(SCUMM, "Scumm Engine")

#ifdef __PALM_OS__
#include "scumm_globals.h"

_GINIT(Scumm_md5table)
_GSETPTR(md5table, GBVARS_MD5TABLE_INDEX, MD5Table, GBVARS_SCUMM)
_GEND

_GRELEASE(Scumm_md5table)
_GRELEASEPTR(GBVARS_MD5TABLE_INDEX, GBVARS_SCUMM)
_GEND

#endif
