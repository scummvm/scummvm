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
#include "mediastation/mediascript/collection.h"
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
		if (instructionResult.getType() != kScriptValueTypeEmpty) {
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
		return returnValue;
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

	ScriptValue returnValue;
	switch (opcode) {
	case kOpcodeAssignVariable: {
		evaluateAssign();
		return returnValue;
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
		returnValue = callBuiltInMethod(methodId, selfObject, args);
		return returnValue;
	}

	case kOpcodeDeclareVariables: {
		uint32 localVariableCount = Datum(*_bytecode).u.i;
		debugC(5, kDebugScript, "%d", localVariableCount);
		assert(_locals == nullptr);
		_locals = new Common::Array<ScriptValue>(localVariableCount);
		_weOwnLocals = true;
		return returnValue;
	}

	case kOpcodeOr: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue.setToBool(value1 || value2);
		return returnValue;
	}

	case kOpcodeXor: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue.setToBool(value1 ^ value2);
		return returnValue;
	}

	case kOpcodeAnd: {
		debugCN(5, kDebugScript, "\n    value: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue.setToBool(value1 && value2);
		return returnValue;
	}

	case kOpcodeIfElse: {
		debugCN(5, kDebugScript, "\n    condition: ");
		ScriptValue condition = evaluateExpression();

		CodeChunk ifBlock(*_bytecode);
		CodeChunk elseBlock(*_bytecode);
		if (condition.asBool()) {
			ifBlock.execute(_args, _locals);
		} else {
			elseBlock.execute(_args, _locals);
		}

		return returnValue;
	}

	case kOpcodeEquals: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue.setToBool(value1 == value2);
		return returnValue;
	}

	case kOpcodeNotEquals: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue.setToBool(!(value1 == value2));
		return returnValue;
	}

	case kOpcodeLessThan: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue.setToBool(value1 < value2);
		return returnValue;
	}

	case kOpcodeGreaterThan: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue.setToBool(value1 > value2);
		return returnValue;
	}

	case kOpcodeLessThanOrEqualTo: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue.setToBool((value1 < value2) || (value1 == value2));
		return returnValue;
	}

	case kOpcodeGreaterThanOrEqualTo: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue.setToBool((value1 > value2) || (value1 == value2));
		return returnValue;
	}

	case kOpcodeAdd: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue = value1 + value2;
		return returnValue;
	}

	case kOpcodeSubtract: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue = value1 - value2;
		return returnValue;
	}

	case kOpcodeMultiply: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue = value1 * value2;
		return returnValue;
	}

	case kOpcodeDivide: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue = value1 / value2;
		return returnValue;
	}

	case kOpcodeModulo: {
		debugCN(5, kDebugScript, "\n    lhs: ");
		ScriptValue value1 = evaluateExpression();
		debugCN(5, kDebugScript, "    rhs: ");
		ScriptValue value2 = evaluateExpression();

		returnValue = value1 % value2;
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
		debugC(5, kDebugScript, "[Indirect function %d] (%d params)", functionId, parameterCount);

		return callFunction(functionId, parameterCount);
	}

	default:
		error("Got unimplemented opcode %s (%d)", opcodeToStr(opcode), static_cast<uint>(opcode));
	}
}

