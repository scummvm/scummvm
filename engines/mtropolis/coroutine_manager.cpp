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

#include "mtropolis/coroutine_manager.h"
#include "mtropolis/coroutines.h"
#include "mtropolis/coroutine_exec.h"

namespace MTropolis {

class CoroutineManager : public ICoroutineManager {
public:
	CoroutineManager();
	~CoroutineManager();

private:
	void registerCoroutine(CompiledCoroutine **compiledCoroPtr) override;
	void compileCoroutine(CompiledCoroutine **compiledCoroPtr, CoroutineCompileFunction_t compileFunction, bool isVoidReturn) override;

	Common::Array<CompiledCoroutine **> _compiledCoroutineRefs;
};

class CoroutineCompiler : public ICoroutineCompiler {
public:
	explicit CoroutineCompiler(ICoroutineManager *coroManager);

	void addFunctionToCompile(CompiledCoroutine **compiledCoroPtr, CoroutineCompileFunction_t compileFunction, bool isVoidReturn);

	void compileAll();

	void defineFunction(CoroutineFrameConstructor_t frameConstructor, CoroutineGetFrameParametersFunction_t frameGetParams) override;
	void addOp(CoroOps op, CoroutineFragmentFunction_t fragmentFunc) override;

private:
	struct PendingCompile {
		CompiledCoroutine *_compiledCoro;
		CoroutineCompileFunction_t _compileFunction;
	};

	enum class ControlFlowType {
		Invalid = 0,

		Function,
		If,			// Label 1 = Else label, Label 2 = End label
		While,		// Label 1 = Loop label, Label 2 = End label
		DoWhile,	// Label 1 = Loop label, Label 2 = End label
		For,		// Label 1 = Loop label, Label 2 = End label, Label 3 = Iterate label
	};

	enum class ControlFlowState {
		Default = 0,

		NoBody,
		NoElse,
		HasElse,
	};

	struct ControlFlowStack {
		ControlFlowStack()
			: _type(ControlFlowType::Invalid), _state(ControlFlowState::Default), _endLabel(0), _loopOrElseLabel(0), _iterateLabel(0) {
		}

		ControlFlowType _type;
		ControlFlowState _state;
		uint _endLabel;
		uint _loopOrElseLabel;
		uint _iterateLabel;
	};

	enum class ProtoOp {
		Invalid = 0,

		Code,

		NoOp,

		Jump,
		JumpIfFalse,
		Label,

		YieldToFunction,
		CheckMiniscript,

		Return,
		Error,

		InfiniteLoop,
	};

	struct ProtoInstr {
		ProtoOp _op;
		uint _value;
		CoroutineFragmentFunction_t _func;
	};

	void compileOne(CompiledCoroutine *compiledCoro, CoroutineCompileFunction_t compileFunction);
	void reportError(const char *str);

	void addProtoInstr(ProtoOp op, CoroutineFragmentFunction_t func);
	void addProtoInstr(ProtoOp op, uint value, CoroutineFragmentFunction_t func);
	void addProtoInstr(ProtoOp op, uint value);
	void addProtoInstr(ProtoOp op);

	static bool isSimpleTerminalOp(ProtoOp op);

	uint allocLabel();

	ICoroutineManager *_coroManager;
	Common::Array<PendingCompile> _pendingCompiles;

	Common::Array<ControlFlowStack> _funcControlFlowStack;
	Common::Array<ProtoInstr> _funcProtoInstrs;
	uint _funcNumLabels;
	bool _funcIsVoidReturn;

