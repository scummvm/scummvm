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

#include "common/config-manager.h"

#include "mediastation/actors/screen.h"
#include "mediastation/debugchannels.h"
#include "mediastation/clients.h"
#include "mediastation/context.h"
#include "mediastation/mediastation.h"

namespace MediaStation {
ParameterClient::ParameterClient() {
	g_engine->getImtGod()->registerParameterClient(this);
}

ParameterClient::~ParameterClient() {
	g_engine->getImtGod()->unregisterParameterClient(this);
}

bool ImtDeviceOwner::attemptToReadFromStream(Chunk &chunk, uint sectionType) {
	bool handledParam = true;
	switch (sectionType) {
	case kDeviceOwnerAllowMultipleSounds:
		_allowMultipleSounds = chunk.readTypedByte();
		break;

	case kDeviceOwnerAllowMultipleStreams:
		_allowMultipleStreams = chunk.readTypedByte();
		break;

	default:
		handledParam = false;
	}

	return handledParam;
}


bool Document::attemptToReadFromStream(Chunk &chunk, uint sectionType) {
	bool handledParam = true;
	switch (sectionType) {
	case kDocumentContextLoadComplete:
		readContextLoadComplete(chunk);
		break;

	case kDocumentStartupInformation:
		readStartupInformation(chunk);
		break;

	default:
		handledParam = false;
	}

	return handledParam;
}

void Document::readStartupInformation(Chunk &chunk) {
	DocumentSectionType sectionType = static_cast<DocumentSectionType>(chunk.readTypedUint16());
	debugC(5, kDebugLoading, "%s: sectionType = 0x%x", __func__, static_cast<uint>(sectionType));
	switch (sectionType) {
	case kDocumentEntryScreen: {
		uint entryPointScreenId = chunk.readTypedUint16();
		if (_entryPointScreenId == 0) {
			// We don't want to reset the overridden screen entry point.
			_entryPointScreenId = entryPointScreenId;
		}
		break;
	}

	default:
		error("%s: Unhandled section type 0x%x", __func__, static_cast<uint>(sectionType));
	}
}

void Document::readContextLoadComplete(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "%s: Context %d", __func__, contextId);
	if (contextId == _loadingContextId) {
		contextLoadDidComplete();
	}

	if (_loadingScreenActorId != 0) {
		uint loadingScreenActorContextId = contextIdForScreenActorId(_loadingScreenActorId);
		if (contextId == loadingScreenActorContextId) {
			screenLoadDidComplete();
		}
	}
}

void Document::beginTitle() {
	_entryPointStreamId = MediaStationEngine::BOOT_STREAM_ID;
	if (ConfMan.hasKey("entry_context")) {
		// For development purposes, we can choose to start at an arbitrary context
		// in this title. This might not work in all cases.
		_entryPointScreenId = ConfMan.get("entry_context").asUint64();
		_entryPointScreenIdWasOverridden = true;
		warning("%s: Starting at user-requested context %d", __func__, _entryPointScreenId);
	}

	startFeed(_entryPointStreamId);
}


void Document::startContextLoad(uint contextId) {
	debugC(5, kDebugLoading, "%s: Loading context %d", __func__, contextId);
	Context *existingContext = g_engine->getImtGod()->getContextById(contextId);
	if (existingContext == nullptr) {
		if (_loadingContextId == 0) {
			const ContextReference &contextRef = g_engine->getImtGod()->contextRefWithId(contextId);
			if (contextRef._contextId != 0) {
				_loadingContextId = contextId;
				startFeed(contextRef._streamId);
			}
		} else {
			addToContextLoadQueue(contextId);
		}
	} else {
		if (_currentScreenActorId != 0 && contextId != _loadingContextId) {
			ScriptValue arg;
			arg.setToActorId(contextId);
			ActorEvent event(_currentScreenActorId, kContextAlreadyLoadedEvent, arg);
			g_engine->getEventLoop()->queueEvent(event);
		}

		if (_loadingContextId == 0) {
			checkQueuedContextLoads();
		}
	}
}

bool Document::isContextLoadInProgress(uint contextId) {
	if (contextId == 0) {
		// If we don't have a valid context ID, just check if we are loading any context.
		return _loadingContextId != 0;
	} else {
		// If the context ID is valid, check if we are loading specifically that context.
		return contextId == _loadingContextId;
	}
}

void Document::branchToScreen(uint screenId, bool disableScreenAutoUpdate) {
	// This is to support not immediately branching to the wrong screen
	// when we are starting at a user-defined context. This is because the click
	// handler usually points to the main menu screen rather than the screen
	// we're starting at. It would be way too complicated to find the right variable
	// and change it at runtime, so we will just branch to the screen we are already on.
	// (We have to branch to something because by this point we have already faded out the screen and such,
	// so we need to run another screen entry event to bring things back in again.)
	if (_entryPointScreenIdWasOverridden) {
		_entryPointScreenIdWasOverridden = false;
		screenId = _entryPointScreenId;
	}

	if (_loadingScreenActorId == 0) {
		_loadingScreenActorId = screenId;

		if (disableScreenAutoUpdate) {
			_disabledScreenAutoUpdateToken = g_engine->getDisplayUpdateManager()->disableAutoUpdate();
		}

		uint contextId = contextIdForScreenActorId(_loadingScreenActorId);
		if (contextId == 0) {
			error("%s: Screen %d doesn't have a context in current title", __func__, _loadingScreenActorId);
		}

		blowAwayCurrentScreen();
		preloadParentContexts(contextId);
		addToContextLoadQueue(contextId);

		if (_loadingContextId == 0) {
			checkQueuedContextLoads();
		}
	}
}

