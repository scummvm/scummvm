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

#include "common/ptr.h"

#include "mediastation/mediastation.h"
#include "mediastation/mediascript/codechunk.h"
#include "mediastation/datum.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

CodeChunk::CodeChunk(Common::SeekableReadStream &chunk) {
	uint lengthInBytes = Datum(chunk, kDatumTypeUint32_1).u.i;
	debugC(5, kDebugLoading, "CodeChunk::CodeChunk(): Length 0x%x (@0x%llx)", lengthInBytes, static_cast<long long int>(chunk.pos()));
	_bytecode = chunk.readStream(lengthInBytes);
}

ScriptValue CodeChunk::execute(Common::Array<ScriptValue> *args, Common::Array<ScriptValue> *locals) {
	_locals = locals;
	_args = args;
	ScriptValue returnValue;
	while (_bytecode->pos() < _bytecode->size()) {
		ScriptValue instructionResult = evaluateExpression();
		if (instructionResult.getType() != kOperandTypeEmpty) {
			returnValue = instructionResult;
		}
	}

	// Rewind the stream once we're finished, in case we need to execute
	// this code again!
	_bytecode->seek(0);
	// We don't own the args, so we will prevent a potentially out-of-scope
	// variable from being re-accessed.
	_args = nullptr;

	if (_weOwnLocals) {
		delete _locals;
	}
	_locals = nullptr;

	return returnValue;
}

ScriptValue CodeChunk::evaluateExpression() {
	if (_bytecode->eos()) {
		error("CodeChunk::evaluateExpression(): Attempt to read past end of bytecode chunk");
	}

	ExpressionType instructionType = static_cast<ExpressionType>(Datum(*_bytecode).u.i);
	debugCN(5, kDebugScript, "(%s) ", expressionTypeToStr(instructionType));

	ScriptValue returnValue;
	switch (instructionType) {
	case kExpressionTypeEmpty: {
		return ScriptValue();
	}

	case kExpressionTypeOperation:
		returnValue = evaluateOperation();
		break;

	case kExpressionTypeValue:
		returnValue = evaluateValue();
		break;

	case kExpressionTypeVariable:
		returnValue = evaluateVariable();
		break;

	default:
		error("CodeChunk::getNextStatement(): Got unimplemented instruction type %s (%d)", expressionTypeToStr(instructionType), static_cast<uint>(instructionType));
	}

	return returnValue;
}

