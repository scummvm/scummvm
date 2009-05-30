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

// Console module

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/resource.h"
#include "sci/vocabulary.h"
#include "sci/engine/savegame.h"
#include "sci/engine/state.h"
#include "sci/engine/gc.h"
#include "sci/gfx/gfx_state_internal.h"
#include "sci/vocabulary.h"

#include "common/savefile.h"

namespace Sci {

extern EngineState *g_EngineState;

int _kdebug_cheap_event_hack = 0;

Console::Console(SciEngine *vm) : GUI::Debugger() {
	_vm = vm;

	DCmd_Register("version",			WRAP_METHOD(Console, cmdGetVersion));
//	DCmd_Register("classes",			WRAP_METHOD(Console, cmdClasses));	// TODO
	DCmd_Register("opcodes",			WRAP_METHOD(Console, cmdOpcodes));
	DCmd_Register("selectors",			WRAP_METHOD(Console, cmdSelectors));
	DCmd_Register("kernel_names",		WRAP_METHOD(Console, cmdKernelNames));
	DCmd_Register("suffixes",			WRAP_METHOD(Console, cmdSuffixes));
	DCmd_Register("kernel_words",		WRAP_METHOD(Console, cmdKernelWords));
	DCmd_Register("hexdump",			WRAP_METHOD(Console, cmdHexDump));
	DCmd_Register("dissect_script",		WRAP_METHOD(Console, cmdDissectScript));
	DCmd_Register("room",				WRAP_METHOD(Console, cmdRoomNumber));
	DCmd_Register("size",				WRAP_METHOD(Console, cmdResourceSize));
	DCmd_Register("resource_types",		WRAP_METHOD(Console, cmdResourceTypes));
	DCmd_Register("sci0_palette",		WRAP_METHOD(Console, cmdSci0Palette));
	DCmd_Register("hexgrep",			WRAP_METHOD(Console, cmdHexgrep));
	DCmd_Register("list",				WRAP_METHOD(Console, cmdList));
	DCmd_Register("clear_screen",		WRAP_METHOD(Console, cmdClearScreen));
	DCmd_Register("redraw_screen",		WRAP_METHOD(Console, cmdRedrawScreen));
	DCmd_Register("save_game",			WRAP_METHOD(Console, cmdSaveGame));
	DCmd_Register("restore_game",		WRAP_METHOD(Console, cmdRestoreGame));
	DCmd_Register("restart_game",		WRAP_METHOD(Console, cmdRestartGame));
	DCmd_Register("class_table",		WRAP_METHOD(Console, cmdClassTable));
	DCmd_Register("parser_words",		WRAP_METHOD(Console, cmdParserWords));
	DCmd_Register("current_port",		WRAP_METHOD(Console, cmdCurrentPort));
	DCmd_Register("parse_grammar",		WRAP_METHOD(Console, cmdParseGrammar));
	DCmd_Register("visual_state",		WRAP_METHOD(Console, cmdVisualState));
	DCmd_Register("dynamic_views",		WRAP_METHOD(Console, cmdDynamicViews));
	DCmd_Register("dropped_views",		WRAP_METHOD(Console, cmdDroppedViews));
	DCmd_Register("simkey",				WRAP_METHOD(Console, cmdSimulateKey));
	DCmd_Register("segment_table",		WRAP_METHOD(Console, cmdPrintSegmentTable));
	DCmd_Register("show_map",			WRAP_METHOD(Console, cmdShowMap));
	DCmd_Register("gc",					WRAP_METHOD(Console, cmdInvokeGC));
	DCmd_Register("gc_objects",			WRAP_METHOD(Console, cmdGCObjects));
	DCmd_Register("exit",				WRAP_METHOD(Console, cmdExit));

	// These were in sci.cpp
	/*
	con_hook_int(&(gfx_options.buffer_pics_nr), "buffer_pics_nr",
		"Number of pics to buffer in LRU storage\n");
	con_hook_int(&(gfx_options.pic0_dither_mode), "pic0_dither_mode",
		"Mode to use for pic0 dithering\n");
	con_hook_int(&(gfx_options.pic0_dither_pattern), "pic0_dither_pattern",
		"Pattern to use for pic0 dithering\n");
	con_hook_int(&(gfx_options.pic0_unscaled), "pic0_unscaled",
		"Whether pic0 should be drawn unscaled\n");
	con_hook_int(&(gfx_options.dirty_frames), "dirty_frames",
		"Dirty frames management\n");
	*/
}

Console::~Console() {
}

static ResourceType parseResourceType(const char *resid) {
	// Gets the resource number of a resource string, or returns -1
	ResourceType res = kResourceTypeInvalid;

	for (int i = 0; i < kResourceTypeInvalid; i++)
		if (strcmp(getResourceTypeName((ResourceType)i), resid) == 0)
			res = (ResourceType)i;

	return res;
}

bool Console::cmdGetVersion(int argc, const char **argv) {
	int ver = _vm->getVersion();

	DebugPrintf("Resource file version:        %s\n", sci_version_types[_vm->getResMgr()->_sciVersion]);
	DebugPrintf("Emulated interpreter version: %s\n", versionNames[ver]);

	return true;
}

bool Console::cmdOpcodes(int argc, const char **argv) {
	Common::Array<opcode> opcodes;

	if (!vocab_get_opcodes(_vm->getResMgr(), opcodes)) {
		DebugPrintf("No opcode name table found!\n");
		return true;
	}

	DebugPrintf("Opcode names in numeric order [index: type name]:\n");
	for (uint seeker = 0; seeker < opcodes.size(); seeker++) {
		opcode &op = opcodes[seeker];
		DebugPrintf("%03x: %03x %20s | ", seeker, op.type, op.name.c_str());
		if ((seeker % 3) == 2)
			DebugPrintf("\n");
	}

	DebugPrintf("\n");

	return true;
}

bool Console::cmdSelectors(int argc, const char **argv) {
	Common::StringList selectorNames;

	if (!vocab_get_snames(_vm->getResMgr(), (_vm->getFlags() & GF_SCI0_OLD), selectorNames)) {
		DebugPrintf("No selector name table found!\n");
		return true;
	}

	DebugPrintf("Selector names in numeric order:\n");
	for (uint seeker = 0; seeker < selectorNames.size(); seeker++) {
		DebugPrintf("%03x: %20s | ", seeker, selectorNames[seeker].c_str());
		if ((seeker % 3) == 2)
			DebugPrintf("\n");
	}

	DebugPrintf("\n");

	return true;
}

bool Console::cmdKernelNames(int argc, const char **argv) {
	Common::StringList kernelNames;

	vocab_get_knames(_vm->getResMgr(), kernelNames);

	if (kernelNames.empty()) {
		DebugPrintf("No kernel name table found!\n");
		return true;
	}

	DebugPrintf("Selector names in numeric order:\n");
	for (uint seeker = 0; seeker < kernelNames.size(); seeker++) {
		DebugPrintf("%03x: %20s | ", seeker, kernelNames[seeker].c_str());
		if ((seeker % 3) == 2)
			DebugPrintf("\n");
	}

	DebugPrintf("\n");

	return true;
}

bool Console::cmdSuffixes(int argc, const char **argv) {
	SuffixList suffixes;
	char word_buf[256], alt_buf[256];

	if (!vocab_get_suffixes(_vm->getResMgr(), suffixes)) {
		DebugPrintf("No suffix vocabulary.\n");
		return true;
	}

	int i = 0;
	for (SuffixList::const_iterator suf = suffixes.begin(); suf != suffixes.end(); ++suf) {
		strncpy(word_buf, suf->word_suffix, suf->word_suffix_length);
		word_buf[suf->word_suffix_length] = 0;
		strncpy(alt_buf, suf->alt_suffix, suf->alt_suffix_length);
		alt_buf[suf->alt_suffix_length] = 0;

		DebugPrintf("%4d: (%03x) -%12s  =>  -%12s (%03x)\n", i, suf->class_mask, word_buf, alt_buf, suf->result_class);
		++i;
	}

	vocab_free_suffixes(_vm->getResMgr(), suffixes);

	return true;
}

bool Console::cmdKernelWords(int argc, const char **argv) {
	WordMap words;

	vocab_get_words(_vm->getResMgr(), words);

	if (words.empty()) {
		DebugPrintf("No vocabulary.\n");
		return true;
	}

	int j = 0;
	for (WordMap::iterator i = words.begin(); i != words.end(); ++i) {
		DebugPrintf("%4d: %03x [%03x] %20s |", j, i->_value._class, i->_value._group, i->_key.c_str());
		if (j % 3 == 0)
			DebugPrintf("\n");
		j++;
	}

	DebugPrintf("\n");

	return true;
}

bool Console::cmdHexDump(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage: %s <resource type> <resource number>\n", argv[0]);
		cmdResourceTypes(argc, argv);
		return true;
	}

