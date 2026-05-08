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

static uint getIndentSize(uint indentLevel) {
	constexpr uint INDENT_SIZE_IN_SPACES = 4;
	return indentLevel * INDENT_SIZE_IN_SPACES;
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
	debugCN(5, kDebugScript, "\n%*scondition: ", getIndentSize(_debugIndentLevel), "");
	ScriptValue condition = evaluateExpression();
	if (condition.getType() != kScriptValueTypeBool) {
		error("%s: Expected bool condition, got %s", __func__, scriptValueTypeToStr(condition.getType()));
	}

	if (condition.asBool()) {
		debugC(5, kDebugScript, "%*s=> TRUE", getIndentSize(_debugIndentLevel), "");
		_debugIndentLevel--;
		executeNextBlock();
		debugC(6, kDebugScript, "%s: Taking TRUE branch", __func__);
	} else {
		debugC(5, kDebugScript, "%*s=> FALSE", getIndentSize(_debugIndentLevel), "");
		_debugIndentLevel--;
		skipNextBlock();
		debugC(6, kDebugScript, "%s: Skipping TRUE branch", __func__);
	}
}

void CodeChunk::evaluateIfElse() {
	_debugIndentLevel++;
	debugCN(5, kDebugScript, "\n%*scondition: ", getIndentSize(_debugIndentLevel), "");
	ScriptValue condition = evaluateExpression();
	if (condition.getType() != kScriptValueTypeBool) {
		error("%s: Expected bool condition, got %s", __func__, scriptValueTypeToStr(condition.getType()));
	}

	if (condition.asBool()) {
		debugC(5, kDebugScript, "%*s=> TRUE", getIndentSize(_debugIndentLevel), "");
		_debugIndentLevel--;

		debugC(6, kDebugScript, "%s: Taking TRUE branch", __func__);
		executeNextBlock();

		debugC(6, kDebugScript, "%s: Skipping FALSE branch", __func__);
		skipNextBlock();
	} else {
		debugC(5, kDebugScript, "%*s=> FALSE", getIndentSize(_debugIndentLevel), "");
		_debugIndentLevel--;

		debugC(6, kDebugScript, "%s: Skipping TRUE branch", __func__);
		skipNextBlock();

		debugC(6, kDebugScript, "%s: Taking FALSE branch", __func__);
		executeNextBlock();
	}
}

ScriptValue CodeChunk::evaluateAssign() {
	_debugIndentLevel++;
	debugCN(5, kDebugScript, "\n%*svariable: ", getIndentSize(_debugIndentLevel), "");
	ScriptValue *targetVariable = readAndReturnVariable();
	debugCN(5, kDebugScript, "%*svalue: ", getIndentSize(_debugIndentLevel), "");
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
	debugCN(5, kDebugScript, "\n%*slhs: ", getIndentSize(_debugIndentLevel), "");
	ScriptValue value1 = evaluateExpression();
	debugCN(5, kDebugScript, "%*srhs: ", getIndentSize(_debugIndentLevel), "");
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
		debugC(5, kDebugScript, "%*s=> %s", getIndentSize(_debugIndentLevel), "", returnValue.asBool() ? "TRUE" : "FALSE");
	}

	_debugIndentLevel--;
	return returnValue;
}

