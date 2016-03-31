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
#include "adl/adl.h"

namespace Adl {

Console::Console(AdlEngine *engine) : GUI::Debugger() {
	_engine = engine;

	registerCmd("nouns", WRAP_METHOD(Console, Cmd_Nouns));
	registerCmd("verbs", WRAP_METHOD(Console, Cmd_Verbs));
	registerCmd("dump_scripts", WRAP_METHOD(Console, Cmd_DumpScripts));
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

void Console::printWordMap(const WordMap &wordMap) {
	Common::StringArray words;
	WordMap::const_iterator verb;

	for (verb = wordMap.begin(); verb != wordMap.end(); ++verb)
		words.push_back(verb->_key);

	Common::sort(words.begin(), words.end());

	Common::StringArray::const_iterator word;
	for (word = words.begin(); word != words.end(); ++word)
		debugPrintf("%s: %d\n", toAscii(*word).c_str(), wordMap[*word]);
}

} // End of namespace Adl
