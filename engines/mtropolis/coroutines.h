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

#ifndef MTROPOLIS_COROUTINES_H
#define MTROPOLIS_COROUTINES_H

#include "mtropolis/coroutine_protos.h"
#include "mtropolis/miniscript_protos.h"
#include "mtropolis/vthread.h"

namespace MTropolis {

struct CoroExecInstr;
struct CoroutineRuntimeState;

typedef VThreadState (*CoroutineFragmentFunction_t)(CoroutineRuntimeState &coroState);

struct CoroutineRuntimeState {
	CoroutineRuntimeState(VThread *vthread, CoroutineStackFrame2 *frame);

	VThread *_vthread;
	CoroutineStackFrame2 *_frame;
	bool _condition;
	MiniscriptInstructionOutcome _miniscriptOutcome;

private:
	CoroutineRuntimeState() = delete;
	CoroutineRuntimeState(const CoroutineRuntimeState &) = delete;
	CoroutineRuntimeState &operator=(const CoroutineRuntimeState&) = delete;
};

struct CompiledCoroutine {
	CompiledCoroutine();
	~CompiledCoroutine();

	CoroutineFrameConstructor_t _frameConstructor;
	CoroutineGetFrameParametersFunction_t _getFrameParameters;
	bool _isVoidReturn;

	CoroExecInstr *_instructions;
	uint _numInstructions;

private:
	CompiledCoroutine(const CompiledCoroutine &) = delete;
	CompiledCoroutine &operator =(const CompiledCoroutine &) = delete;
};

struct CoroutineStackFrame2 : public VThreadTaskData {
	explicit CoroutineStackFrame2(const CompiledCoroutine *compiledCoro);
	virtual ~CoroutineStackFrame2();

	VThreadState execute(VThread *thread) override;

	const CompiledCoroutine *getCompiledCoroutine() const;

private:
	CoroutineStackFrame2() = delete;
	CoroutineStackFrame2(const CoroutineStackFrame2&) = delete;

	const CompiledCoroutine *_compiledCoro;
	uint _nextInstr;
};

enum class CoroOps {
	Invalid = 0,

	BeginFunction,
	EndFunction,

	IfCond,
	IfBody,
	Else,
	ElseIfCond,
	ElseIfBody,
	EndIf,

	WhileCond,
	WhileBody,
	EndWhile,

	ForNext,
	ForCond,
	ForBody,
	EndFor,

	Do,
	DoWhile,
	DoWhileCond,

	Return,

	Error,

	YieldToFunction,

	CheckMiniscript,

	Code,
};

struct ICoroutineCompiler {
	virtual ~ICoroutineCompiler();

