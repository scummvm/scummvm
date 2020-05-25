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

bool EmulateKernel32ExternalCalls(BaseGame *inGame, ScStack *stack, ScStack *thisStack, ScScript::TExternalFunction *function) {
	//////////////////////////////////////////////////////////////////////////
	// LoadLibraryA
	// Used for checking library availability at games by Corbomite Games
	// Specification: external "kernel32.dll" stdcall long LoadLibraryA(string)
	// Known usage: LoadLibraryA("httpconnect.dll"), LoadLibraryA("dlltest.dll")
	// Return values are only compared with zero and are never used in other APIs
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "LoadLibraryA") == 0) {
		stack->correctParams(1);
		const char *dllName = stack->pop()->getString();
		int result = 0;

		if (strcmp(dllName, "httpconnect.dll") == 0) {
			result = 1; // some non-zero value
		} else if (strcmp(dllName, "dlltest.dll") == 0) {
			result = 2; // some other non-zero value
		} else {
			warning("LoadLibraryA(\"%s\") is not implemented", dllName);
		}

		stack->pushInt(result);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FreeLibrary
	// Declared at games by Corbomite Games
	// Seems to be unused, probably was used for unloading IRC & HTTP libraries
	// Specification: external "kernel32.dll" stdcall FreeLibrary(long)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(function->name, "FreeLibrary") == 0) {
		stack->correctParams(1);
		/*int dllId =*/ stack->pop()->getInt();

		// do nothing

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetEnvironmentVariableA
	// Used for getting environment variables at Pizza Morgana: Episode 1 - Monsters and Manipulations in the Magical Forest
	// Specification: external "kernel32.dll" stdcall long GetEnvironmentVariableA(string, string, long)
	// Known usage: GetEnvironmentVariableA(<EnvName>, <buffer>, 65535)
	// Known EnvName values used in debug code: "USERKEY", "ALTUSERNAME", "ENHFINGERPRINT", "EXTRAINFO", "FINGERPRINT", "KEYSTRING", "STOLENKEY", "TRIAL"
	// Known EnvName values used in licensing code: "FULLGAME"
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(function->name, "GetEnvironmentVariableA") == 0) {
		stack->correctParams(3);
		const char *name = stack->pop()->getString();
		/*ScValue *buf =*/ stack->pop();
		/*int bufMaxLen =*/ stack->pop()->getInt();

		warning("Assuming variable \"%s\" is not set", name);

		stack->pushInt(0);
		return STATUS_OK;
	}

	return STATUS_FAILED;
}

} // End of namespace Wintermute
