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

namespace MutationOfJB {

class EndBlockCommandParser : public CommandParser {
public:
	EndBlockCommandParser() : _elseFound(false), _ifTag(0) {}

	virtual bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command);
	virtual void transition(ScriptParseContext &parseCtx, Command *oldCommand, Command *newCommand);

private:
	bool _elseFound;
	char _ifTag;
};

class EndBlockCommand : public Command {
public:
	static bool ParseFunc(const Common::String &line, ScriptParseContext &parseContext, Command *&command);

	virtual ExecuteResult execute(GameData &gameData) override;
	virtual Command *next() const override;
};

}

#endif
