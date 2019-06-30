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

#include "mutationofjb/commands/playanimationcommand.h"
#include "mutationofjb/game.h"
#include "mutationofjb/room.h"
#include "mutationofjb/script.h"

/** @file
 * ( "FLB " | "FLX" ) <fromFrame> " " <toFrame>
 *
 * Plays the specified frames from room animation.
 *
 * TODO: Parse all arguments of this command.
 * TODO: Actually play the animation instead of just showing last frame.
 */

namespace MutationOfJB {

bool PlayAnimationCommandParser::parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) {
	if (line.size() < 11 || (!line.hasPrefix("FLB ") && !line.hasPrefix("FLX ")))
		return false;

	const int fromFrame = atoi(line.c_str() + 4);
	const int toFrame = atoi(line.c_str() + 8);

	command = new PlayAnimationCommand(fromFrame, toFrame);
	return true;
}


Command::ExecuteResult PlayAnimationCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	scriptExecCtx.getGame().getRoom().drawFrames(_fromFrame - 1, _toFrame - 1);

	return Finished;
}

Common::String PlayAnimationCommand::debugString() const {
	return Common::String::format("PLAYROOMANIM %u %u", (unsigned int) _fromFrame, (unsigned int) _toFrame);
}

}
