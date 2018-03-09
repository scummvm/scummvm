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
		parseCtx._lookActionInfos.push_back(ai);
		_pendingActionInfos.push_back(&parseCtx._lookActionInfos.back());
	} else if (line.size() >= 4 && (line.hasPrefix("#W ") || line.hasPrefix("-W "))) {
		ActionInfo ai = {ActionInfo::Walk, line.c_str() + 3, "", firstChar == '#', nullptr};
		parseCtx._walkActionInfos.push_back(ai);
		_pendingActionInfos.push_back(&parseCtx._walkActionInfos.back());
	} else if (line.size() >= 4 && (line.hasPrefix("#T ") || line.hasPrefix("-T "))) {
		ActionInfo ai = {ActionInfo::Talk, line.c_str() + 3, "", firstChar == '#', nullptr};
		parseCtx._talkActionInfos.push_back(ai);
		_pendingActionInfos.push_back(&parseCtx._talkActionInfos.back());
	} else if (line.size() >= 4 && (line.hasPrefix("#U ") || line.hasPrefix("-U "))) {
		int secondObjPos = -1;
		for (int i = 3; i < (int) line.size(); ++i) {
			if (line[i] == ' ') {
				secondObjPos = i + 1;
				break;
			}
		}
		ActionInfo ai = {
			ActionInfo::Use,
			line.c_str() + 3,
			(secondObjPos != -1) ? line.c_str() + secondObjPos : "",
			firstChar == '#',
			nullptr
		};
		parseCtx._useActionInfos.push_back(ai); 
		_pendingActionInfos.push_back(&parseCtx._useActionInfos.back());
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

	if (!_pendingActionInfos.empty() && newCommandParser != this) {
		debug("Fixing pending action info.");
		for (Common::Array<ActionInfo *>::iterator it = _pendingActionInfos.begin(); it != _pendingActionInfos.end(); ++it) {
			(*it)->_command = newCommand;
		}
		_pendingActionInfos.clear();
	}
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
