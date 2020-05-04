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

// Implemented in their respective .cpp-files
bool EmulateGetURLExternalCalls(BaseGame *inGame, ScStack *stack, ScStack *thisStack, ScScript::TExternalFunction *function) {
	//////////////////////////////////////////////////////////////////////////
	// getURLContent
	// Used to download news headlines at Demo 2012 of James Peris
	// HTTP GET result is stored in 3rd param of the call as a plain string
	// Specification: external "geturl.dll" cdecl getURLContent(string, string, string)
	// Known usage: getURLContent("http://www.lacosaweb.com", <DirURL>, <Buffer>)
	// Sets 3rd param to "Request Error." on error
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "getURLContent") == 0) {
		stack->correctParams(3);
		const char *domain = stack->pop()->getString();
		const char *dirurl = stack->pop()->getString();
		ScValue *buf = stack->pop();

		if (strcmp(dirurl, "jpnews/demo-es1.txt") == 0) {
			buf->setString("Ya disponible el juego completo en jamesperis.com");
		} else if (strcmp(dirurl, "jpnews/demo-es2.txt") == 0) {
			buf->setString("Cons\355guelo por solo 3,95 euros");
		} else if (strcmp(dirurl, "jpnews/demo-en1.txt") == 0) {
			buf->setString("You can get the full game in jamesperis.com");
		} else if (strcmp(dirurl, "jpnews/demo-en2.txt") == 0) {
			buf->setString("Get it for 3.95 euros");
		} else {
			warning("getURLContent(\"%s\",\"%s\",buf) is not implemented", domain, dirurl);
			buf->setString("Request Error.");
		}

		stack->pushNULL();
		return STATUS_OK;
	}

	return STATUS_FAILED;
}

} // End of namespace Wintermute
