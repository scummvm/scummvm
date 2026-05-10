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

#include "mediastation/actors/screen.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

void ScreenActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderCursorResourceId:
		_cursorResourceId = chunk.readTypedUint16();
		if (_cursorResourceId != 0) {
			g_engine->getCursorManager()->registerAsPermanent(_cursorResourceId);
		}
		break;

	default:
		Actor::readParameter(chunk, paramType);
	}
}

void ScreenActor::onEvent(const ActorEvent &event) {
	switch (event.type) {
	case kScreenEntryEvent:
		g_engine->getImtGod()->clearAllContextLocks();
		runScriptResponseIfExists(event.type);
		break;


	case kScreenExitEvent:
		runScriptResponseIfExists(event.type);
		break;

	case kContextLoadCompleteEvent:
	case kContextAlreadyLoadedEvent: {
		uint contextId = event.arg.asActorId();
		Context *context = g_engine->getImtGod()->getContextById(contextId);
		if (context != nullptr) {
			runScriptResponseIfExists(event.type, event.arg);
		}
		break;
	}

	case kContextReleaseCompleteEvent:
	case kContextAlreadyReleasedEvent: {
		uint contextId = event.arg.asActorId();
		Context *context = g_engine->getImtGod()->getContextById(contextId);
		if (context == nullptr) {
			runScriptResponseIfExists(event.type, event.arg);
		}
		break;
	}

	default:
		Actor::onEvent(event);
	}
}
} // End of namespace MediaStation
