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
#include "common/stream.h"

#include "mediastation/mediascript/variable.h"
#include "mediastation/mediascript/operand.h"
#include "mediastation/mediascript/builtins.h"

namespace MediaStation {

enum InstructionType {
	kInstructionTypeEmpty = 0x0000,
	kInstructionTypeFunctionCall = 0x0067,
	kInstructionTypeOperand = 0x0066,
	kInstructionTypeVariableRef = 0x0065
};

enum Opcode {
	kOpcodeIfElse = 202,
	kOpcodeAssignVariable = 203,
	kOpcodeOr = 204,
	kOpcodeAnd = 206,
	kOpcodeEquals = 207,
	kOpcodeNotEquals = 208,
	kOpcodeLessThan = 209,
	kOpcodeGreaterThan = 210,
	kOpcodeLessThanOrEqualTo = 211,
	kOpcodeGreaterThanOrEqualTo = 212,
	kOpcodeAdd = 213,
	kOpcodeSubtract = 214,
	kOpcodeMultiply = 215,
	kOpcodeDivide = 216,
	kOpcodeModulo = 217,
	kOpcodeUnk2 = 218, // TODO: Likely something with ## constants like ##DOWN?
	kOpcodeCallRoutine = 219,
	// Method calls are like routine calls, but they have an implicit "self"
	// parameter that is always the first. For example:
	//  @self . mouseActivate ( TRUE ) ;
	kOpcodeCallMethod = 220,
	// This seems to appear at the start of a function to declare the number of
	// local variables used in the function. It seems to be the `Declare`
	// keyword. In the observed examples, the number of variables to create is
	// given, then the next instructions are variable assignments for that number
	// of variables.
	kOpcodeDeclareVariables = 221,
	kOpcodeWhile = 224,
	kOpcodeReturn = 222,
	kOpcodeUnk1 = 223
};

enum VariableScope {
	kVariableScopeLocal = 1,
	kVariableScopeParameter = 2,
	kVariableScopeGlobal = 4
};

class CodeChunk {
public:
	CodeChunk(Common::SeekableReadStream &chunk);
	~CodeChunk();

	Operand execute(Common::Array<Operand> *args = nullptr);

private:
	Operand executeNextStatement();
	Operand callBuiltInFunction(BuiltInFunction id, Common::Array<Operand> &args);
	Operand callBuiltInMethod(BuiltInMethod method, Operand self, Common::Array<Operand> &args);
	Operand getVariable(uint32 id, VariableScope scope);
	void putVariable(uint32 id, VariableScope scope, Operand value);

	Common::Array<Operand> _locals;
	Common::Array<Operand> *_args;
	Common::SeekableReadStream *_bytecode;
};

} // End of namespace MediaStation

#endif
