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

bool EmulateImgExternalCalls(BaseGame *inGame, ScStack *stack, ScStack *thisStack, ScScript::TExternalFunction *function) {
	//////////////////////////////////////////////////////////////////////////
	// changeWindowCaption
	// Used to change game's window caption at games by HeroCraft
	// Specification: external "img.dll" cdecl changeWindowCaption(long, string)
	// Known usage: changeWindowCaption(Game.Hwnd, <Title>)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "changeWindowCaption") == 0) {
		stack->correctParams(2);
		/*int hwnd =*/ stack->pop()->getInt();
		/*const char *title =*/ stack->pop()->getString();

		// do nothing

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// maximizedWindow
	// Used to change game's window size at games by HeroCraft
	// Specification: external "img.dll" cdecl maximizedWindow(long, long, long)
	// Known usage: maximizedWindow(Game.Hwnd, 1024, 768)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(function->name, "maximizedWindow") == 0) {
		stack->correctParams(3);
		/*int hwnd =*/ stack->pop()->getInt();
		/*int width =*/ stack->pop()->getInt();
		/*int height =*/ stack->pop()->getInt();

		// do nothing

		stack->pushNULL();
		return STATUS_OK;
	}

	return STATUS_FAILED;
}

} // End of namespace Wintermute
