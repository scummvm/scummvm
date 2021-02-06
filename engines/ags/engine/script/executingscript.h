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

#ifndef AGS_ENGINE_SCRIPT_EXECUTINGSCRIPT_H
#define AGS_ENGINE_SCRIPT_EXECUTINGSCRIPT_H

#include "ags/engine/script/cc_instance.h"

namespace AGS3 {

enum PostScriptAction {
	ePSANewRoom,
	ePSAInvScreen,
	ePSARestoreGame,
	ePSARestoreGameDialog,
	ePSARunAGSGame,
	ePSARunDialog,
	ePSARestartGame,
	ePSASaveGame,
	ePSASaveGameDialog
};

#define MAX_QUEUED_SCRIPTS 4
#define MAX_QUEUED_ACTIONS 5
#define MAX_FUNCTION_NAME_LEN 60

enum ScriptInstType {
	kScInstGame,
	kScInstRoom
};

struct QueuedScript {
	Shared::String     FnName;
	ScriptInstType     Instance;
	size_t             ParamCount;
	RuntimeScriptValue Param1;
	RuntimeScriptValue Param2;

	QueuedScript();
};

struct ExecutingScript {
	ccInstance *inst;
	PostScriptAction postScriptActions[MAX_QUEUED_ACTIONS];
	const char *postScriptActionNames[MAX_QUEUED_ACTIONS];
	ScriptPosition  postScriptActionPositions[MAX_QUEUED_ACTIONS];
	char postScriptSaveSlotDescription[MAX_QUEUED_ACTIONS][100];
	int  postScriptActionData[MAX_QUEUED_ACTIONS];
	int  numPostScriptActions;
	QueuedScript ScFnQueue[MAX_QUEUED_SCRIPTS];
	int  numanother;
	char forked;

	int queue_action(PostScriptAction act, int data, const char *aname);
	void run_another(const char *namm, ScriptInstType scinst, size_t param_count, const RuntimeScriptValue &p1, const RuntimeScriptValue &p2);
	void init();
	ExecutingScript();
};

} // namespace AGS3

#endif
