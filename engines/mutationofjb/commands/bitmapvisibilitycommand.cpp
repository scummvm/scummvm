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

#include "mutationofjb/commands/bitmapvisibilitycommand.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/script.h"

/** @file
 * "RB " <sceneId> " " <bitmapId> " " <visible>
 *
 * Changes visibility of a bitmap in the specified scene.
 */

namespace MutationOfJB {

bool BitmapVisibilityCommandParser::parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) {
	if (line.size() < 10 || !line.hasPrefix("RB "))
		return false;

	const uint8 sceneId = (uint8) atoi(line.c_str() + 3);
	const uint8 bitmapId = (uint8) atoi(line.c_str() + 6);
	const bool visible = (line[9] == '1');

	command = new BitmapVisibilityCommand(sceneId, bitmapId, visible);
	return true;
}


Command::ExecuteResult BitmapVisibilityCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	scriptExecCtx.getGameData().getScene(_sceneId)->getBitmap(_bitmapId)->_isVisible = _visible;

	return Finished;
}

Common::String BitmapVisibilityCommand::debugString() const {
	return Common::String::format("SETBITMAPVIS %u %u %s", (unsigned int) _sceneId, (unsigned int) _bitmapId, _visible ? "true" : "false");
}

}
