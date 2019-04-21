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

#include "mutationofjb/commands/setcolorcommand.h"

#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/script.h"

#include "common/str.h"

/** @file
 * "SETCOL " <colorString>
 *
 * Sets subtitle color used by SayCommand.
 */

namespace MutationOfJB {

bool SetColorCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (line.size() < 8 || !line.hasPrefix("SETCOL")) {
		return false;
	}

	const char *const colorStr = line.c_str() + 7;
	const uint8 color = Game::colorFromString(colorStr);

	command = new SetColorCommand(color);
	return true;
}

Command::ExecuteResult SetColorCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	scriptExecCtx.getGameData()._color = _color;
	return Finished;
}

Common::String SetColorCommand::debugString() const {
	return Common::String::format("SETCOL %u", _color);
}

}
