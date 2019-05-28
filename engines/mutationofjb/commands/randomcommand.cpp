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

#include "mutationofjb/commands/randomcommand.h"

#include "mutationofjb/game.h"
#include "mutationofjb/script.h"
#include "common/debug.h"
#include "common/random.h"

/** @file
 * "RANDOM " <numChoices>
 *
 * RANDOM command randomly picks one of the command blocks that
 * follow it and jumps to its start.
 *
 * These blocks start with "/" and end with "\". The end of a random
 * block also ends the current section. The number of blocks must
 * match numChoices.
 */

namespace MutationOfJB {

bool RandomCommandParser::parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) {
	if (line.size() < 8 || !line.hasPrefix("RANDOM")) {
		return false;
	}

	int numChoices = atoi(line.c_str() + 7);
	if (parseCtx._pendingRandomCommand) {
		// Nested RANDOM commands are unused and not properly supported by the original game.
		warning("Ignoring nested RANDOM command.");
	} else if (numChoices >= 1) {
		RandomCommand *randomCommand = new RandomCommand(static_cast<uint>(numChoices));
		parseCtx._pendingRandomCommand = randomCommand;
		command = randomCommand;
	} else {
		warning("Ignoring malformed RANDOM command with %d choices.", numChoices);
	}

	return true;
}


bool RandomBlockStartParser::parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&) {
	if (line != "/") {
		return false;
	}

	if (!parseCtx._pendingRandomCommand) {
		warning("Unexpected start of RANDOM block");
	}

	return true;
}

void RandomBlockStartParser::transition(ScriptParseContext &parseCtx, Command *, Command *newCommand, CommandParser *) {
	RandomCommand *randomCommand = parseCtx._pendingRandomCommand;
	if (newCommand && randomCommand) {
		randomCommand->_choices.push_back(newCommand);

		if (randomCommand->_choices.size() == randomCommand->_numChoices) {
			parseCtx._pendingRandomCommand = nullptr;
		}
	}
}


RandomCommand::RandomCommand(uint numChoices)
	: _numChoices(numChoices),
	  _chosenNext(nullptr) {
	_choices.reserve(numChoices);
}

Command::ExecuteResult RandomCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	assert(!_choices.empty());

	Common::RandomSource &rng = scriptExecCtx.getGame().getRandomSource();
	uint choice = rng.getRandomNumber(_choices.size() - 1);
	_chosenNext = _choices[choice];
	return Finished;
}

Command *RandomCommand::next() const {
	return _chosenNext;
}

Common::String RandomCommand::debugString() const {
	return Common::String::format("RANDOM %u", _numChoices);
}

const RandomCommand::Choices &RandomCommand::getChoices() const {
	return _choices;
}

}
