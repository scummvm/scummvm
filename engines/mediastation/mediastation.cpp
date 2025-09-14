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

#include "mediastation/mediastation.h"
#include "mediastation/debugchannels.h"
#include "mediastation/detection.h"
#include "mediastation/boot.h"
#include "mediastation/context.h"
#include "mediastation/actor.h"
#include "mediastation/actors/document.h"
#include "mediastation/actors/movie.h"
#include "mediastation/actors/screen.h"
#include "mediastation/actors/palette.h"
#include "mediastation/actors/hotspot.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

MediaStationEngine *g_engine;

MediaStationEngine::MediaStationEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc),
	_randomSource("MediaStation"),
	_spatialEntities(MediaStationEngine::compareActorByZIndex) {
	g_engine = this;

	_gameDataDir = Common::FSNode(ConfMan.getPath("path"));
	SearchMan.addDirectory(_gameDataDir, 0, 3);
	for (uint i = 0; MediaStation::directoryGlobs[i]; i++) {
		Common::String directoryGlob = directoryGlobs[i];
		SearchMan.addSubDirectoryMatching(_gameDataDir, directoryGlob, 0, 5);
	}
}

MediaStationEngine::~MediaStationEngine() {
	delete _displayManager;
	_displayManager = nullptr;

	delete _cursor;
	_cursor = nullptr;

	delete _boot;
	_boot = nullptr;

	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		delete it->_value;
	}
	_loadedContexts.clear();

	// Delete the document actor. The rest are deleted from their contexts.
	delete _actors[0];
}

Actor *MediaStationEngine::getActorById(uint actorId) {
	for (auto actor : _actors) {
		if (actor->id() == actorId) {
			return actor;
		}
	}
	return nullptr;
}

Actor *MediaStationEngine::getActorByChunkReference(uint chunkReference) {
	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		Actor *actor = it->_value->getActorByChunkReference(chunkReference);
		if (actor != nullptr) {
			return actor;
		}
	}
	return nullptr;
}

ScriptFunction *MediaStationEngine::getFunctionById(uint functionId) {
	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		ScriptFunction *function = it->_value->getFunctionById(functionId);
		if (function != nullptr) {
			return function;
		}
	}
	return nullptr;
}

ScriptValue *MediaStationEngine::getVariable(uint variableId) {
	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		ScriptValue *variable = it->_value->getVariable(variableId);
		if (variable != nullptr) {
			return variable;
		}
	}
	return nullptr;
}

uint32 MediaStationEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String MediaStationEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Platform MediaStationEngine::getPlatform() const {
	return _gameDescription->platform;
}

const char *MediaStationEngine::getAppName() const {
	return _gameDescription->filesDescriptions[0].fileName;
}

bool MediaStationEngine::isFirstGenerationEngine() {
	if (_boot == nullptr) {
		error("%s: Attempted to get engine version before BOOT.STM was read", __func__);
	} else {
		return (_boot->_versionInfo.major == 0);
	}
}

