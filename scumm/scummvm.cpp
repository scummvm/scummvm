/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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

#include "gui/message.h"
#include "gui/newgui.h"

#include "scumm/actor.h"
#include "scumm/akos.h"
#include "scumm/boxes.h"
#include "scumm/charset.h"
#include "scumm/costume.h"
#include "scumm/debugger.h"
#include "scumm/dialogs.h"
#include "scumm/imuse_digi.h"
#include "scumm/imuse.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/player_v1.h"
#include "scumm/player_v2.h"
#include "scumm/player_v2a.h"
#include "scumm/player_v3a.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
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

extern uint16 _debugLevel;

namespace Scumm {

enum MouseButtonStatus {
	msDown = 1,
	msClicked = 2
};

// Use g_scumm from error() ONLY
ScummEngine *g_scumm = 0;

struct ScummGameSettings {
	const char *gameName;
	const char *description;
	byte id, version;
	int midi; // MidiDriverType values
	uint32 features;
	const char *baseFilename;

	GameSettings toGameSettings() const {
		GameSettings dummy = { gameName, description, midi, features, 0 };
		return dummy;
	}
};


static const ScummGameSettings scumm_settings[] = {
	/* Scumm Version 1 */
	/* Scumm Version 2 */

	{"maniac", "Maniac Mansion", GID_MANIAC, 2, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, 0},
	//{"maniacnes", "Maniac Mansion (NES)", GID_MANIAC, 2, MDT_NONE,
	// GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING | GF_NES},
	{"zak",         "Zak McKracken and the Alien Mindbenders", GID_ZAK, 2, MDT_PCSPK,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_16COLOR | GF_OLD_BUNDLE | GF_NO_SCALING, 0},

	/* Scumm Version 3 */
	{"indy3EGA", "Indiana Jones and the Last Crusade", GID_INDY3, 3, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALING | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE, 0},
	{"indy3Towns", "Indiana Jones and the Last Crusade (FM Towns)", GID_INDY3, 3, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS | GF_FMTOWNS | GF_AUDIOTRACKS, 0},
	{"indy3", "Indiana Jones and the Last Crusade (256)", GID_INDY3, 3, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALING | GF_OLD256 | GF_FEW_LOCALS, 0},
	{"zak256", "Zak McKracken 256 (deprecated)", GID_ZAK256, 3, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALING | GF_OLD256 | GF_FMTOWNS | GF_AUDIOTRACKS, 0},
	{"zakTowns", "Zak McKracken and the Alien Mindbenders (FM Towns)", GID_ZAK256, 3, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALING | GF_OLD256 | GF_FMTOWNS | GF_AUDIOTRACKS, 0},
	{"loom", "Loom", GID_LOOM, 3, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALING | GF_USE_KEY | GF_16COLOR | GF_OLD_BUNDLE, 0},
	{"loomTowns", "Loom (FM Towns)", GID_LOOM, 3, MDT_TOWNS,
	 GF_SMALL_HEADER | GF_SMALL_NAMES | GF_NO_SCALING | GF_OLD256 | GF_FMTOWNS | GF_AUDIOTRACKS, 0},

	/* Scumm Version 4 */
	{"monkeyEGA", "Monkey Island 1 (EGA)", GID_MONKEY_EGA, 4, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, 0},
	{"pass", "Passport to Adventure", GID_PASS, 4, MDT_PCSPK | MDT_ADLIB,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR, 0},

	/* Scumm version 5 */
	{"monkeyVGA", "Monkey Island 1 (256 color Floppy version)", GID_MONKEY_VGA,  4, MDT_PCSPK | MDT_ADLIB | MDT_NATIVE,
	 GF_SMALL_HEADER | GF_USE_KEY, 0},
	{"loomcd", "Loom (256 color CD version)", GID_LOOM256, 4, MDT_NONE,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_AUDIOTRACKS, 0},
	{"monkey", "Monkey Island 1", GID_MONKEY, 5, /*MDT_PCSPK |*/ MDT_ADLIB,
	 GF_USE_KEY | GF_AUDIOTRACKS, 0},
	{"monkey1", "Monkey Island 1 (alt)", GID_MONKEY, 5, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY | GF_AUDIOTRACKS, 0},
	{"game", "Monkey Island 1 (SegaCD version)", GID_MONKEY_SEGA, 5, MDT_NONE,
	 GF_USE_KEY | GF_AUDIOTRACKS, 0},
	{"monkey2", "Monkey Island 2: LeChuck's revenge", GID_MONKEY2, 5, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, 0},
	{"mi2demo", "Monkey Island 2: LeChuck's revenge (Demo)", GID_MONKEY2, 5, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, 0},
	{"indydemo", "Indiana Jones and the Fate of Atlantis (FM Towns Demo)", GID_INDY4, 5, MDT_ADLIB,
	 GF_USE_KEY, 0},
	{"indy4", "Indiana Jones and the Fate of Atlantis (FM Towns)", GID_INDY4, 5, MDT_ADLIB,
	 GF_USE_KEY, 0},
	{"atlantis", "Indiana Jones and the Fate of Atlantis", GID_INDY4, 5, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, 0},
	{"playfate", "Indiana Jones and the Fate of Atlantis (Demo)", GID_INDY4, 5, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, 0},
	{"fate", "Indiana Jones and the Fate of Atlantis (Demo)", GID_INDY4, 5, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_USE_KEY, 0},

	/* Scumm Version 6 */
	{"puttputt", "Putt-Putt Joins The Parade (DOS)", GID_PUTTPUTT, 6, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"puttdemo", "Putt-Putt Joins The Parade (DOS Demo)", GID_PUTTDEMO, 6, MDT_ADLIB | MDT_NATIVE,
	  GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS, 0},
	{"moondemo", "Putt-Putt Goes To The Moon (DOS Demo)", GID_PUTTPUTT, 6, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"puttmoon", "Putt-Putt Goes To The Moon (DOS)", GID_PUTTPUTT, 6, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"funpack", "Putt-Putt's Fun Pack", GID_PUTTPUTT, 6, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"fbpack", "Fatty Bear's Fun Pack", GID_PUTTPUTT, 6, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"fbear", "Fatty Bear's Birthday Surprise (DOS)", GID_PUTTPUTT, 6, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"fbdemo", "Fatty Bear's Birthday Surprise (DOS Demo)", GID_PUTTPUTT, 6, MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"tentacle", "Day Of The Tentacle", GID_TENTACLE, 6, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY, 0},
	{"dottdemo", "Day Of The Tentacle (Demo)", GID_TENTACLE, 6, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY, 0},

	{"samnmax", "Sam & Max", GID_SAMNMAX, 6, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY | GF_DRAWOBJ_OTHER_ORDER, 0},

	{"samdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY  | GF_DRAWOBJ_OTHER_ORDER, 0},
	{"snmdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY  | GF_DRAWOBJ_OTHER_ORDER, 0},
	{"snmidemo", "Sam & Max (Interactive WIP Demo)", GID_SAMNMAX, 6, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE,
	 GF_NEW_OPCODES | GF_USE_KEY  | GF_DRAWOBJ_OTHER_ORDER, 0},

//	{"test", "Test demo game", GID_SAMNMAX, 6, /*MDT_PCSPK |*/ MDT_ADLIB | MDT_NATIVE, GF_NEW_OPCODES, 0},

	/* Scumm Version 7 */
	{"ft", "Full Throttle", GID_FT, 7, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, 0},
	{"ftdemo", "Full Throttle (Mac. Demo)", GID_FT, 7, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, 0},
	{"ftpcdemo", "Full Throttle (PC Demo)", GID_FT, 7, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, "ft"},


	{"dig", "The Dig", GID_DIG, 7, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE, 0},
	{"digdemo", "The Dig (Demo)", GID_DIG, 7, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEMO, "dig"},

#ifndef __PALM_OS__ // these are SVGA games not supported under PalmOS
	/* Scumm Version 8 */
	{"comi", "The Curse of Monkey Island", GID_CMI, 8, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEFAULT_TO_1X_SCALER, 0},
	{"comidemo", "The Curse of Monkey Island (Demo)", GID_CMI, 8, MDT_NONE,
	 GF_NEW_OPCODES | GF_NEW_COSTUMES | GF_NEW_CAMERA | GF_DIGI_IMUSE | GF_DEFAULT_TO_1X_SCALER | GF_DEMO, "comi"},

	 /* Note that both full versions of Humongous games and demos were often released for
	  * several interpreter versions... */
/*	 
	// Humongous Entertainment Scumm Version 7 
	{"catalog", "Humongous Interactive Catalog", GID_PUTTPUTT, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"farm", "Let's Explore the Farm with Buzzy", GID_PUTTPUTT, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"farmdemo", "Let's Explore the Farm with Buzzy (Demo)", GID_PUTTPUTT, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"airport", "Let's Explore the Airport with Buzzy", GID_PUTTPUTT, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"airdemo", "Let's Explore the Airport with Buzzy (Demo)", GID_PUTTPUTT, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"jungle", "Let's Explore the Jungle with Buzzy", GID_PUTTPUTT, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"puttzoo", "Putt-Putt Saves the Zoo", GID_PUTTPUTT, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"zoodemo", "Putt-Putt Saves the Zoo (Demo)", GID_PUTTPUTT, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"freddi", "Freddi Fish 1: Missing Kelp Seeds", GID_PUTTPUTT, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"freddemo", "Freddi Fish 1: Missing Kelp Seeds (Demo)", GID_PUTTPUTT, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},

	// Humongous Entertainment Scumm Version 8.0 ?  Scummsrc.80 
	{"pajama", "Pajama Sam 1: No Need to Hide", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"pjs-demo", "Pajama Sam 1: No Need to Hide (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"ffhsdemo", "Freddi Fish 2: Haunted Schoolbus (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"ff2-demo", "Freddi Fish 2: Haunted Schoolbus (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"freddi2", "Freddi Fish 2: Haunted Schoolbus", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},

	// Humongous Entertainment Scumm Version 9.0 ?  Scummsys.90
	{"timedemo", "Putt-Putt Travels Through Time (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"f3-mdemo", "Freddi Fish 3: Stolen Conch Shell (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"spyfox", "Spyfox 1: Dry Cereal", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"foxdemo", "Spyfox 1: Dry Cereal (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"spydemo", "Spyfox 1: Dry Cereal (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"kinddemo", "Big Thinkers Kindergarten (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"1grademo", "Big Thinkers First Grade (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},

	// Humongous Entertainment Scumm Version 9.5 ?  Scummsys.95
	{"pj2demo", "Pajama Sam 2: Thunder and Lightning (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},

	// Humongous Entertainment Scumm Version 9.8 ?  Scummsys.98
	// these and later games can easily be identified by the .(a) file instead of a .he1
	{"racedemo", "Putt-Putt Enters the Race (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"puttrace", "Putt-Putt Enters the Race", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"freddi4", "Freddi Fish 4: Hogfish Rustlers of Briny Gulch", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"f4-demo", "Freddi Fish 4: Hogfish Rustlers of Briny Gulch (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"bluesabctimedemo", "Blue's ABC Time Activities (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"bluesabctime", "Blue's ABC Time Activities", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"blue'sbirthday-yellow", "Blue's Birthday Yellow", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},

	// Humongous Entertainment Scumm Version ?  engine moved to c++ 
	{"pj3-demo", "Pajama Sam 3: You Are What You Eat (Demo)", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"freddicove", "Freddi Fish 5: The Case of the Creature of Coral Cave", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	{"putttime", "Putt-Putt Travels Through Time", GID_PJSDEMO, 6, MDT_NONE,
	 GF_NEW_OPCODES | GF_AFTER_HEV7 | GF_USE_KEY | GF_HUMONGOUS | GF_NEW_COSTUMES, 0},
	// bunch of backyard sports games...
*/
#endif
	{NULL, NULL, 0, 0, MDT_NONE, 0, 0}
};

ScummEngine::ScummEngine(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs)
	: Engine(syst),
	  _gameId(gs.id),
	  _version(gs.version),
	  _features(gs.features),
	  gdi(this), _pauseDialog(0), _optionsDialog(0), _mainMenuDialog(0),
	  _targetName(detector->_targetName) {
	OSystem::Property prop;

	// Init all vars - maybe now we can get rid of our custom new/delete operators?
	_imuse = NULL;
	_imuseDigital = NULL;
	_musicEngine = NULL;
	_verbs = NULL;
	_objs = NULL;
	_debugger = NULL;
	_sound = NULL;
	memset(&res, 0, sizeof(res));
	memset(&vm, 0, sizeof(vm));
	_smushFrameRate = 0;
	_insaneState = false;
	_videoFinished = false;
	_smushPlay = false;
	_quit = false;
	_pauseDialog = NULL;
	_optionsDialog = NULL;
	_mainMenuDialog = NULL;
	_confirmExitDialog = NULL;
	_fastMode = 0;
	_actors = NULL;
	_inventory = NULL;
	_newNames = NULL;
	_scummVars = NULL;
	_varwatch = 0;
	_bitVars = NULL;
	_numVariables = 0;
	_numBitVariables = 0;
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
	_numActors = 0;
	_numCostumes = 0;
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
	_language = 0;
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
	_saveLoadCompatible = false;
	memset(_saveLoadName, 0, sizeof(_saveLoadName));
	_maxHeapThreshold = 0;
	_minHeapThreshold = 0;
	memset(_localScriptList, 0, sizeof(_localScriptList));
	_scriptPointer = NULL;
	_scriptOrgPointer = NULL;
	_opcode = 0;
	vm.numNestedScripts = 0;
	_currentScript = 0;
	_curExecScript = 0;
	_lastCodePtr = NULL;
	_resultVarNumber = 0;
	_scummStackPos = 0;
	memset(_scummStack, 0, sizeof(_scummStack));
	_keyScriptKey = 0;
	_keyScriptNo = 0;
	_fileOffset = 0;
	_dynamicRoomOffsets = false;
	memset(_resourceMapper, 0, sizeof(_resourceMapper));
	_allocatedSize = 0;
	_expire_counter = 0;
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
	_IM00_offs = 0;
	_PALS_offs = 0;
	_fullRedraw = false;
	_BgNeedsRedraw = false;
	_verbRedraw = false;
	_screenEffectFlag = false;
	_completeScreenRedraw = false;
	memset(&_cursor, 0, sizeof(_cursor));
	memset(_grabbedCursor, 0, sizeof(_grabbedCursor));
	_currentCursor = 0;
	_newEffect = 0;
	_switchRoomEffect2 = 0;
	_switchRoomEffect = 0;
	_doEffect = false;
	memset(&_flashlight, 0, sizeof(_flashlight));
	_roomStrips = 0;
	_bompActorPalettePtr = NULL;
	_shakeEnabled= false;
	_shakeFrame = 0;
	_screenStartStrip = 0;
	_screenEndStrip = 0;
	_screenLeft = 0;
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
	memset(_proc_special_palette, 0, sizeof(_proc_special_palette));
	_palDirtyMin = 0;
	_palDirtyMax = 0;
	_haveMsg = 0;
	_useTalkAnims = false;
	_defaultTalkDelay = 0;
	_midiDriver = MD_NULL;
	tempMusic = 0;
	_silentDigitalImuse = 0;
	_noDigitalSamples = 0;
	_saveSound = 0;
	memset(_extraBoxFlags, 0, sizeof(_extraBoxFlags));
	memset(_scaleSlots, 0, sizeof(_scaleSlots));
	_charset = NULL;
	_charsetColor = 0;
	memset(_charsetColorMap, 0, sizeof(_charsetColorMap));
	memset(_charsetData, 0, sizeof(_charsetData));
	_charsetBufPos = 0;
	memset(_charsetBuffer, 0, sizeof(_charsetBuffer));
	_noSubtitles = false;
	_confirmExit = false;
	_numInMsgStack = 0;
	_msgPtrToAdd = NULL;
	_messagePtr = NULL;
	_talkDelay = 0;
	_keepText = false;
	_existLanguageFile = false;
	_languageBuffer = NULL;
	_languageIndex = NULL;
	memset(_transText, 0, sizeof(_transText));
	_costumeRenderer = NULL;
	_2byteFontPtr = 0;

	//
	// Init all VARS to 0xFF
	//
	VAR_LANGUAGE = 0xFF;
	VAR_KEYPRESS = 0xFF;
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
	VAR_59 = 0xFF;
	VAR_NOSUBTITLES = 0xFF;

	VAR_SOUNDPARAM = 0xFF;
	VAR_SOUNDPARAM2 = 0xFF;
	VAR_SOUNDPARAM3 = 0xFF;
	VAR_MOUSEPRESENT = 0xFF;
	VAR_PERFORMANCE_1 = 0xFF;
	VAR_PERFORMANCE_2 = 0xFF;
	VAR_ROOM_FLAG = 0xFF;
	VAR_GAME_LOADED = 0xFF;
	VAR_NEW_ROOM = 0xFF;
	VAR_VERSION = 0xFF;

	VAR_V5_TALK_STRING_Y = 0xFF;

	VAR_V6_SCREEN_WIDTH = 0xFF;
	VAR_V6_SCREEN_HEIGHT = 0xFF;
	VAR_V6_EMSSPACE = 0xFF;
	VAR_V6_RANDOM_NR = 0xFF;

	VAR_STRING2DRAW = 0xFF;
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

	// Use g_scumm from error() ONLY
	g_scumm = this;

	// Read settings from the detector & config manager
	_debugMode = ConfMan.hasKey("debuglevel");
	_debugLevel = ConfMan.getInt("debuglevel");
	_dumpScripts = detector->_dumpScripts;
	_bootParam = ConfMan.getInt("boot_param");

	// Allow the user to override the game name with a custom string.
	// This allows some game versions to work which use filenames
	// differing from the regular version(s) of that game.
	_gameName = ConfMan.hasKey("basename") ? ConfMan.get("basename") : gs.baseFilename ? gs.baseFilename : gs.gameName;

	_midiDriver = GameDetector::detectMusicDriver(gs.midi);

	_demoMode = ConfMan.getBool("demo_mode");
	_noSubtitles = ConfMan.getBool("subtitles");
	_noSubtitles ^=1;
	_confirmExit = ConfMan.getBool("confirm_exit");
	_defaultTalkDelay = ConfMan.getInt("talkspeed");
	_native_mt32 = ConfMan.getBool("native_mt32");
	// TODO: We shouldn't rely on the global Language values matching those COMI etc. expect.
	// Rather we should explicitly translate them.
	_language = Common::parseLanguage(ConfMan.get("language"));
	memset(&res, 0, sizeof(res));
	_hexdumpScripts = false;
	_showStack = false;

	if (_features & GF_FMTOWNS) {	// FmTowns is 320x240
		_screenWidth = 320;
		_screenHeight = 240;
	} else if (_gameId == GID_CMI) {
		_screenWidth = 640;
		_screenHeight = 480;
	} else if (_features & GF_NES) {
		_screenWidth = 256;
		_screenHeight = 240;
	} else {
		_screenWidth = 320;
		_screenHeight = 200;
	}

	// Initialize backend
	syst->init_size(_screenWidth, _screenHeight);
	prop.cd_num = ConfMan.getInt("cdrom");
	if (prop.cd_num >= 0 && (_features & GF_AUDIOTRACKS))
		syst->property(OSystem::PROP_OPEN_CD, &prop);

	// Setup GDI object
	gdi._numStrips = _screenWidth / 8;

	// Create debugger
	_debugger = new ScummDebugger(this);

	_sound = new Sound(this);

#ifndef __GP32__ //ph0x FIXME, "quick dirty hack"
	/* Bind the mixer to the system => mixer will be invoked
	 * automatically when samples need to be generated */
	_silentDigitalImuse = false;
	if (!_mixer->bindToSystem(syst)) {
		warning("Sound mixer initialization failed");
		if (_midiDriver == MD_ADLIB ||
				_midiDriver == MD_PCSPK ||
				_midiDriver == MD_PCJR)	{
			_midiDriver = MD_NULL;
			warning("MIDI driver depends on sound mixer, switching to null MIDI driver");
		}
		_silentDigitalImuse = true;
		_noDigitalSamples = true;
	}
	_mixer->setVolume(ConfMan.getInt("sfx_volume") * ConfMan.getInt("master_volume") / 255);
	_mixer->setMusicVolume(ConfMan.getInt("music_volume"));

	// Init iMuse
	if (_features & GF_DIGI_IMUSE) {
		_musicEngine = _imuseDigital = new IMuseDigital(this);
	} else if ((_features & GF_AMIGA) && (_version == 2)) {
		_musicEngine = new Player_V2A(this);
	} else if ((_features & GF_AMIGA) && (_version == 3)) {
		_musicEngine = new Player_V3A(this);
	} else if ((_features & GF_AMIGA) && (_version < 5)) {
		_musicEngine = NULL;
	} else if (((_midiDriver == MD_PCJR) || (_midiDriver == MD_PCSPK)) && ((_version > 2) && (_version < 5))) {
		_musicEngine = new Player_V2(this, _midiDriver != MD_PCSPK);
	} else if (_version > 2) {
		MidiDriver *driver = GameDetector::createMidi(_midiDriver);
		if (driver && _native_mt32)
			driver->property (MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
		_musicEngine = _imuse = IMuse::create(syst, _mixer, driver);
		if (_imuse) {
			if (ConfMan.hasKey("tempo"))
				_imuse->property(IMuse::PROP_TEMPO_BASE, ConfMan.getInt("tempo"));
			_imuse->property(IMuse::PROP_OLD_ADLIB_INSTRUMENTS, (_features & GF_SMALL_HEADER) ? 1 : 0);
			_imuse->property(IMuse::PROP_MULTI_MIDI, ConfMan.getBool("multi_midi") &&
			                 _midiDriver != MD_NULL && (gs.midi & MDT_ADLIB));
			_imuse->property(IMuse::PROP_NATIVE_MT32, _native_mt32);
			if (_features & GF_HUMONGOUS || _features & GF_FMTOWNS) {
				_imuse->property(IMuse::PROP_LIMIT_PLAYERS, 1);
				_imuse->property(IMuse::PROP_RECYCLE_PLAYERS, 1);
			}
			if (_features & GF_FMTOWNS)
				_imuse->property(IMuse::PROP_DIRECT_PASSTHROUGH, 1);
			_imuse->set_music_volume(ConfMan.getInt("music_volume"));
		}
	}
#endif // ph0x-hack

	// Load game from specified slot, if any
	if (ConfMan.hasKey("save_slot")) {
		_saveLoadSlot = ConfMan.getInt("save_slot");
		_saveLoadFlag = 2;
		_saveLoadCompatible = false;
	}
	loadLanguageBundle();

	// Load CJK font
	_CJKMode = false;
	if ((_gameId == GID_DIG || _gameId == GID_CMI) && (_language == Common::KO_KOR || _language == Common::JA_JPN || _language == Common::ZH_TWN)) {
		File fp;
		const char *fontFile = NULL;
		switch(_language) {
		case Common::KO_KOR:
			fontFile = "korean.fnt";
			break;
		case Common::JA_JPN:
			fontFile = (_gameId == GID_DIG) ? "kanji16.fnt" : "japanese.fnt";
			break;
		case Common::ZH_TWN:
			if (_gameId == GID_CMI) {
				fontFile = "chinese.fnt";
			}
			break;
		}
		if (fontFile && fp.open(fontFile, getGameDataPath(), 1)) {
			debug(2, "Loading CJK Font");
			_CJKMode = true;
			fp.seek(2, SEEK_CUR);
			_2byteWidth = fp.readByte();
			_2byteHeight = fp.readByte();

			int numChar = 0;
			switch(_language) {
			case Common::KO_KOR:
				numChar = 2350;
				break;
			case Common::JA_JPN:
				numChar = (_gameId == GID_DIG) ? 1024 : 2048; //FIXME
				break;
			case Common::ZH_TWN:
				numChar = 1; //FIXME
				break;
			}
			_2byteFontPtr = new byte[((_2byteWidth + 7) / 8) * _2byteHeight * numChar];
			fp.read(_2byteFontPtr, ((_2byteWidth + 7) / 8) * _2byteHeight * numChar);
			fp.close();
		}
	} else if (_language == Common::JA_JPN && _version == 5) { //FM Towns Kanji
		File fp;
		int numChar = 256 * 32;
		_2byteWidth = 16;
		_2byteHeight = 16;
		//use FM Towns font rom, since game files don't have kanji font resources
		if (fp.open("fmt_fnt.rom", getGameDataPath(), 1) || fp.open("fmt_fnt.rom", "./", 1)) { 
			_CJKMode = true;
			debug(2, "Loading FM Towns Kanji rom");
			_2byteFontPtr = new byte[((_2byteWidth + 7) / 8) * _2byteHeight * numChar];
			fp.read(_2byteFontPtr, ((_2byteWidth + 7) / 8) * _2byteHeight * numChar);
			fp.close();
		}
	}

	// Create the charset renderer
	if (_version <= 2)
		_charset = new CharsetRendererV2(this);
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
}

ScummEngine::~ScummEngine() {
	_mixer->stopAll();

	delete [] _actors;

	delete _2byteFontPtr;
	delete _charset;
	delete _pauseDialog;
	delete _optionsDialog;
	delete _mainMenuDialog;
	delete _confirmExitDialog;

	delete _sound;
	if (_musicEngine) {
		_musicEngine->terminate();
		delete _musicEngine;
	}
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
	free(_scummVars);
	free(_bitVars);
	free(_newNames);
	free(_classData);

	free(_roomStrips);
	free(_languageIndex);

	delete _debugger;
}

void ScummEngine::go() {
	launch();
	mainRun();
}

#pragma mark -
#pragma mark --- Initialization ---
#pragma mark -

void ScummEngine::launch() {

#ifdef __PALM_OS__
	// revert to old value (450000) and make ScummVM works again in some devices with same problem as below.
	// 2500000 is too big and make ScummVM crashes : MemMove to NULL or immediate exit if try to allocate
	// memory with new operator
	_maxHeapThreshold = 450000;
#else
	// Since the new costumes are very big, we increase the heap limit, to avoid having
	// to constantly reload stuff from the data files.
	if (_features & GF_NEW_COSTUMES)
		_maxHeapThreshold = 2500000;
	else
		_maxHeapThreshold = 550000;
#endif
	_minHeapThreshold = 400000;

	_verbRedraw = false;

	allocResTypeData(rtBuffer, MKID('NONE'), 10, "buffer", 0);
	initVirtScreen(0, 0, 0, _screenWidth, _screenHeight, false, false);

	setupScummVars();

	setupOpcodes();

	if (_version == 8)
		_numActors = 80;
	else if ((_version == 7) || (_gameId == GID_SAMNMAX))
		_numActors = 30;
	else if (_gameId == GID_MANIAC)
		_numActors = 25;
	else 
		_numActors = 13;

	if (_version >= 7)
		OF_OWNER_ROOM = 0xFF;
	else
		OF_OWNER_ROOM = 0x0F;

	// if (_gameId==GID_MONKEY2 && _bootParam == 0)
	//	_bootParam = 10001;

	if (_gameId == GID_INDY4 && _bootParam == 0) {
		_bootParam = -7873;
	}

	if (_features & GF_OLD_BUNDLE)
		_resourceHeaderSize = 4; // FIXME - to be rechecked
	else if (_features & GF_SMALL_HEADER)
		_resourceHeaderSize = 6;
	else
		_resourceHeaderSize = 8;

	readIndexFile();

	scummInit();

	if (_version > 2) {
		if (_version < 7)
			VAR(VAR_VERSION) = 21;
	
		if (!((_features & GF_MACINTOSH) && (_version == 3))) {
			// This is NOT for the Mac version of Indy3/Loom
			VAR(VAR_DEBUGMODE) = _debugMode;
		}
	}

	if (_gameId == GID_MONKEY || _gameId == GID_MONKEY_SEGA)
		_scummVars[74] = 1225;

	_sound->setupSound();

	// If requested, load a save game instead of running the boot script
	if (_saveLoadFlag != 2 || !loadState(_saveLoadSlot, _saveLoadCompatible)) {
		if (_gameId == GID_MANIAC && _version == 1 && _demoMode)
			runScript(9, 0, 0, &_bootParam);
		else
			runScript(1, 0, 0, &_bootParam);
	}
	_saveLoadFlag = 0;
}

void ScummEngine::setFeatures(uint32 newFeatures) {
	bool amigaPalette = (_features & GF_AMIGA) != 0;
	bool newAmigaPalette = (newFeatures & GF_AMIGA) != 0;

	if ((_features ^ newFeatures) & ~GF_AMIGA) {
		error("setFeatures may only be used to toggle GF_AMIGA flag!");
	}

	_features = newFeatures;
	
	if ((_features & GF_16COLOR) && amigaPalette != newAmigaPalette) {
		if (_features & GF_AMIGA)
			setupAmigaPalette();
		else
			setupEGAPalette();
	}
}

void ScummEngine::scummInit() {
	int i;

	tempMusic = 0;
	debug(9, "scummInit");

	if ((_gameId == GID_MANIAC) && (_version == 1)) {
		initScreens(0, 16, _screenWidth, 152);
	} else if (_version >= 7) {
		initScreens(0, 0, _screenWidth, _screenHeight);
	} else {
		initScreens(0, 16, _screenWidth, 144);
	}

	for (i = 0; i < 256; i++)
		_roomPalette[i] = i;
	if (_version == 1) {
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

	if (_version <= 2) {
		initV2MouseOver();

		// Seems in V2 there was only a single room effect (iris),
		// so we set that here.
		_switchRoomEffect2 = 1;
		_switchRoomEffect = 5;
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
		if (_version == 1)
			_actors[i].setActorCostume(i);
	}

	vm.numNestedScripts = 0;
	vm.cutSceneStackPointer = 0;

	memset(vm.cutScenePtr, 0, sizeof(vm.cutScenePtr));
	memset(vm.cutSceneData, 0, sizeof(vm.cutSceneData));

	for (i = 0; i < _maxVerbs; i++) {
		_verbs[i].verbid = 0;
		_verbs[i].right = _screenWidth - 1;
		_verbs[i].old.left = -1;
		_verbs[i].type = 0;
		_verbs[i].color = 2;
		_verbs[i].hicolor = 0;
		_verbs[i].charset_nr = 1;
		_verbs[i].curmode = 0;
		_verbs[i].saveid = 0;
		_verbs[i].center = 0;
		_verbs[i].key = 0;
	}

	if (!(_features & GF_NEW_CAMERA)) {
		camera._leftTrigger = 10;
		camera._rightTrigger = 30;
		camera._mode = 0;
	}
	camera._follows = 0;

	virtscr[0].xstart = 0;

	if (!(_features & GF_NEW_OPCODES)) {
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
			_string[i].t_xpos = 0;
			_string[i].t_ypos = 0;
		} else {
			_string[i].t_xpos = 2;
			_string[i].t_ypos = 5;
		}
		_string[i].t_right = _screenWidth - 1;
		_string[i].t_color = 0xF;
		_string[i].t_center = 0;
		_string[i].t_charset = 0;
	}

	_numInMsgStack = 0;

	createResource(rtTemp, 6, 500);

	initScummVars();

	// FIXME: disabled, why we need this, it's looks completly dummy and slow down startup
	//	getGraphicsPerformance();

	_lastSaveTime = _system->get_msecs();
}


void ScummEngine::initScummVars() {

	// FIXME
	if (_version <= 2) {
		// This needs to be at least greater than 40 to get the more
		// elaborate version of the EGA Zak into. I don't know where
		// else it makes any difference.
		VAR(VAR_MACHINE_SPEED) = 0x7FFF;
		return;
	}

	if (_version < 6)
		VAR(VAR_V5_TALK_STRING_Y) = -0x50;

	if (_version == 8) {	// Fixme: How do we deal with non-cd installs?
		VAR(VAR_CURRENTDISK) = 1;
		VAR(VAR_LANGUAGE) = _language;
	} else if (_version >= 7) {
		VAR(VAR_V6_EMSSPACE) = 10000;
	} else {
		VAR(VAR_CURRENTDRIVE) = 0;
		VAR(VAR_FIXEDDISK) = true;
		switch (_midiDriver) {
		case MD_NULL:  VAR(VAR_SOUNDCARD) = 0; break;
		case MD_ADLIB: VAR(VAR_SOUNDCARD) = 3; break;
		case MD_PCSPK:
		case MD_PCJR:  VAR(VAR_SOUNDCARD) = 1; break;
		default:       
			if ((_gameId == GID_MONKEY_EGA || _gameId == GID_MONKEY_VGA || _gameId == GID_LOOM)
			   &&  (_features & GF_PC)) {
				if (_gameId == GID_LOOM) {
					char buf[50];
					uint i = 82;
					File f;
					while (i < 85) {
						sprintf(buf, "%d.LFL", i);
						f.open(buf, _gameDataPath);
						if (f.isOpen() == false)
							error("Native MIDI support requires Roland patch from LucasArts");
						f.close();
						i++;
					}
				} else if (_gameId == GID_MONKEY_EGA) {
						File f;
						f.open("DISK09.LEC", _gameDataPath);
						if (f.isOpen() == false)
							error("Native MIDI support requires Roland patch from LucasArts");
				}
				VAR(VAR_SOUNDCARD) = 4;
			} else
				VAR(VAR_SOUNDCARD) = 3;
		}
		VAR(VAR_VIDEOMODE) = 0x13;
		VAR(VAR_HEAPSPACE) = 1400;
		VAR(VAR_MOUSEPRESENT) = true; // FIXME - used to be 0, but that seems odd?!?
		if (_features & GF_HUMONGOUS)
			VAR(VAR_SOUNDPARAM) = 1; // soundblaster for music
		else
			VAR(VAR_SOUNDPARAM) = 0;
		VAR(VAR_SOUNDPARAM2) = 0;
		VAR(VAR_SOUNDPARAM3) = 0;
		if (_version >= 6 && VAR_V6_EMSSPACE != 0xFF)
			VAR(VAR_V6_EMSSPACE) = 10000;

		VAR(VAR_59) = 3;
	}
	
	if ((_features & GF_MACINTOSH) && (_version == 3)) {
		// This is the for the Mac version of Indy3/Loom
		VAR(39) = 320;
	}

	if (!(_features & GF_NEW_OPCODES)) {
		// Setup light
		VAR(VAR_CURRENT_LIGHTS) = LIGHTMODE_actor_base | LIGHTMODE_actor_color | LIGHTMODE_screen;
	}
	
	VAR(VAR_CHARINC) = 4;
	VAR(VAR_TALK_ACTOR) = 0;
}

#pragma mark -
#pragma mark --- Main loop ---
#pragma mark -

void ScummEngine::mainRun() {
	int delta = 0;
	int diff = _system->get_msecs();

	while (!_quit) {

		updatePalette();
		_system->update_screen();

		diff -= _system->get_msecs();
		waitForTimer(delta * 15 + diff);
		diff = _system->get_msecs();
		delta = scummLoop(delta);

		if (delta < 1)	// Ensure we don't get into a loop
			delta = 1;  // by not decreasing sleepers.

		if (_quit) {
			// TODO: Maybe perform an autosave on exit?
			// TODO: Also, we could optionally show a "Do you really want to quit?" dialog here
		}
	}
}

void ScummEngine::waitForTimer(int msec_delay) {
	uint32 start_time;

	if (_fastMode & 2)
		msec_delay = 0;
	else if (_fastMode & 1)
		msec_delay = 10;

	start_time = _system->get_msecs();

	while (!_quit) {
		parseEvents();

		_sound->updateCD(); // Loop CD Audio if needed
		if (_system->get_msecs() >= start_time + msec_delay)
			break;
		_system->delay_msecs(10);
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
	}
	if (VAR_TMR_4 != 0xFF)
		VAR(VAR_TMR_4) += delta;

	if (delta > 15)
		delta = 15;

	decreaseScriptDelay(delta);

	// If _talkDelay is -1, that means the text should never time out.
	// This is used for drawing verb texts, e.g. the Full Throttle
	// dialogue choices.

	if (_talkDelay != -1) {
		_talkDelay -= delta;
		if (_talkDelay < 0)
			_talkDelay = 0;
	}

	// Record the current ego actor before any scripts (including input scripts)
	// get a chance to run.
	int oldEgo = 0;
	if (VAR_EGO != 0xFF)
		oldEgo = VAR(VAR_EGO);

	// In V1-V3 games, CHARSET_1 is called much earlier than in newer games.
	// See also bug #770042 for a case were this makes a difference.
	// FIXME: Actually I am only sure that this is correct for V1-V2 and Loom.
	// We should also check Indy3 & Zak256.
	if (_version <= 3)
		CHARSET_1();

	processKbd();

	if (_features & GF_NEW_CAMERA) {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x;
		VAR(VAR_CAMERA_POS_Y) = camera._cur.y;
	} else if (_version <= 2) {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x / 8;
	} else {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x;
	}
	VAR(VAR_HAVE_MSG) = (_haveMsg == 0xFE) ? 0xFF : _haveMsg;
	if (_version <= 2) {
		VAR(VAR_VIRT_MOUSE_X) = _virtualMouse.x / 8;
		VAR(VAR_VIRT_MOUSE_Y) = _virtualMouse.y / 2;
	} else {
		VAR(VAR_VIRT_MOUSE_X) = _virtualMouse.x;
		VAR(VAR_VIRT_MOUSE_Y) = _virtualMouse.y;
		VAR(VAR_MOUSE_X) = _mouse.x;
		VAR(VAR_MOUSE_Y) = _mouse.y;
		if ((_features & GF_MACINTOSH) && (_version == 3))  {
			// This is for the Mac version of Indy3/Loom
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
		tempMusic += delta * 15;	// Convert delta to milliseconds
		if (tempMusic >= MUSIC_DELAY) {
			tempMusic -= MUSIC_DELAY;
			VAR(VAR_MUSIC_TIMER) += 1;
		}
	}

	// Trigger autosave all 5 minutes.
	if (!_saveLoadFlag && _system->get_msecs() > _lastSaveTime + 5 * 60 * 1000) {
		_saveLoadSlot = 0;
		sprintf(_saveLoadName, "Autosave %d", _saveLoadSlot);
		_saveLoadFlag = 1;
		_saveLoadCompatible = false;
	}

	if (VAR_GAME_LOADED != 0xFF)
		VAR(VAR_GAME_LOADED) = 0;
	if (_saveLoadFlag) {
load_game:
		bool success;
		const char *errMsg = 0;
		char filename[256];

		if (_saveLoadFlag == 1) {
			success = saveState(_saveLoadSlot, _saveLoadCompatible);
			if (!success)
				errMsg = "Failed to save game state to file:\n\n%s";

			// Ender: Disabled for small_header games, as can overwrite game
			//  variables (eg, Zak256 cashcard values). Temp disabled for V8
			// because of odd timing issue with scripts and the variable reset
			if (success && _saveLoadCompatible && !(_features & GF_SMALL_HEADER) && _version < 8)
				VAR(VAR_GAME_LOADED) = 201;
		} else {
			success = loadState(_saveLoadSlot, _saveLoadCompatible);
			if (!success)
				errMsg = "Failed to load game state from file:\n\n%s";

			// Ender: Disabled for small_header games, as can overwrite game
			//  variables (eg, Zak256 cashcard values).
			if (success && _saveLoadCompatible && !(_features & GF_SMALL_HEADER))
				VAR(VAR_GAME_LOADED) = 203;
		}

		makeSavegameName(filename, _saveLoadSlot, _saveLoadCompatible);
		if (!success) {
			displayError(false, errMsg, filename);
		} else if (_saveLoadFlag == 1 && _saveLoadSlot != 0 && !_saveLoadCompatible) {
			// Display "Save successful" message, except for auto saves
#ifdef __PALM_OS__
			char buf[256]; // 1024 is too big overflow the stack
#else
			char buf[1024];
#endif
			sprintf(buf, "Successfully saved game state in file:\n\n%s", filename);
	
			GUI::TimedMessageDialog dialog(buf, 1500);
			runDialog(dialog);
		}
		if (success && _saveLoadFlag != 1)
			clearClickedStatus();
		_saveLoadFlag = 0;
		_lastSaveTime = _system->get_msecs();
	}

	if (_completeScreenRedraw) {
		_completeScreenRedraw = false;
		gdi.clearCharsetMask();
		_charset->_hasMask = false;

		// HACK as in game save stuff isn't supported currently
		if (_gameId == GID_LOOM || _gameId == GID_LOOM256) {
			int args = 2;
			uint value;
			if (_features & GF_MACINTOSH)
				value = 105;
			else
 				value = (_gameId == GID_LOOM256) ? 150 : 100;
			byte restoreScript = (_features & GF_FMTOWNS) ? 17 : 18;
			// if verbs should be shown restore them
			if (VAR(value) == 2)
				runScript(restoreScript, 0, 0, &args);
		} else if (_version > 3) {
			for (int i = 0; i < _maxVerbs; i++)
				drawVerb(i, 0);
		} else {
			redrawVerbs();
		}

		verbMouseOver(0);

		if (_version <= 2) {
			redrawV2Inventory();
			checkV2MouseOver(_mouse);
		}

		_verbRedraw = false;
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

		if (camera._cur.x != camera._last.x || _BgNeedsRedraw || _fullRedraw
				|| ((_features & GF_NEW_CAMERA) && camera._cur.y != camera._last.y)) {
			redrawBGAreas();
		}

		processDrawQue();

		if (_verbRedraw) {
			redrawVerbs();
		}
	
		setActorRedrawFlags();
		resetActorBgs();

		if (!(_features & GF_NEW_OPCODES) &&
		    !(VAR(VAR_CURRENT_LIGHTS) & LIGHTMODE_screen) &&
		      VAR(VAR_CURRENT_LIGHTS) & LIGHTMODE_flashlight) {
			drawFlashlight();
			setActorRedrawFlags();
		}

		processActors();
		_fullRedraw = false;
		cyclePalette();
		palManipulate();

		if (_doEffect) {
			_doEffect = false;
			fadeIn(_newEffect);
			clearClickedStatus();
		}


		if (!_verbRedraw && _cursor.state > 0) {
			verbMouseOver(checkMouseOver(_mouse.x, _mouse.y));
		}
		_verbRedraw = false;

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

		if (_gameId == GID_FT) {
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
	camera._last = camera._cur;

	if (!(++_expire_counter)) {
		increaseResourceCounter();
	}

	animateCursor();
	
	/* show or hide mouse */
	_system->show_mouse(_cursor.state > 0);

	if (VAR_TIMER != 0xFF)
		VAR(VAR_TIMER) = 0;
	return VAR(VAR_TIMER_NEXT);

}

#pragma mark -
#pragma mark --- Events / Input ---
#pragma mark -

void ScummEngine::parseEvents() {
	OSystem::Event event;

	while (_system->poll_event(&event)) {

		switch(event.event_code) {
		case OSystem::EVENT_KEYDOWN:
			if (event.kbd.keycode >= '0' && event.kbd.keycode<='9'
				&& (event.kbd.flags == OSystem::KBD_ALT ||
					event.kbd.flags == OSystem::KBD_CTRL)) {
				_saveLoadSlot = event.kbd.keycode - '0';

				//  don't overwrite autosave (slot 0)
				if (_saveLoadSlot == 0)
					_saveLoadSlot = 10;

				sprintf(_saveLoadName, "Quicksave %d", _saveLoadSlot);
				_saveLoadFlag = (event.kbd.flags == OSystem::KBD_ALT) ? 1 : 2;
				_saveLoadCompatible = false;
			} else if (event.kbd.flags == OSystem::KBD_CTRL) {
				if (event.kbd.keycode == 'f')
					_fastMode ^= 1;
				else if (event.kbd.keycode == 'g')
					_fastMode ^= 2;
				else if (event.kbd.keycode == 'd')
					_debugger->attach();
				else if (event.kbd.keycode == 's')
					resourceStats();
				else
					_keyPressed = event.kbd.ascii;	// Normal key press, pass on to the game.
			} else if (event.kbd.flags & OSystem::KBD_ALT) {
				// The result must be 273 for Alt-W
				// because that's what MI2 looks for in
				// its "instant win" cheat.
				_keyPressed = event.kbd.keycode + 154;
			} else if (event.kbd.ascii == 315 && _gameId == GID_CMI) {
				// FIXME: support in-game menu screen. For now, this remaps F1 to F5 in COMI
				_keyPressed = 319;
			} else if (_gameId == GID_INDY4 && event.kbd.ascii >= '0' && event.kbd.ascii <= '9') {
				// To support keyboard fighting in FOA, we need to remap the number keys.
				// FOA apparently expects PC scancode values (see script 46 if you want
				// to know where I got these numbers from).
				static const int numpad[10] = {
						'0',
						335, 336, 337,
						331, 332, 333,
						327, 328, 329
					};
				_keyPressed = numpad[event.kbd.ascii - '0'];
			} else if (event.kbd.ascii < 273 || event.kbd.ascii > 276) {
				// don't let game have arrow keys as we currently steal them
				// for keyboard cursor control
				// this fixes bug with up arrow (273) corresponding to
				// "instant win" cheat in MI2 mentioned above
				_keyPressed = event.kbd.ascii;	// Normal key press, pass on to the game.
			}
			break;

		case OSystem::EVENT_MOUSEMOVE:
			_mouse.x = event.mouse.x;
			_mouse.y = event.mouse.y;
			break;

		case OSystem::EVENT_LBUTTONDOWN:
			_leftBtnPressed |= msClicked|msDown;
#if defined(_WIN32_WCE) || defined(__PALM_OS__)
			_mouse.x = event.mouse.x;
			_mouse.y = event.mouse.y;
#endif
			break;

		case OSystem::EVENT_RBUTTONDOWN:
			_rightBtnPressed |= msClicked|msDown;
#if defined(_WIN32_WCE) || defined(__PALM_OS__)
			_mouse.x = event.mouse.x;
			_mouse.y = event.mouse.y;
#endif
			break;

		case OSystem::EVENT_LBUTTONUP:
			_leftBtnPressed &= ~msDown;
			break;

		case OSystem::EVENT_RBUTTONUP:
			_rightBtnPressed &= ~msDown;
			break;
	
		case OSystem::EVENT_QUIT:
			if (_confirmExit)
				confirmexitDialog();
			else
			_quit = true;
			break;
	
		default:
			break;
		}
	}
}

void ScummEngine::clearClickedStatus() {
	_keyPressed = 0;
	_mouseButStat = 0;
	_leftBtnPressed &= ~msClicked;
	_rightBtnPressed &= ~msClicked;
}

void ScummEngine::processKbd() {
	int saveloadkey;

	_lastKeyHit = _keyPressed;
	_keyPressed = 0;
	if (_version <= 2 && 315 <= _lastKeyHit && _lastKeyHit < 315+12) {
		// Convert F-Keys for V1/V2 games (they start at 1 instead of at 315)
		_lastKeyHit -= 314;
	}
	
	
	//
	// Clip the mouse coordinates, and compute _virtualMouse.x (and clip it, too)
	//
	if (_mouse.x < 0)
		_mouse.x = 0;
	if (_mouse.x > _screenWidth-1)
		_mouse.x = _screenWidth-1;
	if (_mouse.y < 0)
		_mouse.y = 0;
	if (_mouse.y > _screenHeight-1)
		_mouse.y = _screenHeight-1;

	_virtualMouse.x = _mouse.x + virtscr[0].xstart;
	_virtualMouse.y = _mouse.y - virtscr[0].topline;
	if (_features & GF_NEW_CAMERA)
		_virtualMouse.y += _screenTop;

	if (_virtualMouse.y < 0)
		_virtualMouse.y = -1;
	if (_virtualMouse.y >= virtscr[0].height)
		_virtualMouse.y = -1;

	//
	// Determine the mouse button state.
	//
	_mouseButStat = 0;

	// Interpret 'return' as left click and 'tab' as right click
	if (_lastKeyHit && _cursor.state > 0) {
		if (_lastKeyHit == 9) {
			_mouseButStat = MBS_RIGHT_CLICK;
			_lastKeyHit = 0;
		} else if (_lastKeyHit == 13) {
			_mouseButStat = MBS_LEFT_CLICK;
			_lastKeyHit = 0;
		}
	}

	if (_leftBtnPressed & msClicked && _rightBtnPressed & msClicked && _version > 3) {
		// Pressing both mouse buttons is treated as if you pressed
		// the cutscene exit key (i.e. ESC in most games). That mimicks
		// the behaviour of the original engine where pressing both
		// mouse buttons also skips the current cutscene.
		_mouseButStat = 0;
		_lastKeyHit = (uint)VAR(VAR_CUTSCENEEXIT_KEY);
	} else if (_rightBtnPressed & msClicked && (_version < 4 && _gameId != GID_LOOM)) {
		// Pressing right mouse button is treated as if you pressed
		// the cutscene exit key (i.e. ESC in most games). That mimicks
		// the behaviour of the original engine where pressing right
		// mouse button also skips the current cutscene.
		_mouseButStat = 0;
		_lastKeyHit = (uint)VAR(VAR_CUTSCENEEXIT_KEY);
	} else if (_leftBtnPressed & msClicked) {
		_mouseButStat = MBS_LEFT_CLICK;
	} else if (_rightBtnPressed & msClicked) {
		_mouseButStat = MBS_RIGHT_CLICK;
	}

	if (_version == 8) {
		VAR(VAR_MOUSE_BUTTONS) = 0;
		VAR(VAR_MOUSE_HOLD) = 0;
		VAR(VAR_RIGHTBTN_HOLD) = 0;

		if (_leftBtnPressed & msClicked)
			VAR(VAR_MOUSE_BUTTONS) += 1;

		if (_rightBtnPressed & msClicked)
			VAR(VAR_MOUSE_BUTTONS) += 2;

		if (_leftBtnPressed & msDown)
			VAR(VAR_MOUSE_HOLD) += 1;

		if (_rightBtnPressed & msDown) {
			VAR(VAR_RIGHTBTN_HOLD) = 1;
			VAR(VAR_MOUSE_HOLD) += 2;
		}
	} else if (_version == 7) {
		VAR(VAR_LEFTBTN_HOLD) = (_leftBtnPressed & msDown) != 0;
		VAR(VAR_RIGHTBTN_HOLD) = (_rightBtnPressed & msDown) != 0;
	}

	_leftBtnPressed &= ~msClicked;
	_rightBtnPressed &= ~msClicked;

	if (!_lastKeyHit)
		return;

	// If a key script was specified (a V8 feature), and it's trigger
	// key was pressed, run it.
	if (_keyScriptNo && (_keyScriptKey == _lastKeyHit)) {
		runScript(_keyScriptNo, 0, 0, 0);
		return;
	}

#ifdef _WIN32_WCE
	if (_lastKeyHit == KEY_SET_OPTIONS) {
		//_newgui->optionsDialog();
		return;
	}

	if (_lastKeyHit == KEY_ALL_SKIP) {
		// Skip cutscene
		if (_insaneState) {
			_videoFinished = true;
			return;
		}
		else
		if (vm.cutScenePtr[vm.cutSceneStackPointer])
			_lastKeyHit = (uint16)VAR(VAR_CUTSCENEEXIT_KEY);
		else 
		// Skip talk 
		if (_talkDelay > 0) 
			_lastKeyHit = (uint16)VAR(VAR_TALKSTOP_KEY);
		else
		// Escape
			_lastKeyHit = 27;
	}
#endif

	if (VAR_RESTART_KEY != 0xFF && _lastKeyHit == VAR(VAR_RESTART_KEY)) {
		warning("Restart not implemented");
		//restart();
		return;
	}

	if ((VAR_PAUSE_KEY != 0xFF && _lastKeyHit == VAR(VAR_PAUSE_KEY)) ||
		(VAR_PAUSE_KEY == 0xFF && _lastKeyHit == ' ')) {
		pauseGame();
		return;
	}

	if (_version <= 2)
		saveloadkey = 5;	// F5
	else if ((_version <= 3) || (_gameId == GID_SAMNMAX) || (_gameId == GID_CMI))
		saveloadkey = 319;	// F5
	else
		saveloadkey = VAR(VAR_MAINMENU_KEY);

	if (_lastKeyHit == VAR(VAR_CUTSCENEEXIT_KEY) ||
		(VAR(VAR_CUTSCENEEXIT_KEY) == 4 && _lastKeyHit == 27)) {
		// Skip cutscene (or active SMUSH video). For the V2 games, which
		// normally use F4 for this, we add in a hack that makes escape work,
		// too (just for convenience).
		if (_insaneState) {
			_videoFinished = true;
		} else
			abortCutscene();
		if (_version <= 2) {
			// Ensure that the input script also sees the key press.
			// This is necessary so you can abort the airplane travel
			// in Zak.
			VAR(VAR_KEYPRESS) = VAR(VAR_CUTSCENEEXIT_KEY);
		}
	} else if (_lastKeyHit == saveloadkey) {
		if (VAR_SAVELOAD_SCRIPT != 0xFF && _currentRoom != 0)
			runScript(VAR(VAR_SAVELOAD_SCRIPT), 0, 0, 0);

		mainMenuDialog();		// Display NewGui

		if (VAR_SAVELOAD_SCRIPT != 0xFF && _currentRoom != 0)
			runScript(VAR(VAR_SAVELOAD_SCRIPT2), 0, 0, 0);
		return;
	} else if (VAR_TALKSTOP_KEY != 0xFF && _lastKeyHit == VAR(VAR_TALKSTOP_KEY)) {
		_talkDelay = 0;
		if (_sound->_sfxMode & 2)
			stopTalk();
		return;
	} else if (_lastKeyHit == '[') { // [ Music volume down
		int vol = ConfMan.getInt("music_volume");
		if (!(vol & 0xF) && vol)
			vol -= 16;
		vol = vol & 0xF0;
		ConfMan.set("music_volume", vol);
		if (_imuse)
			_imuse->set_music_volume (vol);
	} else if (_lastKeyHit == ']') { // ] Music volume up
		int vol = ConfMan.getInt("music_volume");
		vol = (vol + 16) & 0xFF0;
		if (vol > 255) vol = 255;
		ConfMan.set("music_volume", vol);
		if (_imuse)
			_imuse->set_music_volume (vol);
	} else if (_lastKeyHit == '-') { // - text speed down
		_defaultTalkDelay += 5;
		if (_defaultTalkDelay > 90)
			_defaultTalkDelay = 90;

		VAR(VAR_CHARINC) = _defaultTalkDelay / 20;
	} else if (_lastKeyHit == '+') { // + text speed up
		_defaultTalkDelay -= 5;
		if (_defaultTalkDelay < 5)
			_defaultTalkDelay = 5;

		VAR(VAR_CHARINC) = _defaultTalkDelay / 20;
	} else if (_lastKeyHit == '~' || _lastKeyHit == '#') { // Debug console
		_debugger->attach();
	} else if (_version <= 2) {
		// Store the input type. So far we can't distinguish
		// between 1, 3 and 5.
		// 1) Verb	2) Scene	3) Inv.		4) Key
		// 5) Sentence Bar

		if (_lastKeyHit) {		// Key Input
			VAR(VAR_KEYPRESS) = _lastKeyHit;
		}
	}

	_mouseButStat = _lastKeyHit;
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
	debug(1, "Loading room %d", room);

	clearMsgQueue();

	fadeOut(_switchRoomEffect2);
	_newEffect = _switchRoomEffect;

	ScriptSlot *ss =  &vm.slot[_currentScript];

	if (_currentScript != 0xFF) {
		if (ss->where == WIO_ROOM || ss->where == WIO_FLOBJECT) {
			if (ss->cutsceneOverride != 0)
				error("Object %d stopped with active cutscene/override in exit", ss->number);
			_currentScript = 0xFF;
		} else if (ss->where == WIO_LOCAL) {
			if (ss->cutsceneOverride != 0) {
				if (_gameId == GID_ZAK256 && _roomResource == 15 && ss->number == 202) {
					// HACK to make Zak256 work (see bug #770093)
					warning("Script %d stopped with active cutscene/override in exit", ss->number);
				} else if (_gameId == GID_INDY3 && (_features & GF_OLD_BUNDLE) && _roomResource == 3) {
					// HACK to make Indy3 Demo work
					warning("Script %d stopped with active cutscene/override in exit", ss->number);
				} else {
					error("Script %d stopped with active cutscene/override in exit", ss->number);
				}
			}
			_currentScript = 0xFF;
		}
	}

	if (!(_features & GF_SMALL_HEADER) && VAR_NEW_ROOM != 0xFF)  // Disable for SH games. Overwrites
		VAR(VAR_NEW_ROOM) = room; // gamevars, eg Zak cashcards

	runExitScript();
	killScriptsAndResources();
	clearEnqueue();
	stopCycle(0);
	_sound->processSoundQues();

	for (i = 1; i < _numActors; i++) {
		_actors[i].hideActor();
	}

	if (_version < 7) {
		for (i = 0; i < 256; i++) {
			_roomPalette[i] = i;
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

	
	if (VAR_V6_SCREEN_WIDTH != 0xFF && VAR_V6_SCREEN_HEIGHT != 0xFF) {
		VAR(VAR_V6_SCREEN_WIDTH) = _roomWidth;
		VAR(VAR_V6_SCREEN_HEIGHT) = _roomHeight;
	}

	VAR(VAR_CAMERA_MIN_X) = _screenWidth / 2;
	VAR(VAR_CAMERA_MAX_X) = _roomWidth - (_screenWidth / 2);

	if (_features & GF_NEW_CAMERA) {
		VAR(VAR_CAMERA_MIN_Y) = _screenHeight / 2;
		VAR(VAR_CAMERA_MAX_Y) = _roomHeight - (_screenHeight / 2);
		setCameraAt(_screenWidth / 2, _screenHeight / 2);
	} else {
		camera._mode = CM_NORMAL;
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
		a->moving = 0;
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
	} else {
		if (a) {
			// FIXME: This hack mostly is there to fix the tomb/statue room
			// in The Dig. What happens there is that when you enter, you are
			// placed at object 399, coords (307,141), which is in box 25.
			// But then the entry script locks that and other boxes. Hence
			// after the entry script runs, you basically can only do one thing
			// in that room, and that is to leave it - which means the game
			// is unfinishable.
			// By calling adjustActorPos, we can solve the problem in this case:
			// there is a very close box (box 12) which contains point (307,144).
			// If we call adjustActorPos, Commander Low is moved into that box,
			// and we can go on. But aqudran looked this up in his IMB DB for
			// The DIG; and nothing like this is done there. Also I am pretty
			// sure this used to work in 0.3.1. So apparently something broke
			// down here, and I have no clue what that might be :-/
			a->adjustActorPos();
		}
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
	byte *roomptr, *searchptr, *roomResPtr;
	const RoomHeader *rmhd;

	_ENCD_offs = 0;
	_EXCD_offs = 0;
	_CLUT_offs = 0;
	_PALS_offs = 0;

	nukeResource(rtMatrix, 1);
	nukeResource(rtMatrix, 2);

	for (i = 1; i < _maxScaleTable; i++)
		nukeResource(rtScaleTable, i);

	memset(_localScriptList, 0, sizeof(_localScriptList));

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
	if (_features & GF_OLD_BUNDLE)
		ptr = 0;
	else
		ptr = findResourceData(MKID('SCAL'), roomptr);
	if (ptr) {
		int s1, s2, y1, y2;
		if (_version == 8) {
			for (i = 1; i < _maxScaleTable; i++, ptr += 16) {
				s1 = READ_LE_UINT32(ptr);
				y1 = READ_LE_UINT32(ptr + 4);
				s2 = READ_LE_UINT32(ptr + 8);
				y2 = READ_LE_UINT32(ptr + 12);
				setScaleSlot(i, 0, y1, s1, 0, y2, s2);
			}
		} else {
			for (i = 1; i < _maxScaleTable; i++, ptr += 8) {
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

				_localScriptList[id - _numGlobalScripts] = READ_LE_UINT16(ptr + 1);
				ptr += 3;
	
				if (_dumpScripts) {
					char buf[32];
					sprintf(buf, "room-%d-", _roomResource);

					// HACK: to determine the sizes of the local scripts, we assume that
					// a) their order in the data file is the same as in the index
					// b) the last script at the same time is the last item in the room "header"
					int len = - (int)_localScriptList[id - _numGlobalScripts] + _resourceHeaderSize;
					if (*ptr)
						len += READ_LE_UINT16(ptr + 1);
					else
						len += READ_LE_UINT16(roomResPtr);
					dumpResource(buf, id, roomResPtr + _localScriptList[id - _numGlobalScripts] - _resourceHeaderSize, len);
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

			_localScriptList[id - _numGlobalScripts] = ptr + 1 - roomptr;
		}
	} else {
		ResourceIterator localScriptIterator(searchptr, false);
		while ((ptr = localScriptIterator.findNext(MKID('LSCR'))) != NULL) {
			int id = 0;

			ptr += _resourceHeaderSize;	/* skip tag & size */

			if (_version == 8) {
				id = READ_LE_UINT32(ptr);
				checkRange(NUM_LOCALSCRIPT + _numGlobalScripts, _numGlobalScripts, id, "Invalid local script %d");
				_localScriptList[id - _numGlobalScripts] = ptr + 4 - roomResPtr;
			} else if (_version == 7) {
				id = READ_LE_UINT16(ptr);
				checkRange(NUM_LOCALSCRIPT + _numGlobalScripts, _numGlobalScripts, id, "Invalid local script %d");
				_localScriptList[id - _numGlobalScripts] = ptr + 2 - roomResPtr;
			} else {
				id = ptr[0];
				_localScriptList[id - _numGlobalScripts] = ptr + 1 - roomResPtr;
			}

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}
		}
	}

	if (_features & GF_OLD_BUNDLE)
		ptr = 0; // TODO ? do 16 bit games use a palette?!?
	else if (_features & GF_SMALL_HEADER)
		ptr = findResourceSmall(MKID('CLUT'), roomptr);
	else
		ptr = findResourceData(MKID('CLUT'), roomptr);

	if (ptr) {
		_CLUT_offs = ptr - roomptr;
		setPaletteFromRes();
	}

	if (_version >= 6) {
		ptr = findResource(MKID('PALS'), roomptr);
		if (ptr) {
			_PALS_offs = ptr - roomptr;
			setPalette(0);
		}
	}

	// Color cycling
	if (_version >= 4) {
		if (_features & GF_SMALL_HEADER)
			ptr = findResourceSmall (MKID('CYCL'), roomptr);
		else 
			ptr = findResourceData(MKID('CYCL'), roomptr);
		if (ptr) {
			initCycl(ptr);
		}
	}

	// Transparent color
	if (_features & GF_OLD_BUNDLE)
		gdi._transparentColor = 255;	// TODO - FIXME
	else {
		ptr = findResourceData(MKID('TRNS'), roomptr);
		if (ptr)
			gdi._transparentColor = ptr[0];
		else if (_version == 8)
			gdi._transparentColor = 5;	// FIXME
		else
			gdi._transparentColor = 255;
	}

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

	int i;

	// Reset some stuff
	_currentRoom = 0;
	_currentScript = 0xFF;
	killAllScriptsExceptCurrent();
	setShake(0);
	_sound->stopAllSounds();

	// Clear the script variables
	for (i = 0; i < 255; i++)
		_scummVars[i] = 0;

	// Empty inventory
	for (i = 0; i < _numGlobalObjects; i++)
		clearOwnerOf(i);

	// Reinit things
	allocateArrays();                   // Reallocate arrays
	readIndexFile();                    // Reread index (reset objectstate etc)
	createResource(rtTemp, 6, 500);     // Create temp buffer
	initScummVars();                    // Reinit scumm variables
	_sound->setupSound();               // Reinit sound engine

	// Re-run bootscript
	runScript(1, 0, 0, &_bootParam);
}

void ScummEngine::startManiac() {
	warning("stub startManiac()");
}

#pragma mark -
#pragma mark --- GUI ---
#pragma mark -

int ScummEngine::runDialog(Dialog &dialog) {
	// Pause sound put
	bool old_soundsPaused = _sound->_soundsPaused;
	_sound->pauseSounds(true);

	// Pause playing smush movie
	bool oldSmushPlay = _smushPlay;
	_smushPlay = false;

	// Open & run the dialog
	int result = dialog.runModal();

	// Restore old cursor
	updateCursor();

	// Resume playing smush movie, if any
	_smushPlay = oldSmushPlay;

	// Resume sound output
	_sound->pauseSounds(old_soundsPaused);
	
	// Return the result
	return result;
}

void ScummEngine::pauseDialog() {
	if (!_pauseDialog)
		_pauseDialog = new PauseDialog(this);
	runDialog(*_pauseDialog);
}

void ScummEngine::mainMenuDialog() {
	if (!_mainMenuDialog)
		_mainMenuDialog = new MainMenuDialog(this);
	runDialog(*_mainMenuDialog);
}

void ScummEngine::optionsDialog() {
	if (!_optionsDialog)
		_optionsDialog = new OptionsDialog(this);
	runDialog(*_optionsDialog);
}

void ScummEngine::confirmexitDialog() {
	if (!_confirmExitDialog)
		_confirmExitDialog = new ConfirmExitDialog(this);

	if (runDialog(*_confirmExitDialog)) {
		_quit = true;
	}
}

char ScummEngine::displayError(bool showCancel, const char *message, ...) {
#ifdef __PALM_OS__
	char buf[256]; // 1024 is too big overflow the stack
#else
	char buf[1024];
#endif
	va_list va;

	va_start(va, message);
	vsprintf(buf, message, va);
	va_end(va);

	GUI::MessageDialog dialog(buf, "OK", "Cancel");
	return runDialog(dialog);
}

#pragma mark -
#pragma mark --- Miscellaneous ---
#pragma mark -

int SJIStoFMTChunk(int f, int s) //convert sjis code to fmt font offset
{
	enum {
		KANA = 0,
		KANJI = 1,
		EKANJI = 2
	};
	int base = s - (s % 32) - 1;
	int c = 0, p = 0, chunk_f = 0, chunk = 0, cr, kanjiType = KANA;

	if (f >= 0x81 && f <= 0x84) kanjiType = KANA;
	if (f >= 0x88 && f <= 0x9f) kanjiType = KANJI;
	if (f >= 0xe0 && f <= 0xea) kanjiType = EKANJI;

	if ((f > 0xe8 || (f == 0xe8 && base >= 0x9f)) || (f > 0x90 || (f == 0x90 && base >= 0x9f))) {
		c = 48; //correction
		p = -8; //correction
	}

	if (kanjiType == KANA) {//Kana
		chunk_f = (f - 0x81) * 2;
	} else if (kanjiType == KANJI) {//Standard Kanji
		p += f - 0x88;
		chunk_f = c + 2 * p;
	} else if (kanjiType == EKANJI) {//Enhanced Kanji
		p += f - 0xe0;
		chunk_f = c + 2 * p;
	}

	if (base == 0x7f && s == 0x7f)
		base -= 0x20; //correction
	if ((base == 0x7f && s == 0x9e) || (base == 0x9f && s == 0xbe) || (base == 0xbf && s == 0xde))
		base += 0x20; //correction

	switch(base) {
	case 0x3f:
		cr = 0; //3f
		if (kanjiType == KANA) chunk = 1;
		else if (kanjiType == KANJI) chunk = 31;
		else if (kanjiType == EKANJI) chunk = 111;
		break;
	case 0x5f:
		cr = 0; //5f
		if (kanjiType == KANA) chunk = 17;
		else if (kanjiType == KANJI) chunk = 47;
		else if (kanjiType == EKANJI) chunk = 127;
		break;
	case 0x7f:
		cr = -1; //80
		if (kanjiType == KANA) chunk = 9;
		else if (kanjiType == KANJI) chunk = 63;
		else if (kanjiType == EKANJI) chunk = 143;
		break;
	case 0x9f:
		cr = 1; //9e
		if (kanjiType == KANA) chunk = 2;
		else if (kanjiType == KANJI) chunk = 32;
		else if (kanjiType == EKANJI) chunk = 112;
		break;
	case 0xbf:
		cr = 1; //be
		if (kanjiType == KANA) chunk = 18;
		else if (kanjiType == KANJI) chunk = 48;
		else if (kanjiType == EKANJI) chunk = 128;
		break;
	case 0xdf:
		cr = 1; //de
		if (kanjiType == KANA) chunk = 10;
		else if (kanjiType == KANJI) chunk = 64;
		else if (kanjiType == EKANJI) chunk = 144;
		break;
	default:
		return 0;
	}
	
	return ((chunk_f + chunk) * 32 + (s - base)) + cr;
}

byte *ScummEngine::get2byteCharPtr(int idx) {
	switch(_language) {
	case Common::KO_KOR:
		idx = ((idx % 256) - 0xb0) * 94 + (idx / 256) - 0xa1;
		break;
	case Common::JA_JPN:
		idx = SJIStoFMTChunk((idx % 256), (idx / 256));
		break;
	case Common::ZH_TWN:
	default:
		idx = 0;
	}
	return 	_2byteFontPtr + ((_2byteWidth + 7) / 8) * _2byteHeight * idx;
}


const char *ScummEngine::getGameDataPath() const {
#ifdef MACOSX
	if (_version == 8 && _gameDataPath == "/Volumes/MONKEY3_") {
		// Special case for COMI on Mac OS X. The mount points on OS X depend
		// on the volume name. Hence if playing from CD, we'd get a problem.
		// So if loading of a resource file fails, we fall back to the (fixed)
		// CD mount points (/Volumes/MONKEY3_1 and /Volumes/MONKEY3_2).
		//
		// The check for whether we play from CD or not is very hackish, though.
		static char buf[256];
		struct stat st;
		int disk = (_scummVars && _scummVars[VAR_CURRENTDISK] == 2) ? 2 : 1;
		sprintf(buf, "/Volumes/MONKEY3_%d", disk);
	
		if (!stat(buf, &st)) {
			return buf;
		}
	
		// Apparently that disk is not inserted. However since many data files
		// (fonts, comi.la0) are on both disks, we also try the other CD.
		disk = (disk == 1) ? 2 : 1;
		sprintf(buf, "/Volumes/MONKEY3_%d", disk);
		return buf;
	}
#endif

	return _gameDataPath.c_str();
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
	if (!_debugger->isAttached()) {
		printf("%s\n", buf2);	// (Print it again in case debugger segfaults)
		_debugger->attach(buf2);
		_debugger->onFrame();
	}
}

#pragma mark -
#pragma mark --- Utilities ---
#pragma mark -

void checkRange(int max, int min, int no, const char *str) {
	if (no < min || no > max) {
#ifdef __PALM_OS__
		char buf[256]; // 1024 is too big overflow the stack
#else
		char buf[1024];
#endif
		sprintf(buf, str, no);
		error("Value %d is out of bounds (%d,%d) (%s)", no, min, max, buf);
	}
}

/**
 * Convert an old style direction to a new style one (angle),
 */
int newDirToOldDir(int dir) {
	if (dir >= 71 && dir <= 109)
		return 1;
	if (dir >= 109 && dir <= 251)
		return 2;
	if (dir >= 251 && dir <= 289)
		return 0;
	return 3;
}

/**
 * Convert an new style (angle) direction to an old style one.
 */
int oldDirToNewDir(int dir) {
	assert(0 <= dir && dir <= 3);
	const int new_dir_table[4] = { 270, 90, 180, 0 };
	return new_dir_table[dir];
}

/**
 * Convert an angle to a simple direction.
 */
int toSimpleDir(int dirType, int dir) {
	if (dirType) {
		const int16 directions[] = { 22,  72, 107, 157, 202, 252, 287, 337 };
		for (int i = 0; i < 7; i++)
			if (dir >= directions[i] && dir <= directions[i+1])
				return i+1;
	} else {
		const int16 directions[] = { 71, 109, 251, 289 };
		for (int i = 0; i < 3; i++)
			if (dir >= directions[i] && dir <= directions[i+1])
				return i+1;
	}

	return 0;
}

/**
 * Convert a simple direction to an angle.
 */
int fromSimpleDir(int dirType, int dir) {
	if (dirType)
		return dir * 45;
	else
		return dir * 90;
}

/**
 * Normalize the given angle - that means, ensure it is positive, and
 * change it to the closest multiple of 45 degree by abusing toSimpleDir.
 */
int normalizeAngle(int angle) {
	int temp;

	temp = (angle + 360) % 360;

	return toSimpleDir(1, temp) * 45;
}

const char *tag2str(uint32 tag) {
	static char str[5];
	str[0] = (char)(tag >> 24);
	str[1] = (char)(tag >> 16);
	str[2] = (char)(tag >> 8);
	str[3] = (char)tag;
	str[4] = '\0';
	return str;
}

} // End of namespace Scumm

using namespace Scumm;

GameList Engine_SCUMM_gameList() {
	const ScummGameSettings *g = scumm_settings;
	GameList games;
	while (g->gameName) {
		games.push_back(g->toGameSettings());
		g++;
	}
	return games;
}

GameList Engine_SCUMM_detectGames(const FSList &fslist) {
	GameList detectedGames;
	const ScummGameSettings *g;
	char detectName[128];
	char detectName2[128];

	for (g = scumm_settings; g->gameName; ++g) {
		// Determine the 'detectname' for this game, that is, the name of a 
		// file that *must* be presented if the directory contains the data
		// for this game. For example, FOA requires atlantis.000
		if (g->version <= 3) {
			strcpy(detectName, "00.LFL");
			detectName2[0] = '\0';
		} else if (g->version == 4) {
			strcpy(detectName, "000.LFL");
			detectName2[0] = '\0';
		} else {
			const char *base = g->baseFilename ? g->baseFilename : g->gameName;
			strcpy(detectName, base);
			strcat(detectName, ".000");
			strcpy(detectName2, base);
			if (g->features & GF_HUMONGOUS) {
				strcat(detectName2, ".he0");
			} else if (g->version >= 7) {
				strcat(detectName2, ".la0");
			} else
				strcat(detectName2, ".sm0");
		}

		// Iterate over all files in the given directory
		for (FSList::ConstIterator file = fslist.begin(); file != fslist.end(); ++file) {
			const char *gameName = file->displayName().c_str();

			if ((0 == scumm_stricmp(detectName, gameName))  || 
				(0 == scumm_stricmp(detectName2, gameName))) {
				// Match found, add to list of candidates, then abort inner loop.
				detectedGames.push_back(g->toGameSettings());
				break;
			}
		}
	}
	return detectedGames;
}

Engine *Engine_SCUMM_create(GameDetector *detector, OSystem *syst) {
	Engine *engine;

	
	const ScummGameSettings *g = scumm_settings;
	while (g->gameName) {
		if (!scumm_stricmp(detector->_gameName.c_str(), g->gameName))
			break;
		g++;
	}
	if (!g->gameName)
		error("Invalid game '%s'\n", detector->_gameName.c_str());

	ScummGameSettings game = *g;

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
	default:
		if (!(game.features & GF_FMTOWNS))
			game.features |= GF_PC;
		break;
	}

	switch (game.version) {
	case 1:
	case 2:
		engine = new ScummEngine_v2(detector, syst, game);
		break;
	case 3:
		engine = new ScummEngine_v3(detector, syst, game);
		break;
	case 4:
		engine = new ScummEngine_v4(detector, syst, game);
		break;
	case 5:
		engine = new ScummEngine_v5(detector, syst, game);
		break;
	case 6:
		engine = new ScummEngine_v6(detector, syst, game);
		break;
	case 7:
		engine = new ScummEngine_v7(detector, syst, game);
		break;
#ifndef __PALM_OS__
	case 8:
		engine = new ScummEngine_v8(detector, syst, game);
		break;
#endif
	default:
		error("Engine_SCUMM_create(): Unknown version of game engine");
	}

	return engine;
}

REGISTER_PLUGIN("Scumm Engine", Engine_SCUMM_gameList, Engine_SCUMM_create, Engine_SCUMM_detectGames)
