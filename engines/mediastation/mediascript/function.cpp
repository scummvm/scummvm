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

#include "common/array.h"

#include "mediastation/datum.h"
#include "mediastation/mediascript/function.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

Function::Function(Chunk &chunk) {
	_fileId = Datum(chunk).u.i;
	_id = Datum(chunk).u.i; // + 19900;
	uint lengthInBytes = Datum(chunk, kDatumTypeUint32_1).u.i;
	debugC(5, kDebugLoading, "Function::Function(): id = 0x%x, size = 0x%x bytes", _id, lengthInBytes);
	_code = new CodeChunk(chunk);
}

Function::~Function() {
	delete _code;
	_code = nullptr;
}

Operand Function::execute(Common::Array<Operand> args) {
	debugC(5, kDebugScript, "\n********** FUNCTION %d **********", _id);
	Operand returnValue = _code->execute(&args);
	debugC(5, kDebugScript, "********** END FUNCTION **********");
	return returnValue;
}

} // End of namespace MediaStation