void Document::streamDidClose(uint streamId) {
	bool currentStreamIsTargetStream = _currentStreamFeed != nullptr && streamId == _currentStreamFeed->_id;
	if (!currentStreamIsTargetStream) {
		return;
	}

	_currentStreamFeed = nullptr;
}

void Document::streamDidFinish(uint streamId) {
	bool currentStreamIsTargetStream = _currentStreamFeed != nullptr && streamId == _currentStreamFeed->_id;
	if (currentStreamIsTargetStream) {
		stopFeed();
		if (streamId == _entryPointStreamId) {
			branchToScreen(_entryPointScreenId, false);
		} else {
			checkQueuedContextLoads();
		}
	}
}

void Document::contextLoadDidComplete() {
	if (_currentScreenActorId != 0) {
		ScriptValue arg;
		arg.setToActorId(_loadingContextId);
		ActorEvent event(_currentScreenActorId, kContextLoadCompleteEvent, arg);
		g_engine->getEventLoop()->queueEvent(event);
	}
	_loadingContextId = 0;
}

void Document::screenLoadDidComplete() {
	_currentScreenActorId = _loadingScreenActorId;
	ActorEvent event(_currentScreenActorId, kScreenEntryEvent);
	g_engine->getEventLoop()->queueEvent(event);
	_loadingScreenActorId = 0;

	if (_disabledScreenAutoUpdateToken != 0) {
		g_engine->getDisplayUpdateManager()->enableAutoUpdate(_disabledScreenAutoUpdateToken);
		_disabledScreenAutoUpdateToken = 0;
	}
}

void Document::blowAwayCurrentScreen() {
	if (_currentScreenActorId != 0) {
		uint contextId = contextIdForScreenActorId(_currentScreenActorId);
		if (contextId != 0) {
			ScreenActor *screenActor = static_cast<ScreenActor *>(g_engine->getImtGod()->getActorByIdAndType(_currentScreenActorId, kActorTypeScreen));
			if (screenActor != nullptr) {
				ActorEvent event(_currentScreenActorId, kScreenExitEvent);
				screenActor->onEvent(event);
			}
			g_engine->getImtGod()->destroyContext(contextId);
			// There is a contextWasDestroyed call in here, but it just
			// does OS-specific memory management which we don't need to replicate.
		}
		_currentScreenActorId = 0;
	}
}

uint Document::contextIdForScreenActorId(uint screenActorId) {
	ScreenReference screenRef = g_engine->getImtGod()->screenRefWithId(screenActorId);
	return screenRef._contextId;
}

void Document::startFeed(uint streamId) {
	// The original had some more stuff here, including cache management and device ownership,
	// but since we don't need these things right now, this function is rather empty.
	_currentStreamFeed = g_engine->getStreamFeedManager()->openStreamFeed(streamId);
	_currentStreamFeed->readData();
}

void Document::stopFeed() {
	_currentStreamFeed->stopFeed();
	g_engine->getStreamFeedManager()->closeStreamFeed(_currentStreamFeed);
	_currentStreamFeed = nullptr;
}

void Document::preloadParentContexts(uint contextId) {
	ContextReference contextReference = g_engine->getImtGod()->contextRefWithId(contextId);
	for (uint parentContextId : contextReference._parentContextIds) {
		if (parentContextId != 0) {
			Context *existingContext = g_engine->getImtGod()->getContextById(parentContextId);
			if (existingContext == nullptr && parentContextId != contextId) {
				debugC(5, kDebugLoading, "%s: Loading parent context %d", __func__, parentContextId);
				addToContextLoadQueue(parentContextId);
			}
			g_engine->getImtGod()->lockContext(parentContextId);
		}
	}
}

void Document::addToContextLoadQueue(uint contextId) {
	if (!isContextLoadQueued(contextId)) {
		_contextLoadQueue.push_back(contextId);
	} else {
		warning("%s: Context %d already queued for load", __func__, contextId);
	}
}

void Document::removeFromContextLoadQueue(uint contextId) {
	for (uint i = 0; i < _contextLoadQueue.size(); i++) {
		if (_contextLoadQueue[i] == contextId) {
			_contextLoadQueue.remove_at(i);
			return;
		}
	}
}

bool Document::isContextLoadQueued(uint contextId) {
	for (uint queuedContextId : _contextLoadQueue) {
		if (queuedContextId == contextId) {
			return true;
		}
	}
	return false;
}

void Document::checkQueuedContextLoads() {
	while (!_contextLoadQueue.empty()) {
		uint contextId = _contextLoadQueue.front();
		_contextLoadQueue.erase(_contextLoadQueue.begin());
		startContextLoad(contextId);
	}
}

void Document::contextReleaseComplete(uint contextId) {
	if (_currentScreenActorId != 0) {
		ScriptValue arg;
		arg.setToActorId(contextId);
		ActorEvent actorEvent(_currentScreenActorId, kContextReleaseCompleteEvent, arg);
		g_engine->getEventLoop()->queueEvent(actorEvent);
	}
}

void Document::contextAlreadyReleased(uint contextId) {
	ScriptValue arg;
	arg.setToActorId(contextId);
	ActorEvent actorEvent(_currentScreenActorId, kContextAlreadyReleasedEvent, arg);
	g_engine->getEventLoop()->queueEvent(actorEvent);
}

} // End of namespace MediaStation
