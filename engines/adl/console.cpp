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

#include "common/debug-channels.h"

#include "adl/console.h"
#include "adl/display.h"
#include "adl/adl.h"

namespace Adl {

Console::Console(AdlEngine *engine) : GUI::Debugger() {
	_engine = engine;

	registerCmd("nouns", WRAP_METHOD(Console, Cmd_Nouns));
	registerCmd("verbs", WRAP_METHOD(Console, Cmd_Verbs));
	registerCmd("dump_scripts", WRAP_METHOD(Console, Cmd_DumpScripts));
	registerCmd("valid_cmds", WRAP_METHOD(Console, Cmd_ValidCommands));
	registerCmd("room", WRAP_METHOD(Console, Cmd_Room));
}

Common::String Console::toAscii(const Common::String &str) {
	Common::String ascii(str);

	for (uint i = 0; i < ascii.size(); ++i)
		ascii.setChar(ascii[i] & 0x7f, i);

	return ascii;
}

bool Console::Cmd_Verbs(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	debugPrintf("Verbs in alphabetical order:\n");
	printWordMap(_engine->_verbs);
	return true;
}

bool Console::Cmd_Nouns(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	debugPrintf("Nouns in alphabetical order:\n");
	printWordMap(_engine->_nouns);
	return true;
}

bool Console::Cmd_ValidCommands(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	WordMap::const_iterator verb, noun;
	bool is_any;

	for (verb = _engine->_verbs.begin(); verb != _engine->_verbs.end(); ++verb) {
		for (noun = _engine->_nouns.begin(); noun != _engine->_nouns.end(); ++noun) {
			if (_engine->isInputValid(verb->_value, noun->_value, is_any) && !is_any)
				debugPrintf("%s %s\n", toAscii(verb->_key).c_str(), toAscii(noun->_key).c_str());
		}
		if (_engine->isInputValid(verb->_value, IDI_ANY, is_any))
			debugPrintf("%s *\n", toAscii(verb->_key).c_str());
	}
	if (_engine->isInputValid(IDI_ANY, IDI_ANY, is_any))
		debugPrintf("* *\n");

	return true;
}

bool Console::Cmd_DumpScripts(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	bool oldFlag = DebugMan.isDebugChannelEnabled(kDebugChannelScript);

	DebugMan.enableDebugChannel("Script");

	_engine->_dumpFile = new Common::DumpFile();

	for (byte roomNr = 1; roomNr <= _engine->_state.rooms.size(); ++roomNr) {
		_engine->loadRoom(roomNr);
		if (_engine->_roomData.commands.size() != 0) {
			_engine->_dumpFile->open(Common::String::format("%03d.ADL", roomNr).c_str());
			_engine->doAllCommands(_engine->_roomData.commands, IDI_ANY, IDI_ANY);
			_engine->_dumpFile->close();
		}
	}
	_engine->loadRoom(_engine->_state.room);

	_engine->_dumpFile->open("GLOBAL.ADL");
	_engine->doAllCommands(_engine->_globalCommands, IDI_ANY, IDI_ANY);
	_engine->_dumpFile->close();

	_engine->_dumpFile->open("RESPONSE.ADL");
	_engine->doAllCommands(_engine->_roomCommands, IDI_ANY, IDI_ANY);
	_engine->_dumpFile->close();

	delete _engine->_dumpFile;
	_engine->_dumpFile = nullptr;

	if (!oldFlag)
		DebugMan.disableDebugChannel("Script");

	return true;
}

bool Console::Cmd_Room(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("Usage: %s [<new_room>]\n", argv[0]);
		return true;
	}

	if (argc == 2) {
		if (!_engine->_canRestoreNow) {
			debugPrintf("Cannot change rooms right now\n");
			return true;
		}

		int roomCount = _engine->_state.rooms.size();
		int room = strtoul(argv[1], NULL, 0);
		if (room < 1 || room > roomCount) {
			debugPrintf("Room %i out of valid range [1, %d]\n", room, roomCount);
			return true;
		}

		_engine->_state.room = room;
		_engine->clearScreen();
		_engine->loadRoom(_engine->_state.room);
		_engine->showRoom();
		_engine->_display->updateTextScreen();
		_engine->_display->updateHiResScreen();
	}

	debugPrintf("Current room: %d\n", _engine->_state.room);

	return true;
}

void Console::printWordMap(const WordMap &wordMap) {
	Common::StringArray words;
	WordMap::const_iterator verb;

	for (verb = wordMap.begin(); verb != wordMap.end(); ++verb)
		words.push_back(Common::String::format("%s: %3d", toAscii(verb->_key).c_str(), wordMap[verb->_key]));

	Common::sort(words.begin(), words.end());

	debugPrintColumns(words);
}

} // End of namespace Adl
