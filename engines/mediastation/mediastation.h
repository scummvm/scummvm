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

#ifndef MEDIASTATION_H
#define MEDIASTATION_H

#include "audio/mixer.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-ptr.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/events.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"

#include "mediastation/actor.h"
#include "mediastation/actors/stage.h"
#include "mediastation/boot.h"
#include "mediastation/clients.h"
#include "mediastation/context.h"
#include "mediastation/cursors.h"
#include "mediastation/datafile.h"
#include "mediastation/detection.h"
#include "mediastation/events.h"
#include "mediastation/graphics.h"
#include "mediastation/mediascript/function.h"
#include "mediastation/profile.h"

namespace MediaStation {

struct MediaStationGameDescription;
class AudioSequence;
class HotspotActor;
class RootStage;
class PixMapImage;
class ImtGod;
class EventLoop;

// Most Media Station titles follow this file structure from the root directory
// of the CD-ROM:
// - [TITLE].EXE (main game executable, name vares based on game)
// - DATA/ (subdirectory that holds actual game data including bytecode)
//   - 100.CXT
//   - ... other CXTs, varies per title
static const char *const directoryGlobs[] = {
	"DATA", // For most titles
	"program", // For D.W. the Picky Eater
	"PZDATA", // For Puzzle Castle demo
	nullptr
};

class MediaStationEngine : public Engine {
friend class ImtGod;

public:
	MediaStationEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~MediaStationEngine() override;

	uint32 getFeatures() const;
	Common::String getGameId() const;
	Common::Platform getPlatform() const;
	const char *getAppName() const;
	bool hasFeature(EngineFeature f) const override;
	void dispatchOneSystemEvent(const Common::Event &event);

	VideoDisplayManager *getDisplayManager() { return _displayManager; }
	DisplayUpdateManager *getDisplayUpdateManager() { return _displayUpdateManager; }
	CursorManager *getCursorManager() { return _cursorManager; }
	FunctionManager *getFunctionManager() { return _functionManager; }
	RootStage *getRootStage() { return _stageDirector->getRootStage(); }
	StageDirector *getStageDirector() { return _stageDirector; }
	StreamFeedManager *getStreamFeedManager() { return _streamFeedManager; }
	EventLoop *getEventLoop() { return _eventLoop; }
	Document *getDocument() { return _document; }
	TimerService *getTimerService() { return _timerService; }
	ImtGod *getImtGod() { return _imtGod; }

	void registerAudioSequence(AudioSequence *sequence);
	void unregisterAudioSequence(AudioSequence *sequence);
	void serviceSounds();

	Common::String formatActorName(uint actorId, bool attemptToGetType = false) { return _profile->formatActorName(actorId, attemptToGetType); }
	Common::String formatActorName(const Actor *actor) { return _profile->formatActorName(actor); }
	Common::String formatFunctionName(uint functionId) { return _profile->formatFunctionName(functionId); }
	Common::String formatFileName(uint fileId) { return _profile->formatFileName(fileId); }
	Common::String formatVariableName(uint variableId) { return _profile->formatVariableName(variableId); }
	Common::String formatParamTokenName(uint paramToken) { return _profile->formatParamTokenName(paramToken); }
	Common::String formatAssetNameForChannelIdent(uint channelIdent) { return _profile->formatAssetNameForChannelIdent(channelIdent); }

	SpatialEntity *getMouseInsideHotspot() { return _mouseInsideHotspot; }
	void setMouseInsideHotspot(SpatialEntity *entity) { _mouseInsideHotspot = entity; }
	void clearMouseInsideHotspot() { _mouseInsideHotspot = nullptr; }

	SpatialEntity *getMouseDownHotspot() { return _mouseDownHotspot; }
	void setMouseDownHotspot(SpatialEntity *entity) { _mouseDownHotspot = entity; }
	void clearMouseDownHotspot() { _mouseDownHotspot = nullptr; }

	static const uint SCREEN_WIDTH = 640;
	static const uint SCREEN_HEIGHT = 480;
	static const uint BOOT_STREAM_ID = 1;
	Common::RandomSource _randomSource;

protected:
	Common::Error run() override;

private:
	Common::FSNode _gameDataDir;
	const ADGameDescription *_gameDescription;

	SpatialEntity *_mouseInsideHotspot = nullptr;
	SpatialEntity *_mouseDownHotspot = nullptr;

