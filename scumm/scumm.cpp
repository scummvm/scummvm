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

#include "scumm/actor.h"
#include "scumm/akos.h"
#include "scumm/boxes.h"
#include "scumm/charset.h"
#include "scumm/costume.h"
#include "scumm/debugger.h"
#include "scumm/dialogs.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse.h"
#include "scumm/insane/insane.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/player_v1.h"
#include "scumm/player_v2.h"
#include "scumm/player_v2a.h"
#include "scumm/player_v3a.h"
#include "scumm/resource.h"
#include "scumm/resource_v7he.h"
#include "scumm/scumm.h"
#include "scumm/scumm-md5.h"
#include "scumm/sound.h"
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

static int generateMacFileName_(const char *filename, char *buf, int bufsize, int cont = 0, int index = 0);

namespace Scumm {

// Use g_scumm from error() ONLY
ScummEngine *g_scumm = 0;

struct ScummGameSettings {
	const char *name;
	const char *description;
	byte id, version, heversion, numActors;
	int midi; // MidiDriverType values
	uint32 features;
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

	{"maniac", "Maniac Mansion", GID_MANIAC, 2, 0, 25, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, 0, 0},
	//{"maniacnes", "Maniac Mansion (NES)", GID_MANIAC, 2, 0, 25, MDT_NONE,
	// GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING | GF_NES, 0, 0},
	{"zak",         "Zak McKracken and the Alien Mindbenders", GID_ZAK, 2, 0, 13, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, 0, 0},

	/* Scumm Version 3 */
	{"indy3EGA", "Indiana Jones and the Last Crusade", GID_INDY3, 3, 0, 13, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALING | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE, 0, 0},
	{"indy3Towns", "Indiana Jones and the Last Crusade (FM-TOWNS)", GID_INDY3, 3, 0, 13, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS | GF_FMTOWNS | GF_AUDIOTRACKS, 0, 0},
	{"indy3", "Indiana Jones and the Last Crusade (256)", GID_INDY3, 3, 0, 13, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS, 0, 0},

	{"zakTowns", "Zak McKracken and the Alien Mindbenders (FM-TOWNS)", GID_ZAK256, 3, 0, 13, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALING | GF_OLD256 | GF_FMTOWNS | GF_AUDIOTRACKS, 0, 0},
	{"loom", "Loom", GID_LOOM, 3, 0, 13, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALING | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE, 0, 0},
	{"loomTowns", "Loom (FM Towns)", GID_LOOM, 3, 0, 13, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALING | GF_OLD256 | GF_FMTOWNS | GF_AUDIOTRACKS, 0, 0},

	/* Scumm Version 4 */
	{"monkeyEGA", "Monkey Island 1 (EGA)", GID_MONKEY_EGA, 4, 0, 13, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, 0, 0},
	{"pass", "Passport to Adventure", GID_PASS, 4, 0, 13, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, 0, 0},

	/* Scumm version 5 */
	{"monkeyVGA", "Monkey Island 1 (256 color Floppy version)", GID_MONKEY_VGA, 4, 0, 13, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY, 0, 0},
	{"loomcd", "Loom (256 color CD version)", GID_LOOM256, 4, 0, 13, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_AUDIOTRACKS, 0, 0},
	{"monkey", "Monkey Island 1", GID_MONKEY, 5, 0, 13, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, 0, 0},
	{"monkey1", "Monkey Island 1 (alt)", GID_MONKEY, 5, 0, 13, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_AUDIOTRACKS, 0, 0},
	{"game", "Monkey Island 1 (SegaCD version)", GID_MONKEY_SEGA, 5, 0, 13, MDT_NONE,
	 GF_USE_KEY | GF_AUDIOTRACKS, 0, 0},
	{"monkey2", "Monkey Island 2: LeChuck's revenge", GID_MONKEY2, 5, 0, 13, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, 0, 0},
	{"mi2demo", "Monkey Island 2: LeChuck's revenge (Demo)", GID_MONKEY2, 5, 0, 13, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, 0, 0},

	{"atlantis", "Indiana Jones and the Fate of Atlantis", GID_INDY4, 5, 0, 13, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, 0, 0},
	{"playfate", "Indiana Jones and the Fate of Atlantis (Demo)", GID_INDY4, 5, 0, 13, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, 0, 0},
	{"fate", "Indiana Jones and the Fate of Atlantis (Demo)", GID_INDY4, 5, 0, 13, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, 0, 0},
	{"indy4", "Indiana Jones and the Fate of Atlantis (FM-TOWNS)", GID_INDY4, 5, 0, 13, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, 0, 0},
	{"indydemo", "Indiana Jones and the Fate of Atlantis (FM-TOWNS Demo)", GID_INDY4, 5, 0, 13, MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, 0, 0},

	/* Scumm Version 6 */
	{"tentacle", "Day Of The Tentacle", GID_TENTACLE, 6, 0, 13, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY, 0, 0},
	{"tentacleMac", "Day Of The Tentacle", GID_TENTACLE, 6, 0, 13, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY, "tentacle", "Day Of The Tentacle Data"},
	{"dottdemo", "Day Of The Tentacle (Demo)", GID_TENTACLE, 6, 0, 13, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY, 0, 0},