	CoroutineFrameConstructor_t _funcFrameCtor;
	CoroutineGetFrameParametersFunction_t _funcFrameGetParams;
};

CoroutineManager::CoroutineManager() {
}

CoroutineManager::~CoroutineManager() {
	for (CompiledCoroutine **compiledCoroRef : _compiledCoroutineRefs) {
		delete (*compiledCoroRef);
		*compiledCoroRef = nullptr;
	}
}


void CoroutineManager::registerCoroutine(CompiledCoroutine **compiledCoroPtr) {
	_compiledCoroutineRefs.push_back(compiledCoroPtr);
}

void CoroutineManager::compileCoroutine(CompiledCoroutine **compiledCoroPtr, CoroutineCompileFunction_t compileFunction, bool isVoidReturn) {
	CoroutineCompiler coroCompiler(this);

	coroCompiler.addFunctionToCompile(compiledCoroPtr, compileFunction, isVoidReturn);
	coroCompiler.compileAll();
}

CoroutineCompiler::CoroutineCompiler(ICoroutineManager *coroManager)
	: _coroManager(coroManager), _funcFrameCtor(nullptr), _funcFrameGetParams(nullptr), _funcNumLabels(0), _funcIsVoidReturn(false) {
}

void CoroutineCompiler::addFunctionToCompile(CompiledCoroutine **compiledCoroPtr, CoroutineCompileFunction_t compileFunction, bool isVoidReturn) {
	if (*compiledCoroPtr)
		return;

	CompiledCoroutine *compiledCoro = new CompiledCoroutine();

	_coroManager->registerCoroutine(compiledCoroPtr);

	compiledCoro->_isVoidReturn = isVoidReturn;

	PendingCompile pendingCompile;
	pendingCompile._compiledCoro = compiledCoro;
	pendingCompile._compileFunction = compileFunction;

	*compiledCoroPtr = pendingCompile._compiledCoro;

	_pendingCompiles.push_back(pendingCompile);
}

void CoroutineCompiler::compileAll() {
	// pendingCompiles may grow during this
	for (uint i = 0; i < _pendingCompiles.size(); i++) {
		const PendingCompile &pendingCompile = _pendingCompiles[i];
		compileOne(pendingCompile._compiledCoro, pendingCompile._compileFunction);
	}
}

void CoroutineCompiler::defineFunction(CoroutineFrameConstructor_t frameConstructor, CoroutineGetFrameParametersFunction_t frameGetParams) {
	_funcFrameCtor = frameConstructor;
	_funcFrameGetParams = frameGetParams;
}

void CoroutineCompiler::addOp(CoroOps op, CoroutineFragmentFunction_t fragmentFunc) {
	if (op == CoroOps::BeginFunction && _funcProtoInstrs.size() != 0)
		reportError("Begin function came after the start of the function");

	if (op != CoroOps::BeginFunction && _funcProtoInstrs.size() == 0)
		reportError("First op wasn't begin function");

	if (op != CoroOps::BeginFunction && _funcControlFlowStack.size() == 0)
		reportError("Op after end of function");

	switch (op) {
	case CoroOps::BeginFunction: {
			ControlFlowStack cf;
			cf._type = ControlFlowType::Function;

			_funcControlFlowStack.push_back(cf);

			addProtoInstr(ProtoOp::Code, fragmentFunc);
		} break;

	case CoroOps::EndFunction: {
			if (_funcControlFlowStack.size() != 1)
				reportError("End function doesn't close function scope");

			_funcControlFlowStack.pop_back();

			if (_funcIsVoidReturn) {
				addProtoInstr(ProtoOp::Return);
			} else {
				if (_funcProtoInstrs.back()._op != ProtoOp::Return)
					reportError("Value-returning function didn't return a value");
			}
		} break;

	case CoroOps::IfCond: {
			ControlFlowStack cf;
			cf._type = ControlFlowType::If;
			cf._state = ControlFlowState::NoBody;
			cf._endLabel = allocLabel();
			cf._loopOrElseLabel = allocLabel();

			_funcControlFlowStack.push_back(cf);

			addProtoInstr(ProtoOp::Code, fragmentFunc);
		} break;

	case CoroOps::IfBody: {
			ControlFlowStack &cf = _funcControlFlowStack.back();
			if (cf._type != ControlFlowType::If || cf._state != ControlFlowState::NoBody)
				reportError("If body in wrong location");

			cf._state = ControlFlowState::NoElse;

			addProtoInstr(ProtoOp::JumpIfFalse, cf._loopOrElseLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);
		} break;
	case CoroOps::Else: {
			ControlFlowStack &cf = _funcControlFlowStack.back();
			if (cf._type != ControlFlowType::If)
				reportError("Unexpected 'else'");

			if (cf._state != ControlFlowState::NoElse)
				reportError("If block has an 'else' already");

			cf._state = ControlFlowState::HasElse;

			addProtoInstr(ProtoOp::Jump, cf._endLabel);
			addProtoInstr(ProtoOp::Label, cf._loopOrElseLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);
		} break;
	case CoroOps::ElseIfCond: {
			ControlFlowStack &cf = _funcControlFlowStack.back();
			if (cf._type != ControlFlowType::If)
				reportError("Unexpected 'else if'");

			if (cf._state != ControlFlowState::NoElse)
				reportError("If block has an 'else' already");

			addProtoInstr(ProtoOp::Jump, cf._endLabel);
			addProtoInstr(ProtoOp::Label, cf._loopOrElseLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);

			cf._loopOrElseLabel = allocLabel();
		} break;
	case CoroOps::ElseIfBody: {
			ControlFlowStack &cf = _funcControlFlowStack.back();
			if (cf._type != ControlFlowType::If)
				reportError("Else if body in the wrong place");

			addProtoInstr(ProtoOp::JumpIfFalse, cf._loopOrElseLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);
		} break;
	case CoroOps::EndIf: {
			ControlFlowStack &cf = _funcControlFlowStack.back();
			if (cf._type != ControlFlowType::If)
				reportError("Else if body in the wrong place");

			if (cf._state != ControlFlowState::HasElse)
				addProtoInstr(ProtoOp::Label, cf._loopOrElseLabel);

			addProtoInstr(ProtoOp::Label, cf._endLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);

			_funcControlFlowStack.pop_back();
		} break;

	case CoroOps::WhileCond: {
			ControlFlowStack cf;
			cf._type = ControlFlowType::While;
			cf._loopOrElseLabel = allocLabel();
			cf._endLabel = allocLabel();

			_funcControlFlowStack.push_back(cf);

			addProtoInstr(ProtoOp::Label, cf._loopOrElseLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);
		} break;
	case CoroOps::WhileBody: {
			ControlFlowStack &cf = _funcControlFlowStack.back();
			if (cf._type != ControlFlowType::While)
				reportError("While body in the wrong place");

			addProtoInstr(ProtoOp::JumpIfFalse, cf._endLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);
		} break;
	case CoroOps::EndWhile: {
			ControlFlowStack &cf = _funcControlFlowStack.back();
			if (cf._type != ControlFlowType::While)
				reportError("'end while' didn't close while block");

			addProtoInstr(ProtoOp::Jump, cf._loopOrElseLabel);
			addProtoInstr(ProtoOp::Label, cf._endLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);

			_funcControlFlowStack.pop_back();
		} break;

		// Order of for loops is Next->Cond->Body
	case CoroOps::ForNext: {
			ControlFlowStack cf;
			cf._type = ControlFlowType::For;
			cf._iterateLabel = allocLabel();
			cf._loopOrElseLabel = allocLabel();
			cf._endLabel = allocLabel();

			_funcControlFlowStack.push_back(cf);

			addProtoInstr(ProtoOp::Jump, cf._loopOrElseLabel);
			addProtoInstr(ProtoOp::Label, cf._iterateLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);
		} break;

	case CoroOps::ForCond: {
			ControlFlowStack &cf = _funcControlFlowStack.back();
			if (cf._type != ControlFlowType::For)
				reportError("'for' condition in the wrong place");

			addProtoInstr(ProtoOp::Label, cf._loopOrElseLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);
		} break;

	case CoroOps::ForBody: {
			ControlFlowStack &cf = _funcControlFlowStack.back();
			if (cf._type != ControlFlowType::For)
				reportError("'for' body in the wrong place");

			addProtoInstr(ProtoOp::JumpIfFalse, cf._endLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);
		} break;

	case CoroOps::EndFor: {
			ControlFlowStack &cf = _funcControlFlowStack.back();
			if (cf._type != ControlFlowType::For)
				reportError("'end for' didn't close a for loop");

			addProtoInstr(ProtoOp::Jump, cf._iterateLabel);
			addProtoInstr(ProtoOp::Label, cf._endLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);

			_funcControlFlowStack.pop_back();
		} break;

	case CoroOps::Do: {
			ControlFlowStack cf;
			cf._type = ControlFlowType::DoWhile;
			cf._loopOrElseLabel = allocLabel();
			cf._endLabel = allocLabel();

			_funcControlFlowStack.push_back(cf);

			addProtoInstr(ProtoOp::Label, cf._loopOrElseLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);
		} break;

	case CoroOps::DoWhileCond: {
			ControlFlowStack &cf = _funcControlFlowStack.back();
			if (cf._type != ControlFlowType::DoWhile)
				reportError("'do/while' condition didn't close a 'do' block");

			addProtoInstr(ProtoOp::Code, fragmentFunc);
		} break;

	case CoroOps::DoWhile: {
			ControlFlowStack &cf = _funcControlFlowStack.back();
			if (cf._type != ControlFlowType::DoWhile)
				reportError("'do while' in the wrong place");

			addProtoInstr(ProtoOp::JumpIfFalse, fragmentFunc);
			addProtoInstr(ProtoOp::Label, cf._endLabel);
			addProtoInstr(ProtoOp::Code, fragmentFunc);

			_funcControlFlowStack.pop_back();
		} break;

	case CoroOps::Return:
		addProtoInstr(ProtoOp::Return);
		break;

	case CoroOps::Error:
		addProtoInstr(ProtoOp::Error);
		break;

	case CoroOps::Code:
		addProtoInstr(ProtoOp::Code, fragmentFunc);
		break;

	case CoroOps::YieldToFunction:
		addProtoInstr(ProtoOp::YieldToFunction);
		break;

	case CoroOps::CheckMiniscript:
		addProtoInstr(ProtoOp::CheckMiniscript);
		break;

	default:
		reportError("Unimplemented coro opcode");
	}
}


void CoroutineCompiler::compileOne(CompiledCoroutine *compiledCoro, CoroutineCompileFunction_t compileFunction) {
	_funcNumLabels = 0;
	_funcProtoInstrs.clear();

	_funcIsVoidReturn = compiledCoro->_isVoidReturn;

	compileFunction(this);

#if defined(_M_X64) || defined(__x86_64__)
	for (uint i = 0; i < _funcProtoInstrs.size(); i++) {
		ProtoInstr &instr = _funcProtoInstrs[i];
		if (instr._op == ProtoOp::Code) {
			// Empty cdecl function:
			// 33 c0 xor eax,eax
			// c3    ret
			const byte emptyFunctionSignature[] = {0x33u, 0xc0u, 0xc3u};

			if (!memcmp(reinterpret_cast<const void *>(instr._func), emptyFunctionSignature, sizeof(emptyFunctionSignature)))
				instr._op = ProtoOp::NoOp;
		}
	}
#endif

	// Renumber label to instructions
	{
		Common::Array<uint> labelToInstr;
		labelToInstr.resize(_funcNumLabels, (uint)-1);

		for (uint i = 0; i < _funcProtoInstrs.size(); i++) {
			ProtoInstr &instr = _funcProtoInstrs[i];

			if (instr._op == ProtoOp::Label) {
				labelToInstr[instr._value] = i;
				instr._op = ProtoOp::NoOp;
			}
		}

		for (uint i = 0; i < _funcProtoInstrs.size(); i++) {
			ProtoInstr &instr = _funcProtoInstrs[i];

			if (instr._op == ProtoOp::JumpIfFalse || instr._op == ProtoOp::Jump) {
				assert(labelToInstr[instr._value] != (uint)-1);
				instr._value = labelToInstr[instr._value];
			}
		}
	}

	bool haveWork = true;
	while (haveWork) {
		haveWork = false;

		// Locate infinite loops and thread jumps
		{
			Common::Array<uint> chainEndInstr;
			Common::Array<uint> instrJumpRoot;
			Common::Array<uint> rootToChain;

			uint initialNumInstrs = _funcProtoInstrs.size();

			chainEndInstr.push_back(0);
			rootToChain.push_back(0);

			instrJumpRoot.resize(initialNumInstrs, 0);

			for (uint i = 0; i < initialNumInstrs; i++) {
				const ProtoInstr &baseInstr = _funcProtoInstrs[i];

				if (baseInstr._op == ProtoOp::Jump && instrJumpRoot[i] == 0) {
					uint jumpRootID = rootToChain.size();
					uint chainID = chainEndInstr.size();

					chainEndInstr.push_back(0);
					rootToChain.push_back(chainID);

					uint traceInstr = i;
					for (;;) {
						const ProtoInstr &instr = _funcProtoInstrs[traceInstr];

						// Ended as a new chain
						if (instr._op != ProtoOp::Jump) {
							chainEndInstr[chainID] = traceInstr;
							break;
						}

						if (instr._op == ProtoOp::InfiniteLoop || instrJumpRoot[traceInstr] == jumpRootID) {
							// Ended in an infinite loop.
							chainEndInstr[chainID] = (uint)-1;
							break;
						} else if (instrJumpRoot[traceInstr] == 0) {
							// Propgate jump chain
							instrJumpRoot[traceInstr] = jumpRootID;
							traceInstr = instr._value;
						} else {
							// Converge into existing chain
							rootToChain[jumpRootID] = rootToChain[instrJumpRoot[traceInstr]];
							break;
						}
					}
				}
			}

			for (uint i = 0; i < initialNumInstrs; i++) {
				ProtoInstr &instr = _funcProtoInstrs[i];

				if (instr._op == ProtoOp::Jump) {
					uint endInstr = chainEndInstr[rootToChain[instrJumpRoot[i]]];
					if (endInstr == (uint)-1)
						instr._op = ProtoOp::InfiniteLoop;
					else
						instr._value = endInstr;
				}
			}
		}

		for (uint i = 0; i < _funcProtoInstrs.size(); i++) {
			ProtoInstr &instr = _funcProtoInstrs[i];

			if (instr._op == ProtoOp::JumpIfFalse || instr._op == ProtoOp::Jump) {
				// Remove jumps that jump to the next instruction
				if (instr._value == i + 1) {
					instr._op = ProtoOp::NoOp;
					continue;
				}

				const ProtoInstr &targetInstr = _funcProtoInstrs[instr._value];

				if (instr._op == ProtoOp::JumpIfFalse) {
					// Thread conditional jumps to jumps
					if (targetInstr._op == ProtoOp::Jump) {
						instr._value = targetInstr._value;
						haveWork = true;
					}

					// Remove conditional jumps that jump to the same target as the next instruction
					const ProtoInstr &nextInstr = _funcProtoInstrs[i + 1];
					if (nextInstr._op == ProtoOp::Jump && instr._value == nextInstr._value) {
						instr._op = ProtoOp::NoOp;
						continue;
					}

					// Remove conditional jumps to simple terminal ops with the terminal op
					if (isSimpleTerminalOp(nextInstr._op) && targetInstr._op == nextInstr._op) {
						instr._op = ProtoOp::NoOp;
						continue;
					}
				} else if (instr._op == ProtoOp::Jump) {
					// Replace jumps to simple terminal ops
					if (isSimpleTerminalOp(targetInstr._op)) {
						instr._op = targetInstr._op;
						haveWork = true;
					}
				}
			}
		}

		// Remove dead instructions
		{
			Common::Array<bool> instrIsAlive;
			Common::Array<uint> pendingExecRoots;
			pendingExecRoots.push_back(0);

			instrIsAlive.resize(_funcProtoInstrs.size(), false);
			instrIsAlive[0] = true;

			while (pendingExecRoots.size() > 0) {
				uint fillLocation = pendingExecRoots.back();
				pendingExecRoots.pop_back();

				for (;;) {
					const ProtoInstr &instr = _funcProtoInstrs[fillLocation];

					if (instr._op == ProtoOp::Jump || instr._op == ProtoOp::JumpIfFalse) {
						if (!instrIsAlive[instr._value]) {
							pendingExecRoots.push_back(instr._value);
							instrIsAlive[instr._value] = true;
						}
					}

					instrIsAlive[fillLocation] = true;

					if (instr._op == ProtoOp::Jump || instr._op == ProtoOp::Return || instr._op == ProtoOp::InfiniteLoop)
						break;

					fillLocation++;
				}
			}

			for (uint i = 0; i < _funcProtoInstrs.size(); i++) {
				if (_funcProtoInstrs[i]._op == ProtoOp::NoOp)
					instrIsAlive[i] = false;
			}

			uint numDeadInstructions = 0;
			for (uint i = 0; i < _funcProtoInstrs.size(); i++) {
				if (!instrIsAlive[i])
					numDeadInstructions++;
			}

			if (numDeadInstructions > 0) {
				haveWork = true;

				Common::Array<ProtoInstr> newInstrs;

				newInstrs.resize(_funcProtoInstrs.size() - numDeadInstructions);
				uint newInstrWritePos = 0;

				for (uint i = 0; i < _funcProtoInstrs.size(); i++) {
					if (instrIsAlive[i])
						newInstrs[newInstrWritePos++] = _funcProtoInstrs[i];
				}

				assert(newInstrWritePos == newInstrs.size());

				Common::Array<uint> oldInstrToNewInstr;
				oldInstrToNewInstr.resize(_funcProtoInstrs.size());

				uint newInstrIndex = newInstrs.size();
				uint oldInstrIndex = _funcProtoInstrs.size();

				for (;;) {
					oldInstrIndex--;

					if (instrIsAlive[oldInstrIndex]) {
						assert(newInstrIndex > 0);
						newInstrIndex--;
					}

					oldInstrToNewInstr[oldInstrIndex] = newInstrIndex;

					if (oldInstrIndex == 0)
						break;
				}

				for (ProtoInstr &instr : newInstrs) {
					if (instr._op == ProtoOp::Jump || instr._op == ProtoOp::JumpIfFalse)
						instr._value = oldInstrToNewInstr[instr._value];
				}

				_funcProtoInstrs = Common::move(newInstrs);
			}
		}
	}

	compiledCoro->_frameConstructor = _funcFrameCtor;
	compiledCoro->_getFrameParameters = _funcFrameGetParams;
	compiledCoro->_numInstructions = _funcProtoInstrs.size();
	compiledCoro->_instructions = new CoroExecInstr[compiledCoro->_numInstructions];

	CoroExecInstr *outInstrs = compiledCoro->_instructions;

	for (uint i = 0; i < compiledCoro->_numInstructions; i++) {
		const ProtoInstr &instr = _funcProtoInstrs[i];
		CoroExecInstr *outInstr = outInstrs + i;

		switch (instr._op) {
		case ProtoOp::Code:
			outInstr->_opcode = CoroExecOp::Code;
			outInstr->_func = instr._func;
			break;
		case ProtoOp::Jump:
			outInstr->_opcode = CoroExecOp::Jump;
			outInstr->_arg = instr._value;
			break;
		case ProtoOp::JumpIfFalse:
			outInstr->_opcode = CoroExecOp::JumpIfFalse;
			outInstr->_arg = instr._value;
			break;
		case ProtoOp::Return:
			outInstr->_opcode = CoroExecOp::ExitFunction;
			outInstr->_arg = 0;
			break;
		case ProtoOp::YieldToFunction:
			outInstr->_opcode = CoroExecOp::EnterFunction;
			outInstr->_arg = 0;
			break;
		case ProtoOp::Error:
			outInstr->_opcode = CoroExecOp::Error;
			outInstr->_arg = 0;
			break;
		case ProtoOp::CheckMiniscript:
			outInstr->_opcode = CoroExecOp::CheckMiniscript;
			outInstr->_arg = 0;
			break;
		default:
			error("Internal error: Unhandled coro op");
		}
	}
}

void CoroutineCompiler::reportError(const char *str) {
	error("%s", str);
}

void CoroutineCompiler::addProtoInstr(ProtoOp op, CoroutineFragmentFunction_t func) {
	addProtoInstr(op, 0, func);
}

void CoroutineCompiler::addProtoInstr(ProtoOp op, uint value) {
	addProtoInstr(op, value, nullptr);
}

void CoroutineCompiler::addProtoInstr(ProtoOp op) {
	addProtoInstr(op, 0, nullptr);
}

void CoroutineCompiler::addProtoInstr(ProtoOp op, uint value, CoroutineFragmentFunction_t func) {
	ProtoInstr instr;
	instr._func = func;
	instr._op = op;
	instr._value = value;

	_funcProtoInstrs.push_back(instr);
}

bool CoroutineCompiler::isSimpleTerminalOp(ProtoOp op) {
	return op == ProtoOp::InfiniteLoop || op == ProtoOp::Error || op == ProtoOp::Return;
}

uint CoroutineCompiler::allocLabel() {
	return _funcNumLabels++;
}

ICoroutineManager::~ICoroutineManager() {
}

ICoroutineManager *ICoroutineManager::create() {
	return new CoroutineManager();
}

ICoroutineCompiler::~ICoroutineCompiler() {
}

} // End of namespace MTropolis
