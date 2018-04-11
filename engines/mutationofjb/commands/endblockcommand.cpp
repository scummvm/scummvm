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
#include "common/translation.h"

/*
	("#L " | "-L ") <object>
	("#W " | "-W ") <object>
	("#T " | "-T ") <object>
	("#P " | "-P ") <object1>
	("#U " | "-U ") <object1> [<object2>]
	("#ELSE" | "-ELSE") [<tag>]
	"#MACRO " <name>

	If a line starts with '#', '=', '-', it is treated as the end of a section.
	However, at the same time it can also start a new section depending on what follows.

	#L (look), #W (walk), #T (talk), #U (use) sections are executed
	when the user starts corresponding action on the object or in case of "use" up to two objects.
	The difference between '#' and '-' version is whether the player walks towards the object ('#') or not ('-').

	#ELSE is used by conditional commands (see comments for IfCommand and others).

	#MACRO starts a new macro. Global script can call macros from local script and vice versa.
*/

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
	} else if (line.size() >= 4 && (line.hasPrefix("#P ") || line.hasPrefix("-P "))) {
		ActionInfo ai = {ActionInfo::PickUp, line.c_str() + 3, "", firstChar == '#', nullptr};
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
	} else if (line.size() >= 8 && line.hasPrefix("#MACRO")) {
		_foundMacro = line.c_str() + 7;
	} else if (line.size() >= 10 && line.hasPrefix("#STARTUP")) {
		_foundStartup = line.c_str() + 9;
	}

	if (firstChar == '#') {
		_hashFound = true;
	}

	command = new EndBlockCommand();

	return true;
}

void EndBlockCommandParser::transition(ScriptParseContext &parseCtx, Command *, Command *newCommand, CommandParser *newCommandParser) {
	if (_elseFound || _hashFound) {
		if (newCommand) {
			ScriptParseContext::ConditionalCommandInfos::iterator it = parseCtx._pendingCondCommands.begin();

			while (it != parseCtx._pendingCondCommands.end()) {
				if ((it->_firstHash && _hashFound) || (!it->_firstHash && it->_tag == _ifTag)) {
					it->_command->setFalseCommand(newCommand);
					it = parseCtx._pendingCondCommands.erase(it);
				} else {
					++it;
				}
			}
		}

		_elseFound = false;
		_hashFound = false;
		_ifTag = 0;
	}

	if (!_foundMacro.empty()) {
		if (newCommand) {
			if (!parseCtx._macros.contains(_foundMacro)) {
				parseCtx._macros[_foundMacro] = newCommand;
			} else {
				warning(_("Macro '%s' already exists."), _foundMacro.c_str());
			}
		}
		_foundMacro.clear();
	}
	if (!_foundStartup.empty()) {
		if (newCommand) {
			const uint8 startupId = atoi(_foundStartup.c_str());
			if (!parseCtx._startups.contains(startupId)) {
				parseCtx._startups[startupId] = newCommand;
			} else {
				warning(_("Startup %u already exists."), (unsigned int) startupId);
			}
		}
		_foundStartup.clear();
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
	_hashFound = false;
	_ifTag = 0;

	if (!_pendingActionInfos.empty()) {
		debug("Problem: Pending action infos from end block parser is not empty!");
	}
	_pendingActionInfos.clear();
	_foundMacro = "";
}

Command::ExecuteResult EndBlockCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	_nextCmd = scriptExecCtx.popReturnCommand();
	return Finished;
}

Command *EndBlockCommand::next() const {
	return _nextCmd;
}

Common::String EndBlockCommand::debugString() const {
	return "ENDBLOCK";
}

}
