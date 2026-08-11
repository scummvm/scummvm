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

#ifndef AGS_ENGINE_SCRIPT_SCRIPT_RUNTIME_H
#define AGS_ENGINE_SCRIPT_SCRIPT_RUNTIME_H

#include "ags/shared/core/types.h"
#include "ags/shared/script/cc_script.h"      // ccScript
#include "ags/engine/script/cc_instance.h"    // ccInstance

namespace AGS3 {

struct IScriptObject;

using AGS::Shared::String;

// Helper struct for declaring a script API function registration
struct ScFnRegister {
	const char *Name = nullptr;
	RuntimeScriptValue Fn;   // for script VM calls
	RuntimeScriptValue PlFn; // for plugins, direct unsafe call style

	ScFnRegister() = default;
	ScFnRegister(const char *name, ScriptAPIFunction *fn, void *plfn = nullptr)
		: Name(name), Fn(RuntimeScriptValue().SetStaticFunction(fn)), PlFn(RuntimeScriptValue().SetPluginMethod((Plugins::ScriptContainer *)plfn, nullptr)) {}
	ScFnRegister(const char *name, ScriptAPIObjectFunction *fn, void *plfn = nullptr)
		: Name(name), Fn(RuntimeScriptValue().SetObjectFunction(fn)), PlFn(RuntimeScriptValue().SetPluginMethod((Plugins::ScriptContainer *)plfn, nullptr)) {}
	template<typename TPlFn>
	ScFnRegister(const char *name, ScriptAPIFunction *fn, TPlFn plfn)
		: Name(name), Fn(RuntimeScriptValue().SetStaticFunction(fn)), PlFn(RuntimeScriptValue().SetPluginMethod((Plugins::ScriptContainer *)plfn, nullptr)) {}
	template<typename TPlFn>
	ScFnRegister(const char *name, ScriptAPIObjectFunction *fn, TPlFn plfn)
		: Name(name), Fn(RuntimeScriptValue().SetObjectFunction(fn)), PlFn(RuntimeScriptValue().SetPluginMethod((Plugins::ScriptContainer *)plfn, nullptr)) {}
};

// Following functions register engine API symbols for script and plugins.
// Calls from script is handled by specific "translator" functions, which
// unpack script interpreter's values into the real arguments and call the
// actual engine's function. For plugins we have to provide actual engine
// function directly.
bool ccAddExternalStaticFunction(const String &name, ScriptAPIFunction *pfn);
bool ccAddExternalObjectFunction(const String &name, ScriptAPIObjectFunction *pfn);
bool ccAddExternalFunctionForPlugin(const String &name, Plugins::ScriptContainer *sc);
// Register a function, exported from a plugin. Requires direct function pointer only.
bool ccAddExternalPluginFunction(const String &name, Plugins::ScriptContainer *sc);
// Register engine objects for script's access.
bool ccAddExternalStaticArray(const String &name, void *ptr, CCStaticArray *array_mgr);
bool ccAddExternalScriptObject(const String &name, void *ptr, IScriptObject *manager);
// Register script own functions (defined in the linked scripts)
bool ccAddExternalScriptSymbol(const String &name, const RuntimeScriptValue &prval, ccInstance *inst);
// Remove the script access to a variable or function in your program
void ccRemoveExternalSymbol(const String &name);
// Remove all external symbols, allowing you to start from scratch
void ccRemoveAllSymbols();

// FIXME: These functions should replace the older ones; for now they are duplicated to ease
// the transition
bool ccAddExternalStaticFunction361(const String &name, ScriptAPIFunction *scfn, void *dirfn = nullptr);
bool ccAddExternalObjectFunction361(const String &name, ScriptAPIObjectFunction *scfn, void *dirfn = nullptr);
bool ccAddExternalFunction361(const ScFnRegister &scfnreg);
// Registers an array of static functions
template<size_t N>
inline void ccAddExternalFunctions361(const ScFnRegister (&arr)[N]) {
	for (const ScFnRegister *it = arr; it != (arr + N); ++it)
		ccAddExternalFunction361(*it);
}

// Get the address of an exported variable in the script
void *ccGetSymbolAddress(const String &name);
// Get a registered symbol's direct pointer; this is used solely for plugins
Plugins::PluginMethod ccGetSymbolAddressForPlugin(const String &name);
// Get a registered Script Object, optionally restricting to the given type name
void *ccGetScriptObjectAddress(const String &name, const String &type);

// DEBUG HOOK
typedef void (*new_line_hook_type)(ccInstance *, int);
void ccSetDebugHook(new_line_hook_type jibble);

// Set the script interpreter timeout values:
// * sys_poll_timeout - defines the timeout (ms) at which the interpreter will run system events poll;
// * abort_timeout - [temp disabled] defines the timeout (ms) at which the interpreter will cancel with error.
// * abort_loops - max script loops without an engine update after which the interpreter will error;
void ccSetScriptAliveTimer(unsigned sys_poll_timeout, unsigned abort_timeout, unsigned abort_loops);
// reset the current while loop counter
void ccNotifyScriptStillAlive();
// for calling exported plugin functions old-style
NumberPtr call_function(const Plugins::PluginMethod &method, const RuntimeScriptValue *object, int numparm, const RuntimeScriptValue *parms);

} // namespace AGS3

#endif
