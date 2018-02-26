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

#include "script.h"

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stream.h"
#include "common/debug.h"
#include "mutationofjb/commands/command.h"
#include "mutationofjb/commands/ifcommand.h"
#include "mutationofjb/commands/endblockcommand.h"
#include "mutationofjb/commands/changecommand.h"

namespace MutationOfJB {

static CommandParser** getParsers() {
	static CommandParser* parsers[] = {
		new IfCommandParser,
		new EndBlockCommandParser,
		new ChangeDoorCommandParser,
		new ChangeObjectCommandParser,
		new ChangeStaticCommandParser,
		nullptr
	};

	return parsers;
}


ScriptParseContext::ScriptParseContext(Common::SeekableReadStream &stream) :
	_stream(stream),
	_currentCommand(nullptr),
	_lastCommand(nullptr)
{}

bool ScriptParseContext::readLine(Common::String &line) {
	do {
		Common::String str = _stream.readLine();
		if (str.empty())
			continue;

		if (str[0] != '.') {
			line = str;
			if (line[0] == '*') {
				line.deleteChar(0);
			}
			return true;
		}
	} while(_stream.eos());

	return false;
}

void ScriptParseContext::addConditionalCommand(ConditionalCommand *command, char tag) {
	ConditionalCommandInfo cmi = {command, tag};
	_pendingCondCommands.push_back(cmi);
}

bool Script::loadFromStream(Common::SeekableReadStream &stream) {
	destroy();

	CommandParser **parsers = getParsers();

	ScriptParseContext parseCtx(stream);

	Common::String line;

	Command *lastCmd = nullptr;
	CommandParser *lastParser = nullptr;
	while (parseCtx.readLine(line)) {
		Command *currentCmd = nullptr;
		CommandParser *currentParser = nullptr;

		for (CommandParser **parser = parsers; *parser; ++parser) {
			if ((*parser)->parse(line, parseCtx, currentCmd)) {
				currentParser = *parser;
				break;
			}
		}
		if (lastParser) {
			lastParser->transition(parseCtx, lastCmd, currentCmd);
		}

		if (currentCmd) {
			_allCommands.push_back(currentCmd);
		}

		lastCmd = currentCmd;
		lastParser = currentParser;
	}

	Common::HashMap<Common::String, Command *> macros;
	Common::HashMap<Common::String, Command *> labels;

	return true;
}

void Script::destroy() {
	for (Commands::iterator it = _allCommands.begin(); it != _allCommands.end(); ++it) {
		delete *it;
	}
	_allCommands.clear();
}

Script::~Script() {
	destroy();
}

}
