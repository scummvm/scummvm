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

#include "mediastation/mediastation.h"
#include "mediastation/actors/document.h"

namespace MediaStation {

const uint MediaStation::DocumentActor::DOCUMENT_ACTOR_ID;

ScriptValue DocumentActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (methodId) {
	case kDocumentBranchToScreenMethod:
		processBranch(args);
		break;

	case kDocumentQuitMethod:
		g_engine->quitGame();
		break;

	case kDocumentContextLoadInProgressMethod: {
		assert(args.size() == 1);
		uint contextId = args[0].asActorId();
		bool isLoading = g_engine->getDocument()->isContextLoadInProgress(contextId);
		returnValue.setToBool(isLoading);
		break;
	}

	case kDocumentSetMultipleStreamsMethod:
	case kDocumentSetMultipleSoundsMethod: {
		assert(args.size() == 1);
		bool value = args[0].asBool();
		warning("%s: STUB: %s: %d", __func__, builtInMethodToStr(methodId), value);
		break;
	}

	case kDocumentLoadContextMethod: {
		assert(args.size() == 1);
		uint contextId = args[0].asActorId();
		g_engine->getDocument()->startContextLoad(contextId);
		break;
	}

	case kDocumentReleaseContextMethod: {
		assert(args.size() == 1);
		uint contextId = args[0].asActorId();
		g_engine->getDocument()->scheduleContextRelease(contextId);
		break;
	}

	case kDocumentContextIsLoadedMethod: {
		assert(args.size() == 1);
		uint contextId = args[0].asActorId();

		// We are looking for the screen actor with the same ID as the context.
		Actor *screenActor = g_engine->getActorById(contextId);
		bool contextIsLoading = g_engine->getDocument()->isContextLoadInProgress(contextId);
		bool contextIsLoaded = (screenActor != nullptr) && !contextIsLoading;
		returnValue.setToBool(contextIsLoaded);
		break;
	}

	default:
		returnValue = Actor::callMethod(methodId, args);
	}
	return returnValue;
}

void DocumentActor::processBranch(Common::Array<ScriptValue> &args) {
	assert(args.size() >= 1);
	uint contextId = args[0].asActorId();
	if (args.size() > 1) {
		bool disableUpdates = static_cast<bool>(args[1].asParamToken());
		if (disableUpdates)
			warning("%s: disableUpdates parameter not handled yet", __func__);
	}

	g_engine->getDocument()->scheduleScreenBranch(contextId);
}

} // End of namespace MediaStation
