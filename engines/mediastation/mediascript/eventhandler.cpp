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

#include "mediastation/mediascript/eventhandler.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

EventHandler::EventHandler(Chunk &chunk) {
	_type = (EventHandler::Type)(Datum(chunk).u.i);
	debugC(5, kDebugLoading, "EventHandler::EventHandler(): Type 0x%x (@0x%llx)", _type, static_cast<long long int>(chunk.pos()));
	_argumentType = (EventHandler::ArgumentType)(Datum(chunk).u.i);
	debugC(5, kDebugLoading, "EventHandler::EventHandler(): Argument type 0x%x (@0x%llx)", _argumentType, static_cast<long long int>(chunk.pos()));
	_argumentValue = Datum(chunk);

	if (_argumentType != EventHandler::ArgumentType::Null) {
		uint lengthInBytes = Datum(chunk, DatumType::UINT32_1).u.i;
		debugC(5, kDebugLoading, "EventHandler::EventHandler(): Null argument type, length = 0x%x (@0x%llx)", lengthInBytes, static_cast<long long int>(chunk.pos()));
	}

	_code = new CodeChunk(chunk);
}

Operand EventHandler::execute(uint assetId) {
	// TODO: The assetId is only passed in for debug visibility, there should be
	// a better way to handle that.
	switch (_argumentType) {
	case EventHandler::ArgumentType::Null: {
		debugC(5, kDebugScript, "\n********** EVENT HANDLER (asset %d) (type = %d) (no argument) **********", assetId, (uint)_type);
		break;
	}

	case EventHandler::ArgumentType::AsciiCode: {
		debugC(5, kDebugScript, "\n********** EVENT HANDLER (asset %d) (type = %d) (ASCII code = %d) **********", assetId, (uint)_type, _argumentValue.u.i);
		break;
	}

	case EventHandler::ArgumentType::Context: {
		debugC(5, kDebugScript, "\n********** EVENT HANDLER (asset %d) (type = %d) (context = %d) **********", assetId, (uint)_type, _argumentValue.u.i);
		break;
	}

	case EventHandler::ArgumentType::Time:
	case EventHandler::ArgumentType::Unk1: {
		debugC(5, kDebugScript, "\n********** EVENT HANDLER (asset %d) (type = %d) (time = %f) **********", assetId, (uint)_type, _argumentValue.u.f);
		break;
	}
	}

	// The only argument that can be provided to an event handler is the
	// _argumentValue.
	Operand returnValue = _code->execute();
	debugC(5, kDebugScript, "********** END EVENT HANDLER **********");
	return returnValue;
}

EventHandler::~EventHandler() {
	delete _code;
	_code = nullptr;
}

} // End of namespace MediaStation
