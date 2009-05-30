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
#include "sci/engine/state.h"

namespace Sci {

extern EngineState *g_EngineState;

Console::Console(SciEngine *vm) : GUI::Debugger() {
	_vm = vm;

	DCmd_Register("version",			WRAP_METHOD(Console, cmdGetVersion));
//	DCmd_Register("classes",			WRAP_METHOD(Console, cmdClasses));	// TODO
	DCmd_Register("opcodes",			WRAP_METHOD(Console, cmdOpcodes));
	DCmd_Register("selectors",			WRAP_METHOD(Console, cmdSelectors));
	DCmd_Register("kernelnames",		WRAP_METHOD(Console, cmdKernelNames));
	DCmd_Register("suffixes",			WRAP_METHOD(Console, cmdSuffixes));
	DCmd_Register("kernelwords",		WRAP_METHOD(Console, cmdKernelWords));
	DCmd_Register("hexdump",			WRAP_METHOD(Console, cmdHexDump));
	DCmd_Register("dissect_script",		WRAP_METHOD(Console, cmdDissectScript));
	DCmd_Register("room",				WRAP_METHOD(Console, cmdRoomNumber));
	DCmd_Register("size",				WRAP_METHOD(Console, cmdResourceSize));
	DCmd_Register("restypes",			WRAP_METHOD(Console, cmdResourceTypes));
	DCmd_Register("sci0_palette",		WRAP_METHOD(Console, cmdSci0Palette));
	DCmd_Register("hexgrep",			WRAP_METHOD(Console, cmdHexgrep));
	DCmd_Register("list",				WRAP_METHOD(Console, cmdList));
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

bool Console::cmdExit(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("exit game - exit gracefully\n");
		DebugPrintf("exit now - exit ungracefully\n");
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
