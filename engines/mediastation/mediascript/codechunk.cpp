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
#include "mediastation/debugchannels.h"

namespace MediaStation {

Common::String CodeChunk::makeDebugIndent() const {
	Common::String indentation;
	for (uint i = 0; i < _debugIndentLevel; ++i) {
		indentation += "    ";
	}
	return indentation;
}

ScriptValue CodeChunk::executeNextBlock() {
	uint blockSize = _bytecode->readTypedUint32();
	int64 startingPos = _bytecode->pos();
	debugC(7, kDebugScript, "%s: Entering new block (blockSize: %d, startingPos: %lld)",
		__func__, blockSize, static_cast<long long int>(startingPos));

	ExpressionType expressionType = static_cast<ExpressionType>(_bytecode->readTypedUint16());
	while (expressionType != kExpressionTypeEmpty && !_returnImmediately) {
		evaluateExpression(expressionType);
		expressionType = static_cast<ExpressionType>(_bytecode->readTypedUint16());

		if (_returnImmediately) {
			debugC(7, kDebugScript, "%s: Done executing block due to script returning value (%s)", __func__, _returnValue.getDebugString().c_str());
		} else if (expressionType == kExpressionTypeEmpty) {
			debugC(7, kDebugScript, "%s: Done executing block due to end of chunk", __func__);
		}
	}

	// Verify we consumed the right number of script bytes. This is not in the original,
	// but it's a very useful sanity check.
	if (!_returnImmediately) {
		uint bytesRead = _bytecode->pos() - startingPos;
		if (bytesRead != blockSize) {
			error("%s: Expected to have read %d script bytes, actually read %d", __func__, blockSize, bytesRead);
		}
	}

	return _returnValue;
}

void CodeChunk::skipNextBlock() {
	uint lengthInBytes = _bytecode->readTypedUint32();
	_bytecode->skip(lengthInBytes);
}

ScriptValue CodeChunk::executeWithArguments(Common::Array<ScriptValue> *args) {
	// Only functions have this call depth requirement.
	if (g_engine->getFunctionManager()->_scriptBlockCallDepth >= MAX_CALL_DEPTH) {
		error("%s: Exceeded max call stack depth", __func__);
	}

	g_engine->getFunctionManager()->_scriptBlockCallDepth++;
	_args = args;
	ScriptValue returnValue = executeNextBlock();
	g_engine->getFunctionManager()->_scriptBlockCallDepth--;
	return returnValue;
}

ScriptValue CodeChunk::evaluateExpression() {
	ExpressionType expressionType = static_cast<ExpressionType>(_bytecode->readTypedUint16());
	ScriptValue returnValue = evaluateExpression(expressionType);
	return returnValue;
}

void CodeChunk::evaluateLValue(ScriptValue *&targetPtr) {
	// Evaluate an expression as an lvalue (something that can be modified in place).
	ExpressionType expressionType = static_cast<ExpressionType>(_bytecode->readTypedUint16());

	switch (expressionType) {
	case kExpressionTypeVariable:
		// Make target point directly to the variable. This permits modifications to the variable
		// (like adding to a collection) to persist in the original variable.
		targetPtr = readAndReturnVariable();
		break;

	case kExpressionTypeValue:
	case kExpressionTypeOperation:
		// For values/operations, just fill in the temporary value the caller passed to us.
		// This means modifications to the evaluated expression will not be persisted like variables.
		*targetPtr = evaluateExpression(expressionType);
		break;

	default:
		error("%s: Unexpected expression type %s", __func__, expressionTypeToStr(expressionType));
	}
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
		error("%s: Got unimplemented expression type %s (%d)", __func__,
			expressionTypeToStr(expressionType), static_cast<uint>(expressionType));
	}
	return returnValue;
}

ScriptValue CodeChunk::evaluateOperation() {
	Opcode opcode = static_cast<Opcode>(_bytecode->readTypedUint16());
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
		evaluateReturn();
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
		error("%s: Got unimplemented opcode %s (%d)", __func__, opcodeToStr(opcode), static_cast<uint>(opcode));
	}
	return returnValue;
}

