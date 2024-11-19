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

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_scriptable.h"
#include "engines/wintermute/base/file/base_savefile_manager_file.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"

namespace Wintermute {

bool EmulateRoutineExternalCalls(BaseGame *inGame, ScStack *stack, ScStack *thisStack, ScScript::TExternalFunction *function) {
	//////////////////////////////////////////////////////////////////////////
	// GetCaption
	// Used to get game's window caption for "Stroke of Fate" duology
	// Specification: external "routine.dll" cdecl string GetCaption(int)
	// Known usage: GetCaption(Game.Hwnd)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "GetCaption") == 0) {
		stack->correctParams(1);
		/*int hwnd =*/ stack->pop()->getInt();

		stack->pushString("Wintermute Engine");

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetCaption
	// Used to change game's window caption for "Stroke of Fate" duology
	// Specification: external "routine.dll" cdecl void SetCaption(int, string)
	// Known usage: SetCaption(Game.Hwnd, <Title>)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "SetCaption") == 0) {
		stack->correctParams(2);
		/*int hwnd =*/ stack->pop()->getInt();
		/*const char *title =*/ stack->pop()->getString();

		// do nothing

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetDPI
	// Used to get display DPI for "Stroke of Fate" duology
	// Specification: external "routine.dll" cdecl int GetDPI()
	// Known usage: GetDPI()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "GetDPI") == 0) {
		stack->correctParams(0);

		stack->pushInt(120); // standard Windows settings is 96 DPI, but 120 match original game

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveFile
	// Used to remove screenshot files for "Stroke of Fate" duology
	// Specification: external "routine.dll" cdecl bool RemoveFile(string)
	// Known usage: RenameFile(oldname, newname);
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "RemoveFile") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();

		bool ret = sfmFileRemove(filename);
		stack->pushInt(ret);

		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// RenameFile
	// Used to rename screenshot files for "Stroke of Fate" duology
	// Specification: external "routine.dll" cdecl bool RenameFile(string, string)
	// Known usage: RenameFile(oldname, newname);
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "RenameFile") == 0) {
		stack->correctParams(2);
		const char *oldName = stack->pop()->getString();
		const char *newName = stack->pop()->getString();

		bool ret = sfmFileRename(oldName, newName);
		stack->pushInt(ret);

		return STATUS_OK;
	}

	return STATUS_FAILED;
}

} // End of namespace Wintermute