ScriptValue CodeChunk::evaluateOperation() {
	Opcode opcode = static_cast<Opcode>(Datum(*_bytecode).u.i);
	debugCN(5, kDebugScript, "%s ", opcodeToStr(opcode));
	switch (opcode) {
	case kOpcodeAssignVariable: {
		uint32 id = Datum(*_bytecode).u.i;
		VariableScope scope = static_cast<VariableScope>(Datum(*_bytecode).u.i);
		debugC(5, kDebugScript, "%d (%s) ", id, variableScopeToStr(scope));
		debugCN(5, kDebugScript, "  Value: ");
		ScriptValue newValue = evaluateExpression();

		putVariable(id, scope, newValue);
		return ScriptValue();
	}

	case kOpcodeCallFunction: {
		uint functionId = Datum(*_bytecode).u.i;
		uint32 parameterCount = Datum(*_bytecode).u.i;
		debugC(5, kDebugScript, "%d (%d params)", functionId, parameterCount);
		return callFunction(functionId, parameterCount);
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
		ScriptValue selfObject = evaluateExpression();
		Common::Array<ScriptValue> args;
		for (uint i = 0; i < parameterCount; i++) {
			debugCN(5, kDebugScript, "  Param %d: ", i);
			ScriptValue arg = evaluateExpression();
			args.push_back(arg);
		}
		ScriptValue returnValue = callBuiltInMethod(methodId, selfObject, args);
		return returnValue;
	}

	case kOpcodeDeclareVariables: {
		uint32 localVariableCount = Datum(*_bytecode).u.i;
		debugC(5, kDebugScript, "%d", localVariableCount);
		assert(_locals == nullptr);
		_locals = new Common::Array<ScriptValue>(localVariableCount);
		_weOwnLocals = true;
		return ScriptValue();
	}

	case kOpcodeOr: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		ScriptValue returnValue(kOperandTypeBool);
		bool logicalOr = (value1 || value2);
		returnValue.setToParamToken(static_cast<uint>(logicalOr));
		return returnValue;
	}

	case kOpcodeNot: {
		debugCN(5, kDebugScript, "\n    value: ");
		ScriptValue value = evaluateExpression();

		ScriptValue returnValue(kOperandTypeBool);
		bool logicalNot = !(static_cast<bool>(value.asParamToken()));
		returnValue.setToParamToken(static_cast<uint>(logicalNot));
		return returnValue;
	}

	case kOpcodeAnd: {
		debugCN(5, kDebugScript, "\n    value: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		ScriptValue returnValue(kOperandTypeBool);
		bool logicalAnd = (value1 && value2);
		returnValue.setToParamToken(static_cast<uint>(logicalAnd));
		return returnValue;
	}

	case kOpcodeIfElse: {
		debugCN(5, kDebugScript, "\n    condition: ");
		ScriptValue condition = evaluateExpression();

		CodeChunk ifBlock(*_bytecode);
		CodeChunk elseBlock(*_bytecode);
		// Doesn't seem like there is a real bool type for values,
		// ao just get an integer.
		if (condition.asParamToken()) {
			ifBlock.execute(_args, _locals);
		} else {
			elseBlock.execute(_args, _locals);
		}

		// If blocks themselves shouldn't return anything.
		return ScriptValue();
	}

	case kOpcodeEquals: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		ScriptValue returnValue(kOperandTypeBool);
		bool equal = (value1 == value2);
		returnValue.setToParamToken(static_cast<uint>(equal));
		return returnValue;
	}

	case kOpcodeNotEquals: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		ScriptValue returnValue(kOperandTypeBool);
		bool notEqual = !(value1 == value2);
		returnValue.setToParamToken(static_cast<uint>(notEqual));
		return returnValue;
	}

	case kOpcodeLessThan: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		ScriptValue returnValue(kOperandTypeBool);
		bool lessThan = (value1 < value2);
		returnValue.setToParamToken(static_cast<uint>(lessThan));
		return returnValue;
	}

	case kOpcodeGreaterThan: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		ScriptValue returnValue(kOperandTypeBool);
		bool greaterThan = (value1 > value2);
		returnValue.setToParamToken(static_cast<uint>(greaterThan));
		return returnValue;
	}

	case kOpcodeLessThanOrEqualTo: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		ScriptValue returnValue(kOperandTypeBool);
		bool lessThanOrEqualTo = (value1 < value2) || (value1 == value2);
		returnValue.setToParamToken(static_cast<uint>(lessThanOrEqualTo));
		return returnValue;
	}

	case kOpcodeGreaterThanOrEqualTo: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		ScriptValue returnValue(kOperandTypeBool);
		bool greaterThanOrEqualTo = (value1 > value2) || (value1 == value2);
		returnValue.setToParamToken(static_cast<uint>(greaterThanOrEqualTo));
		return returnValue;
	}

	case kOpcodeAdd: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		ScriptValue returnValue = value1 + value2;
		return returnValue;
	}

	case kOpcodeSubtract: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		ScriptValue returnValue = value1 - value2;
		return returnValue;
	}

	case kOpcodeMultiply: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		ScriptValue returnValue = value1 * value2;
		return returnValue;
	}

	case kOpcodeDivide: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		ScriptValue returnValue = value1 / value2;
		return returnValue;
	}

	case kOpcodeModulo: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		ScriptValue returnValue = value1 % value2;
		return returnValue;
	}

	case kOpcodeNegate: {
		ScriptValue value = evaluateExpression();
		debugCN(5, kDebugScript, "    value: ");

		return -value;
	}

	case kOpcodeReturn: {
		debugCN(5, kDebugScript, "    return: ");
		ScriptValue value = evaluateExpression();

		return value;
	}

	case kOpcodeCallFunctionInVariable: {
		uint parameterCount = Datum(*_bytecode).u.i;
		ScriptValue variable = evaluateExpression();
		uint functionId = variable.asFunctionId();
		debugC(5, kDebugScript, "Variable %d [function %d] (%d params)", variable.getVariable()->_id, functionId, parameterCount);

		return callFunction(functionId, parameterCount);
	}

	default:
		error("Got unimplemented opcode %s (%d)", opcodeToStr(opcode), static_cast<uint>(opcode));
	}
}