Common::Error MediaStationEngine::run() {
	_displayManager = new VideoDisplayManager(this);

	Common::Path bootStmFilepath = Common::Path("BOOT.STM");
	_boot = new Boot(bootStmFilepath);

	if (getPlatform() == Common::kPlatformWindows) {
		_cursor = new WindowsCursorManager(getAppName());
	} else if (getPlatform() == Common::kPlatformMacintosh) {
		_cursor = new MacCursorManager(getAppName());
	} else {
		error("%s: Attempted to use unsupported platform %s", __func__, Common::getPlatformDescription(getPlatform()));
	}
	_cursor->showCursor();

	DocumentActor *document = new DocumentActor;
	_actors.push_back(document);

	if (ConfMan.hasKey("entry_context")) {
		// For development purposes, we can choose to start at an arbitrary context
		// in this title. This might not work in all cases.
		uint entryContextId = ConfMan.get("entry_context").asUint64();
		warning("%s: Starting at user-requested context %d", __func__, entryContextId);
		_requestedScreenBranchId = entryContextId;
	} else {
		_requestedScreenBranchId = _boot->_entryContextId;
	}
	doBranchToScreen();

	while (true) {
		processEvents();
		if (shouldQuit()) {
			break;
		}

		if (!_requestedContextReleaseId.empty()) {
			for (uint contextId : _requestedContextReleaseId) {
				releaseContext(contextId);
			}
			_requestedContextReleaseId.clear();
		}

		if (_requestedScreenBranchId != 0) {
			doBranchToScreen();
			_requestedScreenBranchId = 0;
		}

		debugC(5, kDebugGraphics, "***** START SCREEN UPDATE ***");
		for (Actor *actor : _actors) {
			actor->process();

			if (_needsHotspotRefresh) {
				refreshActiveHotspot();
				_needsHotspotRefresh = false;
			}
		}
		draw();
		debugC(5, kDebugGraphics, "***** END SCREEN UPDATE ***");

		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

void MediaStationEngine::processEvents() {
	while (g_system->getEventManager()->pollEvent(_event)) {
		debugC(9, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		debugC(9, kDebugEvents, "@@@@   Processing events");
		debugC(9, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

		switch (_event.type) {
		case Common::EVENT_QUIT: {
			// TODO: Do any necessary clean-up.
			return;
		}

		case Common::EVENT_MOUSEMOVE: {
			_mousePos = g_system->getEventManager()->getMousePos();
			_needsHotspotRefresh = true;
			break;
		}

		case Common::EVENT_KEYDOWN: {
			// Even though this is a keydown event, we need to look at the mouse position.
			Actor *hotspot = findActorToAcceptMouseEvents();
			if (hotspot != nullptr) {
				debugC(1, kDebugEvents, "EVENT_KEYDOWN (%d): Sent to hotspot %d", _event.kbd.ascii, hotspot->id());
				ScriptValue keyCode;
				keyCode.setToFloat(_event.kbd.ascii);
				hotspot->runEventHandlerIfExists(kKeyDownEvent, keyCode);
			}
			break;
		}

		case Common::EVENT_LBUTTONDOWN: {
			Actor *hotspot = findActorToAcceptMouseEvents();
			if (hotspot != nullptr) {
				debugC(1, kDebugEvents, "EVENT_LBUTTONDOWN (%d, %d): Sent to hotspot %d", _mousePos.x, _mousePos.y, hotspot->id());
				hotspot->runEventHandlerIfExists(kMouseDownEvent);
			}
			break;
		}

		case Common::EVENT_RBUTTONDOWN: {
			// We are using the right button as a quick exit since the Media
			// Station engine doesn't seem to use the right button itself.
			warning("%s: EVENT_RBUTTONDOWN: Quitting for development purposes", __func__);
			quitGame();
			break;
		}

		default:
			break;
		}
	}
}

void MediaStationEngine::setCursor(uint id) {
	// The cursor ID is not a resource ID in the executable, but a numeric ID
	// that's a lookup into BOOT.STM, which gives actual name the name of the
	// resource in the executable.
	if (id != 0) {
		const CursorDeclaration &cursorDeclaration = _boot->_cursorDeclarations.getVal(id);
		_cursor->setCursor(cursorDeclaration._name);
	}
}

void MediaStationEngine::refreshActiveHotspot() {
	Actor *actor = findActorToAcceptMouseEvents();
	if (actor != nullptr && actor->type() != kActorTypeHotspot) {
		return;
	}
	HotspotActor *hotspot = static_cast<HotspotActor *>(actor);
	if (hotspot != _currentHotspot) {
		if (_currentHotspot != nullptr) {
			_currentHotspot->runEventHandlerIfExists(kMouseExitedEvent);
			debugC(5, kDebugEvents, "refreshActiveHotspot(): (%d, %d): Exited hotspot %d", _mousePos.x, _mousePos.y, _currentHotspot->id());
		}
		_currentHotspot = hotspot;
		if (hotspot != nullptr) {
			debugC(5, kDebugEvents, "refreshActiveHotspot(): (%d, %d): Entered hotspot %d", _mousePos.x, _mousePos.y, hotspot->id());
			setCursor(hotspot->_cursorResourceId);
			hotspot->runEventHandlerIfExists(kMouseEnteredEvent);
		} else {
			// There is no hotspot, so set the default cursor for this screen instead.
			setCursor(_currentContext->_screenActor->_cursorResourceId);
		}
	}

	if (hotspot != nullptr) {
		debugC(5, kDebugEvents, "refreshActiveHotspot(): (%d, %d): Sent to hotspot %d", _mousePos.x, _mousePos.y, hotspot->id());
		hotspot->runEventHandlerIfExists(kMouseMovedEvent);
	}
}

void MediaStationEngine::draw() {
	if (!_dirtyRects.empty()) {
		for (Actor *actor : _spatialEntities) {
			if (actor->isSpatialActor()) {
				SpatialEntity *entity = static_cast<SpatialEntity *>(actor);
				if (entity->isVisible()) {
					entity->draw(_dirtyRects);
				}
			}
		}

		_dirtyRects.clear();
	}
	_displayManager->updateScreen();
	_displayManager->doTransitionOnSync();
}

Context *MediaStationEngine::loadContext(uint32 contextId) {
	if (_boot == nullptr) {
		error("%s: Cannot load contexts before BOOT.STM is read", __func__);
	}

	debugC(5, kDebugLoading, "MediaStationEngine::loadContext(): Loading context %d", contextId);
	if (_loadedContexts.contains(contextId)) {
		warning("%s: Context %d already loaded, returning existing context", __func__, contextId);
		return _loadedContexts.getVal(contextId);
	}

	// Get the file ID.
	const SubfileDeclaration &subfileDeclaration = _boot->_subfileDeclarations.getVal(contextId);
	// There are other actors in a subfile too, so we need to make sure we're
	// referencing the screen actor, at the start of the file.
	if (subfileDeclaration._startOffsetInFile != 16) {
		warning("%s: Requested ID wasn't for a context.", __func__);
		return nullptr;
	}
	uint fileId = subfileDeclaration._fileId;

	// Get the filename.
	const FileDeclaration &fileDeclaration = _boot->_fileDeclarations.getVal(fileId);
	Common::Path entryCxtFilepath(fileDeclaration._name);

	// Load any child contexts before we actually load this one. The child
	// contexts must be unloaded explicitly later.
	ContextDeclaration contextDeclaration = _boot->_contextDeclarations.getVal(contextId);
	for (uint childContextId : contextDeclaration._parentContextIds) {
		// The root context is referred to by an ID of 0, regardless of what its
		// actual ID is. The root context is already always loaded.
		if (childContextId != 0) {
			debugC(5, kDebugLoading, "MediaStationEngine::loadContext(): Loading child context %d", childContextId);
			loadContext(childContextId);
		}
	}
	Context *context = new Context(entryCxtFilepath);

	_loadedContexts.setVal(contextId, context);
	return context;
}

void MediaStationEngine::registerActor(Actor *actorToAdd) {
	if (getActorById(actorToAdd->id())) {
		error("%s: Actor with ID 0x%d was already defined in this title", __func__, actorToAdd->id());
	}

	_actors.push_back(actorToAdd);
	if (actorToAdd->isSpatialActor()) {
		_spatialEntities.insert(static_cast<SpatialEntity *>(actorToAdd));
	}
}

void MediaStationEngine::scheduleScreenBranch(uint screenId) {
	_requestedScreenBranchId = screenId;
}

void MediaStationEngine::scheduleContextRelease(uint contextId) {
	_requestedContextReleaseId.push_back(contextId);
}

void MediaStationEngine::doBranchToScreen() {
	if (_currentContext != nullptr) {
		_currentContext->_screenActor->runEventHandlerIfExists(kExitEvent);
		releaseContext(_currentContext->_screenActor->id());
	}

	_currentContext = loadContext(_requestedScreenBranchId);
	_currentHotspot = nullptr;
	_displayManager->setRegisteredPalette(_currentContext->_palette);

	if (_currentContext->_screenActor != nullptr) {
		_currentContext->_screenActor->runEventHandlerIfExists(kEntryEvent);
	}

	_requestedScreenBranchId = 0;
}

void MediaStationEngine::releaseContext(uint32 contextId) {
	debugC(5, kDebugScript, "MediaStationEngine::releaseContext(): Releasing context %d", contextId);
	Context *context = _loadedContexts.getValOrDefault(contextId);
	if (context == nullptr) {
		error("%s: Attempted to unload context %d that is not currently loaded", __func__, contextId);
	}

	// Make sure nothing is still using this context.
	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		uint id = it->_key;
		ContextDeclaration contextDeclaration = _boot->_contextDeclarations.getVal(id);
		for (uint32 childContextId : contextDeclaration._parentContextIds) {
			if (childContextId == contextId) {
				return;
			}
		}
	}

	for (auto it = _actors.begin(); it != _actors.end();) {
		uint actorContextId = (*it)->contextId();
		if (actorContextId == contextId) {
			it = _actors.erase(it);
		} else {
			++it;
		}
	}

	for (auto it = _spatialEntities.begin(); it != _spatialEntities.end();) {
		uint actorContextId = (*it)->contextId();
		if (actorContextId == contextId) {
			it = _spatialEntities.erase(it);
		} else {
			++it;
		}
	}

	delete context;
	_loadedContexts.erase(contextId);
}

Actor *MediaStationEngine::findActorToAcceptMouseEvents() {
	Actor *intersectingActor = nullptr;
	// The z-indices seem to be reversed, so the highest z-index number is
	// actually the lowest actor.
	int lowestZIndex = INT_MAX;

	for (Actor *actor : _actors) {
		if (actor->type() == kActorTypeHotspot) {
			HotspotActor *hotspot = static_cast<HotspotActor *>(actor);
			debugC(5, kDebugGraphics, "findActorToAcceptMouseEvents(): Hotspot %d (z-index %d)", hotspot->id(), hotspot->zIndex());
			if (hotspot->isActive() && hotspot->isInside(_mousePos)) {
				if (hotspot->zIndex() < lowestZIndex) {
					lowestZIndex = hotspot->zIndex();
					intersectingActor = actor;
				}
			}
		}
	}
	return intersectingActor;
}

ScriptValue MediaStationEngine::callBuiltInFunction(BuiltInFunction function, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (function) {
	case kEffectTransitionFunction:
		_displayManager->effectTransition(args);
		return returnValue;

	case kEffectTransitionOnSyncFunction:
		_displayManager->setTransitionOnSync(args);
		return returnValue;

	case kDrawingFunction: {
		// Not entirely sure what this function does, but it seems like a way to
		// call into some drawing functions built into the IBM/Crayola executable.
		warning("%s: Built-in drawing function not implemented", __func__);
		return returnValue;
	}

	case kUnk1Function: {
		warning("%s: Function 10 not implemented", __func__);
		returnValue.setToFloat(1.0);
		return returnValue;
	}

	default:
		error("%s: Got unknown built-in function %s (%d)", __func__, builtInFunctionToStr(function), static_cast<uint>(function));
	}
}

int MediaStationEngine::compareActorByZIndex(const SpatialEntity *a, const SpatialEntity *b) {
	int diff = b->zIndex() - a->zIndex();
	if (diff < 0)
		return -1; // a should come before b
	else if (diff > 0)
		return 1; // b should come before a
	else {
		// If z-indices are equal, compare pointers for stable sort
		return (a < b) ? -1 : 1;
	}
}

} // End of namespace MediaStation