	{"samnmax", "Sam & Max", GID_SAMNMAX, 6, 0, 30, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_DRAWOBJ_OTHER_ORDER, 0, 0},
	{"samnmax-alt", "Sam & Max (alt)", GID_SAMNMAX, 6, 0, 30, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_DRAWOBJ_OTHER_ORDER, "samnmax", "samnmax.sm0"},
	{"samnmaxMac", "Sam & Max (Mac)", GID_SAMNMAX, 6, 0, 30, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_DRAWOBJ_OTHER_ORDER, "samnmax", "Sam & Max Data"},
	{"samdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, 0, 30, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_DRAWOBJ_OTHER_ORDER, 0, 0},
	{"samdemoMac", "Sam & Max (Mac Demo)", GID_SAMNMAX, 6, 0, 30, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_DRAWOBJ_OTHER_ORDER, "samdemo", "Sam & Max Demo Data"},
	{"snmdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, 0, 30, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_DRAWOBJ_OTHER_ORDER, 0, "snmdemo.sm0"},
	{"snmidemo", "Sam & Max (Interactive WIP Demo)", GID_SAMNMAX, 6, 0, 30, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_DRAWOBJ_OTHER_ORDER, 0, 0},

//	{"test", "Test demo game", GID_SAMNMAX, 6, 0, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE, GF_NEW_OPCODES, 0, 0},

	/* Scumm Version 7 */
	{"ft", "Full Throttle", GID_FT, 7, 0, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, 0, 0},
	{"ftMac", "Full Throttle (Mac)", GID_FT, 7, 0, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, "ft", "Full Throttle Data"},
	{"ftdemo", "Full Throttle (Mac Demo)", GID_FT, 7, 0, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, 0, "Full Throttle Demo Data"},
	{"ftpcdemo", "Full Throttle (PC Demo)", GID_FT, 7, 0, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, "ft", 0},


	{"dig", "The Dig", GID_DIG, 7, 0, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, 0, 0},
	{"digMac", "The Dig (Mac)", GID_DIG, 7, 0, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, "dig", "The Dig Data"},
	{"digdemo", "The Dig (Demo)", GID_DIG, 7, 0, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, "dig", 0},
	{"digdemoMac", "The Dig (Mac Demo)", GID_DIG, 7, 0, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, "dig", "The Dig Demo Data"},

#ifndef __PALM_OS__
	/* Scumm Version 8 */
	{"comi", "The Curse of Monkey Island", GID_CMI, 8, 0, 80, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEFAULT_TO_1X_SCALER, 0, 0},
	{"comidemo", "The Curse of Monkey Island (Demo)", GID_CMI, 8, 0, 80, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEFAULT_TO_1X_SCALER | GF_DEMO, "comi", 0},
#endif

	// Humongous Entertainment Scumm Version 6
	{"puttputt", "Putt-Putt Joins The Parade", GID_HEGAME, 6, 60, 13, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"puttdemo", "Putt-Putt Joins The Parade (Demo)", GID_PUTTDEMO, 6, 0, 13, MDT_ADLIB | MDT_NATIVE,
	  GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_MULTIPLE_VERSIONS, 0, 0},
	{"moondemo", "Putt-Putt Goes To The Moon (Demo)", GID_HEGAME, 6, 60, 13, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"puttmoon", "Putt-Putt Goes To The Moon", GID_PUTTMOON, 6, 60, 13, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"funpack", "Putt-Putt's Fun Pack", GID_FUNPACK, 6, 60, 13, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"fbpack", "Fatty Bear's Fun Pack", GID_HEGAME, 6, 60, 13, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"fbear", "Fatty Bear's Birthday Surprise", GID_FBEAR, 6, 60, 13, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"fbdemo", "Fatty Bear's Birthday Surprise (DOS Demo)", GID_FBEAR, 6, 60, 13, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},

#ifndef __PALM_OS__
	// Humongous Entertainment Scumm Version 7.1
	// The first version to use 640x480 resolution
	// There are also 7.1 versions of freddi, freddemo, airdemo and farmdemo
	{"catalog", "Humongous Interactive Catalog", GID_HEGAME, 6, 71, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},

	// Humongous Entertainment Scumm Version 7.2
	{"catalog2", "Humongous Interactive Catalog 2", GID_HEGAME, 6, 72, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"freddemo", "Freddi Fish 1: The Case of the Missing Kelp Seeds (Demo)", GID_HEGAME, 6, 72, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"farmdemo", "Let's Explore the Farm with Buzzy (Demo)", GID_HEGAME, 6, 72, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"farm", "Let's Explore the Farm with Buzzy", GID_HEGAME, 6, 72, 28, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"airdemo", "Let's Explore the Airport with Buzzy (Demo)", GID_HEGAME, 6, 72, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"airport", "Let's Explore the Airport with Buzzy", GID_HEGAME, 6, 72, 28, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"jungle", "Let's Explore the Jungle with Buzzy", GID_HEGAME, 6, 72, 28, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},

	{"puttzoo", "Putt-Putt Saves the Zoo", GID_HEGAME, 6, 72, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"zoodemo", "Putt-Putt Saves the Zoo (Demo)", GID_HEGAME, 6, 72, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},

	// Humongous Entertainment Scumm Version 8.0 ?  Scummsrc.80 
	{"ff2-demo", "Freddi Fish 2: The Case of the Haunted Schoolhouse (Demo)", GID_HEGAME, 6, 80, 30, MDT_NONE,
	GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"freddi2", "Freddi Fish 2: The Case of the Haunted Schoolhouse", GID_HEGAME, 6, 80, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"pjs-demo", "Pajama Sam 1: No Need to Hide When It's Dark Outside (Demo)", GID_HEGAME, 6, 80, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"pajama", "Pajama Sam 1: No Need to Hide When It's Dark Outside", GID_HEGAME, 6, 80, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},

	{"dog", "Putt-Putt and Pep's Dog on a Stick", GID_HEGAME, 6, 80, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"balloon", "Putt-Putt and Pep's Balloon-O-Rama", GID_HEGAME, 6, 80, 60, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"maze", "Freddi Fish and Luther's Maze Madness", GID_HEGAME, 6, 80, 60, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"socks", "Pajama Sam's Sock Works", GID_HEGAME, 6, 80, 56, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"water", "Freddi Fish and Luther's Water Worries", GID_HEGAME, 6, 80, 56, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},

	// Humongous Entertainment Scumm Version 9.0 ?  Scummsys.90
	{"freddi", "Freddi Fish 1: The Case of the Missing Kelp Seeds", GID_HEGAME, 6, 90, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"kinddemo", "Big Thinkers Kindergarten (Demo)", GID_HEGAME, 6, 90, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"thinkerk", "Big Thinkers Kindergarten", GID_HEGAME, 6, 90, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"1grademo", "Big Thinkers First Grade (Demo)", GID_HEGAME, 6, 90, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"freddi3", "Freddi Fish 3: The Case of the Stolen Conch Shell", GID_HEGAME, 6, 90, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"f3-mdemo", "Freddi Fish 3: The Case of the Stolen Conch Shell (Demo)", GID_HEGAME, 6, 90, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	// there is also a C++ engine based version of timedemo
	{"TimeDemo", "Putt-Putt Travels Through Time (Demo)", GID_HEGAME, 6, 90, 31, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"spyfox", "Spyfox 1: Dry Cereal", GID_HEGAME, 6, 90, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"foxdemo", "Spyfox 1: Dry Cereal (Demo)", GID_HEGAME, 6, 90, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	// there is also a C++ engine version of spydemo
	{"Spydemo", "Spyfox 1: Dry Cereal (Demo)", GID_HEGAME, 6, 90, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},

	// Humongous Entertainment Scumm Version 9.5 ?  Scummsys.95
	{"pj2demo", "Pajama Sam 2: Thunder and Lightning Aren't so Frightening (Demo)", GID_HEGAME, 6, 90, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"pajama2", "Pajama Sam 2: Thunder and Lightning Aren't so Frightening", GID_HEGAME, 6, 90, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"chase", "Spy Fox in Cheese Chase Game", GID_HEGAME, 6, 90, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},

	// Humongous Entertainment Scumm Version 9.8 ?  Scummsys.98
	// these and later games can easily be identified by the .(a) file instead of a .he1
	// and INIB chunk in the .he0
	{"lost", "Pajama Sam's Lost & Found", GID_HEGAME, 6, 98, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"racedemo", "Putt-Putt Enters the Race (Demo)", GID_HEGAME, 6, 98, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"puttrace", "Putt-Putt Enters the Race", GID_HEGAME, 6, 98, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"BluesABCTimeDemo", "Blue's ABC Time (Demo)", GID_HEGAME, 6, 98, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},

	// Global scripts increased to 2048
	{"500demo", "Putt-Putt Enters the Race (Demo)", GID_FREDDI4, 6, 98, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"freddi4", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch", GID_FREDDI4, 6, 98, 57, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"f4-demo", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch (Demo)", GID_FREDDI4, 6, 98, 57, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},

	// Engine moved to c++ 
	// Humongous Entertainment Scumm Version 9.9 ?  Scummsys.99
	{"smaller", "Pajama Sam's Lost & Found (Test)", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"puttcircus", "Putt-Putt Joins the Circus", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"circdemo", "Putt-Putt Joins the Circus (Demo)", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"sf2-demo", "Spyfox 2: Some Assembly Required (Demo)", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"spyfox2", "Spyfox 2: Some Assembly Required", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"mustard", "Spy Fox in Hold the Mustard", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},

	// Humongous Entertainment Scumm Version ?
	{"freddicove", "Freddi Fish 5: The Case of the Creature of Coral Cave", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_MULTIPLE_VERSIONS, 0, 0},
	{"pj3-demo", "Pajama Sam 3: You Are What You Eat From Your Head to Your Feet (Demo)", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"pajama3", "Pajama Sam 3: You Are What You Eat From Your Head to Your Feet", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"SamsFunShop", "Pajama Sam's One-Stop Fun Shop", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"PuttsFunShop", "Putt-Putt's One-Stop Fun Shop", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"putttime", "Putt-Putt Travels Through Time", GID_HEGAME, 6, 99, 31, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"spyozon", "Spyfox 3: Operation Ozone", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"sf3-demo", "Spyfox 3: Operation Ozone (Demo)", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},

	{"bb2demo", "Backyard Baseball 2001 (Demo)", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"footdemo", "Backyard Football 2002 (Demo)", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},

	// Humongous Entertainment Scumm Version ?
	{"Football2002", "Backyard Football 2002 (Demo)", GID_HEGAME, 6, 100, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"FFHSDemo", "Freddi Fish 2: The Case of the Haunted Schoolhouse (Demo)", GID_HEGAME, 6, 100, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"ff5demo", "Freddi Fish 5: The Case of the Creature of Coral Cave (Demo)", GID_HEGAME, 6, 100, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"PjSamDemo", "Pajama Sam: No Need To Hide When It's Dark Outside (Demo)", GID_HEGAME, 6, 100, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"PajamaNHD", "Pajama Sam: No Need To Hide When It's Dark Outside (Updated)", GID_HEGAME, 6, 100, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"PuttTTT", "Putt-Putt Travels Through Time *updated)", GID_HEGAME, 6, 100, 31, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},


#endif
	{NULL, NULL, 0, 0, 0, 0, MDT_NONE, 0, 0, 0}
};

// This additional table is used for titles where GF_MULTIPLE_VERSIONS is specified.
// Now these are HE games. Some of them were released for different versions of SPUTM,
// and instead of multiplying GIDs this table is used.
//
// Use main table to specify default flags and this table to override defaults.
static const ScummGameSettings he_md5_settings[] = {
	{"0b3222aaa7efcf283eb621e0cefd26cc", "Putt-Putt Joins The Parade (early version)", GID_HEGAME, 6, 0, 13, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS, 0, 0},
	{"6a30a07f353a75cdc602db27d73e1b42", "Putt-Putt Joins The Parade (Windows)", GID_HEGAME, 6, 70, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_WINDOWS, 0, 0},
	{"37ff1b308999c4cca7319edfcc1280a0", "Putt-Putt Joins The Parade (Windows Demo)", GID_PUTTDEMO, 6, 70, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"9c92eeaf517a31b7221ec2546ab669fd", "Putt-Putt Goes To The Moon (Windows)", GID_HEGAME, 6, 70, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_WINDOWS, 0, 0},
	{"9c143c5905055d5df7a0f014ab379aee", "Putt-Putt Goes To The Moon (Windows Demo)", GID_HEGAME, 6, 70, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_WINDOWS, 0, 0},
	{"179879b6e35c1ead0d93aab26db0951b", "Fatty Bear's Birthday Surprise (Windows)", GID_FBEAR, 6, 70, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_WINDOWS, 0, 0},
	{"22c9eb04455440131ffc157aeb8d40a8", "Fatty Bear's Birthday Surprise (Windows Demo)", GID_FBEAR, 6, 70, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES | GF_WINDOWS, 0, 0},
	{"d4cccb5af88f3e77f370896e9ba8c5f9", "Freddi Fish 1: The Case of the Missing Kelp Seeds", GID_HEGAME, 6, 71, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"084ed0fa98a6d1e9368d67fe9cfbd417", "Freddi Fish 1: The Case of the Missing Kelp Seeds (Demo) (puttputt cd)", GID_HEGAME, 6, 71, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"86c9902b7bec1a17926d4dae85beaa45", "Let's Explore the Airport with Buzzy (Demo) (puttputt cd)", GID_HEGAME, 6, 71, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"bf8b52fdd9a69c67f34e8e9fec72661c", "Let's Explore the Farm with Buzzy (Demo) (puttputt cd)", GID_HEGAME, 6, 71, 13, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"92e7727e67f5cd979d8a1070e4eb8cb3", "Putt-Putt Saves the Zoo *updated)", GID_FREDDI4, 6, 98, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"51305e929e330e24a75a0351c8f9975e", "Freddi Fish 2: The Case of the Haunted Schoolhouse (Updated)", GID_HEGAME, 6, 99, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"21abe302e1b1e2b66d6f5c12e241ebfd", "Freddi Fish 5: The Case of the Creature of Coral Cave (Unencrypted)", GID_HEGAME, 6, 99, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"a561d2e2413cc1c71d5a1bf87bf493ea", "Pajama Sam's Lost & Found (Updated)", GID_HEGAME, 6, 100, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"0ab19be9e2a3f6938226638b2a3744fe", "Putt-Putt Travels Through Time (Updated Demo)", GID_HEGAME, 6, 100, 31, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"8e3241ddd6c8dadf64305e8740d45e13", "Putt-Putt and Pep's Balloon-O-Rama (Updated)", GID_HEGAME, 6, 100, 60, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"3de99ef0523f8ca7958faa3afccd035a", "Spyfox 1: Dry Cereal (Updated)", GID_HEGAME, 6, 100, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"9d4ab3e0e1d1ebc6ba8a6a4c470ed184", "Spyfox 1: Dry Cereal (Updated Demo)", GID_HEGAME, 6, 100, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"90c755e1c9b9b8a4129d37b2259d0655", "Spy Fox in Cheese Chase Game (Updated)", GID_HEGAME, 6, 100, 61, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},
	{"d4b8ee426b1afd3e53bc0cf020418cf6", "Putt-Putt and Pep's Dog on a Stick (Updated)", GID_HEGAME, 6, 98, 30, MDT_NONE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0, 0},

	{NULL, NULL, 0, 0, 0, MDT_NONE, 0, 0, 0, 0}
};

static struct heMacFileNames {
	const char *winName;
	const char *macName;
	bool hasParens;
} heMacFileNameTable[] = {
	{ "Intentionally/left/blank", "", false},
	{ "airdemo", "Airport Demo", true},
	{ "balloon", "Balloon-O-Rama", true},
	{ "chase", "Cheese Chase", true},
	{ "circdemo", "Putt Circus Demo", true},
	{ "dog", "Dog on a Stick", true},
	{ "f3-mdemo", "F3-Mdemo", true },
	{ "f4-demo", "Freddi 4 Demo", true },
	{ "farmdemo", "Farm Demo", true},
	{ "fbear", "Fatty Bear", false },
	{ "fbdemo", "Fatty Bear Demo", false },
	{ "ff2-demo", "Freddi Fish 2 Demo", true },
	{ "ffhsdemo", "FFHSDemo", true },
	{ "footdemo", "FootBall Demo", true },
	{ "foxdemo", "FoxDemo", true },
	{ "freddemo", "Freddi Demo", true },
	{ "freddi", "Freddi Fish", true },
	{ "freddi2", "Freddi Fish 2", true },
	{ "freddi3", "Freddi Fish 3", true },
	{ "freddi4", "Freddi 4", true },
	{ "freddicove", "FreddiCove", true },
	{ "kinddemo", "Kinddemo", true },
	{ "lost", "Lost and Found", true},
	{ "maze", "Maze Madness", true},
	{ "moondemo", "Putt-Putt Moon Demo", false },
	{ "mustard", "Mustard", true},
	{ "pajama", "Pajama Sam", true},
	{ "pajama2", "Pajama Sam 2", true},
	{ "pj2demo", "PJ2Demo", true },
	{ "pj3-demo", "Pajama Sam 3-Demo", true },
	{ "pjs-demo", "Pajama Sam Demo", true },
	{ "pjsamdemo", "PjSamDemo", true },
	{ "circdemo", "Putt Circus Demo", true },
	{ "puttdemo", "Putt-Putt's Demo", false },
	{ "puttmoon", "Putt-Putt Moon", false },
	{ "puttputt", "Putt-Putt Parade", false },
	{ "PuttsFunShop", "Putt's FunShop", true },
	{ "putttime-demo", "TimeDemo", true },
	{ "puttzoo", "PuttZoo", true },
	{ "racedemo", "RaceDemo", true },
	{ "SamsFunShop", "Sam's FunShop", true },
	{ "sf2-demo", "Spy Fox 2 - Demo", true },
	{ "sf3-demo", "Spy Ozone Demo", true },
	{ "socks", "SockWorks", true },
	{ "spyfox", "SPYFox", true },
	{ "spyfox2", "Spy Fox 2", true },
	{ "spyozon", "SpyOzon", true },
	{ "thinkerk", "ThinkerK", true },
	{ "timedemo", "TimeDemo", true },
	{ "water", "Water Worries", true },
	{ "zoodemo", "Puttzoo Demo", true },
	{ "zoodemo", "Zoo Demo", true}
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
	  _numActors(gs.numActors),
	  _features(gs.features),
	  gdi(this), _pauseDialog(0), _optionsDialog(0), _mainMenuDialog(0), _versionDialog(0),
	  _targetName(detector->_targetName) {

	// Copy MD5 checksum
	memcpy(_gameMD5, md5sum, 16);

	// Check for unknown MD5
	char md5str[32+1];
	for (int j = 0; j < 16; j++) {
		sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
	}
	const MD5Table *elem;
	elem = (const MD5Table *)bsearch(md5str, md5table, ARRAYSIZE(md5table)-1, sizeof(MD5Table), compareMD5Table);

	if (!elem)
		printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);

	// Add default file directories.
	if (((_features & GF_AMIGA) || (_features & GF_ATARI_ST)) && (_version <= 4)) {
		// This is for the Amiga version of Indy3/Loom/Maniac/Zak
		File::addDefaultDirectory(_gameDataPath + "ROOMS/");
		File::addDefaultDirectory(_gameDataPath + "rooms/");
	}

	if ((_features & GF_MACINTOSH) && (_version == 3)) {
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

	// The mac versions of Sam&Max, DOTT, FT and The Dig used a special meta
	// (container) file format to store the actual SCUMM data files. The
	// rescumm utility used to be used to extract those files. While that is
	// still possible, we now support reading those files directly.
	// The first step is to check whether one of them is present (we do that
	// here); the rest is handled by the  ScummFile class and code in
	// openResourceFile() (and in the Sound class, for MONSTER.SOU handling).
	if (gs.detectFilename) {
		if (_fileHandle.open(gs.detectFilename)) {
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
	memset(&res, 0, sizeof(res));
	memset(&vm, 0, sizeof(vm));
	_smushVideoShouldFinish = false;
	_smushPaused = false;
	_insaneRunning = false;
	_quit = false;
	_pauseDialog = NULL;
	_optionsDialog = NULL;
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
	_audioNames = NULL;
	_numAudioNames = 0;
	_curActor = 0;
	_curVerb = 0;
	_curVerbSlot = 0;
	_curPalIndex = 0;
	_currentRoom = 0;
	_egoPositioned = false;
	_keyPressed = 0;
	_lastKeyHit = 0;
	_mouseButStat = 0;
	_leftBtnPressed = 0;
	_rightBtnPressed = 0;
	_bootParam = 0;
	_dumpScripts = false;
	_debugMode = 0;
	_objectOwnerTable = NULL;
	_objectRoomTable = NULL;
	_objectStateTable = NULL;
	_numObjectsInRoom = 0;
	_userPut = 0;
	_userState = 0;
	_resourceHeaderSize = 0;
	_saveLoadFlag = 0;
	_saveLoadSlot = 0;
	_lastSaveTime = 0;
	_saveTemporaryState = false;
	memset(_saveLoadName, 0, sizeof(_saveLoadName));
	_maxHeapThreshold = 0;
	_minHeapThreshold = 0;
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
	_dynamicRoomOffsets = false;
	memset(_resourceMapper, 0, sizeof(_resourceMapper));
	_allocatedSize = 0;
	_expire_counter = 0;
	_lastLoadedRoom = 0;
	_roomResource = 0;
	_heMacFileNameIndex = 0;
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
	_roomStrips = 0;
	_bompActorPalettePtr = NULL;
	_shakeEnabled= false;
	_shakeFrame = 0;
	_screenStartStrip = 0;
	_screenEndStrip = 0;
	_screenTop = 0;
	_blastObjectQueuePos = 0;
	memset(_blastObjectQueue, 0, sizeof(_blastObjectQueue));
	_blastTextQueuePos = 0;
	memset(_blastTextQueue, 0, sizeof(_blastTextQueue));
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
	_stringLength = 0;
	memset(_stringBuffer, 0, sizeof(_stringBuffer));
	_copyProtection = false;
	_demoMode = false;
	_confirmExit = false;
	_talkDelay = 0;
	_keepText = false;
	_existLanguageFile = false;
	_languageBuffer = NULL;
	_languageIndex = NULL;
	_costumeRenderer = NULL;
	_2byteFontPtr = 0;
	_V1TalkingActor = 0;

	_actorClipOverride.top = 0;
	_actorClipOverride.bottom = 480;
	_actorClipOverride.left = 0;
	_actorClipOverride.right = 640;

	_skipDrawObject = 0;
	_skipProcessActors = 0;
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

	VAR_NUM_SOUND_CHANNELS = 0xFF;
	VAR_MUSIC_CHANNEL = 0xFF;
	VAR_SOUND_CHANNEL = 0xFF;

	VAR_NUM_ROOMS = 0xFF;
	VAR_NUM_SCRIPTS = 0xFF;
	VAR_NUM_SOUNDS = 0xFF;
	VAR_NUM_COSTUMES = 0xFF;
	VAR_NUM_IMAGES = 0xFF;
	VAR_NUM_CHARSETS = 0xFF;
	VAR_NUM_GLOBAL_OBJS = 0xFF;
	VAR_NUM_SPRITES = 0xFF;
	VAR_NUM_PALETTES = 0xFF;
	VAR_NUM_UNK = 0xFF;
	VAR_POLYGONS_ONLY = 0xFF;
	VAR_WINDOWS_VERSION = 0xFF;
	VAR_KEY_STATE = 0xFF;
	VAR_WIZ_TCOLOR = 0xFF;

	// Use g_scumm from error() ONLY
	g_scumm = this;

	// Read settings from the detector & config manager
	_debugMode = ConfMan.hasKey("debuglevel");
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
	_confirmExit = ConfMan.getBool("confirm_exit");

	_hexdumpScripts = false;
	_showStack = false;

	// FIXME: a dirty hack. Currently this is checked before engine
	//  creation.
	if (_heversion >= 71) {
		_features |= GF_DEFAULT_TO_1X_SCALER;
	}

	if (_features & GF_FMTOWNS && _version == 3) {	// FM-TOWNS V3 games use 320x240
		_screenWidth = 320;
		_screenHeight = 240;
	} else if (_features & GF_DEFAULT_TO_1X_SCALER) {
		// FIXME: a dirty hack. Currently this is checked before engine
		//  creation.
		_system->setGraphicsMode("1x");
		_screenWidth = 640;
		_screenHeight = 480;
	} else if (_features & GF_NES) {
		_screenWidth = 256;
		_screenHeight = 240;
	} else {
		_screenWidth = 320;
		_screenHeight = 200;
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

	delete _2byteFontPtr;
	delete _charset;
	delete _pauseDialog;
	delete _optionsDialog;
	delete _mainMenuDialog;
	delete _versionDialog;

	delete _sound;
	free(_languageBuffer);
	free(_audioNames);

	delete _costumeRenderer;

	free(_shadowPalette);
	
	freeResources();

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

	free(_roomStrips);
	free(_languageIndex);

	delete _debugger;
}

ScummEngine_v6::ScummEngine_v6(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16]) 
 : ScummEngine(detector, syst, gs, md5sum) {
	VAR_VIDEONAME = 0xFF;
	VAR_RANDOM_NR = 0xFF;
	VAR_STRING2DRAW = 0xFF;

	VAR_TIMEDATE_YEAR = 0xFF;
	VAR_TIMEDATE_MONTH = 0xFF;
	VAR_TIMEDATE_DAY = 0xFF;
	VAR_TIMEDATE_HOUR = 0xFF;
	VAR_TIMEDATE_MINUTE = 0xFF;
	VAR_TIMEDATE_SECOND = 0xFF;

	_smushFrameRate = 0;
}

ScummEngine_v70he::ScummEngine_v70he(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16])
 : ScummEngine_v60he(detector, syst, gs, md5sum) {
	 _win32ResExtractor = new Win32ResExtractor(this);
	 _macResExtractor = new MacResExtractor(this);

	_heSndSoundId = 0;
	_heSndOffset = 0;
	_heSndChannel = 0;
	_heSndFlags = 0;
	_heSndSoundFreq = 0;
}

#pragma mark -
#pragma mark --- Initialization ---
#pragma mark -

int ScummEngine::init(GameDetector &detector) {

	// Initialize backend
	_system->beginGFXTransaction();
		initCommonGFX(detector);
		_system->initSize(_screenWidth, _screenHeight);

		// FIXME: All this seems a dirty hack to me. We already
		// have this check in constructor
		if (_heversion >= 71) {
			_features |= GF_DEFAULT_TO_1X_SCALER;
			_system->setGraphicsMode("1x");
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
	if (_version <= 2)
		_charset = new CharsetRendererV2(this, _language);
	else if (_version == 3)
		_charset = new CharsetRendererV3(this);
	else if (_version == 8)
		_charset = new CharsetRendererNut(this);
	else
		_charset = new CharsetRendererClassic(this);

	// Create the costume renderer
	if (_features & GF_NEW_COSTUMES)
		_costumeRenderer = new AkosRenderer(this);
	else
		_costumeRenderer = new CostumeRenderer(this);

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
		_maxHeapThreshold = gVars->memory[kMemScummNewCostGames];
	else
		_maxHeapThreshold = gVars->memory[kMemScummOldCostGames];
#else
	// Since the new costumes are very big, we increase the heap limit, to avoid having
	// to constantly reload stuff from the data files.
	if (_features & GF_NEW_COSTUMES)
		_maxHeapThreshold = 2500000;
	else
		_maxHeapThreshold = 550000;
#endif
	_minHeapThreshold = 400000;

	allocResTypeData(rtBuffer, MKID('NONE'), 10, "buffer", 0);

	setupScummVars();

	setupOpcodes();

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

	if (_features & GF_OLD_BUNDLE)
		_resourceHeaderSize = 4;
	else if (_features & GF_SMALL_HEADER)
		_resourceHeaderSize = 6;
	else
		_resourceHeaderSize = 8;

	readIndexFile();

	scummInit();
	initScummVars();

	if (VAR_DEBUGMODE != 0xFF) {
		// This is NOT for the Mac version of Indy3/Loom
		VAR(VAR_DEBUGMODE) = _debugMode;
		if (_heversion >= 80 && _debugMode)
			VAR(85) = 1;
	}

	if (_gameId == GID_MONKEY || _gameId == GID_MONKEY_SEGA)
		_scummVars[74] = 1225;

	if (_imuse) {
		_imuse->setBase(res.address[rtSound]);
	}
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

	if ((_gameId == GID_MANIAC) && (_version == 1)) {
		initScreens(16, 152);
	} else if (_version >= 7 || _heversion >= 71) {
		initScreens(0, _screenHeight);
	} else {
		initScreens(16, 144);
	}

	for (i = 0; i < 256; i++)
		_roomPalette[i] = i;
	if (_version == 1) {
		// Use 17 color table for v1 games to allow
		// correct color for inventory and sentence
		// line
		// Original games used some kind of dynamic
		// color table remapping between rooms
		if (_gameId == GID_MANIAC)
			setupV1ManiacPalette();
		else
			setupV1ZakPalette();
	} else if (_features & GF_16COLOR) {
		for (i = 0; i < 16; i++)
			_shadowPalette[i] = i;
		if ((_features & GF_AMIGA) || (_features & GF_ATARI_ST))
			setupAmigaPalette();
		else
			setupEGAPalette();
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
	for (i = 0; i < _numActors; i++) {
		_actors[i].number = i;
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
		_actors[9].talkColor = 15;
		_actors[10].talkColor = 7;
		_actors[11].talkColor = 2;
		_actors[13].talkColor = 5;
		_actors[23].talkColor = 14;
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
		camera._leftTrigger = 10;
		if (_heversion >= 71)
			camera._rightTrigger = 70;
		else
			camera._rightTrigger = 30;
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

	for (i = 0; i < 6; i++) {
		if (_version == 3) { // FIXME - what is this?
			_string[i]._default.xpos = 0;
			_string[i]._default.ypos = 0;
		} else {
			_string[i]._default.xpos = 2;
			_string[i]._default.ypos = 5;
		}
		_string[i]._default.right = _screenWidth - 1;
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

	initV2MouseOver();

	// Seems in V2 there was only a single room effect (iris),
	// so we set that here.
	_switchRoomEffect2 = 1;
	_switchRoomEffect = 5;
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

void ScummEngine_v90he::scummInit() {
	ScummEngine_v80he::scummInit();

	_heObject = 0;
	_heObjectNum = 0;
	_hePaletteNum = 0;

	memset(&_wizParams, 0, sizeof(_wizParams));
}

void ScummEngine::setupMusic(int midi) {
	_midiDriver = MidiDriver::detectMusicDriver(midi);
	_native_mt32 = (ConfMan.getBool("native_mt32") || (_midiDriver == MD_MT32));

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
	} else if ((_features & GF_AMIGA) && (_version == 2)) {
		_musicEngine = new Player_V2A(this);
	} else if ((_features & GF_AMIGA) && (_version == 3)) {
		_musicEngine = new Player_V3A(this);
	} else if ((_features & GF_AMIGA) && (_version < 5)) {
		_musicEngine = NULL;
	} else if (((_midiDriver == MD_PCJR) || (_midiDriver == MD_PCSPK)) && ((_version > 2) && (_version < 5))) {
		_musicEngine = new Player_V2(this, _midiDriver != MD_PCSPK);
	} else if (_version > 2 && _heversion <= 60) {
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
			if (_features & GF_HUMONGOUS || midi == MDT_TOWNS) {
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

	_mixer->setVolumeForSoundType(SoundMixer::kSFXAudioDataType, soundVolumeSfx);
	_mixer->setVolumeForSoundType(SoundMixer::kMusicAudioDataType, soundVolumeMusic);
	_mixer->setVolumeForSoundType(SoundMixer::kSpeechAudioDataType, soundVolumeSpeech);
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

			if (_features & GF_MACINTOSH)
				value = 105;
			else
 				value = (_gameId == GID_LOOM256) ? 150 : 100;
			byte restoreScript = (_features & GF_FMTOWNS) ? 17 : 18;
			// if verbs should be shown restore them
			if (VAR(value) == 2)
				runScript(restoreScript, 0, 0, args);
		} else if (_version > 3) {
			for (int i = 0; i < _numVerbs; i++)
				drawVerb(i, 0);
		} else {
			redrawVerbs();
		}

		verbMouseOver(0);

		if (_version <= 2) {
			redrawV2Inventory();
			checkV2MouseOver(_mouse);
		}

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
		fixObjectFlags();
		if (_version > 3)
			CHARSET_1();

		if (camera._cur.x != camera._last.x || _bgNeedsRedraw || _fullRedraw
				|| ((_features & GF_NEW_CAMERA) && camera._cur.y != camera._last.y)) {
			redrawBGAreas();
		}

		processDrawQue();

		// Full Throttle always redraws verbs and draws verbs before actors
		if (_version >= 7)
			redrawVerbs();
	
		setActorRedrawFlags();
		resetActorBgs();

		if (VAR_CURRENT_LIGHTS != 0xFF &&
		    !(VAR(VAR_CURRENT_LIGHTS) & LIGHTMODE_screen) &&
		      VAR(VAR_CURRENT_LIGHTS) & LIGHTMODE_flashlight) {
			drawFlashlight();
			setActorRedrawFlags();
		}
		if (_heversion >= 71) {
			preProcessAuxQueue();
			processActors();
			postProcessAuxQueue();
		} else {
			processActors();
		}
		
		_fullRedraw = false;
		if (_version >= 4 && _heversion <= 60)
			cyclePalette();
		palManipulate();
		if (_doEffect) {
			_doEffect = false;
			fadeIn(_newEffect);
			clearClickedStatus();
		}

		if (_cursor.state > 0)
			verbMouseOver(findVerbAtPos(_mouse.x, _mouse.y));

		if (_version <= 2) {
			if (oldEgo != VAR(VAR_EGO)) {
				// FIXME/TODO: Reset and redraw the sentence line
				oldEgo = VAR(VAR_EGO);
				_inventoryOffset = 0;
				redrawV2Inventory();
			}
			checkV2MouseOver(_mouse);
		}

		// For the Full Throttle credits to work properly, the blast
		// texts have to be drawn before the blast objects. Unless
		// someone can think of a better way to achieve this effect.

		if (_version >= 7 && VAR(VAR_BLAST_ABOVE_TEXT) == 1) {
			drawBlastTexts();
			drawBlastObjects();
		} else {
			drawBlastObjects();
			drawBlastTexts();
		}

		if (_version == 8)
			processUpperActors();
		drawDirtyScreenParts();
		removeBlastTexts();
		removeBlastObjects();

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

	if (!(++_expire_counter)) {
		increaseResourceCounter();
	}

	animateCursor();
	
	/* show or hide mouse */
	_system->showMouse(_cursor.state > 0);

	if (VAR_TIMER != 0xFF)
		VAR(VAR_TIMER) = 0;
	return VAR(VAR_TIMER_NEXT);

}

#pragma mark -
#pragma mark --- SCUMM ---
#pragma mark -

/**
 * Start a 'scene' by loading the specified room with the given main actor.
 * The actor is placed next to the object indicated by objectNr.
 */
void ScummEngine::startScene(int room, Actor *a, int objectNr) {
	int i, where;

	CHECK_HEAP;
	debugC(DEBUG_GENERAL, "Loading room %d", room);

	stopTalk();

	fadeOut(_switchRoomEffect2);
	_newEffect = _switchRoomEffect;

	ScriptSlot *ss =  &vm.slot[_currentScript];

	if (_currentScript != 0xFF) {
		if (ss->where == WIO_ROOM || ss->where == WIO_FLOBJECT) {
			if (ss->cutsceneOverride && _version >= 5)
				error("Object %d stopped with active cutscene/override in exit", ss->number);

			nukeArrays(ss->number);
			_currentScript = 0xFF;
		} else if (ss->where == WIO_LOCAL) {
			if (ss->cutsceneOverride && _version >= 5)
				error("Script %d stopped with active cutscene/override in exit", ss->number);

			nukeArrays(ss->number);
			_currentScript = 0xFF;
		}
	}

	if (!(_features & GF_SMALL_HEADER) && VAR_NEW_ROOM != 0xFF)  // Disable for SH games. Overwrites
		VAR(VAR_NEW_ROOM) = room; // gamevars, eg Zak cashcards

	runExitScript();

	killScriptsAndResources();
	clearEnqueue();
	if (_version >= 4 && _heversion <= 60)
		stopCycle(0);
	_sound->processSoundQues();

	if (_heversion >= 71) {
		((ScummEngine_v70he *)this)->_wiz.polygonClear();
	}

	// For HE80+ games
	for (i = 0; i < _numRoomVariables; i++)
		_roomVars[i] = 0;
	nukeArrays((byte)0xFFFFFFFF);

	for (i = 1; i < _numActors; i++) {
		_actors[i].hideActor();
	}

	if (_version >= 7) {
		// Set the shadow palette(s) to all black. This fixes
		// bug #795940, and actually makes some sense (after all,
		// shadows tend to be rather black, don't they? ;-)
		memset(_shadowPalette, 0, NUM_SHADOW_PALETTE * 256);
	} else {
		for (i = 0; i < 256; i++) {
			_roomPalette[i] = i;
			if (_shadowPalette)
				_shadowPalette[i] = i;
		}
		if (_features & GF_SMALL_HEADER)
			setDirtyColors(0, 255);
	}

	clearDrawObjectQueue();

	VAR(VAR_ROOM) = room;
	_fullRedraw = true;

	increaseResourceCounter();

	_currentRoom = room;
	VAR(VAR_ROOM) = room;

	if (room >= 0x80 && _version < 7)
		_roomResource = _resourceMapper[room & 0x7F];
	else
		_roomResource = room;

	if (VAR_ROOM_RESOURCE != 0xFF)
		VAR(VAR_ROOM_RESOURCE) = _roomResource;

	if (room != 0)
		ensureResourceLoaded(rtRoom, room);

	clearRoomObjects();

	if (_currentRoom == 0) {
		_ENCD_offs = _EXCD_offs = 0;
		_numObjectsInRoom = 0;
		return;
	}

	initRoomSubBlocks();

	if (_features & GF_OLD_BUNDLE)
		loadRoomObjectsOldBundle();
	else if (_features & GF_SMALL_HEADER)
		loadRoomObjectsSmall();
	else
		loadRoomObjects();

	
	if (VAR_ROOM_WIDTH != 0xFF && VAR_ROOM_HEIGHT != 0xFF) {
		VAR(VAR_ROOM_WIDTH) = _roomWidth;
		VAR(VAR_ROOM_HEIGHT) = _roomHeight;
	}

	VAR(VAR_CAMERA_MIN_X) = _screenWidth / 2;
	VAR(VAR_CAMERA_MAX_X) = _roomWidth - (_screenWidth / 2);

	if (_features & GF_NEW_CAMERA) {
		VAR(VAR_CAMERA_MIN_Y) = _screenHeight / 2;
		VAR(VAR_CAMERA_MAX_Y) = _roomHeight - (_screenHeight / 2);
		setCameraAt(_screenWidth / 2, _screenHeight / 2);
	} else {
		camera._mode = kNormalCameraMode;
		if (_version > 2)
			camera._cur.x = camera._dest.x = _screenWidth / 2;
		camera._cur.y = camera._dest.y = _screenHeight / 2;
	}

	if (_roomResource == 0)
		return;

	memset(gfxUsageBits, 0, sizeof(gfxUsageBits));

	if (a) {
		where = whereIsObject(objectNr);
		if (where != WIO_ROOM && where != WIO_FLOBJECT)
			error("startScene: Object %d is not in room %d", objectNr,
						_currentRoom);
		int x, y, dir;
		getObjectXYPos(objectNr, x, y, dir);
		a->putActor(x, y, _currentRoom);
		a->setDirection(dir + 180);
		a->stopActorMoving();
	}

	showActors();

	_egoPositioned = false;
	runEntryScript();
	if (_version <= 2)
		runScript(5, 0, 0, 0);
	else if (_version >= 5 && _version <= 6) {
		if (a && !_egoPositioned) {
			int x, y;
			getObjectXYPos(objectNr, x, y);
			a->putActor(x, y, _currentRoom);
			a->moving = 0;
		}
	} else if (_version >= 7) {
		if (camera._follows) {
			a = derefActor(camera._follows, "startScene: follows");
			setCameraAt(a->_pos.x, a->_pos.y);
		}
	}

	_doEffect = true;

	CHECK_HEAP;
}

void ScummEngine::initRoomSubBlocks() {
	int i;
	const byte *ptr;
	byte *roomptr, *searchptr, *roomResPtr = 0;
	const RoomHeader *rmhd;

	_ENCD_offs = 0;
	_EXCD_offs = 0;
	_EPAL_offs = 0;
	_CLUT_offs = 0;
	_PALS_offs = 0;

	memset(_extraBoxFlags, 0, sizeof(_extraBoxFlags));

	// Determine the room and room script base address
	roomResPtr = roomptr = getResourceAddress(rtRoom, _roomResource);
	if (_version == 8)
		roomResPtr = getResourceAddress(rtRoomScripts, _roomResource);
	if (!roomptr || !roomResPtr)
		error("Room %d: data not found (" __FILE__  ":%d)", _roomResource, __LINE__);

	// Reset room color for V1 zak
	if (_version == 1)
		_roomPalette[0] = 0;

	//
	// Determine the room dimensions (width/height)
	//
	if (_features & GF_OLD_BUNDLE)
		rmhd = (const RoomHeader *)(roomptr + 4);
	else
		rmhd = (const RoomHeader *)findResourceData(MKID('RMHD'), roomptr);
	
	if (_version == 1) {
		_roomWidth = roomptr[4] * 8;
		_roomHeight = roomptr[5] * 8;
	} else if (_version == 8) {
		_roomWidth = READ_LE_UINT32(&(rmhd->v8.width));
		_roomHeight = READ_LE_UINT32(&(rmhd->v8.height));
	} else if (_version == 7) {
		_roomWidth = READ_LE_UINT16(&(rmhd->v7.width));
		_roomHeight = READ_LE_UINT16(&(rmhd->v7.height));
	} else {
		_roomWidth = READ_LE_UINT16(&(rmhd->old.width));
		_roomHeight = READ_LE_UINT16(&(rmhd->old.height));
	}

	//
	// Find the room image data
	//
	if (_version == 1) {
		_IM00_offs = 0;
		for (i = 0; i < 4; i++){
			gdi._C64Colors[i] = roomptr[6 + i];
		}
		gdi.decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 10), gdi._C64CharMap, 2048);
		gdi.decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 12), gdi._C64PicMap, roomptr[4] * roomptr[5]);
		gdi.decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 14), gdi._C64ColorMap, roomptr[4] * roomptr[5]);
		gdi.decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 16), gdi._C64MaskMap, roomptr[4] * roomptr[5]);
		gdi.decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 18) + 2, gdi._C64MaskChar, READ_LE_UINT16(roomptr + READ_LE_UINT16(roomptr + 18)));
		gdi._C64ObjectMode = true;
	} else if (_features & GF_OLD_BUNDLE) {
		_IM00_offs = READ_LE_UINT16(roomptr + 0x0A);
		if (_version == 2)
			_roomStrips = gdi.generateStripTable(roomptr + _IM00_offs, _roomWidth, _roomHeight, _roomStrips);
	} else if (_version == 8) {
		_IM00_offs = getObjectImage(roomptr, 1) - roomptr;
	} else if (_features & GF_SMALL_HEADER) {
		_IM00_offs = findResourceData(MKID('IM00'), roomptr) - roomptr;
	} else if (_heversion >= 70) {
		byte *roomImagePtr = getResourceAddress(rtRoomImage, _roomResource);
		_IM00_offs = findResource(MKID('IM00'), roomImagePtr) - roomImagePtr;
	} else {
		_IM00_offs = findResource(MKID('IM00'), findResource(MKID('RMIM'), roomptr)) - roomptr;
	}

	//
	// Look for an exit script
	//
	int EXCD_len = -1;
	if (_version <= 2) {
		_EXCD_offs = READ_LE_UINT16(roomptr + 0x18);
		EXCD_len = READ_LE_UINT16(roomptr + 0x1A) - _EXCD_offs + _resourceHeaderSize;	// HACK
	} else if (_features & GF_OLD_BUNDLE) {
		_EXCD_offs = READ_LE_UINT16(roomptr + 0x19);
		EXCD_len = READ_LE_UINT16(roomptr + 0x1B) - _EXCD_offs + _resourceHeaderSize;	// HACK
	} else {
		ptr = findResourceData(MKID('EXCD'), roomResPtr);
		if (ptr)
			_EXCD_offs = ptr - roomResPtr;
	}
	if (_dumpScripts && _EXCD_offs)
		dumpResource("exit-", _roomResource, roomResPtr + _EXCD_offs - _resourceHeaderSize, EXCD_len);

	//
	// Look for an entry script
	//
	int ENCD_len = -1;
	if (_version <= 2) {
		_ENCD_offs = READ_LE_UINT16(roomptr + 0x1A);
		ENCD_len = READ_LE_UINT16(roomptr) - _ENCD_offs + _resourceHeaderSize; // HACK
	} else if (_features & GF_OLD_BUNDLE) {
		_ENCD_offs = READ_LE_UINT16(roomptr + 0x1B);
		// FIXME - the following is a hack which assumes that immediately after
		// the entry script the first local script follows.
		int num_objects = *(roomResPtr + 20);
		int num_sounds = *(roomResPtr + 23);
		int num_scripts = *(roomResPtr + 24);
		ptr = roomptr + 29 + num_objects * 4 + num_sounds + num_scripts;
		ENCD_len = READ_LE_UINT16(ptr + 1) - _ENCD_offs + _resourceHeaderSize; // HACK
	} else {
		ptr = findResourceData(MKID('ENCD'), roomResPtr);
		if (ptr)
			_ENCD_offs = ptr - roomResPtr;
	}
	if (_dumpScripts && _ENCD_offs)
		dumpResource("entry-", _roomResource, roomResPtr + _ENCD_offs - _resourceHeaderSize, ENCD_len);

	//
	// Load box data
	//
	nukeResource(rtMatrix, 1);
	nukeResource(rtMatrix, 2);
	if (_features & GF_SMALL_HEADER) {
		if (_version <= 2)
			ptr = roomptr + *(roomptr + 0x15);
		else if (_features & GF_OLD_BUNDLE)
			ptr = roomptr + READ_LE_UINT16(roomptr + 0x15);
		else
			ptr = findResourceData(MKID('BOXD'), roomptr);
		if (ptr) {
			byte numOfBoxes = *ptr;
			int size;
			if (_version <= 2)
				size = numOfBoxes * SIZEOF_BOX_V2 + 1;
			else if (_version == 3)
				size = numOfBoxes * SIZEOF_BOX_V3 + 1;
			else
				size = numOfBoxes * SIZEOF_BOX + 1;

			createResource(rtMatrix, 2, size);
			memcpy(getResourceAddress(rtMatrix, 2), ptr, size);
			ptr += size;
			if (_version <= 2) {
				size = numOfBoxes * (numOfBoxes + 1);
			} else if (_features & GF_OLD_BUNDLE)
				// FIXME. This is an evil HACK!!!
				size = (READ_LE_UINT16(roomptr + 0x0A) - READ_LE_UINT16(roomptr + 0x15)) - size;
			else
				size = getResourceDataSize(ptr - size - 6) - size;

			if (size > 0) {					// do this :)
				createResource(rtMatrix, 1, size);
				memcpy(getResourceAddress(rtMatrix, 1), ptr, size);
			}

		}
	} else {
		ptr = findResourceData(MKID('BOXD'), roomptr);
		if (ptr) {
			int size = getResourceDataSize(ptr);
			createResource(rtMatrix, 2, size);
			roomptr = getResourceAddress(rtRoom, _roomResource);
			ptr = findResourceData(MKID('BOXD'), roomptr);
			memcpy(getResourceAddress(rtMatrix, 2), ptr, size);
		}

		ptr = findResourceData(MKID('BOXM'), roomptr);
		if (ptr) {
			int size = getResourceDataSize(ptr);
			createResource(rtMatrix, 1, size);
			roomptr = getResourceAddress(rtRoom, _roomResource);
			ptr = findResourceData(MKID('BOXM'), roomptr);
			memcpy(getResourceAddress(rtMatrix, 1), ptr, size);
		}
	}

	//
	// Load scale data
	//
	for (i = 1; i < res.num[rtScaleTable]; i++)
		nukeResource(rtScaleTable, i);

	if (_features & GF_OLD_BUNDLE)
		ptr = 0;
	else
		ptr = findResourceData(MKID('SCAL'), roomptr);
	if (ptr) {
		int s1, s2, y1, y2;
		if (_version == 8) {
			for (i = 1; i < res.num[rtScaleTable]; i++, ptr += 16) {
				s1 = READ_LE_UINT32(ptr);
				y1 = READ_LE_UINT32(ptr + 4);
				s2 = READ_LE_UINT32(ptr + 8);
				y2 = READ_LE_UINT32(ptr + 12);
				setScaleSlot(i, 0, y1, s1, 0, y2, s2);
			}
		} else {
			for (i = 1; i < res.num[rtScaleTable]; i++, ptr += 8) {
				s1 = READ_LE_UINT16(ptr);
				y1 = READ_LE_UINT16(ptr + 2);
				s2 = READ_LE_UINT16(ptr + 4);
				y2 = READ_LE_UINT16(ptr + 6);
				if (s1 || y1 || s2 || y2) {
					setScaleSlot(i, 0, y1, s1, 0, y2, s2);
				}
			}
		}
	}

	//
	// Setup local scripts
	//

	// Determine the room script base address
	roomResPtr = roomptr = getResourceAddress(rtRoom, _roomResource);
	if (_version == 8)
		roomResPtr = getResourceAddress(rtRoomScripts, _roomResource);
	searchptr = roomResPtr;

	memset(_localScriptOffsets, 0, sizeof(_localScriptOffsets));

	if (_features & GF_OLD_BUNDLE) {
		int num_objects = *(roomResPtr + 20);
		int num_sounds;
		int num_scripts;

		if (_version <= 2) {
			num_sounds = *(roomResPtr + 22);
			num_scripts = *(roomResPtr + 23);
			ptr = roomptr + 28 + num_objects * 4;
			while (num_sounds--)
				loadResource(rtSound, *ptr++);
			while (num_scripts--)
				loadResource(rtScript, *ptr++);
		} else if (_version == 3) {
			num_sounds = *(roomResPtr + 23);
			num_scripts = *(roomResPtr + 24);
			ptr = roomptr + 29 + num_objects * 4 + num_sounds + num_scripts;
			while (*ptr) {
				int id = *ptr;

				_localScriptOffsets[id - _numGlobalScripts] = READ_LE_UINT16(ptr + 1);
				ptr += 3;
	
				if (_dumpScripts) {
					char buf[32];
					sprintf(buf, "room-%d-", _roomResource);

					// HACK: to determine the sizes of the local scripts, we assume that
					// a) their order in the data file is the same as in the index
					// b) the last script at the same time is the last item in the room "header"
					int len = - (int)_localScriptOffsets[id - _numGlobalScripts] + _resourceHeaderSize;
					if (*ptr)
						len += READ_LE_UINT16(ptr + 1);
					else
						len += READ_LE_UINT16(roomResPtr);
					dumpResource(buf, id, roomResPtr + _localScriptOffsets[id - _numGlobalScripts] - _resourceHeaderSize, len);
				}
			}
		}
	} else if (_features & GF_SMALL_HEADER) {
		ResourceIterator localScriptIterator(searchptr, true);
		while ((ptr = localScriptIterator.findNext(MKID('LSCR'))) != NULL) {
			int id = 0;
			ptr += _resourceHeaderSize;	/* skip tag & size */
			id = ptr[0];

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}

			_localScriptOffsets[id - _numGlobalScripts] = ptr + 1 - roomptr;
		}
	} else if (_heversion >= 90) {
		ResourceIterator localScriptIterator2(searchptr, false);
		while ((ptr = localScriptIterator2.findNext(MKID('LSC2'))) != NULL) {
			int id = 0;

			ptr += _resourceHeaderSize;	/* skip tag & size */

			id = READ_LE_UINT32(ptr);

			checkRange(_numLocalScripts + _numGlobalScripts, _numGlobalScripts, id, "Invalid local script %d");
			_localScriptOffsets[id - _numGlobalScripts] = ptr + 4 - roomResPtr;

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}
		}

