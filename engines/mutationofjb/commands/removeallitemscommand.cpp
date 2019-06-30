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

#include "mutationofjb/commands/removeallitemscommand.h"
#include "mutationofjb/script.h"
#include "mutationofjb/gamedata.h"

/** @file
 * "DELALLITEMS"
 *
 * Removes all items from inventory.
 */

namespace MutationOfJB {

bool RemoveAllItemsCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (line != "DELALLITEMS") {
		return false;
	}

	command = new RemoveAllItemsCommand();
	return true;
}

Command::ExecuteResult RemoveAllItemsCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	scriptExecCtx.getGameData()._inventory.removeAllItems();
	return Finished;
}

Common::String RemoveAllItemsCommand::debugString() const {
	return "DELALLITEM";
}

}
