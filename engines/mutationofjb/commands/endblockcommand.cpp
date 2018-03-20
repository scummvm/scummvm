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

#include "mutationofjb/commands/endblockcommand.h"
#include "mutationofjb/script.h"
#include "mutationofjb/commands/conditionalcommand.h"
#include "common/str.h"
#include "common/debug.h"

namespace MutationOfJB {

bool EndBlockCommandParser::parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) {
	if (line.empty()) {
		return false;
	}

	const char firstChar = line.firstChar();
	if (firstChar != '#' && firstChar != '=' && firstChar != '-') {
		return false;
	}

	// This is the start or end of section/block.

	if (line.size() >= 4 && (line.hasPrefix("#L ") || line.hasPrefix("-L "))) {
		ActionInfo ai = {ActionInfo::Look, line.c_str() + 3, "", firstChar == '#', nullptr};
		parseCtx._actionInfos.push_back(ai);
		_pendingActionInfos.push_back(parseCtx._actionInfos.size() - 1);
	} else if (line.size() >= 4 && (line.hasPrefix("#W ") || line.hasPrefix("-W "))) {
		ActionInfo ai = {ActionInfo::Walk, line.c_str() + 3, "", firstChar == '#', nullptr};
		parseCtx._actionInfos.push_back(ai);
		_pendingActionInfos.push_back(parseCtx._actionInfos.size() - 1);
	} else if (line.size() >= 4 && (line.hasPrefix("#T ") || line.hasPrefix("-T "))) {
		ActionInfo ai = {ActionInfo::Talk, line.c_str() + 3, "", firstChar == '#', nullptr};
		parseCtx._actionInfos.push_back(ai);
		_pendingActionInfos.push_back(parseCtx._actionInfos.size() - 1);
	} else if (line.size() >= 4 && (line.hasPrefix("#U ") || line.hasPrefix("-U "))) {
		int secondObjPos = -1;
		for (uint i = 3; i < line.size(); ++i) {
			if (line[i] == ' ') {
				secondObjPos = i + 1;
				break;
			}
		}

		Common::String obj1;
		Common::String obj2;
		if (secondObjPos == -1) {
			obj1 = line.c_str() + 3;
		} else {
			obj1 = Common::String(line.c_str() + 3, secondObjPos - 4);
			obj2 = line.c_str() + secondObjPos;
		}

		ActionInfo ai = {
			ActionInfo::Use,
			obj1,
			obj2,
			firstChar == '#',
			nullptr
		};
		parseCtx._actionInfos.push_back(ai);
		_pendingActionInfos.push_back(parseCtx._actionInfos.size() - 1);
	} else if ((line.hasPrefix("#ELSE") || line.hasPrefix("=ELSE"))) {
		_elseFound = true;
		_ifTag = 0;
		if (line.size() >= 6) {
			_ifTag = line[5];
		}
	}

	command = new EndBlockCommand();

	return true;
}

void EndBlockCommandParser::transition(ScriptParseContext &parseCtx, Command *, Command *newCommand, CommandParser *newCommandParser) {
	if (_elseFound) {
		if (newCommand) {
			ScriptParseContext::ConditionalCommandInfos::iterator it = parseCtx._pendingCondCommands.begin();

			while (it != parseCtx._pendingCondCommands.end()) {
				if (it->_tag == _ifTag) {
					it->_command->setFalseCommand(newCommand);
					it = parseCtx._pendingCondCommands.erase(it);
				} else {
					++it;
				}
			}
		}

		_elseFound = false;
		_ifTag = 0;
	}

	if (newCommandParser != this) {
		if (!_pendingActionInfos.empty()) {
			for (Common::Array<uint>::iterator it = _pendingActionInfos.begin(); it != _pendingActionInfos.end(); ++it) {
				parseCtx._actionInfos[*it]._command = newCommand;
			}
			_pendingActionInfos.clear();
		}
	}
}

void EndBlockCommandParser::finish(ScriptParseContext &) {
	_elseFound = false;
	_ifTag = 0;

	if (!_pendingActionInfos.empty()) {
		debug("Problem: Pending action infos from end block parser is not empty!");
	}
	_pendingActionInfos.clear();
}

Command::ExecuteResult EndBlockCommand::execute(GameData &) {
	return Finished;
}

Command *EndBlockCommand::next() const {
	return nullptr;
}

Common::String EndBlockCommand::debugString() const {
	return "ENDBLOCK";
}

}
