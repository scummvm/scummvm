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
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/events.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"

#include "mediastation/clients.h"
#include "mediastation/detection.h"
#include "mediastation/datafile.h"
#include "mediastation/boot.h"
#include "mediastation/context.h"
#include "mediastation/actor.h"
#include "mediastation/cursors.h"
#include "mediastation/graphics.h"
#include "mediastation/mediascript/function.h"
#include "mediastation/actors/stage.h"

namespace MediaStation {

struct MediaStationGameDescription;
class HotspotActor;
class RootStage;
class Bitmap;

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

// As this is currently structured, some of the methods in the main engine class are from
// the RT_ImtGod class in the original, and others are from the RT_App class in the original.
// In the interest of avoiding more indirection than is already present in the original, we will
// just keep these together for now.
class MediaStationEngine : public Engine, public ChannelClient {
public:
	MediaStationEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~MediaStationEngine() override;

	uint32 getFeatures() const;
	Common::String getGameId() const;
	Common::Platform getPlatform() const;
	const char *getAppName() const;
	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsReturnToLauncher);
	};

	bool isFirstGenerationEngine();
	void dispatchSystemEvents();
	void draw(bool dirtyOnly = true);

	void registerActor(Actor *actorToAdd);
	void destroyActor(uint actorId);
	void destroyContext(uint contextId, bool eraseFromLoadedContexts = true);
	bool contextIsLocked(uint contextId);

	void readUnrecognizedFromStream(Chunk &chunk, uint sectionType);
	void readHeaderSections(Subfile &subfile, Chunk &chunk);

	Actor *getActorById(uint actorId);
	SpatialEntity *getSpatialEntityById(uint spatialEntityId);
	ChannelClient *getChannelClientByChannelIdent(uint channelIdent);
	ScriptValue *getVariable(uint variableId);
	VideoDisplayManager *getDisplayManager() { return _displayManager; }
	CursorManager *getCursorManager() { return _cursorManager; }
	FunctionManager *getFunctionManager() { return _functionManager; }
	RootStage *getRootStage() { return _stageDirector->getRootStage(); }
	StreamFeedManager *getStreamFeedManager() { return _streamFeedManager; }
	Document *getDocument() { return _document; }

	const FileInfo &fileInfoForIdent(uint fileId) { return _fileMap.getValOrDefault(fileId); }
	const StreamInfo &streamInfoForIdent(uint streamId) { return _streamMap.getValOrDefault(streamId); }
	const ScreenReference &screenRefWithId(uint screenActorId) { return _screenReferences.getValOrDefault(screenActorId); }
	const ContextReference &contextRefWithId(uint contextId) { return _contextReferences.getValOrDefault(contextId); }

	Common::Array<ParameterClient *> _parameterClients;
	Common::HashMap<uint, Context *> _loadedContexts;

	SpatialEntity *getMouseInsideHotspot() { return _mouseInsideHotspot; }
	void setMouseInsideHotspot(SpatialEntity *entity) { _mouseInsideHotspot = entity; }
	void clearMouseInsideHotspot() { _mouseInsideHotspot = nullptr; }

	SpatialEntity *getMouseDownHotspot() { return _mouseDownHotspot; }
	void setMouseDownHotspot(SpatialEntity *entity) { _mouseDownHotspot = entity; }
	void clearMouseDownHotspot() { _mouseDownHotspot = nullptr; }

	Common::RandomSource _randomSource;

	static const uint SCREEN_WIDTH = 640;
	static const uint SCREEN_HEIGHT = 480;
	static const uint BOOT_STREAM_ID = 1;

protected:
	Common::Error run() override;

private:
	Common::Event _event;
	Common::FSNode _gameDataDir;
	const ADGameDescription *_gameDescription;

	VideoDisplayManager *_displayManager = nullptr;
	CursorManager *_cursorManager = nullptr;
	FunctionManager *_functionManager = nullptr;
	Document *_document = nullptr;
	DeviceOwner *_deviceOwner = nullptr;
	StageDirector *_stageDirector = nullptr;
	StreamFeedManager *_streamFeedManager = nullptr;

	Common::HashMap<uint, Actor *> _actors;
	SpatialEntity *_mouseInsideHotspot = nullptr;
	SpatialEntity *_mouseDownHotspot = nullptr;

	Common::String _gameTitle;
	VersionInfo _versionInfo;
	Common::String _engineInfo;
	Common::String _sourceString;
	Common::HashMap<uint, ContextReference> _contextReferences;
	Common::HashMap<uint, ScreenReference> _screenReferences;
	Common::HashMap<uint, FileInfo> _fileMap;
	Common::HashMap<uint, StreamInfo> _streamMap;
	Common::HashMap<uint, EngineResourceDeclaration> _engineResourceDeclarations;
	uint _unk1 = 0;
	uint _functionTableSize = 0;
	uint _unk3 = 0;

	void initDisplayManager();
	void initCursorManager();
	void initFunctionManager();
	void initDocument();
	void initDeviceOwner();
	void initStageDirector();
	void initStreamFeedManager();
	void setupInitialStreamMap();

	void runEventLoop();

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
	void readContextNameData(Chunk &chunk);

	void destroyActorsInContext(uint contextId);
};

extern MediaStationEngine *g_engine;
#define SHOULD_QUIT ::MediaStation::g_engine->shouldQuit()

} // End of namespace MediaStation

#endif // MEDIASTATION_H
