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

#include "mutationofjb/commands/camefromcommand.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/script.h"
#include "common/str.h"

/** @file
 * "CAMEFROM " <sceneId>
 *
 * This command tests whether last scene (the scene player came from) is sceneId.
 * If true, the execution continues after this command.
 * Otherwise the execution continues after first '#' found.
 */

namespace MutationOfJB {

bool CameFromCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (line.size() < 10 || !line.hasPrefix("CAMEFROM")) {
		return false;
	}

	const uint8 sceneId = atoi(line.c_str() + 9);
	_tags.push(0);
	command = new CameFromCommand(sceneId);
	return true;
}

Command::ExecuteResult CameFromCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	_cachedResult = (scriptExecCtx.getGameData()._lastScene == _sceneId);

	return Finished;
}

Common::String CameFromCommand::debugString() const {
	return Common::String::format("CAMEFROM %d", _sceneId);
}

}
