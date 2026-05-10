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
	_imtGod->destroyAllContexts();
	delete _deviceOwner;
	// _cacheManager->removeCache();
	delete _stageDirector;
	delete _cursorManager;
	delete _document;
	_imtGod->destroyActor(DocumentActor::DOCUMENT_ACTOR_ID);
	delete _displayManager;
	delete _displayUpdateManager;
	delete _functionManager;
	// delete _printManager;
	delete _imtGod;
	// delete _streamProfiler;
	delete _streamFeedManager;
	// delete _cacheManager;
	delete _timerService;
	delete _eventLoop;
	delete _profile;
}

Actor *ImtGod::getActorById(uint actorId) {
	return _actors.getValOrDefault(actorId);
}

Actor *ImtGod::getActorByIdAndType(uint actorId, ActorType expectedType) {
	Actor *actor = getActorById(actorId);
	if (actor == nullptr) {
		error("[%s] %s: Actor doesn't exist", _vm->formatActorName(actorId).c_str(), __func__);
	} else if (actor->type() != expectedType) {
		error("[%s] %s: Expected type %s, got %s", actor->debugName(), __func__, actorTypeToStr(actor->type()), actorTypeToStr(expectedType));
	}
	return actor;
}

SpatialEntity *ImtGod::getSpatialEntityById(uint spatialEntityId) {
	Actor *actor = getActorById(spatialEntityId);
	if (actor != nullptr) {
		if (!actor->isSpatialActor()) {
			error("[%s] %s: Not a spatial actor", actor->debugName(), __func__);
		}
		return static_cast<SpatialEntity *>(actor);
	}
	return nullptr;
}

ChannelClient *ImtGod::getChannelClientByChannelIdent(uint channelIdent) {
	return _vm->getStreamFeedManager()->channelClientForChannel(channelIdent);
}

ScriptValue *ImtGod::getVariable(uint variableId) {
	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		ScriptValue *variable = it->_value->_variables.getValOrDefault(variableId);
		if (variable != nullptr) {
			return variable;
		}
	}
	return nullptr;
}

Context *ImtGod::getContextById(uint contextId) {
	return _loadedContexts.getValOrDefault(contextId);
}

bool ImtGod::isFirstGenerationEngine() const {
	return _versionInfo.major == 0;
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

bool MediaStationEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher);
}

Common::Error MediaStationEngine::run() {
	_eventLoop = new EventLoop;
	_timerService = new TimerService;
	_streamFeedManager = new StreamFeedManager;
	// _cacheManager = new CacheManager;
	// _streamProfiler = new StreamProfiler;
	_imtGod = new ImtGod(this);
	_deviceOwner = new ImtDeviceOwner;
	_functionManager = new FunctionManager;
	_displayUpdateManager = new DisplayUpdateManager;
	_displayManager = new VideoDisplayManager(this);
	// _printManager = new PrintManager;
	_document = new Document;
	DocumentActor *documentActor = new DocumentActor;
	_imtGod->addConstructedActor(documentActor);
	initCursorManager();
	_stageDirector = new StageDirector;
	_profile = new Profile();
	_profile->load();
	_document->beginTitle();
	_eventLoop->run();
	return Common::kNoError;
}

void MediaStationEngine::initCursorManager() {
	if (getPlatform() == Common::kPlatformWindows) {
		_cursorManager = new WindowsCursorManager(getAppName());
	} else if (getPlatform() == Common::kPlatformMacintosh) {
		_cursorManager = new MacCursorManager(getAppName());
	} else {
		error("%s: Attempted to use unsupported platform %s", __func__, Common::getPlatformDescription(getPlatform()));
	}
	_cursorManager->showCursor();
}

void MediaStationEngine::registerAudioSequence(AudioSequence *sequence) {
	_activeAudioSequences[sequence] = sequence;
}

void MediaStationEngine::unregisterAudioSequence(AudioSequence *sequence) {
	_activeAudioSequences.erase(sequence);
}

void MediaStationEngine::serviceSounds() {
	for (auto it = _activeAudioSequences.begin(); it != _activeAudioSequences.end(); ++it) {
		AudioSequence *sequence = it->_value;
		sequence->service();
	}
}