ScriptValue CodeChunk::evaluateValue() {
	OperandType operandType = static_cast<OperandType>(_bytecode->readTypedUint16());
	debugCN(5, kDebugScript, "%s ", operandTypeToStr(operandType));

	ScriptValue returnValue;
	switch (operandType) {
	case kOperandTypeBool: {
		int b = _bytecode->readTypedByte();
		if (b != 0 && b != 1) {
			error("%s: Got invalid literal bool value %d", __func__, b);
		}
		debugC(5, kDebugScript, "%d ", b);
		returnValue.setToBool(b == 1 ? true : false);
		break;
	}

	case kOperandTypeFloat: {
		double f = _bytecode->readTypedDouble();
		debugC(5, kDebugScript, "%f ", f);
		returnValue.setToFloat(f);
		break;
	}

	case kOperandTypeInt: {
		int i = _bytecode->readTypedSint32();
		debugC(5, kDebugScript, "%d ", i);
		// Ints are stored internally as doubles.
		returnValue.setToFloat(static_cast<double>(i));
		break;
	}

	case kOperandTypeString: {
		// This is indeed a raw string, not a string wrapped in a datum!
		uint size = _bytecode->readTypedUint16();
		Common::String string = _bytecode->readString('\0', size);
		debugC(5, kDebugScript, "%s ", string.c_str());
		returnValue.setToString(string);
		break;
	}

	case kOperandTypeParamToken: {
		uint literal = _bytecode->readTypedUint16();
		Common::String tokenName = g_engine->formatParamTokenName(literal);
		debugC(5, kDebugScript, "%s ", tokenName.c_str());
		returnValue.setToParamToken(literal);
		break;
	}

	case kOperandTypeActorId: {
		uint actorId = _bytecode->readTypedUint16();
		Common::String actorName = g_engine->formatActorName(actorId, true);
		debugC(5, kDebugScript, "%s ", actorName.c_str());
		returnValue.setToActorId(actorId);
		break;
	}

	case kOperandTypeTime: {
		double d = _bytecode->readTypedTime();
		debugC(5, kDebugScript, "%f ", d);
		returnValue.setToTime(d);
		break;
	}

	case kOperandTypeVariable: {
		returnValue = ScriptValue(_bytecode);
		break;
	}

	case kOperandTypeFunctionId: {
		uint functionId = _bytecode->readTypedUint16();
		// Function IDs are included in this same listing that also includes actors.
		Common::String functionName = g_engine->formatFunctionName(functionId);
		debugC(5, kDebugScript, "%s ", functionName.c_str());
		returnValue.setToFunctionId(functionId);
		break;
	}

	case kOperandTypeMethodId: {
		BuiltInMethod methodId = static_cast<BuiltInMethod>(_bytecode->readTypedUint16());
		debugC(5, kDebugScript, "%s (%d)", builtInMethodToStr(methodId), static_cast<uint>(methodId));
		returnValue.setToMethodId(methodId);
		break;
	}

	default:
		error("%s: Got unknown ScriptValue type %s (%d)", __func__, operandTypeToStr(operandType), static_cast<uint>(operandType));
	}
	return returnValue;
}

ScriptValue CodeChunk::evaluateVariable() {
	ScriptValue *variable = readAndReturnVariable();
	return *variable;
}

ScriptValue *CodeChunk::readAndReturnVariable() {
	uint id = _bytecode->readTypedUint16();
	VariableScope scope = static_cast<VariableScope>(_bytecode->readTypedUint16());
	Common::String name = g_engine->formatVariableName(id);

	ScriptValue *variable = nullptr;
	switch (scope) {
	case kVariableScopeGlobal: {
		variable = g_engine->getImtGod()->getVariable(id);
		if (variable == nullptr) {
			error("%s: Global variable %s doesn't exist", __func__, g_engine->formatVariableName(id).c_str());
		}
		break;
	}

	case kVariableScopeLocal: {
		// The ID is actually a one-based index.
		uint index = id - 1;
		variable = &_locals.operator[](index);
		break;
	}

	case kVariableScopeIndirectParameter: {
		// The ID is actually a one-based index.
		uint baseIndex = id - 1;
		ScriptValue indexValue = evaluateExpression();
		uint index = static_cast<uint>(indexValue.asFloat()) + baseIndex;
		variable = &_args->operator[](index);
		break;
	}

	case kVariableScopeParameter: {
		// The ID is actually a one-based index.
		uint index = id - 1;
		if (_args == nullptr) {
			error("%s: Requested a parameter in a code chunk that has no parameters", __func__);
		}
		variable = &_args->operator[](index);
		break;
	}

	default:
		error("%s: Got unknown variable scope %s (%d)", __func__, variableScopeToStr(scope), static_cast<uint>(scope));
	}

	debugC(5, kDebugScript, "%s (%s) [value: %s]", name.c_str(), variableScopeToStr(scope), variable->getDebugString().c_str());
	return variable;
}

void CodeChunk::evaluateIf() {
	_debugIndentLevel++;
	debugCN(5, kDebugScript, "\n%scondition: ", makeDebugIndent().c_str());
	ScriptValue condition = evaluateExpression();
	if (condition.getType() != kScriptValueTypeBool) {
		error("%s: Expected bool condition, got %s", __func__, scriptValueTypeToStr(condition.getType()));
	}

	if (condition.asBool()) {
		debugC(5, kDebugScript, "%s=> TRUE", makeDebugIndent().c_str());
		_debugIndentLevel--;
		executeNextBlock();
		debugC(6, kDebugScript, "%s: Taking TRUE branch", __func__);
	} else {
		debugC(5, kDebugScript, "%s=> FALSE", makeDebugIndent().c_str());
		_debugIndentLevel--;
		skipNextBlock();
		debugC(6, kDebugScript, "%s: Skipping TRUE branch", __func__);
	}
}

