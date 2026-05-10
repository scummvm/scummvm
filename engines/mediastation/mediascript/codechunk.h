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

#ifndef MEDIASTATION_MEDIASCRIPT_CODECHUNK_H
#define MEDIASTATION_MEDIASCRIPT_CODECHUNK_H

#include "common/array.h"

#include "mediastation/datafile.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

class CodeChunk {
public:
	CodeChunk(ParameterReadStream *bytecode) : _bytecode(bytecode) {};
	~CodeChunk();

	ScriptValue executeNextBlock();
	ScriptValue executeWithArguments(Common::Array<ScriptValue> *args);

private:
	// This is not the number of recursive calls, it is as far is the script call stack is
	// ever allowed to get.
	static const uint MAX_CALL_DEPTH = 0x0f;

	void skipNextBlock();

	ScriptValue evaluateExpression();
	ScriptValue evaluateExpression(ExpressionType expressionType);
	void evaluateLValue(ScriptValue *&targetPtr);
	ScriptValue evaluateOperation();
	ScriptValue evaluateValue();
	ScriptValue evaluateVariable();

	ScriptValue *readAndReturnVariable();

	void evaluateIf();
	void evaluateIfElse();
	ScriptValue evaluateAssign();
	ScriptValue evaluateBinaryOperation(Opcode op);
	ScriptValue evaluateUnaryOperation();
	ScriptValue evaluateFunctionCall(bool isIndirect = false);
	ScriptValue evaluateFunctionCall(uint functionId, uint paramCount);
	ScriptValue evaluateMethodCall(bool isIndirect = false);
	ScriptValue evaluateMethodCall(BuiltInMethod method, uint paramCount);
	void evaluateDeclareLocals();
	void evaluateReturn();
	void evaluateReturnNoValue();
	void evaluateWhileLoop();

	static const uint MAX_LOOP_ITERATION_COUNT = 1000;
	bool _returnImmediately = false;
	ScriptValue _returnValue;
	Common::Array<ScriptValue> _locals;
	Common::Array<ScriptValue> *_args = nullptr;
	ParameterReadStream *_bytecode = nullptr;

	// Debug output indentation tracking.
	uint _debugIndentLevel = 0;
	Common::String makeDebugIndent() const;
};

} // End of namespace MediaStation

#endif
