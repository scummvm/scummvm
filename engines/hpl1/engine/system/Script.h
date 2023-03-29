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
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_SCRIPT_H
#define HPL_SCRIPT_H

#include "hpl1/engine/resources/ResourceBase.h"
//#include "hpl1/engine/libraries/angelscript/angelscript.h"

#ifdef __GNUC__
#ifdef __ppc__
#define __stdcall
#else
#define __stdcall __attribute__((stdcall))
#endif
#endif

// Script Macros to build Generic wrappers if necessary
#define SCRIPT_DEFINE_FUNC(return, funcname) \
	SCRIPT_FUNC_BEGIN(funcname, return, "")  \
	SCRIPT_RETURN_CALL_##return funcname();  \
	SCRIPT_FUNC_END(funcname, return )
#define SCRIPT_DEFINE_FUNC_1(return, funcname, arg0) \
	SCRIPT_FUNC_BEGIN(funcname, return, #arg0)       \
	SCRIPT_ARG_##arg0(0);                            \
	SCRIPT_RETURN_CALL_##return funcname(_arg0);     \
	SCRIPT_FUNC_END(funcname, return )
#define SCRIPT_DEFINE_FUNC_2(return, funcname, arg0, arg1) \
	SCRIPT_FUNC_BEGIN(funcname, return, #arg0 "," #arg1)   \
	SCRIPT_ARG_##arg0(0);                                  \
	SCRIPT_ARG_##arg1(1);                                  \
	SCRIPT_RETURN_CALL_##return funcname(_arg0, _arg1);    \
	SCRIPT_FUNC_END(funcname, return )
#define SCRIPT_DEFINE_FUNC_3(return, funcname, arg0, arg1, arg2)   \
	SCRIPT_FUNC_BEGIN(funcname, return, #arg0 "," #arg1 "," #arg2) \
	SCRIPT_ARG_##arg0(0);                                          \
	SCRIPT_ARG_##arg1(1);                                          \
	SCRIPT_ARG_##arg2(2);                                          \
	SCRIPT_RETURN_CALL_##return funcname(_arg0, _arg1, _arg2);     \
	SCRIPT_FUNC_END(funcname, return )
#define SCRIPT_DEFINE_FUNC_4(return, funcname, arg0, arg1, arg2, arg3)       \
	SCRIPT_FUNC_BEGIN(funcname, return, #arg0 "," #arg1 "," #arg2 "," #arg3) \
	SCRIPT_ARG_##arg0(0);                                                    \
	SCRIPT_ARG_##arg1(1);                                                    \
	SCRIPT_ARG_##arg2(2);                                                    \
	SCRIPT_ARG_##arg3(3);                                                    \
	SCRIPT_RETURN_CALL_##return funcname(_arg0, _arg1, _arg2, _arg3);        \
	SCRIPT_FUNC_END(funcname, return )
#define SCRIPT_DEFINE_FUNC_5(return, funcname, arg0, arg1, arg2, arg3, arg4)           \
	SCRIPT_FUNC_BEGIN(funcname, return, #arg0 "," #arg1 "," #arg2 "," #arg3 "," #arg4) \
	SCRIPT_ARG_##arg0(0);                                                              \
	SCRIPT_ARG_##arg1(1);                                                              \
	SCRIPT_ARG_##arg2(2);                                                              \
	SCRIPT_ARG_##arg3(3);                                                              \
	SCRIPT_ARG_##arg4(4);                                                              \
	SCRIPT_RETURN_CALL_##return funcname(_arg0, _arg1, _arg2, _arg3, _arg4);           \
	SCRIPT_FUNC_END(funcname, return )
#define SCRIPT_DEFINE_FUNC_6(return, funcname, arg0, arg1, arg2, arg3, arg4, arg5)               \
	SCRIPT_FUNC_BEGIN(funcname, return, #arg0 "," #arg1 "," #arg2 "," #arg3 "," #arg4 "," #arg5) \
	SCRIPT_ARG_##arg0(0);                                                                        \
	SCRIPT_ARG_##arg1(1);                                                                        \
	SCRIPT_ARG_##arg2(2);                                                                        \
	SCRIPT_ARG_##arg3(3);                                                                        \
	SCRIPT_ARG_##arg4(4);                                                                        \
	SCRIPT_ARG_##arg5(5);                                                                        \
	SCRIPT_RETURN_CALL_##return funcname(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);              \
	SCRIPT_FUNC_END(funcname, return )
#define SCRIPT_DEFINE_FUNC_7(return, funcname, arg0, arg1, arg2, arg3, arg4, arg5, arg6)                   \
	SCRIPT_FUNC_BEGIN(funcname, return, #arg0 "," #arg1 "," #arg2 "," #arg3 "," #arg4 "," #arg5 "," #arg6) \
	SCRIPT_ARG_##arg0(0);                                                                                  \
	SCRIPT_ARG_##arg1(1);                                                                                  \
	SCRIPT_ARG_##arg2(2);                                                                                  \
	SCRIPT_ARG_##arg3(3);                                                                                  \
	SCRIPT_ARG_##arg4(4);                                                                                  \
	SCRIPT_ARG_##arg5(5);                                                                                  \
	SCRIPT_ARG_##arg6(6);                                                                                  \
	SCRIPT_RETURN_CALL_##return funcname(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);                 \
	SCRIPT_FUNC_END(funcname, return )
#define SCRIPT_DEFINE_FUNC_8(return, funcname, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)                       \
	SCRIPT_FUNC_BEGIN(funcname, return, #arg0 "," #arg1 "," #arg2 "," #arg3 "," #arg4 "," #arg5 "," #arg6 "," #arg7) \
	SCRIPT_ARG_##arg0(0);                                                                                            \
	SCRIPT_ARG_##arg1(1);                                                                                            \
	SCRIPT_ARG_##arg2(2);                                                                                            \
	SCRIPT_ARG_##arg3(3);                                                                                            \
	SCRIPT_ARG_##arg4(4);                                                                                            \
	SCRIPT_ARG_##arg5(5);                                                                                            \
	SCRIPT_ARG_##arg6(6);                                                                                            \
	SCRIPT_ARG_##arg7(7);                                                                                            \
	SCRIPT_RETURN_CALL_##return funcname(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);                    \
	SCRIPT_FUNC_END(funcname, return )
#define SCRIPT_DEFINE_FUNC_9(return, funcname, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)                           \
	SCRIPT_FUNC_BEGIN(funcname, return, #arg0 "," #arg1 "," #arg2 "," #arg3 "," #arg4 "," #arg5 "," #arg6 "," #arg7 "," #arg8) \
	SCRIPT_ARG_##arg0(0);                                                                                                      \
	SCRIPT_ARG_##arg1(1);                                                                                                      \
	SCRIPT_ARG_##arg2(2);                                                                                                      \
	SCRIPT_ARG_##arg3(3);                                                                                                      \
	SCRIPT_ARG_##arg4(4);                                                                                                      \
	SCRIPT_ARG_##arg5(5);                                                                                                      \
	SCRIPT_ARG_##arg6(6);                                                                                                      \
	SCRIPT_ARG_##arg7(7);                                                                                                      \
	SCRIPT_ARG_##arg8(8);                                                                                                      \
	SCRIPT_RETURN_CALL_##return funcname(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);                       \
	SCRIPT_FUNC_END(funcname, return )

#define SCRIPT_DEFINE_FUNC_10(return, funcname, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)                              \
	SCRIPT_FUNC_BEGIN(funcname, return, #arg0 "," #arg1 "," #arg2 "," #arg3 "," #arg4 "," #arg5 "," #arg6 "," #arg7 "," #arg8 "," #arg9) \
	SCRIPT_ARG_##arg0(0);                                                                                                                \
	SCRIPT_ARG_##arg1(1);                                                                                                                \
	SCRIPT_ARG_##arg2(2);                                                                                                                \
	SCRIPT_ARG_##arg3(3);                                                                                                                \
	SCRIPT_ARG_##arg4(4);                                                                                                                \
	SCRIPT_ARG_##arg5(5);                                                                                                                \
	SCRIPT_ARG_##arg6(6);                                                                                                                \
	SCRIPT_ARG_##arg7(7);                                                                                                                \
	SCRIPT_ARG_##arg8(8);                                                                                                                \
	SCRIPT_ARG_##arg9(9);                                                                                                                \
	SCRIPT_RETURN_CALL_##return funcname(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9);                          \
	SCRIPT_FUNC_END(funcname, return )

#define SCRIPT_DEFINE_FUNC_12(return, funcname, arg0, arg1, arg2, arg3, arg4, arg5, arg6,                                                                      \
									arg7, arg8, arg9, arg10, arg11)                                                                                            \
	SCRIPT_FUNC_BEGIN(funcname, return, #arg0 "," #arg1 "," #arg2 "," #arg3 "," #arg4 "," #arg5 "," #arg6 "," #arg7 "," #arg8 "," #arg9 "," #arg10 "," #arg11) \
	SCRIPT_ARG_##arg0(0);                                                                                                                                      \
	SCRIPT_ARG_##arg1(1);                                                                                                                                      \
	SCRIPT_ARG_##arg2(2);                                                                                                                                      \
	SCRIPT_ARG_##arg3(3);                                                                                                                                      \
	SCRIPT_ARG_##arg4(4);                                                                                                                                      \
	SCRIPT_ARG_##arg5(5);                                                                                                                                      \
	SCRIPT_ARG_##arg6(6);                                                                                                                                      \
	SCRIPT_ARG_##arg7(7);                                                                                                                                      \
	SCRIPT_ARG_##arg8(8);                                                                                                                                      \
	SCRIPT_ARG_##arg9(9);                                                                                                                                      \
	SCRIPT_ARG_##arg10(10);                                                                                                                                    \
	SCRIPT_ARG_##arg11(11);                                                                                                                                    \
	SCRIPT_RETURN_CALL_##return funcname(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6,                                                                      \
										 _arg7, _arg8, _arg9, _arg10, _arg11);                                                                                 \
	SCRIPT_FUNC_END(funcname, return )
#define SCRIPT_DEFINE_FUNC_17(return, funcname, arg0, arg1, arg2, arg3, arg4, arg5, arg6,                                                                                                                             \
									arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16)                                                                                                                \
	SCRIPT_FUNC_BEGIN(funcname, return, #arg0 "," #arg1 "," #arg2 "," #arg3 "," #arg4 "," #arg5 "," #arg6 "," #arg7 "," #arg8 "," #arg9 "," #arg10 "," #arg11 "," #arg12 "," #arg13 "," #arg14 "," #arg15 "," #arg16) \
	SCRIPT_ARG_##arg0(0);                                                                                                                                                                                             \
	SCRIPT_ARG_##arg1(1);                                                                                                                                                                                             \
	SCRIPT_ARG_##arg2(2);                                                                                                                                                                                             \
	SCRIPT_ARG_##arg3(3);                                                                                                                                                                                             \
	SCRIPT_ARG_##arg4(4);                                                                                                                                                                                             \
	SCRIPT_ARG_##arg5(5);                                                                                                                                                                                             \
	SCRIPT_ARG_##arg6(6);                                                                                                                                                                                             \
	SCRIPT_ARG_##arg7(7);                                                                                                                                                                                             \
	SCRIPT_ARG_##arg8(8);                                                                                                                                                                                             \
	SCRIPT_ARG_##arg9(9);                                                                                                                                                                                             \
	SCRIPT_ARG_##arg10(10);                                                                                                                                                                                           \
	SCRIPT_ARG_##arg11(11);                                                                                                                                                                                           \
	SCRIPT_ARG_##arg12(12);                                                                                                                                                                                           \
	SCRIPT_ARG_##arg13(13);                                                                                                                                                                                           \
	SCRIPT_ARG_##arg14(14);                                                                                                                                                                                           \
	SCRIPT_ARG_##arg15(15);                                                                                                                                                                                           \
	SCRIPT_ARG_##arg16(16);                                                                                                                                                                                           \
	SCRIPT_RETURN_CALL_##return funcname(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6,                                                                                                                             \
										 _arg7, _arg8, _arg9, _arg10, _arg11, _arg12, _arg13, _arg14, _arg15, _arg16);                                                                                                \
	SCRIPT_FUNC_END(funcname, return )

#define SCRIPT_FUNC_BEGIN(funcname, return, args)     \
	namespace GenericScript {                         \
	static const char *funcname##_return = #return;   \
	static const char *funcname##_arg = args;         \
	void funcname##_Generic(asIScriptGeneric * gen) { \
		SCRIPT_RETURN_##return;

#define SCRIPT_FUNC_END(funcname, return ) \
	SCRIPT_SET_RETURN_##return;            \
	}                                      \
	}

// Parameter Macros
// FIXME: string types
#define SCRIPT_ARG_string(n) tString _arg##n = (*(Common::String *)gen->GetArgObject(n))
#define SCRIPT_ARG_float(n) float _arg##n = gen->GetArgFloat(n)
#define SCRIPT_ARG_int(n) int _arg##n = gen->GetArgDWord(n)
#define SCRIPT_ARG_bool(n) bool _arg##n = gen->GetArgByte(n)
// Return Value Macros
#define SCRIPT_RETURN_string tString _ret
#define SCRIPT_RETURN_CALL_string _ret =
#define SCRIPT_SET_RETURN_string gen->SetReturnObject(&_ret)
#define SCRIPT_RETURN_bool bool _ret
#define SCRIPT_RETURN_CALL_bool _ret =
#define SCRIPT_SET_RETURN_bool gen->SetReturnByte(_ret ? -1 : 0)
#define SCRIPT_RETURN_int int _ret
#define SCRIPT_RETURN_CALL_int _ret =
#define SCRIPT_SET_RETURN_int gen->SetReturnDWord(_ret)
#define SCRIPT_RETURN_float float _ret
#define SCRIPT_RETURN_CALL_float _ret =
#define SCRIPT_SET_RETURN_float gen->SetReturnFloat(_ret)
#define SCRIPT_RETURN_void
#define SCRIPT_RETURN_CALL_void
#define SCRIPT_SET_RETURN_void

#define AS_MAX_PORTABILITY
#if defined(AS_MAX_PORTABILITY)
#define SCRIPT_REGISTER_FUNC(funcname) \
	Common::String(GenericScript::funcname##_return) + " " + Common::String(#funcname) + " (" + Common::String(GenericScript::funcname##_arg) + ")", GenericScript::funcname##_Generic, asCALL_GENERIC
#else
#define SCRIPT_REGISTER_FUNC(funcname) \
	GenericScript::funcname##_return + " " #funcname " (" + GenericScript::funcname##_arg + ")", (void *)funcname, asCALL_STDCALL
#endif

namespace hpl {

class iScript : public iResourceBase {
public:
	iScript(const tString &asName) : iResourceBase(asName, 0) {}
	virtual ~iScript() {}

	bool reload() { return false; }
	void unload() {}
	void destroy() {}

	virtual bool CreateFromFile(const tString &asFile) = 0;

	virtual int GetFuncHandle(const tString &asFunc) = 0;

	virtual void AddArg(const tString &asArg) = 0;

	/**
	 * Runs a func in the script, for example "test(15)"
	 * \param asFuncLine the line of code
	 * \return true if everything was ok, else false
	 */
	virtual bool Run(const tString &asFuncLine) = 0;

	virtual bool Run(int alHandle) = 0;
};

} // namespace hpl

#endif // HPL_SCRIPT_H