ScriptValue CodeChunk::evaluateValue() {
	OperandType operandType = static_cast<OperandType>(Datum(*_bytecode).u.i);
	debugCN(5, kDebugScript, "%s ", operandTypeToStr(operandType));

	ScriptValue returnValue;
	switch (operandType) {
	case kOperandTypeBool: {
		int b = Datum(*_bytecode).u.i;
		if (b != 0 && b != 1) {
			error("Got invalid literal bool value %d", b);
		}
		debugC(5, kDebugScript, "%d ", b);
		returnValue.setToBool(b == 1 ? true : false);
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
		// Ints are stored internally as doubles.
		returnValue.setToFloat(static_cast<double>(i));
		return returnValue;
	}

	case kOperandTypeString: {
		// This is indeed a raw string, not a string wrapped in a datum!
		uint size = Datum(*_bytecode, kDatumTypeUint16_1).u.i;
		Common::String string = _bytecode->readString('\0', size);
		debugC(5, kDebugScript, "%s ", string.c_str());
		returnValue.setToString(string);
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
		returnValue.setToTime(d);
		return returnValue;
	}

	case kOperandTypeVariable: {
		returnValue = ScriptValue(_bytecode);
		return returnValue;
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
	ScriptValue *variable = readAndReturnVariable();
	return *variable;
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

ScriptValue *CodeChunk::readAndReturnVariable() {
	uint id = Datum(*_bytecode).u.i;
	VariableScope scope = static_cast<VariableScope>(Datum(*_bytecode).u.i);
	debugC(5, kDebugScript, "%d (%s)", id, variableScopeToStr(scope));

	ScriptValue returnValue;
	switch (scope) {
	case kVariableScopeGlobal: {
		ScriptValue *variable = g_engine->getVariable(id);
		if (variable == nullptr) {
			error("Global variable %d doesn't exist", id);
		}
		return variable;
	}

	case kVariableScopeLocal: {
		uint index = id - 1;
		return &_locals->operator[](index);
	}

	case kVariableScopeIndirectParameter: {
		ScriptValue indexValue = evaluateExpression();
		uint index = static_cast<uint>(indexValue.asFloat() + id);
		return &_args->operator[](index);
	}

	case kVariableScopeParameter: {
		uint index = id - 1;
		if (_args == nullptr) {
			error("Requested a parameter in a code chunk that has no parameters");
		}
		return &_args->operator[](index);
	}

	default:
		error("Got unknown variable scope %s (%d)", variableScopeToStr(scope), static_cast<uint>(scope));
	}
}

ScriptValue CodeChunk::evaluateAssign() {
	debugCN(5, kDebugScript, "Variable ");
	ScriptValue *targetVariable = readAndReturnVariable();

	debugC(5, kDebugScript, "  Value: ");
	ScriptValue value = evaluateExpression();

	if (value.getType() == kScriptValueTypeEmpty) {
		error("Attempt to assign an empty value to a variable");
	}

	if (targetVariable != nullptr) {
		*targetVariable = value;
		return value;
	} else {
		error("Attempt to assign to null variable");
	}
}

ScriptValue CodeChunk::callBuiltInMethod(BuiltInMethod method, ScriptValue &self, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (self.getType()) {
	case kScriptValueTypeAssetId: {
		if (self.asAssetId() == 1) {
			// This is a "document" method that we need to handle specially.
			// The document (@doc) accepts engine-level methods like changing the
			// active screen.
			// HACK: This is so we don't have to implement a separate document class
			// just to house these methods. Rather, we just call in the engine.
			returnValue = g_engine->callMethod(method, args);
			return returnValue;
		} else if (self.asAssetId() == 0) {
			// It seems to be valid to call a method on a null asset ID, in
			// which case nothing happens. Still issue warning for traceability.
			warning("CodeChunk::callBuiltInMethod(): Attempt to call method on a null asset ID");
			return returnValue;
		} else {
			// This is a regular asset that we can process directly.
			uint assetId = self.asAssetId();
			Asset *selfAsset = g_engine->getAssetById(assetId);
			if (selfAsset == nullptr) {
				error("CodeChunk::callBuiltInMethod(): Attempt to call method on asset ID %d, which isn't loaded", self.asAssetId());
			}
			returnValue = selfAsset->callMethod(method, args);
			return returnValue;
		}
	}

	case kScriptValueTypeCollection: {
		Common::SharedPtr<Collection> collection = self.asCollection();
		returnValue = collection->callMethod(method, args);
		return returnValue;
	}

	default:
		error("CodeChunk::callBuiltInMethod(): Attempt to call method on unimplemented ScriptValue type %s (%d)",
			scriptValueTypeToStr(self.getType()), static_cast<uint>(self.getType()));
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
