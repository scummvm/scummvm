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

#include "mutationofjb/commands/renamecommand.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "common/algorithm.h"

/** @file
 * "REN " <oldName> " " <newName>
 *
 * Renames every door, static (in the current scene) and inventory item
 * with the name oldName to newName.
 */

namespace MutationOfJB {

bool RenameCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (line.size() < 7 || !line.hasPrefix("REN")) {
		return false;
	}

	Common::String::const_iterator sep = Common::find(line.begin() + 4, line.end(), ' ');
	if (sep == line.end() || sep + 1 == line.end()) {
		return false;
	}

	const Common::String oldName(line.begin() + 4, sep);
	const Common::String newName(sep + 1, line.end());
	command = new RenameCommand(oldName, newName);

	return true;
}


Command::ExecuteResult RenameCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	Scene *const scene = scriptExecCtx.getGameData().getCurrentScene();

	for (int i = 1; i <= scene->getNoDoors(); ++i) {
		Door *const door = scene->getDoor(i);
		if (strcmp(door->_name, _oldName.c_str()) == 0) {
			strncpy(door->_name, _newName.c_str(), MAX_ENTITY_NAME_LENGTH);
		}
	}
	for (int i = 1; i <= scene->getNoStatics(); ++i) {
		Static *const stat = scene->getStatic(i);
		if (strcmp(stat->_name, _oldName.c_str()) == 0) {
			strncpy(stat->_name, _newName.c_str(), MAX_ENTITY_NAME_LENGTH);
		}
	}

	scriptExecCtx.getGameData().getInventory().renameItem(_oldName, _newName);
	return Finished;
}

Common::String RenameCommand::debugString() const {
	return Common::String::format("RENAME '%s' '%s'", _oldName.c_str(), _newName.c_str());
}

}