ImtGod::ImtGod(MediaStationEngine *vm) : _vm(vm) {
	_channelIdent = MKTAG('i', 'g', 'o', 'd');
	registerWithStreamManager();
	setupInitialStreamMap();
}

void ImtGod::setupInitialStreamMap() {
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

ImtGod::~ImtGod() {
	unregisterWithStreamManager();
	destroyAllContexts();
	_contextReferences.clear();
	_streamMap.clear();
	_paramTokenDeclarations.clear();
	_screenReferences.clear();
	_fileMap.clear();
	_actors.clear();
}

void ImtGod::addConstructedActor(Actor *actorToAdd) {
	if (getActorById(actorToAdd->id())) {
		error("[%s] %s: Already defined in this title", actorToAdd->debugName(), __func__);
	}
	_actors.setVal(actorToAdd->id(), actorToAdd);
}

void ImtGod::destroyActor(uint actorId) {
	// This performs the role of both destroyActor and removeConstructedActor in the original.
	Actor *actorToDestroy = getActorById(actorId);
	if (actorToDestroy) {
		delete _actors[actorId];
		_actors.erase(actorId);
	} else {
		warning("[%s] %s: Not currently loaded", _vm->formatActorName(actorId).c_str(), __func__);
	}
}

void ImtGod::registerParameterClient(ParameterClient *client) {
	_parameterClients[client] = client;
}

void ImtGod::unregisterParameterClient(ParameterClient * client) {
	_parameterClients.erase(client);
}

void ImtGod::destroyContext(uint contextId, bool eraseFromLoadedContexts) {
	debugC(5, kDebugScript, "%s: Context %s", __func__, _vm->formatActorName(contextId).c_str());
	Context *context = _loadedContexts.getValOrDefault(contextId);
	if (context == nullptr) {
		warning("%s: Attempted to unload context %s that is not currently loaded", __func__, _vm->formatActorName(contextId).c_str());
		return;
	}

	_vm->getRootStage()->deleteChildrenFromContextId(contextId);
	destroyActorsInContext(contextId);
	_vm->getFunctionManager()->deleteFunctionsForContext(contextId);

	delete context;
	if (eraseFromLoadedContexts) {
		// If we are deleting all contexts at once, we don't want to actually do this,
		// as it will mess up our iterators - the whole structure should be cleared after this.
		_loadedContexts.erase(contextId);
	}
}

void ImtGod::lockContext(uint contextId) {
	_lockedContextIds.setVal(contextId, true);
}

bool ImtGod::contextIsLocked(uint contextId) {
	return _lockedContextIds.contains(contextId);
}

void ImtGod::clearAllContextLocks() {
	_lockedContextIds.clear();
}

void ImtGod::destroyActorsInContext(uint contextId) {
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

void ImtGod::destroyAllContexts() {
	// The original had a bug where loaded assets would not be explicitly freed upon quitting
	// the engine. To avoid these leaks, call the full destroyContext each time.
	for (auto it = _loadedContexts.begin(); it != _loadedContexts.end(); ++it) {
		destroyContext(it->_value->_id, false);
	}
	_loadedContexts.clear();
}

void ImtGod::readUnrecognizedFromStream(Chunk &chunk, uint sectionType) {
	bool paramHandled = false;
	for (auto it = _parameterClients.begin(); it != _parameterClients.end(); ++it) {
		ParameterClient *client = it->_value;
		if (client->attemptToReadFromStream(chunk, sectionType)) {
			paramHandled = true;
			break;
		}
	}

	if (!paramHandled) {
		warning("%s: Parameter %d not handled (0x%llx)", __func__, sectionType, static_cast<long long int>(chunk.pos()));
	}
}

void ImtGod::readChunk(Chunk &chunk) {
	StreamType streamType = static_cast<StreamType>(chunk.readTypedUint16());
	switch (streamType) {
	case kDocumentDefStream:
		readDocumentDef(chunk);
		break;

	case kControlCommandsStream:
		readControlCommands(chunk);
		break;

	default:
		error("%s: Unhandled section type 0x%x (0x%llx)", __func__, static_cast<uint>(streamType), static_cast<long long int>(chunk.pos()));
	}
}

} // End of namespace MediaStation
