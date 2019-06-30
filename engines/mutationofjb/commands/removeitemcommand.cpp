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

#include "mutationofjb/commands/removeitemcommand.h"
#include "mutationofjb/script.h"
#include "mutationofjb/gamedata.h"

/** @file
 * "DELITEM " <item>
 *
 * Removes item from inventory.
 */

namespace MutationOfJB {

bool RemoveItemCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (!line.hasPrefix("DELITEM") || line.size() < 9) {
		return false;
	}

	command = new RemoveItemCommand(line.c_str() + 8);
	return true;
}

Command::ExecuteResult RemoveItemCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	scriptExecCtx.getGameData()._inventory.removeItem(_item);
	return Finished;
}

Common::String RemoveItemCommand::debugString() const {
	return Common::String::format("DELITEM '%s'", _item.c_str());
}

}
