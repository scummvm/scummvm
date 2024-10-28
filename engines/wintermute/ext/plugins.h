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
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"

#ifndef WINTERMUTE_PLUGINS_H
#define WINTERMUTE_PLUGINS_H

namespace Wintermute {

// Implemented in their respective .cpp-files
BaseScriptable *makeSXSteamAPI(BaseGame *inGame, ScStack *stack);
BaseScriptable *makeSXWMEGalaxyAPI(BaseGame *inGame, ScStack *stack);
BaseScriptable *makeSX3fStatistics(BaseGame *inGame, ScStack *stack);
BaseScriptable *makeSXCommandLineHelper(BaseGame *inGame, ScStack *stack);
BaseScriptable *makeSXSample(BaseGame *inGame, ScStack *stack);
BaseScriptable *makeSXVlink(BaseGame *inGame, ScStack *stack);

bool EmulatePluginCall(BaseGame *inGame, ScStack *stack, ScStack *thisStack, char *name) {
	ScValue *thisObj;

	//////////////////////////////////////////////////////////////////////////
	// SteamAPI (from wme_steam.dll)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SteamAPI") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXSteamAPI(inGame,  stack));

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WMEGalaxyAPI (from GOG version of julia.exe)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WMEGalaxyAPI") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXWMEGalaxyAPI(inGame,  stack));

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Statistics (from wme_3fstatistics.dll)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Statistics") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSX3fStatistics(inGame,  stack));

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Commandline helper (from wme_commandlinehelper.dll)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CommandLineHelper") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXCommandLineHelper(inGame, stack));

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Window mode changer (from wme_windowmode.dll of "lostbride" game)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Sample") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXSample(inGame, stack));

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// BinkVideo player (from wme_vlink.dll of "Sunrise" game)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "BinkVideo") == 0) {
		thisObj = thisStack->getTop();

		thisObj->setNative(makeSXVlink(inGame, stack));

		stack->pushNULL();
		return STATUS_OK;
	}


	return STATUS_FAILED;
}

} // End of namespace Wintermute

#endif