	int resNum = atoi(argv[2]);
	if (resNum == 0) {
		DebugPrintf("The resource number specified is not a number");
		return true;
	}

	ResourceType res = parseResourceType(argv[1]);

	if (res == kResourceTypeInvalid)
		DebugPrintf("Resource type '%s' is not valid\n", argv[1]);
	else {
		Resource *resource = _vm->getResMgr()->findResource(res, resNum, 0);
		if (resource) {
			Common::hexdump(resource->data, resource->size, 16, 0);
			DebugPrintf("Resource %s.%03d has been dumped to standard output\n", argv[1], resNum);
		} else {
			DebugPrintf("Resource %s.%03d not found\n", argv[1], resNum);
		}
	}

	return true;
}

bool Console::cmdDissectScript(int argc, const char **argv) {
	Common::StringList selectorNames;

	if (argc != 2) {
		DebugPrintf("Examines a script\n");
		DebugPrintf("Usage: %s <script number>\n", argv[0]);
		return true;
	}

	if (!vocab_get_snames(_vm->getResMgr(), (_vm->getFlags() & GF_SCI0_OLD), selectorNames)) {
		DebugPrintf("No selector name table found!\n");
		return true;
	}

	script_dissect(_vm->getResMgr(), atoi(argv[1]), selectorNames);

	return true;
}

