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

	// TODO: This is sometimes off by 1... find out why
	//s->_gameObj = g_sci->getResMan()->findGameObject();
	// Replaced by the code below for now
	Script *scr000 = s->_segMan->getScript(1);
	s->_gameObj = make_reg(1, scr000->validateExportFunc(0));
	if (getSciVersion() >= SCI_VERSION_1_1)
		 s->_gameObj.offset += scr000->_scriptSize;

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
