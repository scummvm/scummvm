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
	_type = static_cast<EventType>(chunk.readTypedUint16());
	debugC(5, kDebugLoading, "EventHandler::EventHandler(): Type %s (%d) (@0x%llx)",
		eventTypeToStr(_type), static_cast<uint>(_type), static_cast<long long int>(chunk.pos()));

	_argumentValue = ScriptValue(&chunk);
	_code = new CodeChunk(chunk);
}

ScriptValue EventHandler::execute(uint actorId) {
	// TODO: The actorId is only passed in for debug visibility, there should be
	// a better way to handle that.
	Common::String actorAndType = Common::String::format("(actor %d) (type = %s)", actorId, eventTypeToStr(_type));
	Common::String argValue = getDebugHeader();
	debugC(5, kDebugScript, "\n********** EVENT HANDLER %s %s **********", actorAndType.c_str(), argValue.c_str());

	// The only argument that can be provided to an
	// event handler is the _argumentValue.
	ScriptValue returnValue = _code->execute();

	debugC(5, kDebugScript, "********** END EVENT HANDLER %s %s **********", actorAndType.c_str(), argValue.c_str());
	return returnValue;
}

EventHandler::~EventHandler() {
	delete _code;
	_code = nullptr;
}

Common::String EventHandler::getDebugHeader() {
	switch (_argumentValue.getType()) {
	case kScriptValueTypeEmpty:
		return "(no argument)";

	case kScriptValueTypeFloat:
		return Common::String::format("(float = %f)", _argumentValue.asFloat());

	case kScriptValueTypeActorId:
		return Common::String::format("(context = %d)", _argumentValue.asActorId());

	case kScriptValueTypeTime:
		return Common::String::format("(time = %f)", _argumentValue.asTime());

	case kScriptValueTypeParamToken:
		return Common::String::format("(token = %d)", _argumentValue.asParamToken());

	default:
		return Common::String::format("(arg type %s)", scriptValueTypeToStr(_argumentValue.getType()));
	}
}

} // End of namespace MediaStation
