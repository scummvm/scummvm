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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_scriptable.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"

#ifndef WINTERMUTE_EXTERNALS_H
#define WINTERMUTE_EXTERNALS_H

namespace Wintermute {

// Implemented in their respective .cpp-files
bool EmulateGetURLExternalCalls(BaseGame *, ScStack *, ScStack *, ScScript::TExternalFunction *);
bool EmulateToolsExternalCalls(BaseGame *, ScStack *, ScStack *, ScScript::TExternalFunction *);
bool EmulateImgExternalCalls(BaseGame *, ScStack *, ScStack *, ScScript::TExternalFunction *);
bool EmulateShell32ExternalCalls(BaseGame *, ScStack *, ScStack *, ScScript::TExternalFunction *);
bool EmulateInstallUtilExternalCalls(BaseGame *, ScStack *, ScStack *, ScScript::TExternalFunction *);
bool EmulateDLLTestExternalCalls(BaseGame *, ScStack *, ScStack *, ScScript::TExternalFunction *);
bool EmulateKernel32ExternalCalls(BaseGame *, ScStack *, ScStack *, ScScript::TExternalFunction *);
bool EmulateHTTPConnectExternalCalls(BaseGame *, ScStack *, ScStack *, ScScript::TExternalFunction *);
bool EmulateRoutineExternalCalls(BaseGame *, ScStack *, ScStack *, ScScript::TExternalFunction *);
bool EmulateProtectExternalCalls(BaseGame *, ScStack *, ScStack *, ScScript::TExternalFunction *);

bool EmulateExternalCall(BaseGame *inGame, ScStack *stack, ScStack *thisStack, ScScript::TExternalFunction *function) {

	if (strcmp(function->dll_name, "geturl.dll") == 0) {
		return EmulateGetURLExternalCalls(inGame, stack, thisStack, function);
	}

	if (strcmp(function->dll_name, "tools.dll") == 0) {
		return EmulateToolsExternalCalls(inGame, stack, thisStack, function);
	}

	if (strcmp(function->dll_name, "img.dll") == 0) {
		return EmulateImgExternalCalls(inGame, stack, thisStack, function);
	}

	if (strcmp(function->dll_name, "shell32.dll") == 0) {
		return EmulateShell32ExternalCalls(inGame, stack, thisStack, function);
	}

	if (strcmp(function->dll_name, "installutil.dll") == 0) {
		return EmulateInstallUtilExternalCalls(inGame, stack, thisStack, function);
	}

	if (strcmp(function->dll_name, "dlltest.dll") == 0) {
		return EmulateDLLTestExternalCalls(inGame, stack, thisStack, function);
	}

	if (strcmp(function->dll_name, "kernel32.dll") == 0) {
		return EmulateKernel32ExternalCalls(inGame, stack, thisStack, function);
	}

	if (strcmp(function->dll_name, "httpconnect.dll") == 0) {
		return EmulateHTTPConnectExternalCalls(inGame, stack, thisStack, function);
	}

	if (strcmp(function->dll_name, "routine.dll") == 0) {
		return EmulateRoutineExternalCalls(inGame, stack, thisStack, function);
	}

	if (strcmp(function->dll_name, "protect.dll") == 0) {
		return EmulateProtectExternalCalls(inGame, stack, thisStack, function);
	}

	warning("External function %s from %s library is not known by ScummVM", function->name, function->dll_name);
	return STATUS_FAILED;
}

} // End of namespace Wintermute

#endif