	virtual void defineFunction(CoroutineFrameConstructor_t frameConstructor, CoroutineGetFrameParametersFunction_t frameGetParams) = 0;
	virtual void addOp(CoroOps op, CoroutineFragmentFunction_t fragmentFunc) = 0;
};

#define CORO_START_CODE_BLOCK(op) \
	compiler->addOp(op, [](CoroutineRuntimeState &coroRuntime) -> VThreadState {						\
		Params *params = &static_cast<CoroStackFrame *>(coroRuntime._frame)->_params;											\
		Locals *locals = &static_cast<CoroStackFrame *>(coroRuntime._frame)->_locals;											\
		CoroutineReturnValueRef<ReturnValue_t> coroReturnValueRef = (static_cast<CoroStackFrame *>(coroRuntime._frame)->_rvRef);

#define CORO_AWAIT_PUSHED_TASK					\
	return kVThreadReturn

#define CORO_DISUSE_CODE_BLOCK_VARS				\
		(void)params;							\
		(void)locals;							\
		(void)coroReturnValueRef;

#define CORO_END_CODE_BLOCK						\
		CORO_DISUSE_CODE_BLOCK_VARS				\
		return kVThreadReturn;					\
	});

#define CORO_BEGIN_DEFINITION(type)	\
CompiledCoroutine *type::ms_compiledCoro = nullptr;\
void type::compileCoroutine(ICoroutineCompiler *compiler) {

#define CORO_END_DEFINITION	\
	}

#define CORO_BEGIN_FUNCTION							\
	struct CoroStackFrame : public CoroutineStackFrame2 {\
		Params _params;\
		Locals _locals;\
		CoroutineReturnValueRef<ReturnValue_t> _rvRef;\
		explicit CoroStackFrame(const CompiledCoroutine *compiledCoro, const Params &params, const CoroutineReturnValueRef<ReturnValue_t> &rvRef)\
			: CoroutineStackFrame2(compiledCoro), _params(params), _rvRef(rvRef) {\
		}\
		static CoroutineStackFrame2 *constructFrame(void *ptr, const CompiledCoroutine *compiledCoro, const CoroutineParamsBase &params, const CoroutineReturnValueRefBase &returnValueRef) {\
			return new (ptr) CoroStackFrame(compiledCoro, static_cast<const Params &>(params), static_cast<const CoroutineReturnValueRef<ReturnValue_t>&>(returnValueRef));\
		}\
		static void getFrameParameters(size_t &outSize, size_t &outAlignment) { \
			outSize = sizeof(CoroStackFrame); \
			outAlignment = alignof(CoroStackFrame); \
		}\
	};\
	compiler->defineFunction(CoroStackFrame::constructFrame, CoroStackFrame::getFrameParameters);	\
	CORO_START_CODE_BLOCK(CoroOps::BeginFunction)

#define CORO_END_FUNCTION						\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::EndFunction)	\
	CORO_END_CODE_BLOCK

#define CORO_AWAIT(expr)							\
		(expr);										\
	CORO_END_CODE_BLOCK								\
	CORO_START_CODE_BLOCK(CoroOps::YieldToFunction)	\
	CORO_END_CODE_BLOCK								\
	CORO_START_CODE_BLOCK(CoroOps::Code)

#define CORO_AWAIT_MINISCRIPT(expr)					\
		coroRuntime._miniscriptOutcome = ((expr));	\
	CORO_END_CODE_BLOCK								\
	CORO_START_CODE_BLOCK(CoroOps::CheckMiniscript)	\
	CORO_END_CODE_BLOCK								\
	CORO_START_CODE_BLOCK(CoroOps::Code)	

#define CORO_IF(expr)							\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::IfCond)		\
		coroRuntime._condition = !!(expr);		\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::IfBody)

#define CORO_ELSE								\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::Else)

#define CORO_ELSE_IF(expr)						\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::ElseIfCond)	\
		coroRuntime._condition = !!(expr);		\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::ElseIfBody)

#define CORO_END_IF								\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::EndIf)

#define CORO_WHILE(expr)						\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::WhileCond)	\
		coroRuntime._condition = !!(expr);		\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::WhileBody)

#define CORO_END_WHILE							\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::EndWhile)

#define CORO_DO									\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::Do)

#define CORO_DO_WHILE(expr)						\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::DoWhileCond)	\
		coroCondition = !(expr);				\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::DoWhile)

#define CORO_FOR(initExpr, condExpr, nextExpr)	\
		(initExpr);								\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::ForNext)		\
		(nextExpr);								\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::ForCond)		\
		coroRuntime._condition = !!(condExpr);	\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::ForBody)

#define CORO_END_FOR							\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::EndFor)

#define CORO_RETURN_VALUE(expr)					\
	coroReturnValueRef.set((expr));				\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::Return)

#define CORO_ERROR								\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::Error)
	

#define CORO_RETURN								\
	coroReturnValueRef.voidSet();				\
	CORO_END_CODE_BLOCK							\
	CORO_START_CODE_BLOCK(CoroOps::Return)

#define CORO_RETURN_CALL(func, ...)				\
		coroFrame->pushFrame<CoroAutoFrame<func> >(coroReturnValueRef._returnValue, __VA_ARGS__);	\
	CORO_END_CODE_BLOCK																				\
	CORO_START_CODE_BLOCK(CoroOps::YieldToFunction)													\
	CORO_END_CODE_BLOCK																				\
	CORO_START_CODE_BLOCK(CoroOps::Return)

#define CORO_SET_CALL(dest, func, ...)												\
	CORO_AWAIT(coroRuntime._vthread->pushCoroutineWithReturn<func>(&(dest), __VA_ARGS__))

#define CORO_CALL(func, ...)								\
	CORO_AWAIT(coroRuntime._vthread->pushCoroutine<func>(__VA_ARGS__))

} // Namespace MTropolis

#endif
