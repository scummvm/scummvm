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

#ifndef AGS_ENGINE_SCRIPT_SCRIPT_RUNTIME_H
#define AGS_ENGINE_SCRIPT_SCRIPT_RUNTIME_H

#include "ags/shared/core/types.h"
#include "ags/shared/script/cc_script.h"      // ccScript
#include "ags/engine/script/cc_instance.h"    // ccInstance

namespace AGS3 {

struct ICCStaticObject;
struct ICCDynamicObject;
struct StaticArray;

using AGS::Shared::String;
using AGS::Shared::String;

// ************ SCRIPT LOADING AND RUNNING FUNCTIONS ************

// give the script access to a variable or function in your program
extern bool ccAddExternalStaticFunction(const String &name, ScriptAPIFunction *pfn);
// temporary workaround for plugins
extern bool ccAddExternalPluginFunction(const String &name, Plugins::ScriptContainer *sc);
extern bool ccAddExternalStaticObject(const String &name, void *ptr, ICCStaticObject *manager);
extern bool ccAddExternalStaticArray(const String &name, void *ptr, StaticArray *array_mgr);
extern bool ccAddExternalDynamicObject(const String &name, void *ptr, ICCDynamicObject *manager);
extern bool ccAddExternalObjectFunction(const String &name, ScriptAPIObjectFunction *pfn);
extern bool ccAddExternalScriptSymbol(const String &name, const RuntimeScriptValue &prval, ccInstance *inst);
// remove the script access to a variable or function in your program
extern void ccRemoveExternalSymbol(const String &name);
// removes all external symbols, allowing you to start from scratch
extern void ccRemoveAllSymbols();

// get the address of an exported variable in the script
extern void *ccGetSymbolAddress(const String &name);

// registering functions, compatible with old unsafe call style;
// this is to be used solely by plugins until plugin inteface is redone
extern bool ccAddExternalFunctionForPlugin(const String &name, Plugins::ScriptContainer *instance);
extern Plugins::PluginMethod ccGetSymbolAddressForPlugin(const String &name);

// DEBUG HOOK
typedef void (*new_line_hook_type)(ccInstance *, int);
extern void ccSetDebugHook(new_line_hook_type jibble);

// Set the script interpreter timeout values, in milliseconds:
// * sys_poll_timeout - defines the timeout at which the interpreter will run system events poll;
// * abort_timeout - defines the timeout at which the interpreter will cancel with error.
extern void ccSetScriptAliveTimer(unsigned sys_poll_timeout, unsigned abort_timeout);
// reset the current while loop counter
extern void ccNotifyScriptStillAlive();
// for calling exported plugin functions old-style
extern int call_function(const Plugins::PluginMethod &method,
	const RuntimeScriptValue *obj, int numparm, const RuntimeScriptValue *parms);
extern void nullfree(void *data); // in script/script_runtime

} // namespace AGS3

#endif
