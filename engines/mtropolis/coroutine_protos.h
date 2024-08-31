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

#ifndef MTROPOLIS_COROUTINE_PROTOS_H
#define MTROPOLIS_COROUTINE_PROTOS_H

#include <stddef.h>

namespace MTropolis {

struct ICoroutineCompiler;
struct CompiledCoroutine;

struct CoroutineStackFrame2;
struct CoroutineParamsBase;
struct CoroutineReturnValueRefBase;
class CoroutineManager;

typedef CoroutineStackFrame2 *(*CoroutineFrameConstructor_t)(void *ptr, const CompiledCoroutine *compiledCoro, const CoroutineParamsBase &params, const CoroutineReturnValueRefBase &returnValueRef);
typedef void (*CoroutineGetFrameParametersFunction_t)(size_t &outSize, size_t &outAlignment);
typedef void (*CoroutineCompileFunction_t)(ICoroutineCompiler *compiler);


struct CoroutineParamsBase {
};

} // End of namespace MTropolis

#define CORO_STUB												\
	static void compileCoroutine(ICoroutineCompiler *compiler);	\
	static CompiledCoroutine *ms_compiledCoro;

#define CORO_DEFINE_PARAMS_0()                   \
	CORO_STUB                                    \
	struct Params : public CoroutineParamsBase { \
	}

#define CORO_DEFINE_PARAMS_1(type1, name1)						\
	CORO_STUB													\
	struct Params : public CoroutineParamsBase {				\
		typedef type1 ParamType1_t;								\
																\
		ParamType1_t name1;										\
																\
		inline explicit Params(const ParamType1_t &p_##name1)	\
			: name1(p_##name1) {								\
		}														\
																\
	private:													\
		Params() = delete;										\
	}

#define CORO_DEFINE_PARAMS_2(type1, name1, type2, name2)								\
	CORO_STUB																			\
	struct Params : public CoroutineParamsBase {										\
		typedef type1 ParamType1_t;														\
		typedef type2 ParamType2_t;														\
																						\
		ParamType1_t name1;																\
		ParamType2_t name2;																\
																						\
		explicit Params(const ParamType1_t &p_##name1, const ParamType2_t &p_##name2)	\
			: name1(p_##name1), name2(p_##name2) {										\
		}																				\
																						\
	private:																			\
		Params() = delete;																\
	}

#define CORO_DEFINE_PARAMS_3(type1, name1, type2, name2, type3, name3)					\
	CORO_STUB																			\
	struct Params : public CoroutineParamsBase {										\
		typedef type1 ParamType1_t;														\
		typedef type2 ParamType2_t;														\
		typedef type3 ParamType3_t;														\
																						\
		ParamType1_t name1;																\
		ParamType2_t name2;																\
		ParamType3_t name3;																\
																						\
		explicit Params(const ParamType1_t &p_##name1, const ParamType2_t &p_##name2, const ParamType3_t &p_##name3)	\
			: name1(p_##name1), name2(p_##name2), name3(p_##name3) {					\
		}																				\
																						\
	private:																			\
		Params() = delete;																\
	}

#define CORO_DEFINE_PARAMS_4(type1, name1, type2, name2, type3, name3, type4, name4)	\
	CORO_STUB																			\
	struct Params : public CoroutineParamsBase {										\
		typedef type1 ParamType1_t;														\
		typedef type2 ParamType2_t;														\
		typedef type3 ParamType3_t;														\
		typedef type4 ParamType4_t;														\
																						\
		ParamType1_t name1;																\
		ParamType2_t name2;																\
		ParamType3_t name3;																\
		ParamType4_t name4;																\
																						\
		explicit Params(const ParamType1_t &p_##name1, const ParamType2_t &p_##name2, const ParamType3_t &p_##name3, const ParamType4_t &p_##name4)	\
			: name1(p_##name1), name2(p_##name2), name3(p_##name3), name4(p_##name4) {	\
		}																				\
																						\
	private:																			\
		Params() = delete;																\
	}

#define CORO_DEFINE_RETURN_TYPE(type)	\
	typedef type ReturnValue_t

#endif