		ResourceIterator localScriptIterator(searchptr, false);
		while ((ptr = localScriptIterator.findNext(MKID('LSCR'))) != NULL) {
			int id = 0;

			ptr += _resourceHeaderSize;	/* skip tag & size */

			id = ptr[0];
			_localScriptOffsets[id - _numGlobalScripts] = ptr + 1 - roomResPtr;

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}
		}

	} else {
		ResourceIterator localScriptIterator(searchptr, false);
		while ((ptr = localScriptIterator.findNext(MKID('LSCR'))) != NULL) {
			int id = 0;

			ptr += _resourceHeaderSize;	/* skip tag & size */

			if (_version == 8) {
				id = READ_LE_UINT32(ptr);
				checkRange(_numLocalScripts + _numGlobalScripts, _numGlobalScripts, id, "Invalid local script %d");
				_localScriptOffsets[id - _numGlobalScripts] = ptr + 4 - roomResPtr;
			} else if (_version == 7) {
				id = READ_LE_UINT16(ptr);
				checkRange(_numLocalScripts + _numGlobalScripts, _numGlobalScripts, id, "Invalid local script %d");
				_localScriptOffsets[id - _numGlobalScripts] = ptr + 2 - roomResPtr;
			} else {
				id = ptr[0];
				_localScriptOffsets[id - _numGlobalScripts] = ptr + 1 - roomResPtr;
			}

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}
		}
	}

	if (_features & GF_OLD_BUNDLE)
		ptr = 0;
	else if (_features & GF_SMALL_HEADER)
		ptr = findResourceSmall(MKID('EPAL'), roomptr);
	else
		ptr = findResourceData(MKID('EPAL'), roomptr);

	if (ptr) {
		_EPAL_offs = ptr - roomptr;
	}

	if (_features & GF_OLD_BUNDLE)
		ptr = 0;
	else if (_features & GF_SMALL_HEADER)
		ptr = findResourceSmall(MKID('CLUT'), roomptr);
	else
		ptr = findResourceData(MKID('CLUT'), roomptr);

	if (ptr) {
		_CLUT_offs = ptr - roomptr;
	}

	if (_version >= 6) {
		ptr = findResource(MKID('PALS'), roomptr);
		if (ptr) {
			_PALS_offs = ptr - roomptr;
		}
	}

	// Color cycling
	// HE 7.0 games load resources but don't use them.
	if (_version >= 4 && _heversion <= 60) {
		ptr = findResourceData(MKID('CYCL'), roomptr);
		if (ptr) {
			initCycl(ptr);
		}
	}

	// Transparent color
	if (_version == 8)
		gdi._transparentColor = (byte)READ_LE_UINT32(&(rmhd->v8.transparency));
	else if (_features & GF_OLD_BUNDLE)
		gdi._transparentColor = 255;
	else {
		ptr = findResourceData(MKID('TRNS'), roomptr);
		if (ptr)
			gdi._transparentColor = ptr[0];
		else
			gdi._transparentColor = 255;
	}

	// Actor Palette in HE 70 games
	if (_heversion == 70) {
		ptr = findResourceData(MKID('REMP'), roomptr);
		if (ptr) {
			for (i = 0; i < 256; i++)
				_HEV7ActorPalette[i] = *ptr++;
		} else {
			for (i = 0; i < 256; i++)
				_HEV7ActorPalette[i] = i;
		}
	}
			
	// Polygons in HE 80+ games
	if (_heversion >= 80) {
		ptr = findResourceData(MKID('POLD'), roomptr);
		if (ptr) {
			((ScummEngine_v70he *)this)->_wiz.polygonLoad(ptr);
		}
	}

	if (_PALS_offs || _CLUT_offs)
		setPalette(0, _roomResource);

	initBGBuffers(_roomHeight);
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