bool Console::cmdRoomNumber(int argc, const char **argv) {
	DebugPrintf("Current room number is %d\n", g_EngineState->currentRoomNumber());

	return true;
}

bool Console::cmdResourceSize(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Shows the size of a resource\n");
		DebugPrintf("Usage: %s <resource type> <resource number>\n", argv[0]);
		return true;
	}

	int resNum = atoi(argv[2]);
	if (resNum == 0) {
		DebugPrintf("The resource number specified is not a number");
		return true;
	}

	ResourceType res = parseResourceType(argv[1]);

	if (res == kResourceTypeInvalid)
		DebugPrintf("Resource type '%s' is not valid\n", argv[1]);
	else {
		Resource *resource = _vm->getResMgr()->findResource(res, resNum, 0);
		if (resource) {
			DebugPrintf("Resource size: %d\n", resource->size);
		} else {
			DebugPrintf("Resource %s.%03d not found\n", argv[1], resNum);
		}
	}

	return true;
}

bool Console::cmdResourceTypes(int argc, const char **argv) {
	DebugPrintf("The %d valid resource types are:\n", kResourceTypeInvalid);
	for (int i = 0; i < kResourceTypeInvalid; i++) {
		DebugPrintf("%s", getResourceTypeName((ResourceType) i));
		DebugPrintf((i < kResourceTypeInvalid - 1) ? ", " : "\n");
	}

	return true;
}

extern int sci0_palette;

bool Console::cmdSci0Palette(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Set the SCI0 palette to use - 0: EGA, 1: AGI/Amiga, 2: Grayscale\n");
		return true;
	}

	sci0_palette = atoi(argv[1]);
	// TODO: the current room has to be changed to reset the palette of the views
	game_init_graphics(g_EngineState);

	return false;
}

