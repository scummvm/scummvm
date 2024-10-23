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

#include "ags/globals.h"
#include "ags/engine/script/executing_script.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/script/script.h"
#include "ags/shared/ac/game_version.h"

namespace AGS3 {

QueuedScript::QueuedScript()
	: Instance(kScInstGame)
	, ParamCount(0) {
}

int ExecutingScript::queue_action(PostScriptAction act, int data, const char *aname) {
	if (numPostScriptActions >= MAX_QUEUED_ACTIONS)
		quitprintf("!%s: Cannot queue action, post-script queue full", aname);

	// A strange behavior in pre-2.7.0 games allowed to call NewRoom right after
	// RestartGame, cancelling RestartGame. Probably an unintended effect.
	// We try to emulate this here, by simply removing all ePSARestartGame.
	if ((_G(loaded_game_file_version) < kGameVersion_270) && (act == ePSANewRoom)) {
		for (int i = 0; i < numPostScriptActions; i++) {
			if (postScriptActions[i] == ePSARestartGame) {
				debug("Removing spurious RestartGame event! index = %d numPostScriptActions = %d", i, numPostScriptActions);
				for (int j = i; j < numPostScriptActions; j++) {
					postScriptActions[j] = postScriptActions[j + 1];
					postScriptActionData[j] = postScriptActionData[j + 1];
					postScriptActionNames[j] = postScriptActionNames[j + 1];
					postScriptActionPositions[j] = postScriptActionPositions[j + 1];
				}
				i--; // make sure to remove multiple ePSARestartGame
				numPostScriptActions--;
			}
		}
	}

	if (numPostScriptActions > 0) {
		// if something that will terminate the room has already
		// been queued, don't allow a second thing to be queued
		switch (postScriptActions[numPostScriptActions - 1]) {
		case ePSANewRoom:
		case ePSARestoreGame:
		case ePSARestoreGameDialog:
		case ePSARunAGSGame:
		case ePSARestartGame:
			quitprintf("!%s: Cannot run this command, since there was a %s command already queued to run in \"%s\", line %d",
			           aname, postScriptActionNames[numPostScriptActions - 1],
			           postScriptActionPositions[numPostScriptActions - 1].Section.GetCStr(), postScriptActionPositions[numPostScriptActions - 1].Line);
			break;
		default:
			break;
		}
	}

	postScriptActions[numPostScriptActions] = act;
	postScriptActionData[numPostScriptActions] = data;
	postScriptActionNames[numPostScriptActions] = aname;
	get_script_position(postScriptActionPositions[numPostScriptActions]);
	numPostScriptActions++;
	return numPostScriptActions - 1;
}

void ExecutingScript::run_another(const char *namm, ScriptInstType scinst, size_t param_count, const RuntimeScriptValue *params) {
	if (numanother < MAX_QUEUED_SCRIPTS)
		numanother++;
	else {
		/*debug_script_warn("Warning: too many scripts to run, ignored %s(%d,%d)",
		script_run_another[numanother - 1], run_another_p1[numanother - 1],
		run_another_p2[numanother - 1]);*/
	}
	int thisslot = numanother - 1;
	QueuedScript &script = ScFnQueue[thisslot];
	script.FnName.SetString(namm, MAX_FUNCTION_NAME_LEN);
	script.Instance = scinst;
	script.ParamCount = param_count;
	for (size_t p = 0; p < MAX_QUEUED_PARAMS && p < param_count; ++p)
		script.Params[p] = params[p];
}

} // namespace AGS3
