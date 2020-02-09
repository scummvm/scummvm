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

#ifndef MUTATIONOFJB_RANDOMCOMMAND_H
#define MUTATIONOFJB_RANDOMCOMMAND_H

#include "mutationofjb/commands/command.h"
#include "common/array.h"
#include "common/scummsys.h"

namespace MutationOfJB {

class RandomCommandParser : public CommandParser {
public:
	RandomCommandParser() {}

	bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) override;
};

class RandomBlockStartParser : public CommandParser {
public:
	RandomBlockStartParser() {}

	bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) override;
	void transition(ScriptParseContext &parseCtx, Command *oldCommand, Command *newCommand, CommandParser *newCommandParser) override;
};

class RandomCommand : public Command {
	friend class RandomBlockStartParser;

public:
	typedef Common::Array<Command *> Choices;

	RandomCommand(uint numChoices);

	ExecuteResult execute(ScriptExecutionContext &scriptExecCtx) override;
	Command *next() const override;

	Common::String debugString() const override;

	const Choices &getChoices() const;

private:
	uint _numChoices;
	Choices _choices;
	Command *_chosenNext;
};

}

#endif
