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

#include "mutationofjb/commands/loadplayercommand.h"

#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/script.h"

#include "common/str.h"

/** @file
 * "RABLOAD " <apkFrameFirst> " " <apkFrameLast> " " <playerFrameFirst> " " <palIndexFirst> " " <apkFilename>
 *
 * Load player frames from APK file specified by apkFileName.
 * Only frames between apkFrameFirst and apkFrameLast are loaded onto position defined by playerFrameFirst.
 * Player's palette is loaded at index defined by palIndexFirst.
 */

namespace MutationOfJB {

bool LoadPlayerCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (line.size() < 25 || !line.hasPrefix("RABLOAD ")) {
		return false;
	}

	const uint8 apkFrameFirst = atoi(line.c_str() + 8);
	const uint8 apkFrameLast = atoi(line.c_str() + 12);
	const uint8 playerFrameFirst = atoi(line.c_str() + 16);
	const uint8 palIndexFirst = atoi(line.c_str() + 20);
	const Common::String apkFileName = line.c_str() + 24;

	command = new LoadPlayerCommand(apkFrameFirst, apkFrameLast, playerFrameFirst, palIndexFirst, apkFileName);
	return true;
}

Command::ExecuteResult LoadPlayerCommand::execute(ScriptExecutionContext &scriptExeCtx) {
	scriptExeCtx.getGameData()._currentAPK = _apkFileName;

	return Command::Finished;
}

Common::String LoadPlayerCommand::debugString() const {
	return Common::String::format("LOADPLAYER %u %u %u %u %s", (unsigned int) _apkFrameFirst, (unsigned int) _apkFrameLast, (unsigned int) _playerFrameFirst, (unsigned int) _palIndexFirst, _apkFileName.c_str());
}

}
