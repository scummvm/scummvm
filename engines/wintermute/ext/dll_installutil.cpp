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

bool EmulateInstallUtilExternalCalls(BaseGame *inGame, ScStack *stack, ScStack *thisStack, ScScript::TExternalFunction *function) {
	//////////////////////////////////////////////////////////////////////////
	// _InstallUtilAnsi@0
	// Used to check if DVD is inserted at Art of Murder: FBI Confidential
	// Specification: external "installutil.dll" stdcall long _InstallUtilAnsi@0()
	// Known usage: _InstallUtilAnsi@0()
	// Returns 1 on success, other value on fail (which leads to Game.QuitGame() in non-Debug mode)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "_InstallUtilAnsi@0") == 0) {
		stack->correctParams(0);
		stack->pushInt(1);
		return STATUS_OK;
	}

	return STATUS_FAILED;
}

} // End of namespace Wintermute
