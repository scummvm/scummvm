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

#ifndef MUTATIONOFJB_CONDITIONALCOMMAND_H
#define MUTATIONOFJB_CONDITIONALCOMMAND_H

#include "mutationofjb/commands/command.h"
#include "common/scummsys.h"
#include "common/queue.h"

namespace MutationOfJB {

class ConditionalCommandParser : public CommandParser {
public:
	ConditionalCommandParser(bool firstHash = false) : _firstHash(firstHash) {}
	void transition(ScriptParseContext &parseCtx, Command *oldCommand, Command *newCommand, CommandParser *newCommandParser) override;
	void finish(ScriptParseContext &parseCtx) override;
protected:
	Common::Queue<char> _tags;
private:
	bool _firstHash;
};

class ConditionalCommand : public Command {
public:
	ConditionalCommand();

	Command *getTrueCommand() const;
	Command *getFalseCommand() const;

	void setTrueCommand(Command *command);
	void setFalseCommand(Command *command);

	Command *next() const override;
protected:
	Command *_trueCommand;
	Command *_falseCommand;
	bool _cachedResult;
};

}

#endif
