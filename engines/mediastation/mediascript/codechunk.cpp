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

ScriptValue CodeChunk::executeNextBlock() {
	uint blockSize = Datum(*_bytecode, kDatumTypeUint32_1).u.i;
	uint startingPos = _bytecode->pos();

	ScriptValue returnValue;
	ExpressionType expressionType = static_cast<ExpressionType>(Datum(*_bytecode).u.i);
	while (expressionType != kExpressionTypeEmpty && !_returnImmediately) {
		returnValue = evaluateExpression(expressionType);
		expressionType = static_cast<ExpressionType>(Datum(*_bytecode).u.i);
	}

	// Verify we consumed the right number of script bytes.
	if (!_returnImmediately) {
		uint bytesRead = _bytecode->pos() - startingPos;
		if (bytesRead != blockSize) {
			error("Expected to have read %d script bytes, actually read %d", blockSize, bytesRead);
		}
	}
	return returnValue;
}

void CodeChunk::skipNextBlock() {
	uint lengthInBytes = Datum(*_bytecode, kDatumTypeUint32_1).u.i;
	_bytecode->skip(lengthInBytes);
}

ScriptValue CodeChunk::execute(Common::Array<ScriptValue> *args) {
	_args = args;
	ScriptValue returnValue = executeNextBlock();

	// Rewind the stream once we're finished, in case we need to execute
	// this code again!
	_bytecode->seek(0);
	_locals.clear();
	// We don't own the args, so we will prevent a potentially out-of-scope
	// variable from being re-accessed.
	_args = nullptr;

	return returnValue;
}

ScriptValue CodeChunk::evaluateExpression() {
	ExpressionType expressionType = static_cast<ExpressionType>(Datum(*_bytecode).u.i);
	ScriptValue returnValue = evaluateExpression(expressionType);
	return returnValue;
}

ScriptValue CodeChunk::evaluateExpression(ExpressionType expressionType) {
	debugCN(5, kDebugScript, "(%s) ", expressionTypeToStr(expressionType));

	ScriptValue returnValue;
	switch (expressionType) {
	case kExpressionTypeEmpty:
		break;

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
		error("Got unimplemented expression type %s (%d)",
			expressionTypeToStr(expressionType), static_cast<uint>(expressionType));
	}
	return returnValue;
}

