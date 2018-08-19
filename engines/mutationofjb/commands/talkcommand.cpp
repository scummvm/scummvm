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

#include "mutationofjb/commands/talkcommand.h"

#include "mutationofjb/game.h"
#include "mutationofjb/script.h"
#include "mutationofjb/tasks/conversationtask.h"
#include "mutationofjb/tasks/taskmanager.h"

#include "common/str.h"

/** @file
 * "TALK TO HIM" [ " " <mode> ]
 *
 * Begins interactive conversation defined by DefineStructCommand.
 * The command supports multiple modes:
 *   0 - normal mode,
 *   1 - Ray and Buttleg mode (two responders),
 *   2 - unknown (unused) mode,
 *   3 - carnival ticket seller mode (special animation).
 */

namespace MutationOfJB {

bool TalkCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (line.size() < 11 || !line.hasPrefix("TALK TO HIM")) {
		return false;
	}

	int modeInt = 0;

	if (line.size() >= 13) {
		modeInt = atoi(line.c_str() + 12);
	}

	TalkCommand::Mode mode = TalkCommand::NORMAL_MODE;

	if (modeInt == 1) {
		mode = TalkCommand::RAY_AND_BUTTLEG_MODE;
	} else if (modeInt == 3) {
		mode = TalkCommand::CARNIVAL_TICKET_SELLER_MODE;
	}

	command = new TalkCommand(mode);
	return true;
}

Command::ExecuteResult TalkCommand::execute(ScriptExecutionContext &scriptExeCtx) {
	if (!_task) {
		_task = TaskPtr(new ConversationTask(scriptExeCtx.getGameData()._currentScene, scriptExeCtx.getGame().getGameData()._conversationInfo, _mode));
		scriptExeCtx.getGame().getTaskManager().startTask(_task);
	}

	if (_task->getState() == Task::FINISHED) {
		_task.reset();

		return Command::Finished;
	}

	return Command::InProgress;
}

Common::String TalkCommand::debugString() const {
	const char *modes[] = {"NORMAL", "RAY_AND_BUTTLEG", "CARNIVAL_TICKET_SELLER"};
	return Common::String::format("TALK %s", modes[static_cast<int>(_mode)]);
}

}
