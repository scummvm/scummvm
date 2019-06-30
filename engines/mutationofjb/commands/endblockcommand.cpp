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

/** @file
 * <look> | <walk> | <talk> | <pickup> | <use> | <else> | <macro> | <extra> | <endRandom>
 *
 * look ::= ("#L " | "-L ") <object>
 * walk ::= ("#W " | "-W ") <object>
 * talk ::= ("#T " | "-T ") <object>
 * pickup ::= ("#P " | "-P ") <object1>
 * use ::= ("#U " | "-U ") <object1> [<object2>]
 * else ::= ("#ELSE" | "-ELSE") [<tag>]
 * macro ::= "#MACRO " <name>
 * extra ::= "#EXTRA" <name>
 * endRandom ::= "\"
 *
 * If a line starts with '#', '=', '-', '\' it is treated as the end of a section.
 * However, at the same time it can also start a new section depending on what follows.
 *
 * #L (look), #W (walk), #T (talk), #U (use) sections are executed
 * when the user starts corresponding action on the object or in case of "use" up to two objects.
 * The difference between '#' and '-' version is whether the player walks towards the object ('#') or not ('-').
 *
 * #ELSE is used by conditional commands (see comments for IfCommand and others).
 *
 * #MACRO starts a new macro. Global script can call macros from local script and vice versa.
 *
 * #EXTRA defines an "extra" section. This is called from dialog responses ("TALK TO HIM" command).
 *
 * TODO: TIMERPROC.
 */

namespace MutationOfJB {

bool EndBlockCommandParser::parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) {
	if (line.empty()) {
		return false;
	}

	const char firstChar = line.firstChar();
	if (firstChar != '#' && firstChar != '=' && firstChar != '-' && firstChar != '\\') {
		return false;
	}

	// This is the start or end of section/block.
	command = new EndBlockCommand();

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
		NameAndCommand nc = {line.c_str() + 7, command};
		_foundMacros.push_back(nc);
	} else if (line.size() >= 10 && line.hasPrefix("#STARTUP")) {
		const uint8 startupId = atoi(line.c_str() + 9);
		IdAndCommand ic = {startupId, command};
		_foundStartups.push_back(ic);
	} else if (line.size() >= 7 && line.hasPrefix("#EXTRA")) {
		NameAndCommand nc = {line.c_str() + 6, command};
		_foundExtras.push_back(nc);
	}

	if (firstChar == '#') {
		_hashFound = true;
	}


	return true;
}

void EndBlockCommandParser::transition(ScriptParseContext &parseCtx, Command *oldCommand, Command *newCommand, CommandParser *newCommandParser) {
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

	if (!_foundMacros.empty()) {
		if (newCommand) {
			for (NameAndCommandArray::iterator it = _foundMacros.begin(); it != _foundMacros.end();) {
				if (it->_command != oldCommand) {
					it++;
					continue;
				}

				if (!parseCtx._macros.contains(it->_name)) {
					parseCtx._macros[it->_name] = newCommand;
				} else {
					warning("Macro '%s' already exists", it->_name.c_str());
				}
				it = _foundMacros.erase(it);
			}
		}
	}
	if (!_foundStartups.empty()) {
		if (newCommand) {
			for (IdAndCommandArray::iterator it = _foundStartups.begin(); it != _foundStartups.end();) {
				if (it->_command != oldCommand) {
					it++;
					continue;
				}

				if (!parseCtx._startups.contains(it->_id)) {
					parseCtx._startups[it->_id] = newCommand;
				} else {
					warning("Startup %u already exists", (unsigned int) it->_id);
				}
				it = _foundStartups.erase(it);
			}
		}
	}
	if (!_foundExtras.empty()) {
		if (newCommand) {
			for (NameAndCommandArray::iterator it = _foundExtras.begin(); it != _foundExtras.end();) {
				if (it->_command != oldCommand) {
					it++;
					continue;
				}

				if (!parseCtx._extras.contains(it->_name)) {
					parseCtx._extras[it->_name] = newCommand;
				} else {
					warning("Extra '%s' already exists", it->_name.c_str());
				}
				it = _foundExtras.erase(it);
			}
		}
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
	if (!_foundMacros.empty()) {
		debug("Problem: Found macros from end block parser is not empty!");
	}
	if (!_foundStartups.empty()) {
		debug("Problem: Found startups from end block parser is not empty!");
	}
	if (!_foundExtras.empty()) {
		debug("Problem: Found extras from end block parser is not empty!");
	}
	_pendingActionInfos.clear();
	_foundMacros.clear();
	_foundStartups.clear();
	_foundExtras.clear();
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