ScriptValue CodeChunk::evaluateValue() {
	OperandType operandType = static_cast<OperandType>(Datum(*_bytecode).u.i);
	debugCN(5, kDebugScript, "%s ", operandTypeToStr(operandType));

	ScriptValue returnValue(operandType);
	switch (operandType) {
	case kOperandTypeBool: {
		int b = Datum(*_bytecode).u.i;
		if (b != 0 && b != 1) {
			error("Got invalid boolean value %d", b);
		}
		debugC(5, kDebugScript, "%d ", b);
		returnValue.setToParamToken(b == 1 ? true : false);
		return returnValue;
	}

	case kOperandTypeFloat: {
		double f = Datum(*_bytecode).u.f;
		debugC(5, kDebugScript, "%f ", f);
		returnValue.setToFloat(f);
		return returnValue;
	}

	case kOperandTypeInt: {
		int i = Datum(*_bytecode).u.i;
		debugC(5, kDebugScript, "%d ", i);
		returnValue.setToParamToken(i);
		return returnValue;
	}

	case kOperandTypeString: {
		// This is indeed a raw string, not a string wrapped in a datum!
		// TODO: This copies the string. Can we read it directly from the chunk?
		int size = Datum(*_bytecode, kDatumTypeUint16_1).u.i;
		char *buffer = new char[size + 1];
		_bytecode->read(buffer, size);
		buffer[size] = '\0';
		Common::String *string = new Common::String(buffer);
		debugC(5, kDebugScript, "%s ", string->c_str());
		returnValue.setToString(string);
		delete[] buffer;
		return returnValue;
	}

	case kOperandTypeParamToken: {
		uint literal = Datum(*_bytecode).u.i;
		debugC(5, kDebugScript, "%d ", literal);
		returnValue.setToParamToken(literal);
		return returnValue;
	}

	case kOperandTypeAssetId: {
		uint assetId = Datum(*_bytecode).u.i;
		debugC(5, kDebugScript, "%d ", assetId);
		returnValue.setToAssetId(assetId);
		return returnValue;
	}

	case kOperandTypeTime: {
		double d = Datum(*_bytecode).u.f;
		debugC(5, kDebugScript, "%f ", d);
		returnValue.setToFloat(d);
		return returnValue;
	}

	case kOperandTypeVariable: {
		// TODO: Implement this as we go through the re-architecting.
		error("kOperandTypeVariable not implemented yet");
	}

	case kOperandTypeFunctionId: {
		uint functionId = Datum(*_bytecode).u.i;
		debugC(5, kDebugScript, "%d ", functionId);
		returnValue.setToFunctionId(functionId);
		return returnValue;
	}

	case kOperandTypeMethodId: {
		BuiltInMethod methodId = static_cast<BuiltInMethod>(Datum(*_bytecode).u.i);
		debugC(5, kDebugScript, "%s ", builtInMethodToStr(methodId));
		returnValue.setToMethodId(methodId);
		return returnValue;
	}

	default:
		error("Got unknown ScriptValue type %s (%d)", operandTypeToStr(operandType), static_cast<uint>(operandType));
	}
}

ScriptValue CodeChunk::evaluateVariable() {
	uint32 id = Datum(*_bytecode).u.i;
	VariableScope scope = static_cast<VariableScope>(Datum(*_bytecode).u.i);
	debugC(5, kDebugScript, "Variable %d (%s)", id, variableScopeToStr(scope));
	ScriptValue variable = getVariable(id, scope);
	return variable;
}

