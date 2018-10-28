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

#include "mutationofjb/commands/newroomcommand.h"
#include "mutationofjb/script.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "common/str.h"

/** @file
 * "NEWROOM " <sceneId> " " <x> " " <y> [ " "  <frame> ]
 *
 * NEWROOM changes the current scene. While doing that, it also executes STARTUP section for the new room.
 * However, after that, the execution goes back to the old script to finish commands after NEWROOM.
 *
 * All parameters are supposed to be 3 characters long.
 * SceneId is the scene to load, x and y are the player's new position and frame is the player's new frame (orientation).
 */

namespace MutationOfJB {

bool NewRoomCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (line.size() < 19 || !line.hasPrefix("NEWROOM")) {
		return false;
	}

	const uint8 sceneId = atoi(line.c_str() + 8);
	const uint16 x = atoi(line.c_str() + 12);
	const uint16 y = atoi(line.c_str() + 16);
	uint8 frame = 0;
	if (line.size() >= 21)
		frame = atoi(line.c_str() + 20);
	command = new NewRoomCommand(sceneId, x, y, frame);
	return true;
}


NewRoomCommand::NewRoomCommand(uint8 sceneId, uint16 x, uint16 y, uint8 frame) : _sceneId(sceneId), _x(x), _y(y), _frame(frame), _innerExecCtx(nullptr) {}

Command::ExecuteResult NewRoomCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	Game &game = scriptExecCtx.getGame();

	// Execute new startup section.
	ExecuteResult res;
	if (!_innerExecCtx) {
		Script *newScript = game.changeSceneDelayScript(_sceneId, game.getGameData()._partB);
		_innerExecCtx = new ScriptExecutionContext(scriptExecCtx.getGame(), newScript);
		res = _innerExecCtx->startStartupSection();
	} else {
		res = _innerExecCtx->runActiveCommand();
	}

	if (res == Finished) {
		delete _innerExecCtx;
		_innerExecCtx = nullptr;
	}

	return res;
}

Common::String NewRoomCommand::debugString() const {
	return Common::String::format("NEWROOM %u %u %u %u", (unsigned int) _sceneId, (unsigned int) _x, (unsigned int) _y, (unsigned int) _frame);
}

}
