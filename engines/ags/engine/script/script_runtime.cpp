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

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "script/script_runtime.h"
#include "script/script_common.h"
#include "script/cc_error.h"
#include "script/cc_options.h"
#include "ac/dynobj/cc_dynamicarray.h"
#include "script/systemimports.h"
#include "ac/statobj/staticobject.h"

extern ccInstance *current_instance; // in script/cc_instance

bool ccAddExternalStaticFunction(const String &name, ScriptAPIFunction *pfn)
{
    return simp.add(name, RuntimeScriptValue().SetStaticFunction(pfn), nullptr) == 0;
}

bool ccAddExternalPluginFunction(const String &name, void *pfn)
{
    return simp.add(name, RuntimeScriptValue().SetPluginFunction(pfn), nullptr) == 0;
}

bool ccAddExternalStaticObject(const String &name, void *ptr, ICCStaticObject *manager)
{
    return simp.add(name, RuntimeScriptValue().SetStaticObject(ptr, manager), nullptr) == 0;
}

bool ccAddExternalStaticArray(const String &name, void *ptr, StaticArray *array_mgr)
{
    return simp.add(name, RuntimeScriptValue().SetStaticArray(ptr, array_mgr), nullptr) == 0;
}

bool ccAddExternalDynamicObject(const String &name, void *ptr, ICCDynamicObject *manager)
{
    return simp.add(name, RuntimeScriptValue().SetDynamicObject(ptr, manager), nullptr) == 0;
}

bool ccAddExternalObjectFunction(const String &name, ScriptAPIObjectFunction *pfn)
{
    return simp.add(name, RuntimeScriptValue().SetObjectFunction(pfn), nullptr) == 0;
}

bool ccAddExternalScriptSymbol(const String &name, const RuntimeScriptValue &prval, ccInstance *inst)
{
    return simp.add(name, prval, inst) == 0;
}

void ccRemoveExternalSymbol(const String &name)
{
    simp.remove(name);
}

void ccRemoveAllSymbols()
{
    simp.clear();
}

