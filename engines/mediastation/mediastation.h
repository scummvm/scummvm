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
#include "graphics/screen.h"

#include "mediastation/detection.h"
#include "mediastation/datafile.h"
#include "mediastation/boot.h"
#include "mediastation/context.h"
#include "mediastation/asset.h"
#include "mediastation/cursors.h"

namespace MediaStation {

struct MediaStationGameDescription;

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
	void refreshActiveHotspot();
	void redraw();

	void setPalette(Asset *palette);
	void addPlayingAsset(Asset *assetToAdd);

	Asset *getAssetById(uint assetId);
	Asset *getAssetByChunkReference(uint chunkReference);
	Function *getFunctionById(uint functionId);

	Operand callMethod(BuiltInMethod methodId, Common::Array<Operand> &args);
	Operand callBuiltInFunction(BuiltInFunction function, Common::Array<Operand> &args);
	Common::HashMap<uint32, Variable *> _variables;
	Common::RandomSource _randomSource;

	Graphics::Screen *_screen = nullptr;
	Context *_currentContext = nullptr;

	Common::Point _mousePos;
	Common::Array<Common::Rect> _dirtyRects;
	bool _needsHotspotRefresh = false;

	// All Media Station titles run at 640x480.
	const uint16 SCREEN_WIDTH = 640;
	const uint16 SCREEN_HEIGHT = 480;

protected:
	Common::Error run() override;

private:
	Common::Event _event;
	Common::FSNode _gameDataDir;
	const ADGameDescription *_gameDescription;

	// In Media Station, only the cursors are stored in the executable; everything
	// else is in the Context (*.CXT) data files.
	CursorManager *_cursor;
	void setCursor(uint id);

	Boot *_boot = nullptr;
	Common::List<Asset *> _assetsPlaying;
	Common::HashMap<uint, Context *> _loadedContexts;
	Asset *_currentHotspot = nullptr;

	uint _requestedScreenBranchId = 0;
	Common::Array<uint> _requestedContextReleaseId;
	void doBranchToScreen();

	Context *loadContext(uint32 contextId);
	void setPaletteFromHeader(AssetHeader *header);
	void releaseContext(uint32 contextId);
	Asset *findAssetToAcceptMouseEvents();

	void effectTransition(Common::Array<Operand> &args);
};

extern MediaStationEngine *g_engine;
#define SHOULD_QUIT ::MediaStation::g_engine->shouldQuit()

} // End of namespace MediaStation

#endif // MEDIASTATION_H
