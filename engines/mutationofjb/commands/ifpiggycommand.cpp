/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mutationofjb/commands/ifpiggycommand.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/script.h"
#include "mutationofjb/util.h"
#include "common/str.h"

/** @file
 * "IFPIGGY"
 *
 * IFPIGGY command tests whether current loaded APK file (character animation) is "piggy.apk".
 * If it is, execution continues to the next line.
 * Otherwise execution continues after first "#ELSE" or "=ELSE".
 *
 * Please note that this does not work like you are used to from saner languages.
 * IFPIGGY does not have any blocks. It only searches for first #ELSE, so you can have stuff like:
 *   IFPIGGY
 *   IFITEM someitem
 *   #ELSE
 *   ...
 * This is effectively logical AND.
 */

namespace MutationOfJB {

bool IfPiggyCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (line != "IFPIGGY") {
		return false;
	}

	_tags.push(0);
	command = new IfPiggyCommand();

	return true;
}


Command::ExecuteResult IfPiggyCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	_cachedResult = scriptExecCtx.getGameData()._currentAPK == "piggy.apk";

	return Finished;
}

Common::String IfPiggyCommand::debugString() const {
	return "IFPIGGY";
}

}
