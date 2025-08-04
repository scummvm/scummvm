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

class MediaStationEngine : public Engine {
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
	void processEvents();
	void addDirtyRect(const Common::Rect &rect);
	void draw(bool dirtyOnly = true);

	void registerActor(Actor *actorToAdd);
	void destroyActor(uint actorId);

	void scheduleScreenBranch(uint screenId);
	void scheduleContextRelease(uint contextId);
	void readUnrecognizedFromStream(Chunk &chunk, uint sectionType);
	void releaseContext(uint32 contextId);

	Actor *getActorById(uint actorId);
	SpatialEntity *getSpatialEntityById(uint spatialEntityId);
	Actor *getActorByChunkReference(uint chunkReference);
	ScriptValue *getVariable(uint variableId);
	VideoDisplayManager *getDisplayManager() { return _displayManager; }
	CursorManager *getCursorManager() { return _cursorManager; }
	FunctionManager *getFunctionManager() { return _functionManager; }
	RootStage *getRootStage() { return _stageDirector->getRootStage(); }

	Common::Array<ParameterClient *> _parameterClients;

	SpatialEntity *getMouseInsideHotspot() { return _mouseInsideHotspot; }
	void setMouseInsideHotspot(SpatialEntity *entity) { _mouseInsideHotspot = entity; }
	void clearMouseInsideHotspot() { _mouseInsideHotspot = nullptr; }

	SpatialEntity *getMouseDownHotspot() { return _mouseDownHotspot; }
	void setMouseDownHotspot(SpatialEntity *entity) { _mouseDownHotspot = entity; }
	void clearMouseDownHotspot() { _mouseDownHotspot = nullptr; }

	Common::RandomSource _randomSource;

	Context *_currentContext = nullptr;

	static const uint SCREEN_WIDTH = 640;
	static const uint SCREEN_HEIGHT = 480;

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

	Boot *_boot = nullptr;
	Common::HashMap<uint, Actor *> _actors;
	Common::HashMap<uint, Context *> _loadedContexts;
	SpatialEntity *_mouseInsideHotspot = nullptr;
	SpatialEntity *_mouseDownHotspot = nullptr;
	uint _requestedScreenBranchId = 0;
	Common::Array<uint> _requestedContextReleaseId;

	void initDisplayManager();
	void initCursorManager();
	void initFunctionManager();
	void initDocument();
	void initDeviceOwner();
	void initStageDirector();

	void doBranchToScreen();
	Context *loadContext(uint32 contextId);
};

extern MediaStationEngine *g_engine;
#define SHOULD_QUIT ::MediaStation::g_engine->shouldQuit()

} // End of namespace MediaStation

#endif // MEDIASTATION_H