void ScummEngine::optionsDialog() {
	if (!_optionsDialog)
		_optionsDialog = new ConfigDialog(this);
	runDialog(*_optionsDialog);
}

void ScummEngine::confirmexitDialog() {
	ConfirmDialog confirmExitDialog(this, "Do you really want to quit (y/n)?");

	if (runDialog(confirmExitDialog)) {
		_quit = true;
	}
}

void ScummEngine::confirmrestartDialog() {
	ConfirmDialog confirmRestartDialog(this, "Do you really want to restart (y/n)?");

	if (runDialog(confirmRestartDialog)) {
		restart();
	}
}

char ScummEngine::displayMessage(const char *altButton, const char *message, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, message);
	vsprintf(buf, message, va);
	va_end(va);

	GUI::MessageDialog dialog(buf, "OK", altButton);
	return runDialog(dialog);
}

#pragma mark -
#pragma mark --- Miscellaneous ---
#pragma mark -

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

int ScummEngine::generateMacFileName(const char *filename, char *buf, int bufsize, int cont, int index) {
	return generateMacFileName_(filename, buf, bufsize, cont, index);
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
	bool heOver;
	int heLastName = 0;

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
		} else if (g->features & GF_HUMONGOUS) {
			strcpy(detectName, base);
			strcat(detectName, ".he0");
			strcpy(tempName, base);
			strcat(tempName, ".he0");
		} else {
			strcpy(detectName, base);
			strcat(detectName, ".000");
		}

		heOver = false;
		heLastName = 0;

		while (!heOver) {
			// Iterate over all files in the given directory
			for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
				if (!file->isDirectory()) {
					const char *name = file->displayName().c_str();

					if (0 == scumm_stricmp(detectName, name)) {
						// Match found, add to list of candidates, then abort inner loop.
						if (heLastName > 0) { // HE Mac versions.
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

			if (g->features & GF_HUMONGOUS) {
				if ((heLastName = generateMacFileName_(tempName, detectName, 128, 
													  heLastName)) == -1)
					heOver = true;
			} else {
				heOver = true;
			}
		}
	}
	
	// Now, we check the MD5 sums of the 'candidate' files. If we have an exact match,
	// only return that.
	bool exactMatch = false;
	for (StringSet::const_iterator iter = fileSet.begin(); iter != fileSet.end(); ++iter) {
		uint8 md5sum[16];
		const char *name = iter->_key.c_str();

		if (md5_file(name, md5sum, 0, kMD5FileSizeLimit)) {
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

				// HACK to work around bug #1009344
				if (!strcmp(target, "monkey")) {
					const char *str = name;
					int len = 0;
					// Scan to the end of the string...
					while (*str++)
						len++;
					// ...so that we can check if it ends with 'monkey1.000'
					const char *monkey1 = "monkey1.000";	// Len: 11
					if (len >= 11 && !scumm_stricmp(str-11-1, monkey1))
						target = "monkey1";
				}

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

static int generateMacFileName_(const char *filename, char *buf, int bufsize, int cont, int index) {
	if (cont == -1)
		return -1;

	if (cont >= 0)
		cont++;

	char num = filename[strlen(filename) - 1];
	
	// In some cases we have .(a) and .(b) extensions
	if (num == ')')
		num = filename[strlen(filename) - 2];

	char *n = strrchr(filename, '.');
	int len = n - filename;

	if (index > 0)
		cont = index;

	for (int i = cont; i < ARRAYSIZE(heMacFileNameTable); i++) {
		if (!scumm_strnicmp(filename, heMacFileNameTable[i].winName, len)) {
			if (num == '3') { // special case for cursors
				// For mac they're stored in game binary
				strncpy(buf, heMacFileNameTable[i].macName, bufsize);
			} else {
				if (heMacFileNameTable[i].hasParens)
					snprintf(buf, bufsize, "%s (%c)", heMacFileNameTable[i].macName, num);
				else 
					snprintf(buf, bufsize, "%s %c", heMacFileNameTable[i].macName, num);
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
	if (!g->name)
		error("Invalid game '%s'\n", detector->_game.name);

	ScummGameSettings game = *g;

	// Calculate MD5 of the games detection file, for savegames etc.
	const char *name = g->name;
	char detectName[256], tempName[256], gameMD5[32+1];
	uint8 md5sum[16];
	int heLastName = 0;

	if (g->detectFilename) {
		strcpy(detectName, game.detectFilename);
	} else if (g->version <= 3) {
		strcpy(detectName, "00.LFL");
	} else if (g->version == 4) {
		strcpy(detectName, "000.LFL");
	} else if (g->version >= 7) {
		strcpy(detectName, name);
		strcat(detectName, ".la0");
	} else if (g->features & GF_HUMONGOUS) {
		strcpy(detectName, name);
		strcat(detectName, ".he0");
		strcpy(tempName, name);
		strcat(tempName, ".he0");
		
		bool heOver = false;
		File f;

		while (!heOver) {
			if (f.exists(detectName, ConfMan.get("path").c_str()))
				break;

			if ((heLastName = generateMacFileName_(tempName, detectName, 256, 
													  heLastName)) == -1)
					heOver = true;
		}

		// Force game to have Mac platform
		if (heLastName > 0)
			game.features |= GF_MACINTOSH;
	} else {
		strcpy(detectName, name);
		strcat(detectName, ".000");
	}

	if (md5_file(detectName, md5sum, ConfMan.get("path").c_str(), kMD5FileSizeLimit)) {
		for (int j = 0; j < 16; j++) {
			sprintf(gameMD5 + j*2, "%02x", (int)md5sum[j]);
		}
	}

	// Use MD5 to determine specific game version, if required.
	if (game.features & GF_MULTIPLE_VERSIONS) {	
		g = he_md5_settings;
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

	if (ConfMan.hasKey("amiga")) {
		warning("Configuration key 'amiga' is deprecated. Use 'platform=amiga' instead");
		if (ConfMan.getBool("amiga"))
			game.features |= GF_AMIGA;
	}

	switch (Common::parsePlatform(ConfMan.get("platform"))) {
	case Common::kPlatformAmiga:
		game.features |= GF_AMIGA;
		break;
	case Common::kPlatformAtariST:
		game.features |= GF_ATARI_ST;
		break;
	case Common::kPlatformMacintosh:
		game.features |= GF_MACINTOSH;
		break;
	case Common::kPlatformWindows:
		game.features |= GF_WINDOWS;

		// There are both Windows and DOS versions of early HE titles
		// specify correct version here
		if (game.features & GF_HUMONGOUS && (game.heversion == 60 || game.id == GID_PUTTDEMO)) {
			game.heversion = 70;
			game.features |= GF_NEW_COSTUMES;
		}
		break;
	case Common::kPlatformFMTowns:
		game.features |= GF_FMTOWNS;
		if (game.version == 3) {
			game.midi = MDT_TOWNS;
		}
		break;
	default:
		if (!(game.features & GF_FMTOWNS))
			game.features |= GF_PC;
		break;
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
		case 90:
		case 98:
		case 99:
			engine = new ScummEngine_v90he(detector, syst, game, md5sum);
			break;
		case 80:
			engine = new ScummEngine_v80he(detector, syst, game, md5sum);
			break;
		case 72:
			engine = new ScummEngine_v72he(detector, syst, game, md5sum);
			break;
		case 71:
		case 70:
			engine = new ScummEngine_v70he(detector, syst, game, md5sum);
			break;
#endif
		case 60:
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
	((ScummEngine *)engine)->_heMacFileNameIndex = heLastName;

	return engine;
}

REGISTER_PLUGIN("Scumm Engine", Engine_SCUMM_gameList, Engine_SCUMM_create, Engine_SCUMM_detectGames)