ScriptValue CodeChunk::evaluateUnaryOperation() {
	// The only supported unary operation seems to be negation.
	_debugIndentLevel++;
	debugCN(5, kDebugScript, "\n%*svalue: ", getIndentSize(_debugIndentLevel), "");
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
		debugCN(5, kDebugScript, "%*sparam %d: ", getIndentSize(_debugIndentLevel), "", i);
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
	debugCN(5, kDebugScript, "%*sself: ", getIndentSize(_debugIndentLevel), "");

	// Evaluate target as an lvalue to get a pointer to the actual variable if there is one.
	ScriptValue methodCallTarget;
	ScriptValue *methodCallTargetPtr = &methodCallTarget;
	evaluateLValue(methodCallTargetPtr);
	Common::Array<ScriptValue> args;
	for (uint i = 0; i < paramCount; i++) {
		debugCN(5, kDebugScript, "%*sparam %d: ", getIndentSize(_debugIndentLevel), "", i);
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
		debugCN(5, kDebugScript, "\n%*scondition: ", getIndentSize(_debugIndentLevel), "");
		ScriptValue condition = evaluateExpression();
		_debugIndentLevel--;
		if (condition.getType() != kScriptValueTypeBool) {
			error("%s: Expected loop condition to be bool, not %s", __func__, scriptValueTypeToStr(condition.getType()));
		}

		if (++iterationCount >= MAX_LOOP_ITERATION_COUNT) {
			error("%s: Exceeded max loop iteration count", __func__);
		}

		if (condition.asBool()) {
			debugC(5, kDebugScript, "%*s=> TRUE (continue loop)", getIndentSize(_debugIndentLevel), "");
			executeNextBlock();
		} else {
			debugC(5, kDebugScript, "%*s=> FALSE (exit loop)", getIndentSize(_debugIndentLevel), "");
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

CodeChunkDecompiler::CodeChunkDecompiler(ParameterReadStream *bytecode, uint indentLevel)
	: _indentLevel(indentLevel), _bytecode(bytecode) {
}

Common::String CodeChunkDecompiler::decompileNextBlock() {
	// Mirrors CodeChunk::executeNextBlock
	Common::String result;
	uint blockSize = _bytecode->readTypedUint32();
	int64 blockStart = _bytecode->pos();
	int64 blockEnd = blockStart + blockSize;

	ExpressionType expressionType = static_cast<ExpressionType>(_bytecode->readTypedUint16());
	while (expressionType != kExpressionTypeEmpty) {
		if (_bytecode->pos() >= blockEnd) {
			warning("%s: Reached end of bytecode stream without finding end of code", __func__);
			break;
		}

		Common::String line = decompileExpression(expressionType);
		if (!line.empty()) {
			result += Common::String::format("%*s", getIndentSize(_indentLevel), "") + line + ";\n";
		}

		if (_bytecode->pos() < blockEnd) {
			expressionType = static_cast<ExpressionType>(_bytecode->readTypedUint16());
		} else {
			break;
		}
	}

	return result;
}

Common::String CodeChunkDecompiler::decompileExpression() {
	// Mirrors CodeChunk::evaluateExpression
	ExpressionType expressionType = static_cast<ExpressionType>(_bytecode->readTypedUint16());
	return decompileExpression(expressionType);
}

Common::String CodeChunkDecompiler::decompileExpression(ExpressionType expressionType) {
	// Mirrors CodeChunk::evaluateExpression
	switch (expressionType) {
	case kExpressionTypeEmpty:
		return "";

	case kExpressionTypeOperation:
		return decompileOperation();

	case kExpressionTypeValue:
		return decompileValue();

	case kExpressionTypeVariable:
		return decompileVariable();

	default:
		return Common::String::format("<unknown_expression_%d>", static_cast<uint>(expressionType));
	}
}

Common::String CodeChunkDecompiler::decompileOperation() {
	// Mirrors CodeChunk::evaluateOperation
	Opcode opcode = static_cast<Opcode>(_bytecode->readTypedUint16());
	Common::String result;

	switch (opcode) {
	case kOpcodeIf:
		return decompileIf();

	case kOpcodeIfElse:
		return decompileIfElse();

	case kOpcodeAssignVariable:
		return decompileAssign();

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
		return decompileBinaryOperation(opcode);

	case kOpcodeNegate:
		return decompileUnaryOperation();

	case kOpcodeCallFunction:
		return decompileFunctionCall(false);

	case kOpcodeCallMethod:
		return decompileMethodCall(false);

	case kOpcodeDeclareLocals:
		return decompileDeclareLocals();

	case kOpcodeReturn:
		return decompileReturn();

	case kOpcodeReturnNoValue:
		return decompileReturnNoValue();

	case kOpcodeWhile:
		return decompileWhileLoop();

	case kOpcodeCallFunctionInVariable:
		return decompileFunctionCall(true);

	case kOpcodeCallMethodInVariable:
		return decompileMethodCall(true);

	default:
		return Common::String::format("<unknown_opcode_%d>", static_cast<uint>(opcode));
	}
}

Common::String CodeChunkDecompiler::decompileValue() {
	// Mirrors CodeChunk::evaluateValue
	OperandType operandType = static_cast<OperandType>(_bytecode->readTypedUint16());

	switch (operandType) {
	case kOperandTypeBool: {
		int b = _bytecode->readTypedByte();
		return b ? "TRUE" : "FALSE";
	}

	case kOperandTypeFloat: {
		double f = _bytecode->readTypedDouble();
		return Common::String::format("%g", f);
	}

	case kOperandTypeInt: {
		int i = _bytecode->readTypedSint32();
		return Common::String::format("%d", i);
	}

	case kOperandTypeString: {
		// This doesn't escape quotes in the string.
		uint size = _bytecode->readTypedUint16();
		Common::String string = _bytecode->readString('\0', size);
		return "\"" + string + "\"";
	}

	case kOperandTypeParamToken: {
		uint paramToken = _bytecode->readTypedUint16();
		return g_engine->formatParamTokenName(paramToken, false);
	}

	case kOperandTypeActorId: {
		uint actorId = _bytecode->readTypedUint16();
		Common::String actorName = g_engine->formatActorName(actorId, true, false);
		return "@" + actorName;
	}

	case kOperandTypeTime: {
		double time = _bytecode->readTypedTime();
		return Common::String::format("%g", time);
	}

	case kOperandTypeVariable: {
		return decompileVariable();
	}

	case kOperandTypeFunctionId: {
		uint functionId = _bytecode->readTypedUint16();
		return g_engine->formatFunctionName(functionId, false);
	}

	case kOperandTypeMethodId: {
		BuiltInMethod methodId = static_cast<BuiltInMethod>(_bytecode->readTypedUint16());
		return Common::String(builtInMethodToStr(methodId));
	}

	default:
		return Common::String::format("<unknown_value_type_%d>", static_cast<uint>(operandType));
	}
}

Common::String CodeChunkDecompiler::decompileVariable() {
	// Mirrors CodeChunk::evaluateVariable and CodeChunk::readAndReturnVariable
	uint id = _bytecode->readTypedUint16();
	VariableScope scope = static_cast<VariableScope>(_bytecode->readTypedUint16());

	switch (scope) {
	case kVariableScopeGlobal:
		// Variable names are NOT prefixed with "@" like actor names are.
		return g_engine->formatVariableName(id, false);

	case kVariableScopeLocal:
		// Locals never have saved names, so just give a generic name.
		return Common::String::format("local_%d", id);

	case kVariableScopeParameter:
		// Params never have saved names, so just give a generic name.
		return Common::String::format("param_%d", id);

	case kVariableScopeIndirectParameter: {
		Common::String indexExpr = decompileExpression();
		return Common::String::format("indirect_param(%d, %s)", id, indexExpr.c_str());
	}

	default:
		return Common::String::format("<unknown_var_scope_%d_%d>", static_cast<uint>(scope), id);
	}
}

Common::String CodeChunkDecompiler::decompileIf() {
	// Mirrors CodeChunk::evaluateIf
	Common::String condition = decompileExpression();
	Common::String result = "if (" + condition + ") then\n";

	// Increase indent level for the block.
	_indentLevel++;
	result += decompileNextBlock();
	_indentLevel--;

	result += Common::String::format("%*s", getIndentSize(_indentLevel), "") + "endif";
	return result;
}

Common::String CodeChunkDecompiler::decompileIfElse() {
	// Mirrors CodeChunk::evaluateIfElse
	Common::String condition = decompileExpression();
	Common::String result = "if (" + condition + ") then\n";

	// Decompile true branch.
	_indentLevel++;
	result += decompileNextBlock();
	_indentLevel--;

	// Decompile false branch.
	_indentLevel++;
	Common::String elseBlock = decompileNextBlock();
	_indentLevel--;

	// Only print "else" if the else block is not empty.
	if (!elseBlock.empty()) {
		result += Common::String::format("%*s", getIndentSize(_indentLevel), "") + "else\n";
		result += elseBlock;
	}

	result += Common::String::format("%*s", getIndentSize(_indentLevel), "") + "endif";
	return result;
}

Common::String CodeChunkDecompiler::decompileAssign() {
	// Mirrors CodeChunk::evaluateAssign
	Common::String variable = decompileVariable();
	Common::String value = decompileExpression();
	return variable + " = " + value;
}

Common::String CodeChunkDecompiler::decompileBinaryOperation(Opcode op) {
	// Mirrors CodeChunk::evaluateBinaryOperation
	Common::String lhs = decompileExpression();
	Common::String rhs = decompileExpression();
	Common::String opStr;

	switch (op) {
	case kOpcodeOr: opStr = " or "; break;
	case kOpcodeXor: opStr = " xor "; break;
	case kOpcodeAnd: opStr = " and "; break;
	case kOpcodeEquals: opStr = " == "; break;
	case kOpcodeNotEquals: opStr = " != "; break;
	case kOpcodeLessThan: opStr = " < "; break;
	case kOpcodeGreaterThan: opStr = " > "; break;
	case kOpcodeLessThanOrEqualTo: opStr = " <= "; break;
	case kOpcodeGreaterThanOrEqualTo: opStr = " >= "; break;
	case kOpcodeAdd: opStr = " + "; break;
	case kOpcodeSubtract: opStr = " - "; break;
	case kOpcodeMultiply: opStr = " * "; break;
	case kOpcodeDivide: opStr = " / "; break;
	case kOpcodeModulo: opStr = " % "; break;
	default: opStr = Common::String::format(" <unknown_op_%u> ", static_cast<uint>(op)); break;
	}

	return "(" + lhs + opStr + rhs + ")";
}

Common::String CodeChunkDecompiler::decompileUnaryOperation() {
	// Mirrors CodeChunk::evaluateUnaryOperation
	Common::String value = decompileExpression();
	return "-" + value;
}

void CodeChunkDecompiler::appendDecompiledParameterList(Common::String &result, uint paramCount) {
	for (uint parameterIndex = 0; parameterIndex < paramCount; ++parameterIndex) {
		if (parameterIndex > 0) {
			result += ", ";
		}
		result += decompileExpression();
	}
}

Common::String CodeChunkDecompiler::decompileFunctionCall(bool isIndirect) {
	// Mirrors CodeChunk::evaluateFunctionCall
	uint functionId;
	uint paramCount;

	if (isIndirect) {
		paramCount = _bytecode->readTypedUint16();
		Common::String functionVar = decompileExpression();
		Common::String result = functionVar + "(";
		appendDecompiledParameterList(result, paramCount);
		result += ")";
		return result;
	} else {
		functionId = _bytecode->readTypedUint16();
		paramCount = _bytecode->readTypedUint16();
		return decompileFunctionCall(functionId, paramCount);
	}
}

Common::String CodeChunkDecompiler::decompileFunctionCall(uint functionId, uint paramCount) {
	// Mirrors CodeChunk::evaluateFunctionCall
	Common::String functionName = g_engine->formatFunctionName(functionId, false);
	Common::String result = functionName + "(";
	appendDecompiledParameterList(result, paramCount);
	result += ")";
	return result;
}

Common::String CodeChunkDecompiler::decompileMethodCall(bool isIndirect) {
	// Mirrors CodeChunk::evaluateMethodCall
	BuiltInMethod method;
	uint paramCount;

	if (isIndirect) {
		paramCount = _bytecode->readTypedUint16();
		Common::String methodVar = decompileExpression();
		Common::String target = decompileExpression();
		Common::String result = target + ".(" + methodVar + ")(";
		appendDecompiledParameterList(result, paramCount);
		result += ")";
		return result;
	} else {
		method = static_cast<BuiltInMethod>(_bytecode->readTypedUint16());
		paramCount = _bytecode->readTypedUint16();
		return decompileMethodCall(method, paramCount);
	}
}

Common::String CodeChunkDecompiler::decompileMethodCall(BuiltInMethod method, uint paramCount) {
	// Mirrors CodeChunk::evaluateMethodCall
	Common::String target = decompileExpression();
	Common::String result = target + "." + builtInMethodToStr(method) + "(";
	appendDecompiledParameterList(result, paramCount);
	result += ")";
	return result;
}

Common::String CodeChunkDecompiler::decompileDeclareLocals() {
	// Mirrors CodeChunk::evaluateDeclareLocals
	uint localCount = _bytecode->readTypedUint16();
	Common::String result = "declare ";
	for (uint i = 0; i < localCount; ++i) {
		if (i > 0) {
			result += ", ";
		}
		result += Common::String::format("local_%d", i + 1);
	}
	return result;
}

Common::String CodeChunkDecompiler::decompileReturn() {
	// Mirrors CodeChunk::evaluateReturn
	Common::String value = decompileExpression();
	return "return " + value;
}

Common::String CodeChunkDecompiler::decompileReturnNoValue() {
	// Mirrors CodeChunk::evaluateReturnNoValue
	return "return";
}

Common::String CodeChunkDecompiler::decompileWhileLoop() {
	// Mirrors CodeChunk::evaluateWhileLoop
	Common::String condition = decompileExpression();
	Common::String result = "while (" + condition + ") do\n";

	// Temporarily increase indent level for the block.
	_indentLevel++;
	result += decompileNextBlock();
	_indentLevel--;

	result += Common::String::format("%*s", getIndentSize(_indentLevel), "") + "endwhile";
	return result;
}

} // End of namespace MediaStation
