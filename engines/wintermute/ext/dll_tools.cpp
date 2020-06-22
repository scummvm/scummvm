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
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"

namespace Wintermute {

bool EmulateToolsExternalCalls(BaseGame *inGame, ScStack *stack, ScStack *thisStack, ScScript::TExternalFunction *function) {
	//////////////////////////////////////////////////////////////////////////
	// SetValueToReg
	// Used to switch game's windowed/fullscreen mode at games by HeroCraft
	// Specification: external "tools.dll" cdecl SetValueToReg(string, string, long)
	// Known usage: SetValueToReg("Software\HeroCraft\<GameID>\Video", "Windowed", 1)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "SetValueToReg") == 0) {
		stack->correctParams(3);
		const char *regpath = stack->pop()->getString();
		const char *key = stack->pop()->getString();
		int value = stack->pop()->getInt();

		if (strcmp(key, "Windowed") == 0) {
			inGame->_renderer->setWindowed(value);
		} else {
			warning("SetValueToReg(\"%s\",\"%s\",%d) is not implemented", regpath, key, value);
		}

		stack->pushNULL();
		return STATUS_OK;
	}

	return STATUS_FAILED;
}

} // End of namespace Wintermute
