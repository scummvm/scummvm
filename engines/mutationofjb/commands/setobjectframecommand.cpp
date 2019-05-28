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

#include "mutationofjb/commands/setobjectframecommand.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/room.h"
#include "mutationofjb/script.h"

/** @file
 * "SETANIM " <objectId> " " <frame>
 *
 * Draws the frame for the specified object without changing the object's current frame.
 * If the object is active, it is deactivated.
 */

namespace MutationOfJB {

bool SetObjectFrameCommandParser::parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) {
	if (line.size() < 13 || !line.hasPrefix("SETANIM "))
		return false;

	const uint8 objectId = (uint8) atoi(line.c_str() + 8);
	const unsigned int frame = atoi(line.c_str() + 11);

	command = new SetObjectFrameCommand(objectId, frame);
	return true;
}


Command::ExecuteResult SetObjectFrameCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	Object *const object = scriptExecCtx.getGameData().getCurrentScene()->getObject(_objectId);

	object->_active = 0;
	// The object's current frame is not changed, so use frame override instead.
	scriptExecCtx.getGame().getRoom().drawObject(_objectId, _frame);

	return Finished;
}

Common::String SetObjectFrameCommand::debugString() const {
	return Common::String::format("SETOBJECTFRAME %u %u", (unsigned int) _objectId, (unsigned int) _frame);
}

}
