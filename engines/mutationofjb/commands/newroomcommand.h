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

#ifndef MUTATIONOFJB_NEWROOMCOMMAND_H
#define MUTATIONOFJB_NEWROOMCOMMAND_H

#include "mutationofjb/commands/seqcommand.h"

namespace MutationOfJB {

class ScriptExecutionContext;

class NewRoomCommandParser : public SeqCommandParser {
public:
	NewRoomCommandParser() {}

	bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) override;
};

class NewRoomCommand : public SeqCommand {
public:
	NewRoomCommand(uint8 sceneId, uint16 x, uint16 y, uint8 frame);

	ExecuteResult execute(ScriptExecutionContext &scriptExecCtx) override;
	Common::String debugString() const override;
private:
	uint8 _sceneId;
	uint16 _x;
	uint16 _y;
	uint8 _frame;

	ScriptExecutionContext *_innerExecCtx;
};

}

#endif
