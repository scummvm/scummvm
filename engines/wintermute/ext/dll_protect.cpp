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
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"

namespace Wintermute {

bool EmulateProtectExternalCalls(BaseGame *inGame, ScStack *stack, ScStack *thisStack, ScScript::TExternalFunction *function) {
	//////////////////////////////////////////////////////////////////////////
	// PSA_IsTrialMode
	// Used to get if game is in trial mode for "Stroke of Fate: Operation Bunker"
	// Specification: external "protect.dll" stdcall long PSA_IsTrialMode(membuffer);
	// Known usage: long PSA_IsTrialMode(membuffer)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "PSA_IsTrialMode") == 0) {
		stack->correctParams(1);
		/*void *buffer = */stack->pop()->getMemBuffer();

		// do nothing

		stack->pushInt(0);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PSA_IsDemoMode
	// Used to get if game is in trial mode for "Stroke of Fate: Operation Bunker"
	// Specification: external "protect.dll" stdcall long PSA_IsDemoMode(membuffer);
	// Known usage: long PSA_IsDemoMode(membuffer)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "PSA_IsDemoMode") == 0) {
		stack->correctParams(1);
		/*void *buffer = */stack->pop()->getMemBuffer();

		// do nothing

		stack->pushInt(0);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PSA_GetFeaturesGrantedByLicense
	// Used to get if game features granted for "Stroke of Fate: Operation Bunker"
	// Specification: external "protect.dll" stdcall long PSA_GetFeaturesGrantedByLicense(membuffer);
	// Known usage: long PSA_GetFeaturesGrantedByLicense(membuffer)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "PSA_GetFeaturesGrantedByLicense") == 0) {
		stack->correctParams(1);
		void *buffer = stack->pop()->getMemBuffer();

		bool flag1 = false;
		bool flag2 = false;
		bool flag3 = false;
		bool flag4 = false;
		bool flag5 = false;
		*(uint32 *)(buffer) = (flag1 ? (1 << 0) : 0) | (flag2 ? (1 << 1) : 0) | (flag3 ? (1 << 2) : 0) | (flag4 ? (1 << 3) : 0) | (flag5 ? (1 << 4) : 0);

		stack->pushInt(0);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PSA_DisableFeaturesGrantedByLicense
	// Reference in game scripts in "Stroke of Fate: Operation Bunker"
	// Specification: external "protect.dll" stdcall long PSA_DisableFeaturesGrantedByLicense(membuffer);
	// Known usage: none
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "PSA_DisableFeaturesGrantedByLicense") == 0) {
		stack->correctParams(1);
		/*void *buffer = */stack->pop()->getMemBuffer();

		// do nothing

		stack->pushInt(0);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PSA_DummyFunction
	// Reference in game scripts in "Stroke of Fate: Operation Bunker"
	// Specification: external "protect.dll" stdcall void PSA_DummyFunction;
	// Known usage: none
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(function->name, "PSA_DummyFunction") == 0) {
		stack->correctParams(0);

		// do nothing

		stack->pushNULL();
		return STATUS_OK;
	}

	return STATUS_FAILED;
}

} // End of namespace Wintermute
