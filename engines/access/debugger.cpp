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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"
#include "access/access.h"
#include "access/debugger.h"
#include "access/amazon/amazon_game.h"

namespace Access {

static int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

Debugger *Debugger::init(AccessEngine *vm) {
	switch (vm->getGameID()) {
	case GType_Amazon:
		return new Amazon::AmazonDebugger(vm);
	default:
		return new Debugger(vm);
	}
}

void Debugger::postEnter() {
	if (!_playMovieFile.empty()) {
		_vm->playMovie(_playMovieFile, Common::Point(0, 0));

		_playMovieFile.clear();
	}

	GUI::Debugger::postEnter();
}

/*------------------------------------------------------------------------*/

Debugger::Debugger(AccessEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("continue", WRAP_METHOD(Debugger, cmdExit));
	registerCmd("scene", WRAP_METHOD(Debugger, Cmd_LoadScene));
	registerCmd("cheat", WRAP_METHOD(Debugger, Cmd_Cheat));
	registerCmd("playmovie", WRAP_METHOD(Debugger, Cmd_PlayMovie));
}

Debugger::~Debugger() {
}

bool Debugger::Cmd_LoadScene(int argc, const char **argv) {
	switch (argc) {
	case 1:
		debugPrintf("Current scene is: %d\n\n", _vm->_player->_roomNumber);

		for (uint i = 0; i < _vm->_res->ROOMTBL.size(); i++)
			if (!_vm->_res->ROOMTBL[i]._desc.empty())
				debugPrintf("%d - %s\n", i, _vm->_res->ROOMTBL[i]._desc.c_str());
		return true;

	case 2: {
		int newRoom = strToInt(argv[1]);
		if (newRoom < 0 || newRoom >= (int)_vm->_res->ROOMTBL.size()) {
			debugPrintf("Invalid Room Number\n");
			return true;
		}
		if (_vm->_res->ROOMTBL[newRoom]._desc.empty()) {
			debugPrintf("Unused Room Number\n");
			return true;
		}

		_vm->_player->_roomNumber = newRoom;

		_vm->_room->_function = FN_CLEAR1;
		_vm->freeChar();
		_vm->_converseMode = 0;
		_vm->_scripts->_endFlag = true;
		_vm->_scripts->_returnCode = 0;

		return false;
		}
	default:
		debugPrintf("Current scene is: %d\n", _vm->_player->_roomNumber);
		debugPrintf("Usage: %s <scene number>\n", argv[0]);
		return true;
	}
}

bool Debugger::Cmd_Cheat(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		debugPrintf("Switches on/off the cheat mode\n");
		return true;
	}

	_vm->_cheatFl = !_vm->_cheatFl;
	debugPrintf("Cheat is now %s\n", _vm->_cheatFl ? "ON" : "OFF");
	return true;
}

bool Debugger::Cmd_PlayMovie(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: playmovie <movie-file>\n");
		return true;
	}

	// play gets postponed until debugger is closed
	Common::String filename = argv[1];
	_playMovieFile = filename;

	return cmdExit(0, 0);
}

/*------------------------------------------------------------------------*/

namespace Amazon {

AmazonDebugger::AmazonDebugger(AccessEngine *vm) : Debugger(vm) {
	registerCmd("chapter", WRAP_METHOD(AmazonDebugger, Cmd_StartChapter));
}

bool AmazonDebugger::Cmd_StartChapter(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <chapter number>\n", argv[0]);
		return true;
	}

	// Build up a simple one line script to start the given chapter
	byte *chapterScript = (byte *)malloc(5);
	chapterScript[0] = SCRIPT_START_BYTE;
	chapterScript[1] = ROOM_SCRIPT % 256;
	chapterScript[2] = ROOM_SCRIPT / 256;
	chapterScript[3] = 0x80 + 75;			// cmdChapter
	chapterScript[4] = strToInt(argv[1]);	// chapter number
	_vm->_scripts->setScript(new Resource(chapterScript, 5), true);

	return false;
}

} // End of namespace Amazon

} // End of namespace Access
