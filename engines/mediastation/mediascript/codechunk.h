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

enum class InstructionType {
	EMPTY = 0x0000,
	FUNCTION_CALL = 0x0067,
	OPERAND = 0x0066,
	VARIABLE_REF = 0x0065
};

enum class Opcode {
	IfElse = 202,
	AssignVariable = 203,
	Or = 204,
	And = 206,
	Equals = 207,
	NotEquals = 208,
	LessThan = 209,
	GreaterThan = 210,
	LessThanOrEqualTo = 211,
	GreaterThanOrEqualTo = 212,
	Add = 213,
	Subtract = 214,
	Multiply = 215,
	Divide = 216,
	Modulo = 217,
	Unk2 = 218, // TODO: Likely something with ## constants like ##DOWN?
	CallRoutine = 219,
	// Method calls are like routine calls, but they have an implicit "self"
	// parameter that is always the first. For example:
	//  @self . mouseActivate ( TRUE ) ;
	CallMethod = 220,
	// This seems to appear at the start of a function to declare the number of
	// local variables used in the function. It seems to be the `Declare`
	// keyword. In the observed examples, the number of variables to create is
	// given, then the next instructions are variable assignments for that number
	// of variables.
	DeclareVariables = 221,
	While = 224,
	Return = 222,
	Unk1 = 223
};

enum class VariableScope {
	Local = 1,
	Parameter = 2,
	Global = 4
};

class CodeChunk {
public:
	CodeChunk(Common::SeekableReadStream &chunk);
	~CodeChunk();

	Operand execute(Common::Array<Operand> *args = nullptr);

private:
	Operand executeNextStatement();
	Operand callBuiltInFunction(uint32 id, Common::Array<Operand> &args);
	Operand callBuiltInMethod(uint32 id, Operand self, Common::Array<Operand> &args);
	Operand getVariable(uint32 id, VariableScope scope);
	void putVariable(uint32 id, VariableScope scope, Operand value);

	Common::Array<Operand> _locals;
	Common::Array<Operand> *_args;
	Common::SeekableReadStream *_bytecode;
};

} // End of namespace MediaStation

#endif
