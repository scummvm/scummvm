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
// Script API function type and helper macros for forwarding runtime script
// values to real engine functions.
//
//=============================================================================

#ifndef AGS_ENGINE_SCRIPT_SCRIPT_API_H
#define AGS_ENGINE_SCRIPT_SCRIPT_API_H

//include <stdarg.h>
#include "ags/shared/core/types.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/shared/debugging/out.h"

namespace AGS3 {

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
inline const char *ScriptSprintf(char *buffer, size_t buf_length, const char *format, const RuntimeScriptValue *args, int32_t argc) {
	return ScriptSprintf(buffer, buf_length, format, args, argc, nullptr);
}
// Variadic sprintf (needed, because all arguments are pushed as pointer-sized values). Currently used only when plugin calls
// exported engine function. Should be removed when this plugin issue is resolved.
inline const char *ScriptVSprintf(char *buffer, size_t buf_length, const char *format, va_list &arg_ptr) {
	return ScriptSprintf(buffer, buf_length, format, nullptr, 0, &arg_ptr);
}

// Helper macro for registering an API function for both script and plugin,
// for the common case where they have similar names: the script's "translator"
// function's name is derived from the real one by adding a "Sc_" prefix.
#define API_FN_PAIR(FN_NAME) Sc_##FN_NAME, (void *)FN_NAME

// Helper macros for script functions;
// asserting for internal mistakes; suppressing "unused param" warnings
#define ASSERT_SELF(METHOD) \
    (void)params; (void)param_count; \
    assert((self != NULL) && "Object pointer is null in call to API function")
#define ASSERT_PARAM_COUNT(FUNCTION, X) \
    (void)params; (void)param_count; \
    assert((params != NULL && param_count >= X) && "Not enough parameters in call to API function")
#define ASSERT_VARIABLE_VALUE(VARIABLE) \
    (void)params; (void)param_count; \
    assert((params != NULL && param_count >= 1) && "Not enough parameters to set API property")
#define ASSERT_OBJ_PARAM_COUNT(METHOD, X) \
    ASSERT_SELF(METHOD); \
    ASSERT_PARAM_COUNT(METHOD, X)

//-----------------------------------------------------------------------------
// Calls to ScriptSprintf with automatic translation

#define API_SCALL_SCRIPT_SPRINTF(FUNCTION, PARAM_COUNT) \
	ASSERT_PARAM_COUNT(FUNCTION, PARAM_COUNT); \
	char ScSfBuffer[STD_BUFFER_SIZE]; \
	const char *scsf_buffer = ScriptSprintf(ScSfBuffer, STD_BUFFER_SIZE, get_translation(params[PARAM_COUNT - 1].CStr), params + PARAM_COUNT, param_count - PARAM_COUNT)

#define API_OBJCALL_SCRIPT_SPRINTF(METHOD, PARAM_COUNT) \
	ASSERT_OBJ_PARAM_COUNT(METHOD, PARAM_COUNT); \
	char ScSfBuffer[STD_BUFFER_SIZE]; \
	const char *scsf_buffer = ScriptSprintf(ScSfBuffer, STD_BUFFER_SIZE, get_translation(params[PARAM_COUNT - 1].CStr), params + PARAM_COUNT, param_count - PARAM_COUNT)

//-----------------------------------------------------------------------------
// Calls to ScriptSprintf without translation

#define API_SCALL_SCRIPT_SPRINTF_PURE(FUNCTION, PARAM_COUNT) \
    ASSERT_PARAM_COUNT(FUNCTION, PARAM_COUNT); \
    char ScSfBuffer[STD_BUFFER_SIZE]; \
    const char *scsf_buffer = ScriptSprintf(ScSfBuffer, STD_BUFFER_SIZE, params[PARAM_COUNT - 1].CStr, params + PARAM_COUNT, param_count - PARAM_COUNT)


//-----------------------------------------------------------------------------
// Calls to ScriptSprintfV (unsafe plugin variant)

#define API_PLUGIN_SCRIPT_SPRINTF(FORMAT_STR) \
	va_list args; \
	va_start(args, FORMAT_STR); \
	char ScSfBuffer[STD_BUFFER_SIZE]; \
	const char *scsf_buffer = ScriptVSprintf(ScSfBuffer, STD_BUFFER_SIZE, get_translation(FORMAT_STR), args); \
	va_end(args)

#define API_PLUGIN_SCRIPT_SPRINTF_PURE(FORMAT_STR) \
	va_list args; \
	va_start(args, FORMAT_STR); \
	char ScSfBuffer[STD_BUFFER_SIZE]; \
	const char *scsf_buffer = ScriptVSprintf(ScSfBuffer, STD_BUFFER_SIZE, FORMAT_STR, args); \
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
	(void)params; (void)param_count; \
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
	(void)params; (void)param_count; \
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
	(void)params; (void)param_count; \
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
	(void)params; (void)param_count; \
	return RuntimeScriptValue().SetInt32AsBool(FUNCTION())

#define API_SCALL_BOOL_POBJ(FUNCTION, P1CLASS) \
	ASSERT_PARAM_COUNT(FUNCTION, 1); \
	return RuntimeScriptValue().SetInt32AsBool(FUNCTION((P1CLASS*)params[0].Ptr))

#define API_SCALL_BOOL_PINT(FUNCTION) \
    ASSERT_PARAM_COUNT(FUNCTION, 1); \
    return RuntimeScriptValue().SetInt32AsBool(FUNCTION(params[0].IValue))

#define API_SCALL_BOOL_POBJ_PINT(FUNCTION, P1CLASS) \
	ASSERT_PARAM_COUNT(FUNCTION, 2); \
	return RuntimeScriptValue().SetInt32AsBool(FUNCTION((P1CLASS*)params[0].Ptr, params[1].IValue))

#define API_SCALL_BOOL_POBJ2(FUNCTION, P1CLASS, P2CLASS) \
	ASSERT_PARAM_COUNT(FUNCTION, 2); \
	return RuntimeScriptValue().SetInt32AsBool(FUNCTION((P1CLASS*)params[0].Ptr, (P2CLASS*)params[1].Ptr))

#define API_SCALL_OBJ(RET_CLASS, RET_MGR, FUNCTION) \
	(void)params; (void)param_count; \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)FUNCTION()), &RET_MGR)
