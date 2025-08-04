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
#include "mediastation/actors/stage.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

MediaStationEngine *g_engine;

MediaStationEngine::MediaStationEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc),
	_randomSource("MediaStation") {
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

	delete _cursorManager;
	_cursorManager = nullptr;

	delete _functionManager;
	_functionManager = nullptr;

	delete _document;
	_document = nullptr;

	delete _deviceOwner;
	_deviceOwner = nullptr;

	delete _boot;
	_boot = nullptr;

	delete _stageDirector;
	_stageDirector = nullptr;

	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		delete it->_value;
	}
	_loadedContexts.clear();

	// Only delete the document actor.
	// The root stage is deleted from stage director, and
	// the other actors are deleted from their contexts.
	delete _actors.getVal(1);
	_actors.clear();
}

Actor *MediaStationEngine::getActorById(uint actorId) {
	return _actors.getValOrDefault(actorId);
}

SpatialEntity *MediaStationEngine::getSpatialEntityById(uint spatialEntityId){
	Actor *actor = getActorById(spatialEntityId);
	if (actor != nullptr) {
		if (!actor->isSpatialActor()) {
			error("%s: Actor %d is not a spatial actor", __func__, spatialEntityId);
		}
		return static_cast<SpatialEntity *>(actor);
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

void MediaStationEngine::addDirtyRect(const Common::Rect &rect) {
	getRootStage()->invalidateRect(rect);
}

Common::Error MediaStationEngine::run() {
	initDisplayManager();
	initCursorManager();
	initFunctionManager();
	initDocument();
	initDeviceOwner();
	initStageDirector();

	Common::Path bootStmFilepath = Common::Path("BOOT.STM");
	_boot = new Boot(bootStmFilepath);

	if (ConfMan.hasKey("entry_context")) {
		// For development purposes, we can choose to start at an arbitrary context
		// in this title. This might not work in all cases.
		uint entryContextId = ConfMan.get("entry_context").asUint64();
		warning("%s: Starting at user-requested context %d", __func__, entryContextId);
		_requestedScreenBranchId = entryContextId;
	} else {
		_requestedScreenBranchId = _document->_entryScreenId;
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
		for (auto it = _actors.begin(); it != _actors.end(); ++it) {
			it->_value->process();
		}
		draw();
		debugC(5, kDebugGraphics, "***** END SCREEN UPDATE ***");

		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

void MediaStationEngine::initDisplayManager() {
	_displayManager = new VideoDisplayManager(this);
	_parameterClients.push_back(_displayManager);
}

void MediaStationEngine::initCursorManager() {
	if (getPlatform() == Common::kPlatformWindows) {
		_cursorManager = new WindowsCursorManager(getAppName());
	} else if (getPlatform() == Common::kPlatformMacintosh) {
		_cursorManager = new MacCursorManager(getAppName());
	} else {
		error("%s: Attempted to use unsupported platform %s", __func__, Common::getPlatformDescription(getPlatform()));
	}
	_parameterClients.push_back(_cursorManager);
	_cursorManager->showCursor();
}

void MediaStationEngine::initFunctionManager() {
	_functionManager = new FunctionManager();
	_parameterClients.push_back(_functionManager);
}

void MediaStationEngine::initDocument() {
	_document = new Document();
	_parameterClients.push_back(_document);

	DocumentActor *documentActor = new DocumentActor;
	registerActor(documentActor);
}

void MediaStationEngine::initDeviceOwner() {
	_deviceOwner = new DeviceOwner();
	_parameterClients.push_back(_deviceOwner);
}

void MediaStationEngine::initStageDirector() {
	_stageDirector = new StageDirector;
}

void MediaStationEngine::processEvents() {
	while (g_system->getEventManager()->pollEvent(_event)) {
		debugC(9, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		debugC(9, kDebugEvents, "@@@@   Processing events");
		debugC(9, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

		switch (_event.type) {
		case Common::EVENT_MOUSEMOVE:
			_stageDirector->handleMouseMovedEvent(_event);
			break;

		case Common::EVENT_KEYDOWN:
			_stageDirector->handleKeyboardEvent(_event);
			break;

		case Common::EVENT_LBUTTONDOWN:
			_stageDirector->handleMouseDownEvent(_event);
			break;

		case Common::EVENT_LBUTTONUP:
			_stageDirector->handleMouseUpEvent(_event);
			break;

		case Common::EVENT_FOCUS_LOST:
			_stageDirector->handleMouseOutOfFocusEvent(_event);
			break;

		case Common::EVENT_RBUTTONDOWN:
			// We are using the right button as a quick exit since the Media
			// Station engine doesn't seem to use the right button itself.
			warning("%s: EVENT_RBUTTONDOWN: Quitting for development purposes", __func__);
			quitGame();
			break;

		default:
			// Avoid warnings about unimplemented cases by having an explicit
			// default case.
			break;
		}
	}
}

void MediaStationEngine::draw(bool dirtyOnly) {
	if (dirtyOnly) {
		_stageDirector->drawDirtyRegion();
	} else {
		_stageDirector->drawAll();
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
	const SubfileDeclaration &subfileDeclaration = _boot->_streamMap.getValOrDefault(contextId);
	// There are other actors in a subfile too, so we need to make sure we're
	// referencing the screen actor, at the start of the file.
	if (subfileDeclaration._startOffsetInFile != 16) {
		warning("%s: Requested ID wasn't for a context.", __func__);
		return nullptr;
	}
	uint fileId = subfileDeclaration._fileId;

	// Get the filename.
	const FileDeclaration &fileDeclaration = _boot->_fileMap.getVal(fileId);
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
	_actors.setVal(actorToAdd->id(), actorToAdd);
}

void MediaStationEngine::destroyActor(uint actorId) {
	Actor *actorToDestroy = getActorById(actorId);
	if (actorToDestroy) {
		_actors.erase(actorId);
		// The actor will actually be deleted when the context is destroyed.
	} else {
		warning("%s: Actor %d is not currently loaded", __func__, actorId);
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

	// Collect actors to remove first, then delete them.
	// This is necessary because calling erase on a hashmap invalidates
	// the iterators, so collecting them all first makes more sense.
	Common::Array<uint> actorsToRemove;
	for (auto it = _actors.begin(); it != _actors.end(); ++it) {
		uint actorContextId = it->_value->contextId();
		if (actorContextId == contextId) {
			actorsToRemove.push_back(it->_key);
		}
	}

	// Now remove the collected actors.
	for (uint actorId : actorsToRemove) {
		_actors.erase(actorId);
	}

	getRootStage()->deleteChildrenFromContextId(contextId);
	_functionManager->deleteFunctionsForContext(contextId);

	delete context;
	_loadedContexts.erase(contextId);
}

void MediaStationEngine::readUnrecognizedFromStream(Chunk &chunk, uint sectionType) {
	bool paramHandled = false;
	for (ParameterClient *client : g_engine->_parameterClients) {
		if (client->attemptToReadFromStream(chunk, sectionType)) {
			paramHandled = true;
			break;
		}
	}

	if (!paramHandled) {
		error("%s: Unhandled section type 0x%x", __func__, static_cast<uint>(sectionType));
	}
}

} // End of namespace MediaStation
