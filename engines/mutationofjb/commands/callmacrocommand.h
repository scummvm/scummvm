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

#ifndef MUTATIONOFJB_CALLMACROCOMMAND_H
#define MUTATIONOFJB_CALLMACROCOMMAND_H

#include "mutationofjb/commands/command.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace MutationOfJB {

class CallMacroCommandParser : public CommandParser {
public:
	CallMacroCommandParser() {}

	bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) override;
	void transition(ScriptParseContext &parseCtx, Command *oldCommand, Command *newCommand, CommandParser *newCommandParser) override;
};

class CallMacroCommand : public Command {
public:
	CallMacroCommand(const Common::String &macroName) : _macroName(macroName), _returnCommand(nullptr), _callCommand(nullptr) {}
	void setReturnCommand(Command *);

	Command *getReturnCommand() const;

	ExecuteResult execute(ScriptExecutionContext &scriptExecCtx) override;
	Command *next() const override;
	Common::String debugString() const override;
private:
	Common::String _macroName;
	Command *_returnCommand;
	Command *_callCommand;
};

}

#endif
