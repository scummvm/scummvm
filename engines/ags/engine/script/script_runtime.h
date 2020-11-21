//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
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

#ifndef __CS_RUNTIME_H
#define __CS_RUNTIME_H

#include "script/cc_script.h"      // ccScript
#include "script/cc_instance.h"    // ccInstance

struct ICCStaticObject;
struct ICCDynamicObject;
struct StaticArray;

using AGS::Common::String;
using AGS::Common::String;

// ************ SCRIPT LOADING AND RUNNING FUNCTIONS ************

// give the script access to a variable or function in your program
extern bool ccAddExternalStaticFunction(const String &name, ScriptAPIFunction *pfn);
// temporary workaround for plugins
extern bool ccAddExternalPluginFunction(const String &name, void *pfn);
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
extern bool ccAddExternalFunctionForPlugin(const String &name, void *pfn);
extern void *ccGetSymbolAddressForPlugin(const String &name);

// DEBUG HOOK
typedef void (*new_line_hook_type) (ccInstance *, int);
extern void ccSetDebugHook(new_line_hook_type jibble);
#endif

// Set the number of while loop iterations that aborts the script
extern void ccSetScriptAliveTimer (int);
// reset the current while loop counter
extern void ccNotifyScriptStillAlive ();
// for calling exported plugin functions old-style
extern int call_function(intptr_t addr, const RuntimeScriptValue *object, int numparm, const RuntimeScriptValue *parms);
extern void nullfree(void *data); // in script/script_runtime
