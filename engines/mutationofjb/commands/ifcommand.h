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

#ifndef MUTATIONOFJB_IFCOMMAND_H
#define MUTATIONOFJB_IFCOMMAND_H

#include "mutationofjb/commands/conditionalcommand.h"
#include "common/scummsys.h"

namespace MutationOfJB {

class ScriptParseContext;

class IfCommandParser : public ConditionalCommandParser {
public:
	bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) override;
private:
};

class IfCommand : public ConditionalCommand {
public:
	IfCommand(uint8 sceneId, uint8 objectId, uint16 value, bool negative);

	ExecuteResult execute(ScriptExecutionContext &scriptExecCtx) override;
	Common::String debugString() const override;

private:
	uint8 _sceneId;
	uint8 _objectId;
	uint16 _value;
	bool _negative;
};

}

#endif
