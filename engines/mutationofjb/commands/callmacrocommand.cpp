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

#include "mutationofjb/commands/callmacrocommand.h"
#include "mutationofjb/script.h"
#include "mutationofjb/game.h"

/** @file
 * "_" <name>
 *
 * Calls macro with the specified name.
 */

namespace MutationOfJB {

bool CallMacroCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (line.size() < 2 || line.firstChar() != '_') {
		return false;
	}

	const Common::String macroName = line.c_str() + 1;
	command = new CallMacroCommand(macroName);
	return true;
}

void CallMacroCommandParser::transition(ScriptParseContext &, Command *oldCommand, Command *newCommand, CommandParser *) {
	if (!oldCommand || !newCommand) {
		warning("Unexpected empty command in transition");
		return;
	}

	static_cast<CallMacroCommand *>(oldCommand)->setReturnCommand(newCommand);
}


void CallMacroCommand::setReturnCommand(Command *cmd) {
	_returnCommand = cmd;
}

Command *CallMacroCommand::getReturnCommand() const {
	return _returnCommand;
}

Command::ExecuteResult CallMacroCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	_callCommand = scriptExecCtx.getMacro(_macroName);
	if (_callCommand) {
		scriptExecCtx.pushReturnCommand(_returnCommand);
	} else {
		warning("Macro '%s' not found.", _macroName.c_str());
	}

	return Finished;
}

Command *CallMacroCommand::next() const {
	return _callCommand;
}

Common::String CallMacroCommand::debugString() const {
	return Common::String::format("CALL '%s'", _macroName.c_str());
}

}

