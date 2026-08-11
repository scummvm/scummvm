/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tinsel/tinsel.h"
#include "tinsel/debugger.h"
#include "tinsel/dialogs.h"
#include "tinsel/handle.h"
#include "tinsel/pcode.h"
#include "tinsel/psx_archive.h"
#include "tinsel/scene.h"
#include "tinsel/sound.h"
#include "tinsel/music.h"
#include "tinsel/font.h"
#include "tinsel/strres.h"
#include "tinsel/noir/notebook.h"

namespace Tinsel {

//----------------- EXTERNAL FUNCTIONS ---------------------

// In PDISPLAY.CPP
extern void TogglePathDisplay();
// In tinsel.cpp
extern void SetNewScene(SCNHANDLE scene, int entrance, int transition);
// In scene.cpp
extern SCNHANDLE GetSceneHandle();

//----------------- SUPPORT FUNCTIONS ---------------------

//static
int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp;
	if (!sscanf(s, "%xh", &tmp))
		tmp = 0;
	return (int)tmp;
}

//----------------- CONSOLE CLASS  ---------------------

Console::Console() : GUI::Debugger() {
	if (TinselVersion == 3) {
		registerCmd("add_clue",		WRAP_METHOD(Console, cmd_add_clue));
		registerCmd("add_all_clues",	WRAP_METHOD(Console, cmd_add_all_clues));
		registerCmd("cross_clue",		WRAP_METHOD(Console, cmd_cross_clue));
		registerCmd("list_clues",		WRAP_METHOD(Console, cmd_list_clues));
	}
	registerCmd("item",		WRAP_METHOD(Console, cmd_item));
	registerCmd("scenes",		WRAP_METHOD(Console, cmd_scenes));
	registerCmd("scene",		WRAP_METHOD(Console, cmd_scene));
	registerCmd("music",		WRAP_METHOD(Console, cmd_music));
	registerCmd("sound",		WRAP_METHOD(Console, cmd_sound));
	registerCmd("string",		WRAP_METHOD(Console, cmd_string));
	registerCmd("globals",		WRAP_METHOD(Console, cmd_globals));
	registerCmd("global",		WRAP_METHOD(Console, cmd_global));
	registerCmd("g",		WRAP_METHOD(Console, cmd_global)); // alias
	registerCmd("psxdump",		WRAP_METHOD(Console, cmd_psxdump));
}

Console::~Console() {
}

bool Console::cmd_item(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("%s item_number\n", argv[0]);
		debugPrintf("Sets the currently active 'held' item\n");
		return true;
	}

	_vm->_dialogs->holdItem(INV_NOICON);
	_vm->_dialogs->holdItem(strToInt(argv[1]));
	return false;
}

bool Console::cmd_scenes(int argc, const char **argv) {
	Common::String filter;
	if (argc >= 2) {
		filter = argv[1];
	}

	for (int i = 0; i < _vm->_handle->GetSceneCount(); i++) {
		Common::String name = _vm->_handle->GetSceneName(i);
		if (!name.empty()) {
			if (filter.empty() || name.hasPrefixIgnoreCase(filter)) {
				debugPrintf("scene %d: %s\n", i, name.c_str());
			}
		}
	}
	return true;
}

bool Console::cmd_scene(int argc, const char **argv) {
	if (argc < 1 || argc > 3) {
		debugPrintf("%s [scene [entry number]]\n", argv[0]);
		debugPrintf("If no parameters are given, prints the current scene.\n");
		debugPrintf("Otherwise changes to the specified scene number. Entry number defaults to 1 if none provided\n");
		return true;
	}

	if (argc == 1) {
		int index = GetSceneHandle() >> SCNHANDLE_SHIFT;
		Common::String name = _vm->_handle->GetSceneName(index);
		debugPrintf("Current scene is %d: %s\n", index, name.c_str());
		return true;
	}

	// lookup scene by name first, then by number
	uint32 sceneNumber = 0;
	for (int i = 1; i < _vm->_handle->GetSceneCount(); i++) {
		Common::String name = _vm->_handle->GetSceneName(i);
		if (name.hasPrefixIgnoreCase(argv[1])) {
			sceneNumber = i;
			break;
		}
	}
	if (sceneNumber == 0) {
		sceneNumber = (uint32)strToInt(argv[1]);
	}
	if (!(1 <= sceneNumber && sceneNumber < (uint32)_vm->_handle->GetSceneCount())) {
		debugPrintf("Invalid scene: %s\n", argv[1]);
		return true;
	}

	int entryNumber = (argc >= 3) ? strToInt(argv[2]) : 1;
	if (entryNumber < 1) {
		debugPrintf("Invalid entry: %s\n", argv[2]);
		return true;
	}

	EndDw1Intro();
	SetNewScene(sceneNumber << SCNHANDLE_SHIFT, entryNumber, TRANS_CUT);
	return false;
}

