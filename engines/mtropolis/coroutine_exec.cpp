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

#include "mtropolis/coroutine_exec.h"
#include "mtropolis/coroutines.h"

namespace MTropolis {

CoroutineRuntimeState::CoroutineRuntimeState(VThread *vthread, CoroutineStackFrame2 *frame)
: _vthread(vthread), _frame(frame), _condition(false) {
}

CompiledCoroutine::CompiledCoroutine()
	: _frameConstructor(nullptr), _getFrameParameters(nullptr), _isVoidReturn(false), _instructions(nullptr), _numInstructions(0) {
}

CompiledCoroutine::~CompiledCoroutine() {
	delete[] _instructions;
}

CoroutineStackFrame2::CoroutineStackFrame2(const CompiledCoroutine *compiledCoro)
	: _compiledCoro(compiledCoro), _nextInstr(0) {
}

CoroutineStackFrame2::~CoroutineStackFrame2() {
}

VThreadState CoroutineStackFrame2::execute(VThread *thread) {
	const CoroExecInstr *instrs = _compiledCoro->_instructions;

	uint ip = _nextInstr;

	CoroutineRuntimeState runtimeState(thread, this);

	for (;;) {
		assert(ip < _compiledCoro->_numInstructions);

		const CoroExecInstr &instr = instrs[ip++];

		switch (instr._opcode) {
		case CoroExecOp::Code:
			instr._func(runtimeState);
			break;
		case CoroExecOp::Jump:
			ip = instr._arg;
			break;
		case CoroExecOp::JumpIfFalse:
			if (!runtimeState._condition)
				ip = instr._arg;
			break;
		case CoroExecOp::EnterFunction:
			_nextInstr = ip;
			return kVThreadReturn;
		case CoroExecOp::ExitFunction:
			thread->popFrame();
			return kVThreadReturn;
		case CoroExecOp::Error:
			return kVThreadError;
		case CoroExecOp::CheckMiniscript:
			if (runtimeState._miniscriptOutcome == kMiniscriptInstructionOutcomeFailed)
				return kVThreadError;
			else if (runtimeState._miniscriptOutcome == kMiniscriptInstructionOutcomeContinue)
				break;
			else if (runtimeState._miniscriptOutcome == kMiniscriptInstructionOutcomeYieldToVThread) {
				_nextInstr = ip;
				return kVThreadReturn;
			} else {
				error("Unhandled miniscript result in coro runtime");
			}

			break;
		default:
			error("Internal error: Unhandled coro opcode");
		}
	}
}

const CompiledCoroutine *CoroutineStackFrame2::getCompiledCoroutine() const {
	return _compiledCoro;
}

}
