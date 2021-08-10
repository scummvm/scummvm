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

//=============================================================================
//
// C-Script run-time interpreter (c) 2001 Chris Jones
//
// You must DISABLE OPTIMIZATIONS AND REGISTER VARIABLES in your compiler
// when compiling this, or strange results can happen.
//
// There is a problem with importing functions on 16-bit compilers: the
// script system assumes that all parameters are passed as 4 bytes, which
// ints are not on 16-bit systems. Be sure to define all parameters as longs,
// or join the 21st century and switch to DJGPP or Visual C++.
//
//=============================================================================

#include "ags/engine/script/script_runtime.h"
#include "ags/shared/script/script_common.h"
#include "ags/shared/script/cc_error.h"
#include "ags/shared/script/cc_options.h"
#include "ags/engine/ac/dynobj/cc_dynamic_array.h"
#include "ags/engine/script/system_imports.h"
#include "ags/engine/ac/statobj/static_object.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/plugins/plugin_base.h"
#include "ags/globals.h"

namespace AGS3 {

// static const char ccRunnerCopyright[] = "ScriptExecuter32 v" SCOM_VERSIONSTR " (c) 2001 Chris Jones";

bool ccAddExternalStaticFunction(const String &name, ScriptAPIFunction *pfn) {
	return _GP(simp).add(name, RuntimeScriptValue().SetStaticFunction(pfn), nullptr) == 0;
}

bool ccAddExternalPluginFunction(const String &name, Plugins::ScriptContainer *instance) {
	return _GP(simp).add(name, RuntimeScriptValue().SetPluginMethod(instance, name), nullptr) == 0;
}

bool ccAddExternalStaticObject(const String &name, void *ptr, ICCStaticObject *manager) {
	return _GP(simp).add(name, RuntimeScriptValue().SetStaticObject(ptr, manager), nullptr) == 0;
}

bool ccAddExternalStaticArray(const String &name, void *ptr, StaticArray *array_mgr) {
	return _GP(simp).add(name, RuntimeScriptValue().SetStaticArray(ptr, array_mgr), nullptr) == 0;
}

bool ccAddExternalDynamicObject(const String &name, void *ptr, ICCDynamicObject *manager) {
	return _GP(simp).add(name, RuntimeScriptValue().SetDynamicObject(ptr, manager), nullptr) == 0;
}

bool ccAddExternalObjectFunction(const String &name, ScriptAPIObjectFunction *pfn) {
	return _GP(simp).add(name, RuntimeScriptValue().SetObjectFunction(pfn), nullptr) == 0;
}

bool ccAddExternalScriptSymbol(const String &name, const RuntimeScriptValue &prval, ccInstance *inst) {
	return _GP(simp).add(name, prval, inst) == 0;
}

void ccRemoveExternalSymbol(const String &name) {
	_GP(simp).remove(name);
}

void ccRemoveAllSymbols() {
	_GP(simp).clear();
}

void nullfree(void *data) {
	if (data != nullptr)
		free(data);
}

void *ccGetSymbolAddress(const String &name) {
	const ScriptImport *import = _GP(simp).getByName(name);
	if (import) {
		return import->Value.Ptr;
	}
	return nullptr;
}

bool ccAddExternalFunctionForPlugin(const String &name, Plugins::ScriptContainer *instance) {
	return _GP(simp_for_plugin).add(name, RuntimeScriptValue().SetPluginMethod(instance, name), nullptr) == 0;
}

Plugins::PluginMethod ccGetSymbolAddressForPlugin(const String &name) {
	const ScriptImport *import = _GP(simp_for_plugin).getByName(name);
	if (import) {
		return Plugins::PluginMethod((Plugins::ScriptContainer *)import->Value.Ptr, name);
	} else {
		// Also search the internal symbol table for non-function symbols
		import = _GP(simp).getByName(name);
		if (import) {
			return Plugins::PluginMethod((Plugins::ScriptContainer *)import->Value.Ptr, name);
		}
	}

	return Plugins::PluginMethod();
}

// If a while loop does this many iterations without the
// NofityScriptAlive function getting called, the script
// aborts. Set to 0 to disable.
void ccSetScriptAliveTimer(int numloop) {
	_G(maxWhileLoops) = numloop;
}

void ccNotifyScriptStillAlive() {
	if (_G(current_instance) != nullptr)
		_G(current_instance)->flags |= INSTF_RUNNING;
}

void ccSetDebugHook(new_line_hook_type jibble) {
	_G(new_line_hook) = jibble;
}

int call_function(const Plugins::PluginMethod &method,
		const RuntimeScriptValue *object, int numparm, const RuntimeScriptValue *parms) {
	if (!method) {
		cc_error("invalid method in call_function");
		return -1;
	}
	if (numparm > 0 && !parms) {
		cc_error("invalid parameters array in call_function");
		return -1;
	}

	intptr_t parm_value[9];
	if (object) {
		parm_value[0] = (intptr_t)object->GetPtrWithOffset();
		numparm++;
	}

	for (int ival = object ? 1 : 0, iparm = 0; ival < numparm; ++ival, ++iparm) {
		switch (parms[iparm].Type) {
		case kScValInteger:
		case kScValFloat:   // AGS passes floats, copying their values into long variable
		case kScValPluginArg:
			parm_value[ival] = (intptr_t)parms[iparm].IValue;
			break;
			break;
		default:
			parm_value[ival] = (intptr_t)parms[iparm].GetPtrWithOffset();
			break;
		}
	}

	// AN IMPORTANT NOTE ON PARAMS
	// The original AGS interpreter did a bunch of dodgy function pointers with
	// varying numbers of parameters, which were all intptr_t. To simply matters
	// now that we only supported plugins implemented in code, and not DLLs,
	// we use a simplified Common::Array containing the parameters and result

	if (numparm > 9) {
		cc_error("too many arguments in call to function");
		return -1;
	} else {
		// Build the parameters
		Plugins::ScriptMethodParams params;
		for (int i = 0; i < numparm; ++i)
			params.push_back(parm_value[i]);

		// Call the method
		return method(params);
	}
}

} // namespace AGS3
