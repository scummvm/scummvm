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
	_type = static_cast<EventType>(Datum(chunk).u.i);
	debugC(5, kDebugLoading, "EventHandler::EventHandler(): Type 0x%x (@0x%llx)", static_cast<uint>(_type), static_cast<long long int>(chunk.pos()));
	_argumentType = static_cast<EventHandlerArgumentType>(Datum(chunk).u.i);
	debugC(5, kDebugLoading, "EventHandler::EventHandler(): Argument type 0x%x (@0x%llx)", static_cast<uint>(_argumentType), static_cast<long long int>(chunk.pos()));
	_argumentValue = Datum(chunk);

	if (_argumentType != kNullEventHandlerArgument) {
		uint lengthInBytes = Datum(chunk, kDatumTypeUint32_1).u.i;
		debugC(5, kDebugLoading, "EventHandler::EventHandler(): Null argument type, length = 0x%x (@0x%llx)", lengthInBytes, static_cast<long long int>(chunk.pos()));
	}

	_code = new CodeChunk(chunk);
}

Operand EventHandler::execute(uint assetId) {
	// TODO: The assetId is only passed in for debug visibility, there should be
	// a better way to handle that.
	switch (_argumentType) {
	case kNullEventHandlerArgument: {
		debugC(5, kDebugScript, "\n********** EVENT HANDLER (asset %d) (type = %d) (no argument) **********", assetId, static_cast<uint>(_type));
		break;
	}

	case kAsciiCodeEventHandlerArgument: {
		debugC(5, kDebugScript, "\n********** EVENT HANDLER (asset %d) (type = %d) (ASCII code = %d) **********", assetId, static_cast<uint>(_type), _argumentValue.u.i);
		break;
	}

	case kContextEventHandlerArgument: {
		debugC(5, kDebugScript, "\n********** EVENT HANDLER (asset %d) (type = %d) (context = %d) **********", assetId, static_cast<uint>(_type), _argumentValue.u.i);
		break;
	}

	case kTimeEventHandlerArgument:
	case kUnk1EventHandlerArgument: {
		debugC(5, kDebugScript, "\n********** EVENT HANDLER (asset %d) (type = %d) (time = %f) **********", assetId, static_cast<uint>(_type), _argumentValue.u.f);
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
