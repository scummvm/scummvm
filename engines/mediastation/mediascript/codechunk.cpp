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

#include "mediastation/mediastation.h"
#include "mediastation/mediascript/codechunk.h"
#include "mediastation/datum.h"
#include "mediastation/chunk.h"
#include "mediastation/debugchannels.h"

#include "mediastation/assets/movie.h"
#include "mediastation/assets/path.h"

namespace MediaStation {

CodeChunk::CodeChunk(Common::SeekableReadStream &chunk) : _args(nullptr) {
	uint lengthInBytes = Datum(chunk, kDatumTypeUint32_1).u.i;
	debugC(5, kDebugLoading, "CodeChunk::CodeChunk(): Length 0x%x (@0x%llx)", lengthInBytes, static_cast<long long int>(chunk.pos()));
	_bytecode = chunk.readStream(lengthInBytes);
}

Operand CodeChunk::execute(Common::Array<Operand> *args) {
	_args = args;
	Operand returnValue;
	while (_bytecode->pos() < _bytecode->size()) {
		returnValue = executeNextStatement();
	}

	// Rewind the stream once we're finished, in case we need to execute
	// this code again!
	_bytecode->seek(0);
	// We don't own the args, so we will prevent a potentially out-of-scope
	// variable from being re-accessed.
	_args = nullptr;
	return returnValue;
}

Operand CodeChunk::executeNextStatement() {
	if (_bytecode->eos()) {
		error("CodeChunk::executeNextStatement(): Attempt to read past end of bytecode chunk");
	}

	InstructionType instructionType = InstructionType(Datum(*_bytecode).u.i);
	debugCN(5, kDebugScript, "(%s) ", instructionTypeToStr(instructionType));
	switch (instructionType) {
	case kInstructionTypeEmpty: {
		return Operand();
	}

	case kInstructionTypeFunctionCall: {
		Opcode opcode = Opcode(Datum(*_bytecode).u.i);
		debugCN(5, kDebugScript, "%s ", opcodeToStr(opcode));
		switch (opcode) {
		case kOpcodeAssignVariable: {
			uint32 id = Datum(*_bytecode).u.i;
			VariableScope scope = VariableScope(Datum(*_bytecode).u.i);
			debugC(5, kDebugScript, "%d (%s) ", id, variableScopeToStr(scope));
			debugCN(5, kDebugScript, "  Value: ");
			Operand newValue = executeNextStatement();

			putVariable(id, scope, newValue);
			return Operand();
		}

		case kOpcodeCallRoutine: {
			uint functionId = Datum(*_bytecode).u.i;
			uint32 parameterCount = Datum(*_bytecode).u.i;
			Common::Array<Operand> args;
			debugC(5, kDebugScript, "%d (%d params)", functionId, parameterCount);
			for (uint i = 0; i < parameterCount; i++) {
				debugCN(5, kDebugScript, "  Param %d: ", i);
				Operand arg = executeNextStatement();
				args.push_back(arg);
			}

			Operand returnValue;
			Function *function = g_engine->getFunctionById(functionId);
			if (function != nullptr) {
				// This is a title-defined function.
				returnValue = function->execute(args);
			} else {
				BuiltInFunction builtInFunctionId = static_cast<BuiltInFunction>(functionId);
				debugC(5, kDebugScript, "  Function Name: %s ", builtInFunctionToStr(builtInFunctionId));
				returnValue = callBuiltInFunction(builtInFunctionId, args);
			}
			return returnValue;
		}

		case kOpcodeCallMethod: {
			// In Media Station, all methods seem be built-in - there don't
			// seem to be custom objects or methods individual titles can
			// define. Functions, however, CAN be title-defined. 
			// But here, we're only looking for built-in methods.
			BuiltInMethod methodId = static_cast<BuiltInMethod>(Datum(*_bytecode).u.i);
			uint32 parameterCount = Datum(*_bytecode).u.i;
			debugC(5, kDebugScript, "%s (%d params)", builtInMethodToStr(methodId), parameterCount);
			debugCN(5, kDebugScript, "  Self: ");
			Operand selfObject = executeNextStatement();
			Common::Array<Operand> args;
			for (uint i = 0; i < parameterCount; i++) {
				debugCN(5, kDebugScript, "  Param %d: ", i);
				Operand arg = executeNextStatement();
				args.push_back(arg);
			}
			Operand returnValue = callBuiltInMethod(methodId, selfObject, args);
			return returnValue;
		}

		case kOpcodeDeclareVariables: {
			uint32 localVariableCount = Datum(*_bytecode).u.i;
			debugC(5, kDebugScript, "%d", localVariableCount);
			_locals.resize(localVariableCount);
			return Operand();
		}

		case kOpcodeOr: {
			debugCN(5, kDebugScript, "\n    lhs: ");
			Operand value1 = executeNextStatement();
			debugCN(5, kDebugScript, "    rhs: ");
			Operand value2 = executeNextStatement();

			Operand returnValue(kOperandTypeLiteral1);
			bool logicalOr = (value1 || value2);
			returnValue.putInteger(static_cast<uint>(logicalOr));
			return returnValue;
		}

		case kOpcodeSubtract: {
			debugCN(5, kDebugScript, "\n    lhs: ");
			Operand value1 = executeNextStatement();
			debugCN(5, kDebugScript, "    rhs: ");
			Operand value2 = executeNextStatement();

			Operand returnValue = value1 - value2;
			return returnValue;
		}

		case kOpcodeNegate: {
			Operand value = executeNextStatement();
			debugCN(5, kDebugScript, "    value: ");

			return -value;
		}

		case kOpcodeIfElse: {
			debugCN(5, kDebugScript, "\n    condition: ");
			Operand condition = executeNextStatement();

			CodeChunk ifBlock(*_bytecode);
			CodeChunk elseBlock(*_bytecode);
			// Doesn't seem like there is a real bool type for values,
			// ao just get an integer.
			if (condition.getInteger()) {
				// TODO: If locals are modified in here, they won't be
				// propagated up since it's its own code chunk.
				ifBlock.execute();
			} else {
				elseBlock.execute();
			}

			// If blocks themselves shouldn't return anything.
			return Operand();
		}

		case kOpcodeEquals: {
			debugCN(5, kDebugScript, "\n    lhs: ");
			Operand value1 = executeNextStatement();
			debugCN(5, kDebugScript, "    rhs: ");
			Operand value2 = executeNextStatement();

			// TODO: Confirm this is the correct value type?
			Operand returnValue(kOperandTypeLiteral1);
			bool equal = (value1 == value2);
			returnValue.putInteger(static_cast<uint>(equal));
			return returnValue;
		}

		case kOpcodeGreaterThanOrEqualTo: {
			debugCN(5, kDebugScript, "\n    lhs: ");
			Operand value1 = executeNextStatement();
			debugCN(5, kDebugScript, "    rhs: ");
			Operand value2 = executeNextStatement();

			// TODO: Confirm this is the correct value type?
			Operand returnValue(kOperandTypeLiteral1);
			bool greaterThanOrEqualTo = value1 >= value2;
			returnValue.putInteger(static_cast<uint>(greaterThanOrEqualTo));
			return returnValue;
		}

		default: {
			error("CodeChunk::getNextStatement(): Got unknown opcode %s (%d)", opcodeToStr(opcode), static_cast<uint>(opcode));
		}
		}
		break;
	}

	case kInstructionTypeOperand: {
		OperandType operandType = static_cast<OperandType>(Datum(*_bytecode).u.i);
		debugCN(5, kDebugScript, "%s ", operandTypeToStr(operandType));
		Operand operand(operandType);
		switch (operandType) {
		case kOperandTypeAssetId: {
			uint32 assetId = Datum(*_bytecode).u.i;
			debugC(5, kDebugScript, "%d ", assetId);
			operand.putAsset(assetId);
			return operand;
		}

		case kOperandTypeLiteral1:
		case kOperandTypeLiteral2:
		case kOperandTypeDollarSignVariable: {
			int literal = Datum(*_bytecode).u.i;
			debugC(5, kDebugScript, "%d ", literal);
			operand.putInteger(literal);
			return operand;
		}

		case kOperandTypeFloat1:
		case kOperandTypeFloat2: {
			double d = Datum(*_bytecode).u.f;
			debugC(5, kDebugScript, "%f ", d);
			operand.putDouble(d);
			return operand;
		}

		case kOperandTypeFunction: {
			uint functionId = Datum(*_bytecode).u.i;
			debugC(5, kDebugScript, "%d ", functionId);
			operand.putFunction(functionId);
			return operand;
		}

		case kOperandTypeString: {
			// This is indeed a raw string anot not a string wrapped in a datum!
			// TODO: This copies the string. Can we read it directly from the chunk?
			int size = Datum(*_bytecode, kDatumTypeUint16_1).u.i;
			char *buffer = new char[size + 1];
			_bytecode->read(buffer, size);
			buffer[size] = '\0';
			Common::String *string = new Common::String(buffer);
			debugC(5, kDebugScript, "%s ", string->c_str());
			operand.putString(string);
			delete[] buffer;
			return operand;
		}

		default: {
			error("CodeChunk::getNextStatement(): Got unknown operand type %s (%d)", operandTypeToStr(operandType), static_cast<uint>(operandType));
		}
		}
		break;
	}

	case kInstructionTypeVariableRef: {
		uint32 id = Datum(*_bytecode).u.i;
		VariableScope scope = VariableScope(Datum(*_bytecode).u.i);
		debugC(5, kDebugScript, "Variable %d (%s)", id, variableScopeToStr(scope));
		Operand variable = getVariable(id, scope);
		return variable;
	}

	default: {
		error("CodeChunk::getNextStatement(): Got unknown instruction type %s (%d)", instructionTypeToStr(instructionType), static_cast<uint>(instructionType));
	}
	}
}

Operand CodeChunk::getVariable(uint32 id, VariableScope scope) {
	switch (scope) {
	case kVariableScopeGlobal: {
		Operand returnValue(kOperandTypeVariableDeclaration);
		Variable *variable = g_engine->_variables.getVal(id);
		returnValue.putVariable(variable);
		return returnValue;
	}

	case kVariableScopeLocal: {
		uint index = id - 1;
		return _locals.operator[](index);
		break;
	}

	case kVariableScopeParameter: {
		uint32 index = id - 1;
		if (_args == nullptr) {
			error("CodeChunk::getVariable(): Requested a parameter in a code chunk that has no parameters.");
		}
		return _args->operator[](index);
		break;
	}

	default: {
		error("CodeChunk::getVariable(): Got unknown variable scope %s (%d)", variableScopeToStr(scope), static_cast<uint>(scope));
	}
	}
}

void CodeChunk::putVariable(uint32 id, VariableScope scope, Operand value) {
	switch (scope) {
	case kVariableScopeGlobal: {
		Variable *variable = g_engine->_variables.getVal(id);
		if (variable == nullptr) {
			error("CodeChunk::putVariable(): Attempted to assign to a non-existent global variable %d", id);
		}
		variable->putValue(value);
		break;
	}

	case kVariableScopeLocal: {
		uint index = id - 1;
		_locals[index] = value;
		break;
	}

	case kVariableScopeParameter: {
		error("CodeChunk::putVariable(): Attempted to assign to a parameter");
		break;
	}

	default: {
		error("CodeChunk::getVariable(): Got unknown variable scope %s (%d)", variableScopeToStr(scope), static_cast<uint>(scope));
	}
	}
}

Operand CodeChunk::callBuiltInFunction(BuiltInFunction id, Common::Array<Operand> &args) {
	switch (id) {
	case kEffectTransitionFunction: {
		switch (args.size()) {
		// TODO: Discover and handle the different ways
		// effectTransition can be called.
		case 1: {
			//uint dollarSignVariable = args[0].getInteger();
			break;
		}

		case 3: {
			//uint dollarSignVariable = args[0].getInteger();
			//double percentComplete = args[1].getDouble();

			// TODO: Verify that this is a palette!
			Asset *asset = args[2].getAsset();
			g_engine->setPalette(asset);
			break;
		}

		default: {
			error("CodeChunk::callBuiltInFunction(): (BuiltInFunction::effectTransition) Got %d args, which is unexpected", args.size());
		}
		}

		warning("CodeChunk::callBuiltInFunction(): effectTransition is not implemented");
		return Operand();
		break;
	}

	case kDrawingFunction: {
		// Not entirely sure what this function does, but it seems like a way to
		// call into some drawing functions built into the IBM/Crayola executable.
		warning("CodeChunk::callBuiltInFunction(): Built-in drawing function not implemented");
		return Operand();
		break;
	}

	default: {
		error("CodeChunk::callBuiltInFunction(): Got unknown built-in function ID %d", id);
	}
	}
}

Operand CodeChunk::callBuiltInMethod(BuiltInMethod method, Operand self, Common::Array<Operand> &args) {
	Operand literalSelf = self.getLiteralValue();
	OperandType literalType = literalSelf.getType();
	switch (literalType) {
	case kOperandTypeAssetId: {
		if (self.getAssetId() == 1) {
			// This is a "document" method that we need to handle specially.
			// The document (@doc) accepts engine-level methods like changing the
			// active screen.
			// HACK: This is so we don't have to implement a separate document class
			// just to house these methods. Rather, we just call in the engine.
			Operand returnValue = g_engine->callMethod(method, args);
			return returnValue;
		} else if (self.getAssetId() == 0) {
			// It seems to be valid to call a method on a null asset ID, in
			// which case nothing happens. Still issue warning for traceability.
			warning("CodeChunk::callBuiltInMethod(): Attempt to call method on a null asset ID");
			return Operand();
			break;
		} else {
			// This is a regular asset that we can process directly.
			Asset *selfAsset = self.getAsset();
			if (selfAsset == nullptr) {
				error("CodeChunk::callBuiltInMethod(): Attempt to call method on asset ID %d, which isn't loaded", self.getAssetId());
			}
			Operand returnValue = selfAsset->callMethod(method, args);
			return returnValue;
		}
	}

	default:
		error("CodeChunk::callBuiltInMethod(): Attempt to call method on unsupported operand type %s (%d)", 
			operandTypeToStr(literalType), static_cast<uint>(literalType));
	}
}

CodeChunk::~CodeChunk() {
	// We don't own the args, so we don't need to delete it.
	_args = nullptr;

	delete _bytecode;
	_bytecode = nullptr;
}

} // End of namespace MediaStation