bool Console::cmdHexgrep(int argc, const char **argv) {
	if (argc < 4) {
		DebugPrintf("Searches some resources for a particular sequence of bytes, represented as hexadecimal numbers.\n");
		DebugPrintf("Usage: %s <resource type> <resource number> <search string>\n", argv[0]);
		DebugPrintf("<resource number> can be a specific resource number, or \"all\" for all of the resources of the specified type\n", argv[0]);
		DebugPrintf("EXAMPLES:\n  hexgrep script all e8 03 c8 00\n  hexgrep pic 042 fe");
		cmdResourceTypes(argc, argv);
		return true;
	}

	ResourceType restype = parseResourceType(argv[1]);
	int resNumber = 0, resMax = 0;
	char seekString[500];
	Resource *script = NULL;

	if (restype == kResourceTypeInvalid) {
		DebugPrintf("Resource type '%s' is not valid\n", argv[1]);
		return true;
	}

	if (!scumm_stricmp(argv[2], "all")) {
		resNumber = 0;
		resMax = 999;
	} else {
		resNumber = resMax = atoi(argv[2]);
	}

	strcpy(seekString, argv[3]);

	// Construct the seek string
	for (int i = 4; i < argc; i++) {
		strcat(seekString, argv[i]);
	}

	for (; resNumber <= resMax; resNumber++) {
		if ((script = _vm->getResMgr()->findResource(restype, resNumber, 0))) {
			unsigned int seeker = 0, seekerold = 0;
			uint32 comppos = 0;
			int output_script_name = 0;

			while (seeker < script->size) {
				if (script->data[seeker] == seekString[comppos]) {
					if (comppos == 0)
						seekerold = seeker;

					comppos++;

					if (comppos == strlen(seekString)) {
						comppos = 0;
						seeker = seekerold + 1;

						if (!output_script_name) {
							DebugPrintf("\nIn %s.%03d:\n", getResourceTypeName((ResourceType)restype), resNumber);
							output_script_name = 1;
						}
						DebugPrintf("   0x%04x\n", seekerold);
					}
				} else
					comppos = 0;

				seeker++;
			}
		}
	}

	return true;
}

bool Console::cmdList(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Lists all of the resources of a given type\n");
		cmdResourceTypes(argc, argv);
		return true;
	}


	ResourceType res = parseResourceType(argv[1]);
	if (res == kResourceTypeInvalid)
		DebugPrintf("Unknown resource type: '%s'\n", argv[1]);
	else {
		int j = 0;
		for (int i = 0; i < sci_max_resource_nr[_vm->getResMgr()->_sciVersion]; i++) {
			if (_vm->getResMgr()->testResource(res, i)) {
				DebugPrintf("%s.%03d | ", getResourceTypeName((ResourceType)res), i);
				if (j % 5 == 0)
					DebugPrintf("\n");
				j++;
			}
		}
		DebugPrintf("\n");
	}

	return true;
}

bool Console::cmdClearScreen(int argc, const char **argv) {
	gfxop_clear_box(g_EngineState->gfx_state, gfx_rect(0, 0, 320, 200));
	gfxop_update_box(g_EngineState->gfx_state, gfx_rect(0, 0, 320, 200));
	return false;
}

bool Console::cmdRedrawScreen(int argc, const char **argv) {
	g_EngineState->visual->draw(Common::Point(0, 0));
	gfxop_update_box(g_EngineState->gfx_state, gfx_rect(0, 0, 320, 200));
	gfxop_update(g_EngineState->gfx_state);
	gfxop_sleep(g_EngineState->gfx_state, 0);
	return false;
}

bool Console::cmdSaveGame(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Save the current game state to the hard disk\n");
		DebugPrintf("Usage: %s <filename>\n", argv[0]);
		return true;
	}

	int result = 0;
	for (uint i = 0; i < g_EngineState->_fileHandles.size(); i++)
		if (g_EngineState->_fileHandles[i].isOpen())
			result++;

	if (result)
		DebugPrintf("Note: Game state has %d open file handles.\n", result);

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::OutSaveFile *out;
	if (!(out = saveFileMan->openForSaving(argv[1]))) {
		DebugPrintf("Error opening savegame \"%s\" for writing\n", argv[1]);
		return true;
	}

	// TODO: enable custom descriptions? force filename into a specific format?
	if (gamestate_save(g_EngineState, out, "debugging")) {
		DebugPrintf("Saving the game state to '%s' failed\n", argv[1]);
	}

	return true;
}

bool Console::cmdRestoreGame(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Restores a saved game from the hard disk\n");
		DebugPrintf("Usage: %s <filename>\n", argv[0]);
		return true;
	}

	EngineState *newstate = NULL;

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::SeekableReadStream *in;
	if (!(in = saveFileMan->openForLoading(argv[1]))) {
		// found a savegame file
		newstate = gamestate_restore(g_EngineState, in);
		delete in;
	}

	if (newstate) {
		g_EngineState->successor = newstate; // Set successor

		script_abort_flag = SCRIPT_ABORT_WITH_REPLAY; // Abort current game
		_debugstate_valid = 0;

		shrink_execution_stack(g_EngineState, g_EngineState->execution_stack_base + 1);
		return 0;
	} else {
		sciprintf("Restoring gamestate '%s' failed.\n", argv[1]);
		return 1;
	}

	return false;
}

