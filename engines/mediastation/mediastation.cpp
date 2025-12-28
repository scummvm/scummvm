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

	_channelIdent = MKTAG('i', 'g', 'o', 'd'); // ImtGod
}

MediaStationEngine::~MediaStationEngine() {
	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		destroyContext(it->_value->_id, false);
	}
	_loadedContexts.clear();

	// Only delete the document actor.
	// The root stage is deleted from stage director, and
	// the other actors are deleted from their contexts.
	destroyActor(DocumentActor::DOCUMENT_ACTOR_ID);

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

	delete _stageDirector;
	_stageDirector = nullptr;

	unregisterWithStreamManager();
	delete _streamFeedManager;
	_streamFeedManager = nullptr;

	_contextReferences.clear();
	_streamMap.clear();
	_engineResourceDeclarations.clear();
	_screenReferences.clear();
	_fileMap.clear();
	_actors.clear();
}

Actor *MediaStationEngine::getActorById(uint actorId) {
	return _actors.getValOrDefault(actorId);
}

SpatialEntity *MediaStationEngine::getSpatialEntityById(uint spatialEntityId) {
	Actor *actor = getActorById(spatialEntityId);
	if (actor != nullptr) {
		if (!actor->isSpatialActor()) {
			error("%s: Actor %d is not a spatial actor", __func__, spatialEntityId);
		}
		return static_cast<SpatialEntity *>(actor);
	}
	return nullptr;
}

ChannelClient *MediaStationEngine::getChannelClientByChannelIdent(uint channelIdent) {
	return _streamFeedManager->channelClientForChannel(channelIdent);
}

ScriptValue *MediaStationEngine::getVariable(uint variableId) {
	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		ScriptValue *variable = it->_value->_variables.getValOrDefault(variableId);
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
	return _versionInfo.major == 0;
}

Common::Error MediaStationEngine::run() {
	initDisplayManager();
	initCursorManager();
	initFunctionManager();
	initDocument();
	initDeviceOwner();
	initStageDirector();
	initStreamFeedManager();
	setupInitialStreamMap();

	if (ConfMan.hasKey("entry_context")) {
		// For development purposes, we can choose to start at an arbitrary context
		// in this title. This might not work in all cases.
		uint entryContextId = ConfMan.get("entry_context").asUint64();
		warning("%s: Starting at user-requested context %d", __func__, entryContextId);
		_document->beginTitle(entryContextId);
	} else {
		_document->beginTitle();
	}

	runEventLoop();
	return Common::kNoError;
}

void MediaStationEngine::runEventLoop() {
	while (true) {
		dispatchSystemEvents();
		if (shouldQuit()) {
			break;
		}
		_document->process();

		debugC(5, kDebugGraphics, "***** START SCREEN UPDATE ***");
		for (auto it = _actors.begin(); it != _actors.end(); ++it) {
			it->_value->process();
		}
		draw();
		debugC(5, kDebugGraphics, "***** END SCREEN UPDATE ***");

		g_system->delayMillis(10);
	}
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

void MediaStationEngine::initStreamFeedManager() {
	_streamFeedManager = new StreamFeedManager;
	registerWithStreamManager();
}

void MediaStationEngine::setupInitialStreamMap() {
	StreamInfo streamInfo;
	streamInfo._actorId = 0;
	streamInfo._fileId = MediaStationEngine::BOOT_STREAM_ID;
	streamInfo._startOffsetInFile = 0;
	_streamMap.setVal(streamInfo._fileId, streamInfo);

	const Common::String BOOT_STM_FILENAME("BOOT.STM");
	FileInfo fileInfo;
	fileInfo._id = MediaStationEngine::BOOT_STREAM_ID;
	fileInfo._name = BOOT_STM_FILENAME;
	_fileMap.setVal(fileInfo._id, fileInfo);
}

void MediaStationEngine::dispatchSystemEvents() {
	while (g_system->getEventManager()->pollEvent(_event)) {
		debugC(9, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		debugC(9, kDebugEvents, "@@@@   Dispatching system events");
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

void MediaStationEngine::registerActor(Actor *actorToAdd) {
	if (getActorById(actorToAdd->id())) {
		error("%s: Actor with ID 0x%d was already defined in this title", __func__, actorToAdd->id());
	}
	_actors.setVal(actorToAdd->id(), actorToAdd);
}

void MediaStationEngine::destroyActor(uint actorId) {
	Actor *actorToDestroy = getActorById(actorId);
	if (actorToDestroy) {
		delete _actors[actorId];
		_actors.erase(actorId);
	} else {
		warning("%s: Actor %d is not currently loaded", __func__, actorId);
	}
}

void MediaStationEngine::destroyContext(uint contextId, bool eraseFromLoadedContexts) {
	debugC(5, kDebugScript, "%s: Destroying context %d", __func__, contextId);
	Context *context = _loadedContexts.getValOrDefault(contextId);
	if (context == nullptr) {
		error("%s: Attempted to unload context %d that is not currently loaded", __func__, contextId);
	}

	getRootStage()->deleteChildrenFromContextId(contextId);
	destroyActorsInContext(contextId);
	_functionManager->deleteFunctionsForContext(contextId);

	delete context;
	if (eraseFromLoadedContexts) {
		// If we are deleting all contexts at once, we don't want to actually do this,
		// as it will mess up our iterators - the whole structure should be cleared after this.
		_loadedContexts.erase(contextId);
	}
}

bool MediaStationEngine::contextIsLocked(uint contextId) {
	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		uint id = it->_key;
		ContextReference contextReference = _contextReferences.getVal(id);
		for (uint childContextId : contextReference._parentContextIds) {
			if (childContextId == contextId) {
				return true;
			}
		}
	}
	return false;
}

void MediaStationEngine::destroyActorsInContext(uint contextId) {
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
		destroyActor(actorId);
	}
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
		warning("%s: Parameter %d not handled", __func__, sectionType);
	}
}

void MediaStationEngine::readChunk(Chunk &chunk) {
	StreamType streamType = static_cast<StreamType>(chunk.readTypedUint16());
	switch (streamType) {
	case kDocumentDefStream:
		readDocumentDef(chunk);
		break;

	case kControlCommandsStream:
		readControlCommands(chunk);
		break;

	default:
		error("%s: Unhandled section type 0x%x", __func__, static_cast<uint>(streamType));
	}
}

} // End of namespace MediaStation