void CodeChunk::evaluateIfElse() {
	_debugIndentLevel++;
	debugCN(5, kDebugScript, "\n%scondition: ", makeDebugIndent().c_str());
	ScriptValue condition = evaluateExpression();
	if (condition.getType() != kScriptValueTypeBool) {
		error("%s: Expected bool condition, got %s", __func__, scriptValueTypeToStr(condition.getType()));
	}

	if (condition.asBool()) {
		debugC(5, kDebugScript, "%s=> TRUE", makeDebugIndent().c_str());
		_debugIndentLevel--;

		debugC(6, kDebugScript, "%s: Taking TRUE branch", __func__);
		executeNextBlock();

		debugC(6, kDebugScript, "%s: Skipping FALSE branch", __func__);
		skipNextBlock();
	} else {
		debugC(5, kDebugScript, "%s=> FALSE", makeDebugIndent().c_str());
		_debugIndentLevel--;

		debugC(6, kDebugScript, "%s: Skipping TRUE branch", __func__);
		skipNextBlock();

		debugC(6, kDebugScript, "%s: Taking FALSE branch", __func__);
		executeNextBlock();
	}
}

ScriptValue CodeChunk::evaluateAssign() {
	_debugIndentLevel++;
	debugCN(5, kDebugScript, "\n%svariable: ", makeDebugIndent().c_str());
	ScriptValue *targetVariable = readAndReturnVariable();
	debugCN(5, kDebugScript, "%svalue: ", makeDebugIndent().c_str());
	ScriptValue value = evaluateExpression();
	_debugIndentLevel--;

	if (value.getType() == kScriptValueTypeEmpty) {
		error("%s: Attempt to assign an empty value to a variable", __func__);
	}

	if (targetVariable != nullptr) {
		*targetVariable = value;
		return value;
	} else {
		error("%s: Attempt to assign to null variable", __func__);
	}
}

ScriptValue CodeChunk::evaluateBinaryOperation(Opcode op) {
	_debugIndentLevel++;
	debugCN(5, kDebugScript, "\n%slhs: ", makeDebugIndent().c_str());
	ScriptValue value1 = evaluateExpression();
	debugCN(5, kDebugScript, "%srhs: ", makeDebugIndent().c_str());
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
		error("%s: Got unknown binary operation opcode %s", __func__, opcodeToStr(op));
	}

	// For comparison operations, show the result.
	if (op == kOpcodeOr || op == kOpcodeXor || op == kOpcodeAnd ||
	    op == kOpcodeEquals || op == kOpcodeNotEquals ||
	    op == kOpcodeLessThan || op == kOpcodeGreaterThan ||
	    op == kOpcodeLessThanOrEqualTo || op == kOpcodeGreaterThanOrEqualTo) {
		debugC(5, kDebugScript, "%s=> %s", makeDebugIndent().c_str(), returnValue.asBool() ? "TRUE" : "FALSE");
	}

	_debugIndentLevel--;
	return returnValue;
}

ScriptValue CodeChunk::evaluateUnaryOperation() {
	// The only supported unary operation seems to be negation.
	_debugIndentLevel++;
	debugCN(5, kDebugScript, "\n%svalue: ", makeDebugIndent().c_str());
	ScriptValue value = evaluateExpression();
	_debugIndentLevel--;
	return -value;
}

ScriptValue CodeChunk::evaluateFunctionCall(bool isIndirect) {
	uint functionId, paramCount = 0;
	if (isIndirect) {
		paramCount = _bytecode->readTypedUint16();
		ScriptValue value = evaluateExpression();
		functionId = value.asFunctionId();
	} else {
		functionId = _bytecode->readTypedUint16();
		paramCount = _bytecode->readTypedUint16();
	}

	return evaluateFunctionCall(functionId, paramCount);
}

ScriptValue CodeChunk::evaluateFunctionCall(uint functionId, uint paramCount) {
	Common::String functionName = g_engine->formatFunctionName(functionId);
	debugC(5, kDebugScript, "%s (%d params)", functionName.c_str(), paramCount);

	Common::Array<ScriptValue> args;
	_debugIndentLevel++;
	for (uint i = 0; i < paramCount; i++) {
		debugCN(5, kDebugScript, "%sparam %d: ", makeDebugIndent().c_str(), i);
		ScriptValue arg = evaluateExpression();
		args.push_back(arg);
	}
	_debugIndentLevel--;

	ScriptValue returnValue = g_engine->getFunctionManager()->call(functionId, args);
	return returnValue;
}