/*
	#define API_CONST_SCALL_OBJ(RET_CLASS, RET_MGR, FUNCTION) \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)FUNCTION()), &RET_MGR)
*/
#define API_SCALL_OBJ_PINT(RET_CLASS, RET_MGR, FUNCTION) \
	ASSERT_PARAM_COUNT(FUNCTION, 1); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)FUNCTION(params[0].IValue)), &RET_MGR)
/*
#define API_CONST_SCALL_OBJ_PINT(RET_CLASS, RET_MGR, FUNCTION) \
	ASSERT_PARAM_COUNT(FUNCTION, 1); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)FUNCTION(params[0].IValue)), &RET_MGR)
*/
#define API_SCALL_OBJ_POBJ_PINT_PBOOL(RET_CLASS, RET_MGR, FUNCTION, P1CLASS) \
	ASSERT_PARAM_COUNT(FUNCTION, 3); \
	return RuntimeScriptValue().SetScriptObject((void*)(RET_CLASS*)FUNCTION((P1CLASS*)params[0].Ptr, params[1].IValue, params[2].GetAsBool()), &RET_MGR)

#define API_SCALL_OBJ_PINT2(RET_CLASS, RET_MGR, FUNCTION) \
	ASSERT_PARAM_COUNT(FUNCTION, 2); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)FUNCTION(params[0].IValue, params[1].IValue)), &RET_MGR)
/*
#define API_CONST_SCALL_OBJ_PINT2(RET_CLASS, RET_MGR, FUNCTION) \
	ASSERT_PARAM_COUNT(FUNCTION, 2); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)FUNCTION(params[0].IValue, params[1].IValue)), &RET_MGR)
*/
#define API_SCALL_OBJ_PINT3_POBJ(RET_CLASS, RET_MGR, FUNCTION, P1CLASS) \
	ASSERT_PARAM_COUNT(FUNCTION, 4); \
	return RuntimeScriptValue().SetScriptObject((void*)(RET_CLASS*)FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, (P1CLASS*)params[3].Ptr), &RET_MGR)

#define API_SCALL_OBJ_POBJ(RET_CLASS, RET_MGR, FUNCTION, P1CLASS) \
	ASSERT_PARAM_COUNT(FUNCTION, 1); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)FUNCTION((P1CLASS*)params[0].Ptr)), &RET_MGR)
