/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "common/system.h"
#include "common/file.h"

#include "engines/advancedDetector.h"	// for ADGF_DEMO

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/message.h"
#include "sci/graphics/gui.h"
#include "sci/graphics/menu.h"
#include "sci/sound/audio.h"
#include "sci/sound/music.h"

namespace Sci {

struct OldNewIdTableEntry {
	const char *oldId;
	const char *newId;
	SciVersion version;
};

static const OldNewIdTableEntry s_oldNewTable[] = {
	{ "arthur",		"camelot",			SCI_VERSION_NONE     },
	{ "brain",      "castlebrain",      SCI_VERSION_1_MIDDLE },	// Amiga
	{ "brain",      "castlebrain",      SCI_VERSION_1_LATE   },
	{ "demo",		"christmas1988",	SCI_VERSION_NONE     },
	{ "card",       "christmas1990",    SCI_VERSION_1_EARLY, },
	{ "card",       "christmas1992",    SCI_VERSION_1_1      },
	{ "RH Budget",	"cnick-longbow",	SCI_VERSION_NONE     },
	// iceman is the same
	{ "icedemo",	"iceman",			SCI_VERSION_NONE     },
	// longbow is the same
	{ "eco",		"ecoquest",			SCI_VERSION_NONE     },
	{ "eco2",		"ecoquest2",		SCI_VERSION_NONE     },	// EcoQuest 2 demo
	{ "rain",		"ecoquest2",		SCI_VERSION_NONE     },	// EcoQuest 2 full
	{ "fp",			"freddypharkas",	SCI_VERSION_NONE     },
	{ "emc",		"funseeker",		SCI_VERSION_NONE     },
	{ "gk",			"gk1",				SCI_VERSION_NONE     },
	{ "hoyledemo",	"hoyle1",			SCI_VERSION_NONE     },
	{ "cardgames",	"hoyle1",			SCI_VERSION_NONE     },
	{ "solitare",	"hoyle2",			SCI_VERSION_NONE     },
	// hoyle3 is the same
	// hoyle4 is the same
	{ "brain",      "islandbrain",      SCI_VERSION_1_1      },
	{ "demo000",	"kq1sci",			SCI_VERSION_NONE     },
	{ "kq1",		"kq1sci",			SCI_VERSION_NONE     },
	{ "kq4",		"kq4sci",			SCI_VERSION_NONE     },
	{ "mm1",		"laurabow",			SCI_VERSION_NONE     },
	{ "cb1",		"laurabow",			SCI_VERSION_NONE     },
	{ "lb2",		"laurabow2",		SCI_VERSION_NONE     },
	{ "rh",			"longbow",			SCI_VERSION_NONE     },
	{ "ll1",		"lsl1sci",			SCI_VERSION_NONE     },
	{ "lsl1",		"lsl1sci",			SCI_VERSION_NONE     },
	// lsl2 is the same
	{ "lsl3",		"lsl3",				SCI_VERSION_NONE     },
	{ "ll5",		"lsl5",				SCI_VERSION_NONE     },
	// lsl5 is the same
	// lsl6 is the same
	{ "mg",			"mothergoose",		SCI_VERSION_NONE     },
	{ "twisty",		"pepper",			SCI_VERSION_NONE     },
	{ "pq1",		"pq1sci",			SCI_VERSION_NONE     },
	{ "pq",			"pq2",				SCI_VERSION_NONE     },
	// pq3 is the same
	// pq4 is the same
	{ "tales",		"fairytales",		SCI_VERSION_NONE     },
	{ "hq",			"qfg1",				SCI_VERSION_NONE     },	// QFG1 SCI0/EGA
	{ "glory",      "qfg1",             SCI_VERSION_0_LATE   },	// QFG1 SCI0/EGA
	{ "trial",		"qfg2",				SCI_VERSION_NONE     },
	{ "hq2demo",	"qfg2",				SCI_VERSION_NONE     },
	{ "thegame",	"slater",			SCI_VERSION_NONE     },
	{ "sq1demo",	"sq1sci",			SCI_VERSION_NONE     },
	{ "sq1",		"sq1sci",			SCI_VERSION_NONE     },
	// sq3 is the same
	// sq4 is the same
	// sq5 is the same
	// torin is the same

	// TODO: SCI2.1, SCI3 IDs

	{ "", "", SCI_VERSION_NONE }
};

Common::String convertSierraGameId(const char *gameId, uint32 *gameFlags, ResourceManager *resMan) {
	// Convert the id to lower case, so that we match all upper/lower case variants.
	Common::String sierraId = gameId;
	sierraId.toLowercase();

	// If the game has less than the expected scripts, it's a demo
	uint32 demoThreshold = 100;
	// ...but there are some exceptions
	if (sierraId == "brain" || sierraId == "lsl1" ||
		sierraId == "mg" || sierraId == "pq" ||
		sierraId == "jones" ||
		sierraId == "cardgames" || sierraId == "solitare" ||
		sierraId == "hoyle3" || sierraId == "hoyle4")
		demoThreshold = 40;
	if (sierraId == "fp" || sierraId == "gk" || sierraId == "pq4")
		demoThreshold = 150;

	Common::List<ResourceId> *resources = resMan->listResources(kResourceTypeScript, -1);
	if (resources->size() < demoThreshold) {
		*gameFlags |= ADGF_DEMO;

		// Crazy Nick's Picks
		if (sierraId == "lsl1" && resources->size() == 34)
			return "cnick-lsl";
		if (sierraId == "sq4" && resources->size() == 34)
			return "cnick-sq";

		// TODO: cnick-kq, cnick-laurabow and cnick-longbow (their resources can't be read)

		// Handle Astrochicken 1 (SQ3) and 2 (SQ4)
		if (sierraId == "sq3" && resources->size() == 20)
			return "astrochicken";
		if (sierraId == "sq4")
			return "msastrochicken";
	}

	for (const OldNewIdTableEntry *cur = s_oldNewTable; cur->oldId[0]; ++cur) {
		if (sierraId == cur->oldId) {
			// Distinguish same IDs from the SCI version
			if (cur->version != SCI_VERSION_NONE && cur->version != getSciVersion())
				continue;

			return cur->newId;
		}
	}

	if (sierraId == "glory") {
		// This could either be qfg1 VGA, qfg3 or qfg4 demo (all SCI1.1),
		// or qfg4 full (SCI2)
		// qfg1 VGA doesn't have view 1
		if (!resMan->testResource(ResourceId(kResourceTypeView, 1)))
			return "qfg1";

		// qfg4 full is SCI2
		if (getSciVersion() == SCI_VERSION_2)
			return "qfg4";

		// qfg4 demo has less than 50 scripts
		if (resources->size() < 50)
			return "qfg4";

		// Otherwise it's qfg3
		return "qfg3";
	}

	return sierraId;
}

#ifdef USE_OLD_MUSIC_FUNCTIONS
int game_init_sound(EngineState *s, int sound_flags, SciVersion soundVersion) {
	if (getSciVersion() > SCI_VERSION_0_LATE)
		sound_flags |= SFX_STATE_FLAG_MULTIPLAY;

	s->sfx_init_flags = sound_flags;
	s->_sound.sfx_init(g_sci->getResMan(), sound_flags, soundVersion);

	return 0;
}
#endif

// Architectural stuff: Init/Unintialize engine
int script_init_engine(EngineState *s) {
	s->_msgState = new MessageState(s->_segMan);
	s->gc_countdown = GC_INTERVAL - 1;

	SegmentId script_000_segment = s->_segMan->getScriptSegment(0, SCRIPT_GET_LOCK);

	if (script_000_segment <= 0) {
		debug(2, "Failed to instantiate script.000");
		return 1;
	}

	s->script_000 = s->_segMan->getScript(script_000_segment);

	s->sys_strings = s->_segMan->allocateSysStrings(&s->sys_strings_segment);

	// Allocate static buffer for savegame and CWD directories
	SystemString *str = &s->sys_strings->_strings[SYS_STRING_SAVEDIR];
	str->_name = "savedir";
	str->_maxSize = MAX_SAVE_DIR_SIZE;
	str->_value = (char *)calloc(MAX_SAVE_DIR_SIZE, sizeof(char));

	s->r_acc = s->r_prev = NULL_REG;
	s->restAdjust = 0;

	s->_executionStack.clear();    // Start without any execution stack
	s->execution_stack_base = -1; // No vm is running yet

	s->restarting_flags = SCI_GAME_IS_NOT_RESTARTING;

	debug(2, "Engine initialized");

	return 0;
}

/*************************************************************/
/* Game instance stuff: Init/Unitialize state-dependant data */
/*************************************************************/

int game_init(EngineState *s) {
	// FIXME Use new VM instantiation code all over the place
	DataStack *stack;

	stack = s->_segMan->allocateStack(VM_STACK_SIZE, NULL);
	s->stack_base = stack->_entries;
	s->stack_top = stack->_entries + stack->_capacity;

	if (!script_instantiate(g_sci->getResMan(), s->_segMan, 0)) {
		warning("game_init(): Could not instantiate script 0");
		return 1;
	}

	if (s->_voc) {
		s->_voc->parserIsValid = false; // Invalidate parser
		s->_voc->parser_event = NULL_REG; // Invalidate parser event
		s->_voc->parser_base = make_reg(s->sys_strings_segment, SYS_STRING_PARSER_BASE);
	}

	// Initialize menu TODO: Actually this should be another init()
	if (g_sci->_gfxMenu)
		g_sci->_gfxMenu->reset();

	s->successor = NULL; // No successor

	SystemString *str = &s->sys_strings->_strings[SYS_STRING_PARSER_BASE];
	str->_name = "parser-base";
	str->_maxSize = MAX_PARSER_BASE;
	str->_value = (char *)calloc(MAX_PARSER_BASE, sizeof(char));

	s->game_start_time = g_system->getMillis();
	s->last_wait_time = s->game_start_time;

	srand(g_system->getMillis()); // Initialize random number generator

//	script_dissect(0, s->_selectorNames);
	// The first entry in the export table of script 0 points to the game object
	s->_gameObj = s->_segMan->lookupScriptExport(0, 0);
	uint32 gameFlags = 0;	// unused
	s->_gameId = convertSierraGameId(s->_segMan->getObjectName(s->_gameObj), &gameFlags, g_sci->getResMan());

	debug(2, " \"%s\" at %04x:%04x", s->_gameId.c_str(), PRINT_REG(s->_gameObj));

#ifdef USE_OLD_MUSIC_FUNCTIONS
	if (s->sfx_init_flags & SFX_STATE_FLAG_NOSOUND)
		game_init_sound(s, 0, g_sci->_features->detectDoSoundType());
#endif

	// Load game language into printLang property of game object
	// FIXME: It's evil to achieve this as a side effect of a getter.
	// Much better to have an explicit init method for this.
	g_sci->getSciLanguage();

	return 0;
}

int game_exit(EngineState *s) {
	s->_executionStack.clear();

	if (!s->successor) {
#ifdef USE_OLD_MUSIC_FUNCTIONS
		s->_sound.sfx_exit();
		// Reinit because some other code depends on having a valid state
		game_init_sound(s, SFX_STATE_FLAG_NOSOUND, g_sci->_features->detectDoSoundType());
#else
		g_sci->_audio->stopAllAudio();
		s->_soundCmd->clearPlayList();
#endif
	}

	// Note: It's a bad idea to delete the segment manager here
	// when loading a game.
	// This function is called right after a game is loaded, and
	// the segment manager has already been initialized from the
	// save game. Deleting or resetting it here will result in
	// invalidating the loaded save state
	if (s->restarting_flags & SCI_GAME_IS_RESTARTING_NOW)
		s->_segMan->resetSegMan();

	// TODO Free parser segment here

	// TODO Free scripts here

	// Close all opened file handles
	s->_fileHandles.clear();
	s->_fileHandles.resize(5);

	return 0;
}

} // End of namespace Sci