ScriptValue CodeChunk::evaluateMethodCall(bool isIndirect) {
	BuiltInMethod method;
	uint paramCount = 0;
	if (isIndirect) {
		paramCount = _bytecode->readTypedUint16();
		ScriptValue value = evaluateExpression();
		method = value.asMethodId();
	} else {
		method = static_cast<BuiltInMethod>(_bytecode->readTypedUint16());
		paramCount = _bytecode->readTypedUint16();
	}

	return evaluateMethodCall(method, paramCount);
}

ScriptValue CodeChunk::evaluateMethodCall(BuiltInMethod method, uint paramCount) {
	// In Media Station, all methods are built-in - there aren't
	// custom objects or methods individual titles can
	// define. Functions, however, CAN be title-defined.
	// But here, we're only looking for built-in methods.
	debugC(5, kDebugScript, "%s (%d params)", builtInMethodToStr(method), paramCount);
	_debugIndentLevel++;
	debugCN(5, kDebugScript, "%sself: ", makeDebugIndent().c_str());

	// Evaluate target as an lvalue to get a pointer to the actual variable if there is one.
	ScriptValue methodCallTarget;
	ScriptValue *methodCallTargetPtr = &methodCallTarget;
	evaluateLValue(methodCallTargetPtr);
	Common::Array<ScriptValue> args;
	for (uint i = 0; i < paramCount; i++) {
		debugCN(5, kDebugScript, "%sparam %d: ", makeDebugIndent().c_str(), i);
		ScriptValue arg = evaluateExpression();
		args.push_back(arg);
	}
	_debugIndentLevel--;

	ScriptValue returnValue;
	switch (methodCallTargetPtr->getType()) {
	case kScriptValueTypeActorId: {
		if (methodCallTargetPtr->asActorId() == 0) {
			// It seems to be valid to call a method on a null actor ID, in
			// which case nothing happens. Still log for traceability.
			debugC(5, kDebugScript, "%s: Attempt to call method %s (%d) on null actor ID", __func__, builtInMethodToStr(method), static_cast<uint>(method));
			break;
		} else {
			// This is a regular actor that we can process directly.
			uint actorId = methodCallTargetPtr->asActorId();
			Actor *targetActor = g_engine->getImtGod()->getActorById(actorId);
			if (targetActor == nullptr) {
				warning("[%s] %s: Actor not loaded", g_engine->formatActorName(actorId).c_str(), __func__);
			} else {
				returnValue = targetActor->callMethod(method, args);
			}
			break;
		}
	}

	case kScriptValueTypeCollection: {
		Collection *collection = methodCallTargetPtr->asCollection();
		returnValue = collection->callMethod(method, args);
		break;
	}

	default:
		error("%s: Attempt to call method %s (%d) on unimplemented value type %s (%d)", __func__,
			builtInMethodToStr(method), static_cast<uint>(method),
			scriptValueTypeToStr(methodCallTargetPtr->getType()), static_cast<uint>(methodCallTargetPtr->getType()));
	}
	return returnValue;
}

void CodeChunk::evaluateDeclareLocals() {
	uint localVariableCount = _bytecode->readTypedUint16();
	if (localVariableCount <= 0) {
		error("%s: Got non-positive local variable count", __func__);
	}
	debugC(5, kDebugScript, "%d", localVariableCount);
	_locals = Common::Array<ScriptValue>(localVariableCount);
}

void CodeChunk::evaluateReturn() {
	_returnValue = evaluateExpression();
	_returnImmediately = true;
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
		_debugIndentLevel++;
		debugCN(5, kDebugScript, "\n%scondition: ", makeDebugIndent().c_str());
		ScriptValue condition = evaluateExpression();
		_debugIndentLevel--;
		if (condition.getType() != kScriptValueTypeBool) {
			error("%s: Expected loop condition to be bool, not %s", __func__, scriptValueTypeToStr(condition.getType()));
		}

		if (++iterationCount >= MAX_LOOP_ITERATION_COUNT) {
			error("%s: Exceeded max loop iteration count", __func__);
		}

		if (condition.asBool()) {
			debugC(5, kDebugScript, "%s=> TRUE (continue loop)", makeDebugIndent().c_str());
			executeNextBlock();
		} else {
			debugC(5, kDebugScript, "%s=> FALSE (exit loop)", makeDebugIndent().c_str());
			skipNextBlock();
			break;
		}
	}
}

CodeChunk::~CodeChunk() {
	_locals.clear();

	// We don't own the args or the code stream, so we don't need to delete them.
	_args = nullptr;
	_bytecode = nullptr;
}

} // End of namespace MediaStation
