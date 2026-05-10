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

#include "common/memstream.h"

#include "mediastation/mediascript/scriptresponse.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

ScriptResponse::ScriptResponse(Chunk &chunk) {
	_type = static_cast<EventType>(chunk.readTypedUint16());
	_argumentValue = ScriptValue(&chunk);
	_bytecodeSize = chunk.readTypedUint32();
	debugC(5, kDebugLoading, "%s: %s (%d) [%d bytes]",
		__func__, eventTypeToStr(_type), static_cast<uint>(_type), _bytecodeSize);

	// Store bytecode as a flat buffer rather than a stream, so we can create
	// fresh streams for each execution (necessary for recursive function calls).
	_bytecodeBuffer = static_cast<byte *>(malloc(_bytecodeSize));
	chunk.read(_bytecodeBuffer, _bytecodeSize);
}

ScriptValue ScriptResponse::execute(uint actorId) {
	// TODO: The actorId is only passed in for debug visibility, there should be
	// a better way to handle that.
	Common::String actorName = g_engine->formatActorName(actorId, true);
	Common::String actorAndType = Common::String::format("%s (%s)", actorName.c_str(), eventTypeToStr(_type));
	Common::String argValue = Common::String::format("(%s)", _argumentValue.getDebugString().c_str());
	debugC(5, kDebugScript, "\n********** SCRIPT RESPONSE %s %s **********", actorAndType.c_str(), argValue.c_str());

	// Create a new stream for this execution to avoid conflicts with recursive calls.
	Common::SeekableReadStream *baseStream = new Common::MemoryReadStream(_bytecodeBuffer, _bytecodeSize, DisposeAfterUse::NO);
	ParameterReadStream *bytecodeStream = static_cast<ParameterReadStream *>(baseStream);
	CodeChunk code(bytecodeStream);
	ScriptValue returnValue = code.executeNextBlock();
	delete bytecodeStream;

	debugC(5, kDebugScript, "********** END SCRIPT RESPONSE %s %s **********", actorAndType.c_str(), argValue.c_str());
	return returnValue;
}

ScriptResponse::~ScriptResponse() {
	free(_bytecodeBuffer);
	_bytecodeBuffer = nullptr;
}

int64 ScriptResponse::lengthInBytes() const {
	return _bytecodeSize;
}

} // End of namespace MediaStation
