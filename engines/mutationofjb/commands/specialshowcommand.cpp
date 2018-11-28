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

#include "mutationofjb/commands/specialshowcommand.h"

#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/script.h"

#include "common/str.h"

/** @file
 * "SPECIALSHOW " <mode>
 *
 * Shows special screen.
 * The command supports multiple modes:
 *   1 - show puzzle hint,
 *   2 - show computer puzzle.
 */

namespace MutationOfJB {

bool SpecialShowCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (line.size() < 13 || !line.hasPrefix("SPECIALSHOW ")) {
		return false;
	}

	const int modeInt = atoi(line.c_str() + 12);

	SpecialShowCommand::Mode mode = SpecialShowCommand::PUZZLE_HINT;

	if (modeInt == 1) {
		mode = SpecialShowCommand::PUZZLE_HINT;
	} else if (modeInt == 2) {
		mode = SpecialShowCommand::COMPUTER_PUZZLE;
	} else {
		warning("Invalid special show mode %d", modeInt);
		return false;
	}

	command = new SpecialShowCommand(mode);
	return true;
}

Command::ExecuteResult SpecialShowCommand::execute(ScriptExecutionContext &scriptExeCtx) {
	// TODO: Show UI.
	if (_mode == COMPUTER_PUZZLE) {
		scriptExeCtx.getGameData().getScene(32)->getObject(2, true)->_WX = 255;
		scriptExeCtx.getGameData().getScene(32)->getObject(1, true)->_active = 0;
	}
	return Command::Finished;
}

Common::String SpecialShowCommand::debugString() const {
	const char *modes[] = {"PUZZLE_HINT", "COMPUTER_PUZZLE"};
	return Common::String::format("SPECIALSHOW %s", modes[static_cast<int>(_mode)]);
}

}