bool Console::cmd_music(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("%s track_number or %s -offset\n", argv[0], argv[0]);
		debugPrintf("Plays the MIDI track number provided, or the offset inside midi.dat\n");
		debugPrintf("A positive number signifies a track number, whereas a negative signifies an offset\n");
		return true;
	}

	int param = strToInt(argv[1]);
	if (param == 0) {
		debugPrintf("Track number/offset can't be 0!\n");
	} else if (param > 0) {
		// Track provided
		_vm->_music->PlayMidiSequence(_vm->_music->GetTrackOffset(param - 1), false);
	} else if (param < 0) {
		// Offset provided
		param = param * -1;
		_vm->_music->PlayMidiSequence(param, false);
	}
	return true;
}

bool Console::cmd_sound(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("%s id\n", argv[0]);
		debugPrintf("Plays the sound with the given ID\n");
		return true;
	}

	int id = strToInt(argv[1]);
	if (_vm->_sound->sampleExists(id)) {
		if (TinselVersion <= 1)
			_vm->_sound->playSample(id, Audio::Mixer::kSpeechSoundType);
		else
			_vm->_sound->playSample(id, 0, false, 0, 0, PRIORITY_TALK, Audio::Mixer::kSpeechSoundType);
	} else {
		debugPrintf("Sample %d does not exist!\n", id);
	}

	return true;
}

bool Console::cmd_string(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("%s id\n", argv[0]);
		debugPrintf("Prints the string with the given ID\n");
		return true;
	}

	char tmp[TBUFSZ];
	int id = strToInt(argv[1]);
	LoadStringRes(id, tmp, TBUFSZ);
	debugPrintf("%s\n", tmp);

	return true;
}

bool Console::cmd_globals(int argc, const char **argv) {
	for (int i = 0; i < GetGlobalCount(); i++) {
		int value = GetGlobal(i);
		debugPrintf("global %d == %08x (%d)\n", i, value, value);
	}
	return true;
}

bool Console::cmd_global(int argc, const char **argv) {
	if (!(2 <= argc && argc <= 3)) {
		debugPrintf("Print or set global variable\n");
		debugPrintf("usage: %s <global> [new-value]\n", argv[0]);
		return true;
	}

	int global = strToInt(argv[1]);
	if (!(0 <= global && global < GetGlobalCount())) {
		debugPrintf("maximum global: %d\n", GetGlobalCount() - 1);
		return true;
	}

	if (argc == 3) {
		int newValue = strToInt(argv[2]);
		SetGlobal(global, newValue);
	}

	int value = GetGlobal(global);
	debugPrintf("global %d == %08x (%d)\n", global, value, value);
	return true;
}

bool Console::cmd_psxdump(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Dumps PSX resource files to disk\n");
		debugPrintf("usage: %s <file>\n", argv[0]);
		debugPrintf("       <file> may be '*' to dump all files\n");
		return true;
	}

	if (!TinselV1PSX) {
		return true;
	}

	PsxArchive psxArchive;
	if (!psxArchive.open("discwld.lfi", "discwld.lfd", TinselVersion)) {
		return true;
	}

	bool dumpAll = !strcmp(argv[1], "*");
	Common::ArchiveMemberList members;
	psxArchive.listMembers(members);
	Common::sort(members.begin(), members.end(), Common::ArchiveMemberListComparator());
	for (auto &member : members) {
		if (dumpAll || member->getName().equalsIgnoreCase(argv[1])) {
			Common::Path path = member->getPathInArchive();
			Common::ScopedPtr<Common::SeekableReadStream> resource(psxArchive.createReadStreamForMember(path));
			if (resource) {
				Common::DumpFile dumpFile;
				if (dumpFile.open(member->getPathInArchive())) {
					debugPrintf("Writing %s to disk\n", member->getName().c_str());
					dumpFile.writeStream(resource.get());
				}
			}
		}
	}

	return true;
}

// Noir:
bool Console::cmd_add_clue(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("%s clue_id\n", argv[0]);
		debugPrintf("Adds a clue to the notebook\n");
		return true;
	}

	_vm->_notebook->addClue(strToInt(argv[1]));
	return false;
}

bool Console::cmd_add_all_clues(int argc, const char **argv) {
	auto clues = _vm->_dialogs->getAllNotebookClues();
	for (auto clue : clues) {
		_vm->_notebook->addClue(clue);
	}
	return false;
}

bool Console::cmd_cross_clue(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("%s clue_id\n", argv[0]);
		debugPrintf("Crosses out a clue in the notebook\n");
		return true;
	}

	_vm->_notebook->crossClue(strToInt(argv[1]));
	return false;
}

bool Console::cmd_list_clues(int argc, const char **argv) {
	auto clues = _vm->_dialogs->getAllNotebookClues();
	for (auto clue : clues) {
		debugPrintf("%d\n", clue);
	}
	return true;
}

} // End of namespace Tinsel