ScriptValue CodeChunk::evaluateOperation() {
	Opcode opcode = static_cast<Opcode>(Datum(*_bytecode).u.i);
	debugCN(5, kDebugScript, "%s ", opcodeToStr(opcode));

	ScriptValue returnValue;
	switch (opcode) {
	case kOpcodeIf:
		evaluateIf();
		break;

	case kOpcodeIfElse:
		evaluateIfElse();
		break;

	case kOpcodeAssignVariable:
		evaluateAssign();
		break;

	case kOpcodeOr:
	case kOpcodeXor:
	case kOpcodeAnd:
	case kOpcodeEquals:
	case kOpcodeNotEquals:
	case kOpcodeLessThan:
	case kOpcodeGreaterThan:
	case kOpcodeLessThanOrEqualTo:
	case kOpcodeGreaterThanOrEqualTo:
	case kOpcodeAdd:
	case kOpcodeSubtract:
	case kOpcodeMultiply:
	case kOpcodeDivide:
	case kOpcodeModulo:
		returnValue = evaluateBinaryOperation(opcode);
		break;

	case kOpcodeNegate:
		returnValue = evaluateUnaryOperation();
		break;

	case kOpcodeCallFunction:
		returnValue = evaluateFunctionCall();
		break;

	case kOpcodeCallMethod:
		returnValue = evaluateMethodCall();
		break;

	case kOpcodeDeclareLocals:
		evaluateDeclareLocals();
		break;

	case kOpcodeReturn:
		returnValue = evaluateReturn();
		break;

	case kOpcodeReturnNoValue:
		evaluateReturnNoValue();
		break;

	case kOpcodeWhile:
		evaluateWhileLoop();
		break;

	case kOpcodeCallFunctionInVariable:
		returnValue = evaluateFunctionCall(true);
		break;

	case kOpcodeCallMethodInVariable:
		returnValue = evaluateMethodCall(true);
		break;

	default:
		error("Got unimplemented opcode %s (%d)", opcodeToStr(opcode), static_cast<uint>(opcode));
	}
	return returnValue;
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
		return &_locals.operator[](index);
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

void CodeChunk::evaluateIf() {
	debugCN(5, kDebugScript, "\n    condition: ");
	ScriptValue condition = evaluateExpression();
	if (condition.getType() != kScriptValueTypeBool) {
		error("evaluateIf: Expected bool condition, got %s", scriptValueTypeToStr(condition.getType()));
	}

	if (condition.asBool()) {
		executeNextBlock();
	} else {
		skipNextBlock();
	}
}

void CodeChunk::evaluateIfElse() {
	debugCN(5, kDebugScript, "\n    condition: ");
	ScriptValue condition = evaluateExpression();
	if (condition.getType() != kScriptValueTypeBool) {
		error("evaluateIfElse: Expected bool condition, got %s", scriptValueTypeToStr(condition.getType()));
	}

	if (condition.asBool()) {
		executeNextBlock();
		skipNextBlock();
	} else {
		skipNextBlock();
		executeNextBlock();
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

ScriptValue CodeChunk::evaluateBinaryOperation(Opcode op) {
	debugCN(5, kDebugScript, "\n    lhs: ");
	ScriptValue value1 = evaluateExpression();
	debugCN(5, kDebugScript, "    rhs: ");
	ScriptValue value2 = evaluateExpression();

	ScriptValue returnValue;
	switch (op) {
	case kOpcodeOr:
		returnValue.setToBool(value1 || value2);
		break;

	case kOpcodeXor:
		returnValue.setToBool(value1 ^ value2);
		break;

	case kOpcodeAnd:
		returnValue.setToBool(value1 && value2);
		break;

	case kOpcodeEquals:
		returnValue.setToBool(value1 == value2);
		break;

	case kOpcodeNotEquals:
		returnValue.setToBool(value1 != value2);
		break;

	case kOpcodeLessThan:
		returnValue.setToBool(value1 < value2);
		break;

	case kOpcodeGreaterThan:
		returnValue.setToBool(value1 > value2);
		break;

	case kOpcodeLessThanOrEqualTo:
		returnValue.setToBool(value1 <= value2);
		break;

	case kOpcodeGreaterThanOrEqualTo:
		returnValue.setToBool(value1 >= value2);
		break;

	case kOpcodeAdd:
		returnValue = value1 + value2;
		break;

	case kOpcodeSubtract:
		returnValue = value1 - value2;
		break;

	case kOpcodeMultiply:
		returnValue = value1 * value2;
		break;

	case kOpcodeDivide:
		returnValue = value1 / value2;
		break;

	case kOpcodeModulo:
		returnValue = value1 % value2;
		break;

	default:
		error("Got unknown binary operation opcode %s", opcodeToStr(op));
	}
	return returnValue;
}

ScriptValue CodeChunk::evaluateUnaryOperation() {
	// The only supported unary operation seems to be negation.
	ScriptValue value = evaluateExpression();
	debugCN(5, kDebugScript, "    value: ");
	return -value;
}

ScriptValue CodeChunk::evaluateFunctionCall(bool isIndirect) {
	uint functionId, paramCount = 0;
	if (isIndirect) {
		paramCount = Datum(*_bytecode).u.i;
		ScriptValue value = evaluateExpression();
		functionId = value.asFunctionId();
	} else {
		functionId = Datum(*_bytecode).u.i;
		paramCount = Datum(*_bytecode).u.i;
	}

	return evaluateFunctionCall(functionId, paramCount);
}

ScriptValue CodeChunk::evaluateFunctionCall(uint functionId, uint paramCount) {
	debugC(5, kDebugScript, "%d (%d params)", functionId, paramCount);

	Common::Array<ScriptValue> args;
	for (uint i = 0; i < paramCount; i++) {
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

ScriptValue CodeChunk::evaluateMethodCall(bool isIndirect) {
	BuiltInMethod method;
	uint paramCount = 0;
	if (isIndirect) {
		paramCount = Datum(*_bytecode).u.i;
		ScriptValue value = evaluateExpression();
		method = value.asMethodId();
	} else {
		method = static_cast<BuiltInMethod>(Datum(*_bytecode).u.i);
		paramCount = Datum(*_bytecode).u.i;
	}

	return evaluateMethodCall(method, paramCount);
}

ScriptValue CodeChunk::evaluateMethodCall(BuiltInMethod method, uint paramCount) {
	// In Media Station, all methods are built-in - there aren't
	// custom objects or methods individual titles can
	// define. Functions, however, CAN be title-defined.
	// But here, we're only looking for built-in methods.
	debugC(5, kDebugScript, "%s (%d params)", builtInMethodToStr(method), paramCount);
	debugCN(5, kDebugScript, "  Self: ");

	ScriptValue target = evaluateExpression();
	Common::Array<ScriptValue> args;
	for (uint i = 0; i < paramCount; i++) {
		debugCN(5, kDebugScript, "  Param %d: ", i);
		ScriptValue arg = evaluateExpression();
		args.push_back(arg);
	}

	ScriptValue returnValue;
	switch (target.getType()) {
	case kScriptValueTypeAssetId: {
		if (target.asAssetId() == 1) {
			// This is a "document" method that we need to handle specially.
			// The document (@doc) accepts engine-level methods like changing the
			// active screen.
			// HACK: This is so we don't have to implement a separate document class
			// just to house these methods. Rather, we just call in the engine.
			returnValue = g_engine->callMethod(method, args);
			return returnValue;
		} else if (target.asAssetId() == 0) {
			// It seems to be valid to call a method on a null asset ID, in
			// which case nothing happens. Still issue warning for traceability.
			warning("Attempt to call method on a null asset ID");
			return returnValue;
		} else {
			// This is a regular asset that we can process directly.
			uint assetId = target.asAssetId();
			Asset *targetAsset = g_engine->getAssetById(assetId);
			if (targetAsset == nullptr) {
				error("Attempt to call method on asset ID %d, which isn't loaded", target.asAssetId());
			}
			returnValue = targetAsset->callMethod(method, args);
			return returnValue;
		}
	}

	case kScriptValueTypeCollection: {
		Common::SharedPtr<Collection> collection = target.asCollection();
		returnValue = collection->callMethod(method, args);
		return returnValue;
	}

	default:
		error("Attempt to call method on unimplemented value type %s (%d)",
			scriptValueTypeToStr(target.getType()), static_cast<uint>(target.getType()));
	}
}

void CodeChunk::evaluateDeclareLocals() {
	uint localVariableCount = Datum(*_bytecode).u.i;
	if (localVariableCount <= 0) {
		error("Got non-positive local variable count");
	}
	debugC(5, kDebugScript, "%d", localVariableCount);
	_locals = Common::Array<ScriptValue>(localVariableCount);
}

ScriptValue CodeChunk::evaluateReturn() {
	ScriptValue returnValue = evaluateExpression();
	_returnImmediately = true;
	return returnValue;
}

void CodeChunk::evaluateReturnNoValue() {
	_returnImmediately = true;
}

void CodeChunk::evaluateWhileLoop() {
	uint loopStartPosition = _bytecode->pos();
	uint iterationCount = 0;

	while (true) {
		// Seek to the top of the loop bytecode.
		_bytecode->seek(loopStartPosition);
		ScriptValue condition = evaluateExpression();
		if (condition.getType() != kScriptValueTypeBool) {
			error("Expected loop condition to be bool, not %s", scriptValueTypeToStr(condition.getType()));
		}

		if (++iterationCount >= MAX_LOOP_ITERATION_COUNT) {
			error("Exceeded max loop iteration count");
		}

		if (condition.asBool()) {
			executeNextBlock();
		} else {
			skipNextBlock();
			break;
		}
	}
}

CodeChunk::~CodeChunk() {
	_locals.clear();

	// We don't own the args, so we don't need to delete it.
	_args = nullptr;

	delete _bytecode;
	_bytecode = nullptr;
}

} // End of namespace MediaStation
