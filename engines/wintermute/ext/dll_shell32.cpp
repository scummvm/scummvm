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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_scriptable.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"

namespace Wintermute {

bool EmulateShell32ExternalCalls(BaseGame *inGame, ScStack *stack, ScStack *thisStack, ScScript::TExternalFunction *function) {
	//////////////////////////////////////////////////////////////////////////
	// ShellExecuteA
	// Used to open URL in browser at Wilma Tetris
	// Specification: external "shell32.dll" stdcall long ShellExecuteA(long, string, string, string, string, long)
	// Known usage: ShellExecuteA(0, "open", <URL>, "", "", 3)
	// Returns value >32 on success
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "ShellExecuteA") == 0) {
		stack->correctParams(6);
		int hwnd = stack->pop()->getInt();
		const char *operation = stack->pop()->getString();
		const char *file = stack->pop()->getString();
		const char *params = stack->pop()->getString();
		const char *directory = stack->pop()->getString();
		int cmd = stack->pop()->getInt();

		if (strcmp(operation, "open") == 0 && !strlen(params) && !strlen(directory)) {
			g_system->openUrl(file);
		} else {
			warning("ShellExecuteA(%d,\"%s\",\"%s\",\"%s\",\"%s\",%d) is not implemented", hwnd, operation, file, params, directory, cmd);
		}

		stack->pushInt(42);
		return STATUS_OK;
	}

	return STATUS_FAILED;
}

} // End of namespace Wintermute
