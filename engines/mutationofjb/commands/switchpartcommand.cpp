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

#include "mutationofjb/commands/switchpartcommand.h"

#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/script.h"

#include "common/str.h"

/** @file
 * "SWITCHPART"
 *
 * Switches to the second part of the game (part B).
 */

namespace MutationOfJB {

bool SwitchPartCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (line != "SWITCHPART") {
		return false;
	}

	command = new SwitchPartCommand();
	return true;
}

Command::ExecuteResult SwitchPartCommand::execute(ScriptExecutionContext &scriptExeCtx) {
	scriptExeCtx.getGame().switchToPartB();

	return Command::Finished;
}

Common::String SwitchPartCommand::debugString() const {
	return "SWITCHPART";
}

}
