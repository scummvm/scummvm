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

#ifndef MUTATIONOFJB_SETOBJECTFRAMECOMMAND_H
#define MUTATIONOFJB_SETOBJECTFRAMECOMMAND_H

#include "mutationofjb/commands/seqcommand.h"
#include "common/str.h"

namespace MutationOfJB {

class SetObjectFrameCommandParser : public SeqCommandParser {
public:
	SetObjectFrameCommandParser() {}

	bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) override;
};

class SetObjectFrameCommand : public SeqCommand {
public:
	SetObjectFrameCommand(uint8 objectId, uint8 frame) : _objectId(objectId), _frame(frame) {}

	ExecuteResult execute(ScriptExecutionContext &scriptExecCtx) override;
	Common::String debugString() const override;
private:
	uint8 _objectId;
	uint8 _frame;
};

}

#endif