ccInstance *loadedInstances[MAX_LOADED_INSTANCES] = {nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

void nullfree(void *data)
{
    if (data != nullptr)
        free(data);
}

void *ccGetSymbolAddress(const String &name)
{
    const ScriptImport *import = simp.getByName(name);
    if (import)
    {
        return import->Value.Ptr;
    }
    return nullptr;
}

bool ccAddExternalFunctionForPlugin(const String &name, void *pfn)
{
    return simp_for_plugin.add(name, RuntimeScriptValue().SetPluginFunction(pfn), nullptr) == 0;
}

void *ccGetSymbolAddressForPlugin(const String &name)
{
    const ScriptImport *import = simp_for_plugin.getByName(name);
    if (import)
    {
        return import->Value.Ptr;
    }
    else
    {
        // Also search the internal symbol table for non-function symbols
        import = simp.getByName(name);
        if (import)
        {
            return import->Value.Ptr;
        }
    }
    return nullptr;
}

new_line_hook_type new_line_hook = nullptr;

char ccRunnerCopyright[] = "ScriptExecuter32 v" SCOM_VERSIONSTR " (c) 2001 Chris Jones";
int maxWhileLoops = 0;

// If a while loop does this many iterations without the
// NofityScriptAlive function getting called, the script
// aborts. Set to 0 to disable.
void ccSetScriptAliveTimer (int numloop) {
    maxWhileLoops = numloop;
}

void ccNotifyScriptStillAlive () {
    if (current_instance != nullptr)
        current_instance->flags |= INSTF_RUNNING;
}

void ccSetDebugHook(new_line_hook_type jibble)
{
    new_line_hook = jibble;
}

int call_function(intptr_t addr, const RuntimeScriptValue *object, int numparm, const RuntimeScriptValue *parms)
{
    if (!addr)
    {
        cc_error("null function pointer in call_function");
        return -1;
    }
    if (numparm > 0 && !parms)
    {
        cc_error("invalid parameters array in call_function");
        return -1;
    }

    intptr_t parm_value[9];
    if (object)
    {
        parm_value[0] = (intptr_t)object->GetPtrWithOffset();
        numparm++;
    }

    for (int ival = object ? 1 : 0, iparm = 0; ival < numparm; ++ival, ++iparm)
    {
        switch (parms[iparm].Type)
        {
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

    //
    // AN IMPORTANT NOTE ON PARAM TYPE
    // of 2012-11-10
    //
    //// NOTE of 2012-12-20:
    //// Everything said below is applicable only for calling
    //// exported plugin functions.
    //
    // Here we are sending parameters of type intptr_t to registered
    // function of unknown kind. Intptr_t is 32-bit for x32 build and
    // 64-bit for x64 build.
    // The exported functions usually have two types of parameters:
    // pointer and 'int' (32-bit). For x32 build those two have the
    // same size, but for x64 build first has 64-bit size while the
    // second remains 32-bit.
    // In formal case that would cause 'overflow' - function will
    // receive more data than needed (written to stack), with some
    // values shifted further by 32 bits.
    //
    // Upon testing, however, it was revealed that AMD64 processor,
    // the only platform we support x64 Linux AGS build on right now,
    // treats all the function parameters pushed to stack as 64-bit
    // values (few first parameters are sent via registers, and hence
    // are least concern anyway). Therefore, no 'overflow' occurs,
    // and 64-bit values are being effectively truncated to 32-bit
    // integers in the callee.
    //
    // Since this is still quite unreliable, this should be
    // reimplemented when there's enough free time available for
    // developers both for coding & testing.
    //
    // Most basic idea is to pass array of RuntimeScriptValue
    // objects (that hold type description) and get same RSV as a
    // return result. Keep in mind, though, that this solution will
    // require fixing ALL exported functions, so a good amount of
    // time and energy should be allocated for this task.
    //

    switch (numparm)
    {
    case 0:
        {
            int (*fparam) ();
            fparam = (int (*)())addr;
            return fparam();
        }
    case 1:
        {
            int (*fparam) (intptr_t);
            fparam = (int (*)(intptr_t))addr;
            return fparam(parm_value[0]);
        }
    case 2:
        {
            int (*fparam) (intptr_t, intptr_t);
            fparam = (int (*)(intptr_t, intptr_t))addr;
            return fparam(parm_value[0], parm_value[1]);
        }
    case 3:
        {
            int (*fparam) (intptr_t, intptr_t, intptr_t);
            fparam = (int (*)(intptr_t, intptr_t, intptr_t))addr;
            return fparam(parm_value[0], parm_value[1], parm_value[2]);
        }
    case 4:
        {
            int (*fparam) (intptr_t, intptr_t, intptr_t, intptr_t);
            fparam = (int (*)(intptr_t, intptr_t, intptr_t, intptr_t))addr;
            return fparam(parm_value[0], parm_value[1], parm_value[2], parm_value[3]);
        }
    case 5:
        {
            int (*fparam) (intptr_t, intptr_t, intptr_t, intptr_t, intptr_t);
            fparam = (int (*)(intptr_t, intptr_t, intptr_t, intptr_t, intptr_t))addr;
            return fparam(parm_value[0], parm_value[1], parm_value[2], parm_value[3], parm_value[4]);
        }
    case 6:
        {
            int (*fparam) (intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t);
            fparam = (int (*)(intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t))addr;
            return fparam(parm_value[0], parm_value[1], parm_value[2], parm_value[3], parm_value[4], parm_value[5]);
        }
    case 7:
        {
            int (*fparam) (intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t);
            fparam = (int (*)(intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t))addr;
            return fparam(parm_value[0], parm_value[1], parm_value[2], parm_value[3], parm_value[4], parm_value[5], parm_value[6]);
        }
    case 8:
        {
            int (*fparam) (intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t);
            fparam = (int (*)(intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t))addr;
            return fparam(parm_value[0], parm_value[1], parm_value[2], parm_value[3], parm_value[4], parm_value[5], parm_value[6], parm_value[7]);
        }
    case 9:
        {
            int (*fparam) (intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t);
            fparam = (int (*)(intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t, intptr_t))addr;
            return fparam(parm_value[0], parm_value[1], parm_value[2], parm_value[3], parm_value[4], parm_value[5], parm_value[6], parm_value[7], parm_value[8]);
        }
    }

    cc_error("too many arguments in call to function");
    return -1;
}
