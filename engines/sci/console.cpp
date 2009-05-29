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

namespace Sci {

extern EngineState *g_EngineState;
extern bool g_redirect_sciprintf_to_gui;

class ConsoleFunc : public Common::Functor2<int, const char **, bool> {
public:
	ConsoleFunc(const ConCommand &func, const char *param) : _func(func), _param(param) {}

	bool isValid() const { return _func != 0; }
	bool operator()(int argc, const char **argv) const {
#if 1
		// FIXME: Evil hack: recreate the original input string
		Common::String tmp = argv[0];
		for (int i = 1; i < argc; ++i) {
			tmp += ' ';
			tmp += argv[i];
		}
		g_redirect_sciprintf_to_gui = true;
		con_parse(g_EngineState, tmp.c_str());
		sciprintf("\n");
		g_redirect_sciprintf_to_gui = false;

		return true;
#else
		Common::Array<cmd_param_t> cmdParams;
		for (int i = 1; i < argc; ++i) {
			cmd_param_t tmp;
			tmp.str = argv[i];
			// TODO: Convert argc/argv suitable, using _param
			cmdParams.push_back(tmp);
		}
		assert(g_EngineState);
		return !(*_func)(g_EngineState, cmdParams);
#endif
	}
private:
	EngineState *_s;
	const ConCommand _func;
	const char *_param;
};




Console::Console(SciEngine *vm) : GUI::Debugger() {
	_vm = vm;

	DCmd_Register("version",			WRAP_METHOD(Console, cmdGetVersion));
//	DCmd_Register("classes",			WRAP_METHOD(Console, cmdClasses));	// TODO
	DCmd_Register("opcodes",			WRAP_METHOD(Console, cmdOpcodes));
	DCmd_Register("selectors",			WRAP_METHOD(Console, cmdSelectors));
	DCmd_Register("kernelnames",		WRAP_METHOD(Console, cmdKernelNames));
	DCmd_Register("suffixes",			WRAP_METHOD(Console, cmdSuffixes));
	DCmd_Register("kernelwords",		WRAP_METHOD(Console, cmdKernelWords));
	DCmd_Register("man",				WRAP_METHOD(Console, cmdMan));
	DCmd_Register("hexdump",			WRAP_METHOD(Console, cmdHexDump));
	DCmd_Register("dissect_script",		WRAP_METHOD(Console, cmdDissectScript));
	DCmd_Register("room",				WRAP_METHOD(Console, cmdRoomNumber));
	DCmd_Register("size",				WRAP_METHOD(Console, cmdResourceSize));
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

void Console::con_hook_command(ConCommand command, const char *name, const char *param, const char *description) {
	DCmd_Register(name, new ConsoleFunc(command, param));
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

bool Console::cmdMan(int argc, const char **argv) {
#if 0
	int section = 0;
	unsigned int i;
	char *name = cmd_params[0].str;
	char *c = strchr(name, '.');
	cmd_mm_entry_t *entry = 0;

	if (c) {
		*c = 0;
		section = atoi(c + 1);
	}

	if (section < 0 || section >= CMD_MM_ENTRIES) {
		DebugPrintf("Invalid section %d\n", section);
		return true;
	}

	DebugPrintf("section:%d\n", section);
	if (section)
		entry = cmd_mm_find(name, section - 1);
	else
		for (i = 0; i < CMD_MM_ENTRIES && !section; i++) {
			if ((entry = cmd_mm_find(name, i)))
				section = i + 1;
		}

	if (!entry) {
		DebugPrintf("No manual entry\n");
		return true;
	}

	DebugPrintf("-- %s: %s.%d\n", cmd_mm[section - 1].name, name, section);
	cmd_mm[section - 1].print(entry, 1);

#endif
	return true;
}

bool Console::cmdHexDump(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage: %s <resource type> <resource number>\n", argv[0]);
		DebugPrintf("The 20 valid resource types are:\n");
		// There are 20 resource types supported by SCI1.1
		for (int i = 0; i < 20; i++) {
			DebugPrintf("%s", getResourceTypeName((ResourceType) i));
			DebugPrintf((i < 19) ? ", " : "\n");
		}

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

} // End of namespace Sci
