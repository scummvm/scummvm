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

#ifndef MUTATIONOFJB_CAMEFROMCOMMAND_H
#define MUTATIONOFJB_CAMEFROMCOMMAND_H

#include "mutationofjb/commands/conditionalcommand.h"
#include "common/scummsys.h"

namespace MutationOfJB {

class CameFromCommandParser : public ConditionalCommandParser {
public:
	CameFromCommandParser() : ConditionalCommandParser(true) {}
	bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) override;
};

class CameFromCommand : public ConditionalCommand {
public:
	CameFromCommand(uint8 sceneId) : _sceneId(sceneId) {}
	ExecuteResult execute(ScriptExecutionContext &scriptExecCtx) override;
	Common::String debugString() const override;
private:
	uint8 _sceneId;
};

}

#endif