/*
#define API_CONST_SCALL_OBJ_POBJ(RET_CLASS, RET_MGR, FUNCTION, P1CLASS) \
	ASSERT_PARAM_COUNT(FUNCTION, 1); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)FUNCTION((P1CLASS*)params[0].Ptr)), &RET_MGR)
*/
#define API_SCALL_OBJAUTO(RET_CLASS, FUNCTION) \
	(void)params; (void)param_count; \
	RET_CLASS* ret_obj = FUNCTION(); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PINT(RET_CLASS, FUNCTION) \
	ASSERT_PARAM_COUNT(FUNCTION, 1); \
	RET_CLASS* ret_obj = FUNCTION(params[0].IValue); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PINT2(RET_CLASS, FUNCTION) \
	ASSERT_PARAM_COUNT(FUNCTION, 2); \
	RET_CLASS* ret_obj = FUNCTION(params[0].IValue, params[1].IValue); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PINT3(RET_CLASS, FUNCTION) \
	ASSERT_PARAM_COUNT(FUNCTION, 3); \
	RET_CLASS* ret_obj = FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PINT4(RET_CLASS, FUNCTION) \
	ASSERT_PARAM_COUNT(FUNCTION, 4); \
	RET_CLASS* ret_obj = FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PINT5(RET_CLASS, FUNCTION) \
	ASSERT_PARAM_COUNT(FUNCTION, 5); \
	RET_CLASS* ret_obj = FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PINT2_PBOOL(RET_CLASS, FUNCTION) \
	ASSERT_PARAM_COUNT(FUNCTION, 3); \
	RET_CLASS *ret_obj = FUNCTION(params[0].IValue, params[1].IValue, params[2].GetAsBool()); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PINT3_PBOOL(RET_CLASS, FUNCTION) \
	ASSERT_PARAM_COUNT(FUNCTION, 4); \
	RET_CLASS *ret_obj = FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, params[3].GetAsBool()); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PINT3_PBOOL2(RET_CLASS, FUNCTION) \
	ASSERT_PARAM_COUNT(FUNCTION, 5); \
	RET_CLASS *ret_obj = FUNCTION(params[0].IValue, params[1].IValue, params[2].IValue, params[3].GetAsBool(), params[4].GetAsBool()); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_PBOOL2(RET_CLASS, FUNCTION) \
	ASSERT_PARAM_COUNT(FUNCTION, 2); \
	RET_CLASS* ret_obj = FUNCTION(params[0].GetAsBool(), params[1].GetAsBool()); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_POBJ(RET_CLASS, FUNCTION, P1CLASS) \
	ASSERT_PARAM_COUNT(FUNCTION, 1); \
	RET_CLASS* ret_obj = FUNCTION((P1CLASS*)params[0].Ptr); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_POBJ_PINT(RET_CLASS, FUNCTION, P1CLASS) \
	ASSERT_PARAM_COUNT(FUNCTION, 2); \
	RET_CLASS* ret_obj = (RET_CLASS*)FUNCTION((P1CLASS*)params[0].Ptr, params[1].IValue); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_SCALL_OBJAUTO_POBJ_PINT4(RET_CLASS, FUNCTION, P1CLASS) \
	ASSERT_PARAM_COUNT(FUNCTION, 5); \
	RET_CLASS* ret_obj = FUNCTION((P1CLASS*)params[0].Ptr, params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)


#define API_SCALL_STOBJ_POBJ2(RET_CLASS, FUNCTION, P1CLASS, P2CLASS) \
	ASSERT_PARAM_COUNT(FUNCTION, 2); \
	RET_CLASS* ret_obj = FUNCTION((P1CLASS*)params[0].Ptr, (P2CLASS*)params[1].Ptr); \
	return RuntimeScriptValue().SetStaticObject(ret_obj, &_GP(GlobalStaticManager))

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

#define API_OBJCALL_VOID_PINT7(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 7); \
    METHOD((CLASS*)self, params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue, params[5].IValue, params[6].IValue); \
    return RuntimeScriptValue((int32_t)0)

#define API_OBJCALL_VOID_PINT8(CLASS, METHOD) \
    ASSERT_OBJ_PARAM_COUNT(METHOD, 8); \
    METHOD((CLASS*)self, params[0].IValue, params[1].IValue, params[2].IValue, params[3].IValue, params[4].IValue, params[5].IValue, params[6].IValue, params[7].IValue); \
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
	return RuntimeScriptValue().SetInt32(METHOD((CLASS*)self, params[0].IValue, params[1].CStr))

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
	return RuntimeScriptValue().SetScriptObject((void*)METHOD((CLASS*)self, params[0].IValue, (P1CLASS*)params[1].Ptr), &RET_MGR)