bool Console::cmdRestartGame(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Restarts the game. There are two ways to restart a SCI game:\n");
		DebugPrintf("%s play - calls the game object's play() method\n", argv[0]);
		DebugPrintf("%s replay - calls the replay() methody\n", argv[0]);
		return true;
	}

	if (!scumm_stricmp(argv[1], "play")) {
		g_EngineState->restarting_flags |= SCI_GAME_WAS_RESTARTED_AT_LEAST_ONCE;
	} else if (!scumm_stricmp(argv[1], "replay")) {
		g_EngineState->restarting_flags &= ~SCI_GAME_WAS_RESTARTED_AT_LEAST_ONCE;
	} else {
		DebugPrintf("Invalid usage of %s\n", argv[0]);
		return true;
	}

	g_EngineState->restarting_flags |= SCI_GAME_IS_RESTARTING_NOW;
	script_abort_flag = 1;
	_debugstate_valid = 0;

	return false;
}

bool Console::cmdClassTable(int argc, const char **argv) {
	DebugPrintf("Available classes:\n");
	for (uint i = 0; i < g_EngineState->_classtable.size(); i++) {
		if (g_EngineState->_classtable[i].reg.segment) {
			DebugPrintf(" Class 0x%x at %04x:%04x (script 0x%x)\n", i, 
					PRINT_REG(g_EngineState->_classtable[i].reg), g_EngineState->_classtable[i].script);
		}
	}

	return true;
}

bool Console::cmdParserWords(int argc, const char **argv) {
	if (g_EngineState->_parserWords.empty()) {
		DebugPrintf("No words.\n");
		return true;
	}

	for (WordMap::iterator i = g_EngineState->_parserWords.begin(); i != g_EngineState->_parserWords.end(); ++i)
		DebugPrintf("%s: C %03x G %03x\n", i->_key.c_str(), i->_value._class, i->_value._group);

	DebugPrintf("%d words\n", g_EngineState->_parserWords.size());

	return true;
}

bool Console::cmdCurrentPort(int argc, const char **argv) {
	if (!g_EngineState->port)
		DebugPrintf("Current port number: none.\n");
	else
		DebugPrintf("Current port number: %d\n", g_EngineState->port->_ID);

	return true;
}

bool Console::cmdParseGrammar(int argc, const char **argv) {
	DebugPrintf("Parse grammar, in strict GNF:\n");

	parse_rule_list_t *tlist = vocab_build_gnf(g_EngineState->_parserBranches, 1);
	DebugPrintf("%d allocd rules\n", getAllocatedRulesCount());
	vocab_free_rule_list(tlist);

	return true;
}

bool Console::cmdVisualState(int argc, const char **argv) {
	DebugPrintf("State of the current visual widget:\n");

	if (g_EngineState->visual)
		g_EngineState->visual->print(0);
	else
		DebugPrintf("The visual widget is uninitialized.\n");

	return true;
}

bool Console::cmdDynamicViews(int argc, const char **argv) {
	DebugPrintf("List of active dynamic views:\n");

	if (g_EngineState->dyn_views)
		g_EngineState->dyn_views->print(0);
	else
		DebugPrintf("The list is empty.\n");

	return true;
}

bool Console::cmdDroppedViews(int argc, const char **argv) {
	DebugPrintf("List of dropped dynamic views:\n");

	if (g_EngineState->drop_views)
		g_EngineState->drop_views->print(0);
	else
		DebugPrintf("The list is empty.\n");

	return true;
}

bool Console::cmdSimulateKey(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Simulate a keypress with the specified scancode\n");
		DebugPrintf("Usage: %s <key scan code>\n", argv[0]);
		return true;
	}

	_kdebug_cheap_event_hack = atoi(argv[1]);

	return true;
}

