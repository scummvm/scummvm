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

#include "mediastation/datafile.h"
#include "mediastation/mediascript/variable.h"
#include "mediastation/mediascript/operand.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

class CodeChunk {
public:
	CodeChunk(Common::SeekableReadStream &chunk);
	~CodeChunk();

	Operand execute(Common::Array<Operand> *args = nullptr, Common::Array<Operand> *locals = nullptr);

	static Operand callBuiltInMethod(BuiltInMethod method, Operand &self, Common::Array<Operand> &args);

private:
	Operand executeNextStatement();
	Operand callFunction(uint functionId, uint parameterCount);
	Operand getVariable(uint32 id, VariableScope scope);
	void putVariable(uint32 id, VariableScope scope, Operand &value);

	bool _weOwnLocals = false;
	Common::Array<Operand> *_locals = nullptr;
	Common::Array<Operand> *_args = nullptr;
	Common::SeekableReadStream *_bytecode = nullptr;
};

} // End of namespace MediaStation

#endif