#define API_OBJCALL_OBJ_POBJ2_PINT(CLASS, RET_CLASS, RET_MGR, METHOD, P1CLASS, P2CLASS) \
	ASSERT_OBJ_PARAM_COUNT(METHOD, 3); \
	return RuntimeScriptValue().SetScriptObject((void*)METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr, (P2CLASS*)params[1].Ptr, params[2].IValue), &RET_MGR)

#define API_OBJCALL_OBJ_POBJ2_PBOOL(CLASS, RET_CLASS, RET_MGR, METHOD, P1CLASS, P2CLASS) \
	ASSERT_OBJ_PARAM_COUNT(METHOD, 3); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr, (P2CLASS*)params[1].Ptr, params[2].GetAsBool())), &RET_MGR)
/*
#define API_CONST_OBJCALL_OBJ_POBJ2_PBOOL(CLASS, RET_CLASS, RET_MGR, METHOD, P1CLASS, P2CLASS) \
	ASSERT_OBJ_PARAM_COUNT(METHOD, 3); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr, (P2CLASS*)params[1].Ptr, params[2].GetAsBool())), &RET_MGR)
*/
#define API_OBJCALL_OBJ(CLASS, RET_CLASS, RET_MGR, METHOD) \
	ASSERT_SELF(METHOD); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)METHOD((CLASS*)self)), &RET_MGR)
/*
#define API_CONST_OBJCALL_OBJ(CLASS, RET_CLASS, RET_MGR, METHOD) \
	ASSERT_SELF(METHOD); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)METHOD((CLASS*)self)), &RET_MGR)
*/
#define API_OBJCALL_OBJ_PINT(CLASS, RET_CLASS, RET_MGR, METHOD) \
	ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)METHOD((CLASS*)self, params[0].IValue)), &RET_MGR)
/*
#define API_CONST_OBJCALL_OBJ_PINT(CLASS, RET_CLASS, RET_MGR, METHOD) \
	ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)METHOD((CLASS*)self, params[0].IValue)), &RET_MGR)
*/
#define API_OBJCALL_OBJ_PINT2(CLASS, RET_CLASS, RET_MGR, METHOD) \
	ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)METHOD((CLASS*)self, params[0].IValue, params[1].IValue)), &RET_MGR)
/*
#define API_CONST_OBJCALL_OBJ_PINT2(CLASS, RET_CLASS, RET_MGR, METHOD) \
	ASSERT_OBJ_PARAM_COUNT(METHOD, 2); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)METHOD((CLASS*)self, params[0].IValue, params[1].IValue)), &RET_MGR)
*/
#define API_OBJCALL_OBJ_PINT3(CLASS, RET_CLASS, RET_MGR, METHOD) \
	ASSERT_OBJ_PARAM_COUNT(METHOD, 3); \
	return RuntimeScriptValue().SetScriptObject((void*)(RET_CLASS*)METHOD((CLASS*)self, params[0].IValue, params[1].IValue, params[2].IValue), &RET_MGR)

#define API_OBJCALL_OBJ_POBJ(CLASS, RET_CLASS, RET_MGR, METHOD, P1CLASS) \
	ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr)), &RET_MGR)
/*
#define API_CONST_OBJCALL_OBJ_POBJ(CLASS, RET_CLASS, RET_MGR, METHOD, P1CLASS) \
	ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
	return RuntimeScriptValue().SetScriptObject(const_cast<void *>((const void *)(RET_CLASS*)METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr)), &RET_MGR)
*/
#define API_OBJCALL_OBJAUTO(CLASS, RET_CLASS, METHOD) \
	ASSERT_SELF(METHOD); \
	RET_CLASS* ret_obj = METHOD((CLASS*)self); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_OBJCALL_OBJAUTO_PINT2_PBOOL(CLASS, RET_CLASS, METHOD) \
	ASSERT_OBJ_PARAM_COUNT(METHOD, 3); \
	RET_CLASS* ret_obj = METHOD((CLASS*)self, params[0].IValue, params[1].IValue, params[2].GetAsBool()); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

#define API_OBJCALL_OBJAUTO_POBJ(CLASS, RET_CLASS, METHOD, P1CLASS) \
	ASSERT_OBJ_PARAM_COUNT(METHOD, 1); \
	RET_CLASS* ret_obj = METHOD((CLASS*)self, (P1CLASS*)params[0].Ptr); \
	return RuntimeScriptValue().SetScriptObject(ret_obj, ret_obj)

} // namespace AGS3

#endif
