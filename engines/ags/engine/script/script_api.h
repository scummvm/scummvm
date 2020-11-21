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
// Script API function type and helper macros for forwarding runtime script
// values to real engine functions.
//
//=============================================================================
#ifndef __AGS_EE_SCRIPT__SCRIPTAPI_H
#define __AGS_EE_SCRIPT__SCRIPTAPI_H

#include <stdarg.h>
#include "core/types.h"
#include "ac/runtime_defines.h"
#include "ac/statobj/agsstaticobject.h"
#include "debug/out.h"

struct RuntimeScriptValue;

// TODO: replace void* with base object class when possible; also put array class for parameters
typedef RuntimeScriptValue ScriptAPIFunction(const RuntimeScriptValue *params, int32_t param_count);
typedef RuntimeScriptValue ScriptAPIObjectFunction(void *self, const RuntimeScriptValue *params, int32_t param_count);

// Sprintf that takes either script values or common argument list from plugin.
// Uses EITHER sc_args/sc_argc or varg_ptr as parameter list, whichever is not
// NULL, with varg_ptr having HIGHER priority.
const char *ScriptSprintf(char *buffer, size_t buf_length, const char *format,
                          const RuntimeScriptValue *sc_args, int32_t sc_argc, va_list *varg_ptr);
// Sprintf that takes script values as arguments
inline const char *ScriptSprintf(char *buffer, size_t buf_length, const char *format, const RuntimeScriptValue *args, int32_t argc)
{
    return ScriptSprintf(buffer, buf_length, format, args, argc, nullptr);
}
// Variadic sprintf (needed, because all arguments are pushed as pointer-sized values). Currently used only when plugin calls
// exported engine function. Should be removed when this plugin issue is resolved.
inline const char *ScriptVSprintf(char *buffer, size_t buf_length, const char *format, va_list &arg_ptr)
{
    return ScriptSprintf(buffer, buf_length, format, nullptr, 0, &arg_ptr);
}

// Helper macros for script functions
#define ASSERT_SELF(METHOD) \
    assert((self != NULL) && "Object pointer is null in call to API function")
#define ASSERT_PARAM_COUNT(FUNCTION, X) \
    assert((params != NULL && param_count >= X) && "Not enough parameters in call to API function")
#define ASSERT_VARIABLE_VALUE(VARIABLE) \
    assert((params != NULL && param_count >= 1) && "Not enough parameters to set API property")
#define ASSERT_OBJ_PARAM_COUNT(METHOD, X) \
    ASSERT_SELF(METHOD); \
    ASSERT_PARAM_COUNT(METHOD, X)

//-----------------------------------------------------------------------------
// Get/set variables

#define API_VARGET_INT(VARIABLE) \
    return RuntimeScriptValue().SetInt32(VARIABLE)

#define API_VARSET_PINT(VARIABLE) \
    ASSERT_VARIABLE_VALUE(VARIABLE); \
    VARIABLE = params[0].IValue; \
    return RuntimeScriptValue()

//-----------------------------------------------------------------------------
// Calls to ScriptSprintf

#define API_SCALL_SCRIPT_SPRINTF(FUNCTION, PARAM_COUNT) \
    ASSERT_PARAM_COUNT(FUNCTION, PARAM_COUNT); \
    char ScSfBuffer[STD_BUFFER_SIZE]; \
    const char *scsf_buffer = ScriptSprintf(ScSfBuffer, STD_BUFFER_SIZE, get_translation(params[PARAM_COUNT - 1].Ptr), params + PARAM_COUNT, param_count - PARAM_COUNT)

#define API_OBJCALL_SCRIPT_SPRINTF(METHOD, PARAM_COUNT) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, PARAM_COUNT); \
    char ScSfBuffer[STD_BUFFER_SIZE]; \
    const char *scsf_buffer = ScriptSprintf(ScSfBuffer, STD_BUFFER_SIZE, get_translation(params[PARAM_COUNT - 1].Ptr), params + PARAM_COUNT, param_count - PARAM_COUNT)

//-----------------------------------------------------------------------------
// Calls to ScriptSprintfV (unsafe plugin variant)

