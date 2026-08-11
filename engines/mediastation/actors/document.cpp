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
		ARGCOUNTMIN(1);
		processBranch(args);
		break;

	case kDocumentQuitMethod:
		ARGCOUNTCHECK(0);
		g_engine->quitGame();
		break;

	case kIsLoadingMethod: {
		ARGCOUNTCHECK(1);
		uint contextId = args[0].asActorId();
		bool isLoading = g_engine->getDocument()->isContextLoadInProgress(contextId);
		returnValue.setToBool(isLoading);
		break;
	}

	case kDocumentSetMultipleStreamsMethod:
	case kDocumentSetMultipleSoundsMethod: {
		ARGCOUNTCHECK(1);
		bool value = args[0].asBool();
		warning("[%s] %s: STUB: %s: %d", debugName(), __func__, builtInMethodToStr(methodId), value);
		break;
	}

	case kDocumentLoadContextMethod: {
		ARGCOUNTCHECK(1);
		ActorEvent event(_id, kContextLoadStartEvent, args[0]);
		g_engine->getEventLoop()->queueEvent(event);
		break;
	}

	case kDocumentReleaseContextMethod: {
		ARGCOUNTCHECK(1);
		ActorEvent event(_id, kContextReleaseStartEvent, args[0]);
		g_engine->getEventLoop()->queueEvent(event);
		break;
	}

	case kDocumentContextIsLoadedMethod: {
		ARGCOUNTCHECK(1);
		uint contextId = args[0].asActorId();

		// Check if the context exists in loaded contexts and is not currently loading.
		Context *context = g_engine->getImtGod()->getContextById(contextId);
		bool contextIsLoading = g_engine->getDocument()->isContextLoadInProgress(contextId);
		bool contextIsLoaded = (context != nullptr) && !contextIsLoading;
		returnValue.setToBool(contextIsLoaded);
		break;
	}

	default:
		returnValue = Actor::callMethod(methodId, args);
	}
	return returnValue;
}

void DocumentActor::onEvent(const ActorEvent &event) {
	switch (event.type) {
	case kScreenBranchEvent: {
		const ScreenBranchEvent &branchEvent = static_cast<const ScreenBranchEvent &>(event);
		g_engine->getDocument()->branchToScreen(branchEvent.screenId, branchEvent.disableScreenAutoUpdate);
		break;
	}

	case kContextLoadStartEvent: {
		uint contextId = event.arg.asActorId();
		g_engine->getDocument()->startContextLoad(contextId);
		break;
	}

	case kContextReleaseStartEvent: {
		uint contextId = event.arg.asActorId();
		if (g_engine->getImtGod()->contextIsLocked(contextId)) {
			break;
		}

		Context *context = g_engine->getImtGod()->getContextById(contextId);
		if (context == nullptr) {
			if (g_engine->getDocument()->isContextLoadQueued(contextId)) {
				g_engine->getDocument()->removeFromContextLoadQueue(contextId);
				g_engine->getDocument()->contextReleaseComplete(contextId);
			} else {
				g_engine->getDocument()->contextAlreadyReleased(contextId);
			}
		} else {
			if (g_engine->getDocument()->isContextLoadInProgress(contextId)) {
				// The original calls interruptFeed here. Not implemented because the
				// streaming logic isn't complete (and might not be needed).
			}
			g_engine->getImtGod()->destroyContext(contextId);
			g_engine->getDocument()->contextReleaseComplete(contextId);
		}
		break;
	}

	default:
		Actor::onEvent(event);
	}
}

void DocumentActor::processBranch(Common::Array<ScriptValue> &args) {
	bool disableScreenAutoUpdate = true;
	uint contextId = args[0].asActorId();
	if (args.size() > 1) {
		uint disablescreenAutoUpdateUpdateParamToken = args[1].asParamToken();
		const uint DISABLE_SCREEN_AUTO_UPDATE_PARAM_TOKEN = 0x708;
		if (disablescreenAutoUpdateUpdateParamToken == DISABLE_SCREEN_AUTO_UPDATE_PARAM_TOKEN) {
			disableScreenAutoUpdate = false;
		}
	}

	ScreenBranchEvent event(_id, contextId, disableScreenAutoUpdate);
	g_engine->getEventLoop()->queueEvent(event);
}

} // End of namespace MediaStation