	EventLoop *_eventLoop = nullptr;
	TimerService *_timerService = nullptr;
	StreamFeedManager *_streamFeedManager = nullptr;
	// CacheManager *_cacheManager = nullptr;
	// StreamProfiler *_streamProfiler = nullptr;
	ImtGod *_imtGod = nullptr;
	ImtDeviceOwner *_deviceOwner = nullptr;
	FunctionManager *_functionManager = nullptr;
	DisplayUpdateManager *_displayUpdateManager = nullptr;
	VideoDisplayManager *_displayManager = nullptr;
	// PrintManager *_printManager = nullptr;
	Document *_document = nullptr;
	CursorManager *_cursorManager = nullptr;
	StageDirector *_stageDirector = nullptr;
	Profile *_profile = nullptr;

	Common::HashMap<AudioSequence *, AudioSequence *> _activeAudioSequences;

	void initCursorManager();
	void queueMouseEvent(EventType type, const Common::Point &point);
};

class ImtGod : public ChannelClient {
friend class EventLoop;

public:
	ImtGod(MediaStationEngine *vm);
	virtual ~ImtGod() override;

	void addConstructedActor(Actor *actorToAdd);
	void destroyActor(uint actorId);
	void registerParameterClient(ParameterClient *client);
	void unregisterParameterClient(ParameterClient *client);
	void destroyContext(uint contextId, bool eraseFromLoadedContexts = true);
	void destroyAllContexts();
	void lockContext(uint contextId);
	bool contextIsLocked(uint contextId);
	void clearAllContextLocks();

	void readUnrecognizedFromStream(Chunk &chunk, uint sectionType);
	void readHeaderSections(Subfile &subfile, Chunk &chunk);

	const FileInfo &fileInfoForIdent(uint fileId) { return _fileMap.getValOrDefault(fileId); }
	const StreamInfo &streamInfoForIdent(uint streamId) { return _streamMap.getValOrDefault(streamId); }
	const ScreenReference &screenRefWithId(uint screenActorId) { return _screenReferences.getValOrDefault(screenActorId); }
	const ContextReference &contextRefWithId(uint contextId) { return _contextReferences.getValOrDefault(contextId); }

	Actor *getActorById(uint actorId);
	Actor *getActorByIdAndType(uint actorId, ActorType expectedType);
	SpatialEntity *getSpatialEntityById(uint spatialEntityId);
	ChannelClient *getChannelClientByChannelIdent(uint channelIdent);
	ScriptValue *getVariable(uint variableId);
	Context *getContextById(uint contextId);

	bool isFirstGenerationEngine() const;
	void setupInitialStreamMap();

private:
	MediaStationEngine *_vm = nullptr;

	Common::String _gameTitle;
	VersionInfo _versionInfo;
	Common::String _engineInfo;
	Common::String _sourceString;
	Common::HashMap<uint, Actor *> _actors;
	Common::HashMap<uint, ContextReference> _contextReferences;
	Common::HashMap<uint, ScreenReference> _screenReferences;
	Common::HashMap<uint, FileInfo> _fileMap;
	Common::HashMap<uint, StreamInfo> _streamMap;
	Common::HashMap<Common::String, ScriptValue> _paramTokenDeclarations;
	Common::HashMap<uint, bool> _lockedContextIds;
	Common::HashMap<ParameterClient *, ParameterClient *> _parameterClients;
	Common::HashMap<uint, Context *> _loadedContexts;
	uint _unk1 = 0;
	uint _functionTableSize = 0;
	uint _unk3 = 0;

	virtual void readChunk(Chunk &chunk) override;
	void readDocumentDef(Chunk &chunk);
	void readDocumentInfoFromStream(Chunk &chunk, BootSectionType sectionType);
	void readVersionInfoFromStream(Chunk &chunk);
	void readContextReferencesFromStream(Chunk &chunk);
	void readScreenReferencesFromStream(Chunk &chunk);
	void readAndAddFileMaps(Chunk &chunk);
	void readAndAddStreamMaps(Chunk &chunk);

	void readControlCommands(Chunk &chunk);
	void readCommandFromStream(Chunk &chunk, ContextSectionType sectionType);
	void readCreateContextData(Chunk &chunk);
	void readDestroyContextData(Chunk &chunk);
	void readCreateActorData(Chunk &chunk);
	void readDestroyActorData(Chunk &chunk);
	void readActorLoadComplete(Chunk &chunk);
	void readCreateVariableData(Chunk &chunk);
	void readSetContextName(Chunk &chunk);

	void destroyActorsInContext(uint contextId);
};

extern MediaStationEngine *g_engine;
#define SHOULD_QUIT ::MediaStation::g_engine->shouldQuit()

} // End of namespace MediaStation

#endif // MEDIASTATION_H