#define API_PLUGIN_SCRIPT_SPRINTF(FORMAT_STR) \
    va_list args; \
    va_start(args, FORMAT_STR); \
    char ScSfBuffer[STD_BUFFER_SIZE]; \
    const char *scsf_buffer = ScriptVSprintf(ScSfBuffer, STD_BUFFER_SIZE, get_translation(FORMAT_STR), args); \
    va_end(args)

//-----------------------------------------------------------------------------
// Calls to static functions
//
// IMPORTANT: please note following: historically AGS compiler did not have
// proper "void" type and allowed to store the return value of "void" API
// functions as an integer (although that value did not have any practical
// meaning). For backwards compatibility we actually return integer value
// of '0' in all the VOID script API functions!
//

#define API_SCALL_VOID(FUNCTION) \
    FUNCTION(); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_PBOOL(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 1); \
    FUNCTION(params[0].GetAsBool()); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_PINT(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 1); \
    FUNCTION(params[0].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_PINT2(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    FUNCTION(params[0].IValue, params[1].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_PINT3(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 3); \
    FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_PINT4(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 4); \
    FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_PINT5(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 5); \
    FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_PINT6(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 6); \
    FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue, params[5].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_PINT_POBJ(FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    FUNCTION(params[0].IValue, (P1CLASS*)params[1].Ptr); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_PINT_POBJ2(FUNCTION, P1CLASS, P2CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 3); \
    FUNCTION(params[0].IValue, (P1CLASS*)params[1].Ptr, (P2CLASS*)params[2].Ptr); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_PINT2_POBJ(FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 3); \
    FUNCTION(params[0].IValue, params[1].IValue, (P1CLASS*)params[2].Ptr); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_PINT3_POBJ_PINT(FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 5); \
    FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, (P1CLASS*)params[3].Ptr, params[4].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_PINT4_POBJ(FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 5); \
    FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue, (P1CLASS*)params[4].Ptr); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_PFLOAT2(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    FUNCTION(params[0].FValue, params[1].FValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_POBJ(FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 1); \
    FUNCTION((P1CLASS*)params[0].Ptr); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_POBJ_PINT(FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    FUNCTION((P1CLASS*)params[0].Ptr, params[1].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_POBJ_PINT2(FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 3); \
    FUNCTION((P1CLASS*)params[0].Ptr, params[1].IValue, params[2].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_VOID_POBJ2(FUNCTION, P1CLASS, P2CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    FUNCTION((P1CLASS*)params[0].Ptr, (P2CLASS*)params[1].Ptr); \
    return RuntimeScriptValue((int32_t)0)

#define API_SCALL_INT(FUNCTION) \
    return RuntimeScriptValue().SetInt32(FUNCTION())

#define API_SCALL_INT_PINT(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 1); \
    return RuntimeScriptValue().SetInt32(FUNCTION(params[0].IValue))

#define API_SCALL_INT_PINT2(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    return RuntimeScriptValue().SetInt32(FUNCTION(params[0].IValue, params[1].IValue))

#define API_SCALL_INT_PINT3(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 3); \
    return RuntimeScriptValue().SetInt32(FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue))

#define API_SCALL_INT_PINT4(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 4); \
    return RuntimeScriptValue().SetInt32(FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue))

#define API_SCALL_INT_PINT4_PFLOAT(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 5) \
    return RuntimeScriptValue().SetInt32(FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue, params[3].FValue))

#define API_SCALL_INT_PINT5(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 5); \
    return RuntimeScriptValue().SetInt32(FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue))

#define API_SCALL_INT_PFLOAT_PINT(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    return RuntimeScriptValue().SetInt32(FUNCTION(params[0].FValue, params[1].IValue))

#define API_SCALL_INT_POBJ(FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 1); \
    return RuntimeScriptValue().SetInt32(FUNCTION((P1CLASS*)params[0].Ptr))

#define API_SCALL_INT_POBJ_PINT(FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    return RuntimeScriptValue().SetInt32(FUNCTION((P1CLASS*)params[0].Ptr, params[1].IValue))

#define API_SCALL_INT_POBJ_PINT2(FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 3); \
    return RuntimeScriptValue().SetInt32(FUNCTION((P1CLASS*)params[0].Ptr, params[1].IValue, params[2].IValue))

#define API_SCALL_INT_POBJ2(FUNCTION, P1CLASS, P2CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    return RuntimeScriptValue().SetInt32(FUNCTION((P1CLASS*)params[0].Ptr, (P2CLASS*)params[1].Ptr))

#define API_SCALL_INT_PINT_POBJ(FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    return RuntimeScriptValue().SetInt32(FUNCTION(params[0].IValue, (P1CLASS*)params[1].Ptr))

#define API_SCALL_FLOAT(FUNCTION) \
    return RuntimeScriptValue().SetFloat(FUNCTION())

#define API_SCALL_FLOAT_PINT(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 1); \
    return RuntimeScriptValue().SetFloat(FUNCTION(params[0].IValue))

#define API_SCALL_FLOAT_PFLOAT(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 1); \
    return RuntimeScriptValue().SetFloat(FUNCTION(params[0].FValue))

#define API_SCALL_FLOAT_PFLOAT2(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    return RuntimeScriptValue().SetFloat(FUNCTION(params[0].FValue, params[1].FValue))

#define API_SCALL_BOOL(FUNCTION) \
    return RuntimeScriptValue().SetInt32AsBool(FUNCTION())

#define API_SCALL_BOOL_OBJ(FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 1); \
    return RuntimeScriptValue().SetInt32AsBool(FUNCTION((P1CLASS*)params[0].Ptr))

#define API_SCALL_BOOL_POBJ_PINT(FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    return RuntimeScriptValue().SetInt32AsBool(FUNCTION((P1CLASS*)params[0].Ptr, params[1].IValue))

#define API_SCALL_BOOL_POBJ2(FUNCTION, P1CLASS, P2CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    return RuntimeScriptValue().SetInt32AsBool(FUNCTION((P1CLASS*)params[0].Ptr, (P2CLASS*)params[1].Ptr))

#define API_SCALL_OBJ(RET_CLASS, RET_MGR, FUNCTION) \
    return RuntimeScriptValue().SetDynamicObject((void*)(RET_CLASS*)FUNCTION(), &RET_MGR)

#define API_SCALL_OBJ_PINT(RET_CLASS, RET_MGR, FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 1); \
    return RuntimeScriptValue().SetDynamicObject((void*)(RET_CLASS*)FUNCTION(params[0].IValue), &RET_MGR)

#define API_SCALL_OBJ_POBJ_PINT_PBOOL(RET_CLASS, RET_MGR, FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 3); \
    return RuntimeScriptValue().SetDynamicObject((void*)(RET_CLASS*)FUNCTION((P1CLASS*)params[0].Ptr, params[1].IValue, params[2].GetAsBool()), &RET_MGR)

#define API_SCALL_OBJ_PINT2(RET_CLASS, RET_MGR, FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    return RuntimeScriptValue().SetDynamicObject((void*)(RET_CLASS*)FUNCTION(params[0].IValue, params[1].IValue), &RET_MGR)

#define API_SCALL_OBJ_PINT3_POBJ(RET_CLASS, RET_MGR, FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 4); \
    return RuntimeScriptValue().SetDynamicObject((void*)(RET_CLASS*)FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, (P1CLASS*)params[3].Ptr), &RET_MGR)

#define API_SCALL_OBJ_POBJ(RET_CLASS, RET_MGR, FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 1); \
    return RuntimeScriptValue().SetDynamicObject((void*)(RET_CLASS*)FUNCTION((P1CLASS*)params[0].Ptr), &RET_MGR)

#define API_SCALL_OBJAUTO(RET_CLASS, FUNCTION) \
    RET_CLASS* ret_obj = FUNCTION(); \
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PINT(RET_CLASS, FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 1); \
    RET_CLASS* ret_obj = FUNCTION(params[0].IValue); \
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PINT2(RET_CLASS, FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    RET_CLASS* ret_obj = FUNCTION(params[0].IValue, params[1].IValue); \
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PINT3(RET_CLASS, FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 3); \
    RET_CLASS* ret_obj = FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue); \
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PINT4(RET_CLASS, FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 4); \
    RET_CLASS* ret_obj = FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue); \
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PINT5(RET_CLASS, FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 5); \
    RET_CLASS* ret_obj = FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue); \
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PBOOL2(RET_CLASS, FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    RET_CLASS* ret_obj = FUNCTION(params[0].GetAsBool(), params[1].GetAsBool()); \
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_POBJ(RET_CLASS, FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 1); \
    RET_CLASS* ret_obj = FUNCTION((P1CLASS*)params[0].Ptr); \
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_POBJ_PINT(RET_CLASS, FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    RET_CLASS* ret_obj = (RET_CLASS*)FUNCTION((P1CLASS*)params[0].Ptr, params[1].IValue); \
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_POBJ_PINT4(RET_CLASS, FUNCTION, P1CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 5); \
    RET_CLASS* ret_obj = FUNCTION((P1CLASS*)params[0].Ptr, params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue); \
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj)


#define API_SCALL_STOBJ_POBJ2(RET_CLASS, FUNCTION, P1CLASS, P2CLASS) \
    ASSERT_PARAM_COUNT(FUNCTION, 2); \
    RET_CLASS* ret_obj = FUNCTION((P1CLASS*)params[0].Ptr, (P2CLASS*)params[1].Ptr); \
    return RuntimeScriptValue().SetStaticObject(ret_obj, &GlobalStaticManager)

//-----------------------------------------------------------------------------
// Calls to object functions

#define API_OBJCALL_VOID(CLASS, METHOD) \
    ASSERT_SELF(METHOD); \
    METHOD((CLASS*)self); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_PINT(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
    METHOD((CLASS*)self, params[0].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_PINT2(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    METHOD((CLASS*)self, params[0].IValue, params[1].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_PINT3(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 3); \
    METHOD((CLASS*)self, params[0].IValue, params[1].IValue, params[2].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_PINT4(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 4); \
    METHOD((CLASS*)self, params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_PINT5(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 5); \
    METHOD((CLASS*)self, params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_PINT6(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 6); \
    METHOD((CLASS*)self, params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue, params[5].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_PFLOAT(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
    METHOD((CLASS*)self, params[0].FValue); \
    return RuntimeScriptValue()

#define API_OBJCALL_VOID_PFLOAT2(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    METHOD((CLASS*)self, params[0].FValue, params[1].FValue); \
    return RuntimeScriptValue()

#define API_OBJCALL_VOID_PBOOL(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
    METHOD((CLASS*)self, params[0].GetAsBool()); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_PINT_PBOOL(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    METHOD((CLASS*)self, params[0].IValue, params[1].GetAsBool()); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_PINT_POBJ(CLASS, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    METHOD((CLASS*)self, params[0].IValue, (P1CLASS*)params[1].Ptr); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_PINT3_POBJ(CLASS, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 4); \
    METHOD((CLASS*)self, params[0].IValue, params[1].IValue, params[2].IValue, (P1CLASS*)params[3].Ptr); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_PINT5_POBJ(CLASS, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 6); \
    METHOD((CLASS*)self, params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue, (P1CLASS*)params[5].Ptr); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_POBJ(CLASS, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
    METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_POBJ_PINT(CLASS, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr, params[1].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_POBJ_PINT2(CLASS, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 3); \
    METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr, params[1].IValue, params[2].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_POBJ2(CLASS, METHOD, P1CLASS, P2CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr, (P2CLASS*)params[1].Ptr); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_INT(CLASS, METHOD) \
    ASSERT_SELF(METHOD); \
    return RuntimeScriptValue().SetInt32(METHOD((CLASS*)self))

#define API_OBJCALL_INT_PINT(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
    return RuntimeScriptValue().SetInt32(METHOD((CLASS*)self, params[0].IValue))

#define API_OBJCALL_INT_PINT_POBJ(CLASS, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    return RuntimeScriptValue().SetInt32(METHOD((CLASS*)self, params[0].IValue, params[1].Ptr))

#define API_OBJCALL_INT_PINT2(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    return RuntimeScriptValue().SetInt32(METHOD((CLASS*)self, params[0].IValue, params[1].IValue))

#define API_OBJCALL_INT_POBJ(CLASS, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
    return RuntimeScriptValue().SetInt32(METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr))

#define API_OBJCALL_INT_POBJ_PINT(CLASS, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    return RuntimeScriptValue().SetInt32(METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr, params[1].IValue))

#define API_OBJCALL_INT_POBJ_PBOOL(CLASS, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    return RuntimeScriptValue().SetInt32(METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr, params[1].GetAsBool()))

#define API_OBJCALL_FLOAT(CLASS, METHOD) \
    ASSERT_SELF(METHOD); \
    return RuntimeScriptValue().SetFloat(METHOD((CLASS*)self))

#define API_OBJCALL_BOOL(CLASS, METHOD) \
    ASSERT_SELF(METHOD); \
    return RuntimeScriptValue().SetInt32AsBool(METHOD((CLASS*)self))

#define API_OBJCALL_BOOL_PINT(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
    return RuntimeScriptValue().SetInt32AsBool(METHOD((CLASS*)self, params[0].IValue))

#define API_OBJCALL_BOOL_POBJ(CLASS, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
    return RuntimeScriptValue().SetInt32AsBool(METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr))

#define API_OBJCALL_BOOL_POBJ_PINT(CLASS, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    return RuntimeScriptValue().SetInt32AsBool(METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr, params[1].IValue))

#define API_OBJCALL_BOOL_POBJ2(CLASS, METHOD, P1CLASS, P2CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    return RuntimeScriptValue().SetInt32AsBool(METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr, (P2CLASS*)params[1].Ptr))

#define API_OBJCALL_BOOL(CLASS, METHOD) \
    ASSERT_SELF(METHOD); \
    return RuntimeScriptValue().SetInt32AsBool(METHOD((CLASS*)self))

#define API_OBJCALL_OBJ_PINT_POBJ(CLASS, RET_CLASS, RET_MGR, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    return RuntimeScriptValue().SetDynamicObject((void*)METHOD((CLASS*)self, params[0].IValue, (P1CLASS*)params[1].Ptr), &RET_MGR)

#define API_OBJCALL_OBJ_POBJ2_PINT(CLASS, RET_CLASS, RET_MGR, METHOD, P1CLASS, P2CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 3); \
    return RuntimeScriptValue().SetDynamicObject((void*)METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr, (P2CLASS*)params[1].Ptr, params[2].IValue), &RET_MGR)

#define API_OBJCALL_OBJ_POBJ2_PBOOL(CLASS, RET_CLASS, RET_MGR, METHOD, P1CLASS, P2CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 3); \
    return RuntimeScriptValue().SetDynamicObject((void*)METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr, (P2CLASS*)params[1].Ptr, params[2].GetAsBool()), &RET_MGR)

#define API_OBJCALL_OBJ(CLASS, RET_CLASS, RET_MGR, METHOD) \
    ASSERT_SELF(METHOD); \
    return RuntimeScriptValue().SetDynamicObject((void*)(RET_CLASS*)METHOD((CLASS*)self), &RET_MGR)

#define API_OBJCALL_OBJ_PINT(CLASS, RET_CLASS, RET_MGR, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
    return RuntimeScriptValue().SetDynamicObject((void*)(RET_CLASS*)METHOD((CLASS*)self, params[0].IValue), &RET_MGR)

#define API_OBJCALL_OBJ_PINT2(CLASS, RET_CLASS, RET_MGR, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
    return RuntimeScriptValue().SetDynamicObject((void*)(RET_CLASS*)METHOD((CLASS*)self, params[0].IValue, params[1].IValue), &RET_MGR)

#define API_OBJCALL_OBJ_PINT3(CLASS, RET_CLASS, RET_MGR, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 3); \
    return RuntimeScriptValue().SetDynamicObject((void*)(RET_CLASS*)METHOD((CLASS*)self, params[0].IValue, params[1].IValue, params[2].IValue), &RET_MGR)

#define API_OBJCALL_OBJ_POBJ(CLASS, RET_CLASS, RET_MGR, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
    return RuntimeScriptValue().SetDynamicObject((void*)(RET_CLASS*)METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr), &RET_MGR)

#define API_OBJCALL_OBJAUTO(CLASS, RET_CLASS, METHOD) \
    ASSERT_SELF(METHOD); \
    RET_CLASS* ret_obj = METHOD((CLASS*)self); \
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj)

#define API_OBJCALL_OBJAUTO_PINT2_PBOOL(CLASS, RET_CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 3); \
    RET_CLASS* ret_obj = METHOD((CLASS*)self, params[0].IValue, params[1].IValue, params[2].GetAsBool()); \
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj)

#define API_OBJCALL_OBJAUTO_POBJ(CLASS, RET_CLASS, METHOD, P1CLASS) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
    RET_CLASS* ret_obj = METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr); \
    return RuntimeScriptValue().SetDynamicObject(ret_obj, ret_obj)

#endif // __AGS_EE_SCRIPT__SCRIPTAPI_H