ScriptValue CodeChunk::callFunction(uint functionId, uint parameterCount) {
	Common::Array<ScriptValue> args;
	for (uint i = 0; i < parameterCount; i++) {
		debugCN(5, kDebugScript, "  Param %d: ", i);
		ScriptValue arg = evaluateExpression();
		args.push_back(arg);
	}

	ScriptValue returnValue;
	Function *function = g_engine->getFunctionById(functionId);
	if (function != nullptr) {
		// This is a title-defined function.
		returnValue = function->execute(args);
	} else {
		// This is a function built in (and global to) the engine.
		BuiltInFunction builtInFunctionId = static_cast<BuiltInFunction>(functionId);
		debugC(5, kDebugScript, "  Function Name: %s ", builtInFunctionToStr(builtInFunctionId));
		returnValue = g_engine->callBuiltInFunction(builtInFunctionId, args);
	}

	return returnValue;
}

ScriptValue CodeChunk::getVariable(uint32 id, VariableScope scope) {
	switch (scope) {
	case kVariableScopeGlobal: {
		ScriptValue returnValue(kOperandTypeVariable);
		Variable *variable = g_engine->_variables.getVal(id);
		returnValue.putVariable(variable);
		return returnValue;
	}

	case kVariableScopeLocal: {
		uint index = id - 1;
		return _locals->operator[](index);
	}

	case kVariableScopeParameter: {
		uint32 index = id - 1;
		if (_args == nullptr) {
			error("CodeChunk::getVariable(): Requested a parameter in a code chunk that has no parameters");
		}
		return _args->operator[](index);
	}

	default:
		error("CodeChunk::getVariable(): Got unimplemented variable scope %s (%d)", variableScopeToStr(scope), static_cast<uint>(scope));
	}
}

void CodeChunk::putVariable(uint32 id, VariableScope scope, ScriptValue &value) {
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
		_locals->operator[](index) = value;
		break;
	}

	case kVariableScopeParameter: {
		error("CodeChunk::putVariable(): Attempted to assign to a parameter");
		break;
	}

	default:
		error("CodeChunk::getVariable(): Got unimplemented variable scope %s (%d)", variableScopeToStr(scope), static_cast<uint>(scope));
	}
}

ScriptValue CodeChunk::callBuiltInMethod(BuiltInMethod method, ScriptValue &self, Common::Array<ScriptValue> &args) {
	ScriptValue literalSelf = self.getLiteralValue();
	OperandType literalType = literalSelf.getType();
	switch (literalType) {
	case kOperandTypeAssetId: {
		if (self.asAssetId() == 1) {
			// This is a "document" method that we need to handle specially.
			// The document (@doc) accepts engine-level methods like changing the
			// active screen.
			// HACK: This is so we don't have to implement a separate document class
			// just to house these methods. Rather, we just call in the engine.
			ScriptValue returnValue = g_engine->callMethod(method, args);
			return returnValue;
		} else if (self.asAssetId() == 0) {
			// It seems to be valid to call a method on a null asset ID, in
			// which case nothing happens. Still issue warning for traceability.
			warning("CodeChunk::callBuiltInMethod(): Attempt to call method on a null asset ID");
			return ScriptValue();
		} else {
			// This is a regular asset that we can process directly.
			Asset *selfAsset = self.getAsset();
			if (selfAsset == nullptr) {
				error("CodeChunk::callBuiltInMethod(): Attempt to call method on asset ID %d, which isn't loaded", self.asAssetId());
			}
			ScriptValue returnValue = selfAsset->callMethod(method, args);
			return returnValue;
		}
	}

	case kOperandTypeCollection: {
		Common::SharedPtr<Collection> collection = literalSelf.asCollection();
		ScriptValue returnValue = collection->callMethod(method, args);
		return returnValue;
	}

	default:
		error("CodeChunk::callBuiltInMethod(): Attempt to call method on unimplemented ScriptValue type %s (%d)",
			operandTypeToStr(literalType), static_cast<uint>(literalType));
	}
}

CodeChunk::~CodeChunk() {
	// We don't own the args, so we don't need to delete it.
	_args = nullptr;

	if (_weOwnLocals) {
		delete _locals;
	}
	_locals = nullptr;

	delete _bytecode;
	_bytecode = nullptr;
}

} // End of namespace MediaStation