bool Console::cmdPrintSegmentTable(int argc, const char **argv) {
	DebugPrintf("Segment table:\n");

	for (uint i = 0; i < g_EngineState->seg_manager->_heap.size(); i++) {
		MemObject *mobj = g_EngineState->seg_manager->_heap[i];
		if (mobj && mobj->getType()) {
			DebugPrintf(" [%04x] ", i);

			switch (mobj->getType()) {
			case MEM_OBJ_SCRIPT:
				DebugPrintf("S  script.%03d l:%d ", (*(Script *)mobj).nr, (*(Script *)mobj).lockers);
				break;

			case MEM_OBJ_CLONES:
				DebugPrintf("C  clones (%d allocd)", (*(CloneTable *)mobj).entries_used);
				break;

			case MEM_OBJ_LOCALS:
				DebugPrintf("V  locals %03d", (*(LocalVariables *)mobj).script_id);
				break;

			case MEM_OBJ_STACK:
				DebugPrintf("D  data stack (%d)", (*(DataStack *)mobj).nr);
				break;

			case MEM_OBJ_SYS_STRINGS:
				DebugPrintf("Y  system string table");
				break;

			case MEM_OBJ_LISTS:
				DebugPrintf("L  lists (%d)", (*(ListTable *)mobj).entries_used);
				break;

			case MEM_OBJ_NODES:
				DebugPrintf("N  nodes (%d)", (*(NodeTable *)mobj).entries_used);
				break;

			case MEM_OBJ_HUNK:
				DebugPrintf("H  hunk (%d)", (*(HunkTable *)mobj).entries_used);
				break;

			case MEM_OBJ_DYNMEM:
				DebugPrintf("M  dynmem: %d bytes", (*(DynMem *)mobj)._size);
				break;

			case MEM_OBJ_STRING_FRAG:
				DebugPrintf("F  string fragments");
				break;

			default:
				DebugPrintf("I  Invalid (type = %x)", mobj->getType());
				break;
			}

			DebugPrintf("  seg_ID = %d \n", mobj->getSegMgrId());
		}
	}
	DebugPrintf("\n");

	return true;
}

bool Console::cmdShowMap(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Shows one of the screen maps\n");
		DebugPrintf("Usage: %s <screen map>\n", argv[0]);
		DebugPrintf("Screen maps:\n");
		DebugPrintf("- 0: visual map (back buffer)\n");
		DebugPrintf("- 1: priority map (back buffer)\n");
		DebugPrintf("- 2: control map (static buffer)\n");
		return true;
	}

	gfxop_set_clip_zone(g_EngineState->gfx_state, gfx_rect_fullscreen);

	int map = atoi(argv[1]);

	switch (map) {
	case 0:
		g_EngineState->visual->add_dirty_abs((GfxContainer *)g_EngineState->visual, gfx_rect(0, 0, 320, 200), 0);
		g_EngineState->visual->draw(Common::Point(0, 0));
		break;

	case 1:
		gfx_xlate_pixmap(g_EngineState->gfx_state->pic->priority_map, g_EngineState->gfx_state->driver->mode, GFX_XLATE_FILTER_NONE);
		gfxop_draw_pixmap(g_EngineState->gfx_state, g_EngineState->gfx_state->pic->priority_map, gfx_rect(0, 0, 320, 200), Common::Point(0, 0));
		break;

	case 2:
		gfx_xlate_pixmap(g_EngineState->gfx_state->control_map, g_EngineState->gfx_state->driver->mode, GFX_XLATE_FILTER_NONE);
		gfxop_draw_pixmap(g_EngineState->gfx_state, g_EngineState->gfx_state->control_map, gfx_rect(0, 0, 320, 200), Common::Point(0, 0));
		break;

	default:
		DebugPrintf("Map %d is not available.\n", map);
		return true;
	}

	gfxop_update(g_EngineState->gfx_state);

	return false;
}

bool Console::cmdInvokeGC(int argc, const char **argv) {
	DebugPrintf("Performing garbage collection...\n");
	run_gc(g_EngineState);
	return true;
}

bool Console::cmdGCObjects(int argc, const char **argv) {
	reg_t_hash_map *use_map = find_all_used_references(g_EngineState);

	DebugPrintf("Reachable object references (normalised):\n");
	for (reg_t_hash_map::iterator i = use_map->begin(); i != use_map->end(); ++i) {
		DebugPrintf(" - %04x:%04x\n", PRINT_REG(i->_key));
	}

	delete use_map;

	return true;
}

bool Console::cmdExit(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("%s game - exit gracefully\n", argv[0]);
		DebugPrintf("%s now - exit ungracefully\n", argv[0]);
		return true;
	}

	if (!scumm_stricmp(argv[1], "game")) {
		// Quit gracefully
		script_abort_flag = 1; // Terminate VM
		_debugstate_valid = 0;
		_debug_seeking = 0;
		_debug_step_running = 0;

	} else if (!scumm_stricmp(argv[1], "now")) {
		// Quit ungracefully
		exit(0);
	}

	return false;
}

} // End of namespace Sci
