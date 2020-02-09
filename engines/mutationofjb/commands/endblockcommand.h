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

#ifndef MUTATIONOFJB_ENDBLOCKCOMMAND_H
#define MUTATIONOFJB_ENDBLOCKCOMMAND_H

#include "mutationofjb/commands/command.h"
#include "common/scummsys.h"
#include "common/array.h"

namespace MutationOfJB {

struct ActionInfo;

class EndBlockCommandParser : public CommandParser {
public:
	EndBlockCommandParser() : _elseFound(false), _hashFound(false), _ifTag(0) {}

	bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) override;
	void transition(ScriptParseContext &parseCtx, Command *oldCommand, Command *newCommand, CommandParser *newCommandParser) override;
	void finish(ScriptParseContext &parseCtx) override;
private:
	bool _elseFound;
	bool _hashFound;
	char _ifTag;

	Common::Array<uint> _pendingActionInfos;
	struct NameAndCommand {
		Common::String _name;
		Command *_command;
	};
	struct IdAndCommand {
		uint8 _id;
		Command *_command;
	};
	typedef Common::Array<NameAndCommand> NameAndCommandArray;
	typedef Common::Array<IdAndCommand> IdAndCommandArray;
	NameAndCommandArray _foundMacros;
	IdAndCommandArray _foundStartups;
	NameAndCommandArray _foundExtras;
};

class EndBlockCommand : public Command {
public:
	EndBlockCommand() : _nextCmd(nullptr) {}
	static bool ParseFunc(const Common::String &line, ScriptParseContext &parseContext, Command *&command);

	ExecuteResult execute(ScriptExecutionContext &scriptExecCtx) override;
	Command *next() const override;
	Common::String debugString() const override;
private:
	Command *_nextCmd;
};

}

#endif
