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

#include "mutationofjb/commands/conditionalcommand.h"
#include "mutationofjb/script.h"
#include "common/scummsys.h"

namespace MutationOfJB {

void ConditionalCommandParser::transition(ScriptParseContext &parseContext, Command *oldCommand, Command *newCommand, CommandParser *) {
	if (!oldCommand || !newCommand) {
		warning("Unexpected empty command in transition");
		return;
	}

	ConditionalCommand *const condCommand = static_cast<ConditionalCommand *>(oldCommand);
	parseContext.addConditionalCommand(condCommand, _tags.pop(), _firstHash);
	condCommand->setTrueCommand(newCommand);
}

void ConditionalCommandParser::finish(ScriptParseContext &) {
	_tags.clear();
}


ConditionalCommand::ConditionalCommand() :
	_trueCommand(nullptr),
	_falseCommand(nullptr),
	_cachedResult(false) {}

Command *ConditionalCommand::getTrueCommand() const {
	return _trueCommand;
}

Command *ConditionalCommand::getFalseCommand() const {
	return _falseCommand;
}

void ConditionalCommand::setTrueCommand(Command *command) {
	_trueCommand = command;
}

void ConditionalCommand::setFalseCommand(Command *command) {
	_falseCommand = command;
}

Command *ConditionalCommand::next() const {
	if (_cachedResult) {
		return _trueCommand;
	} else {
		return _falseCommand;
	}
}

}
