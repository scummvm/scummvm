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

#ifndef MACS2_H
#define MACS2_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "common/file.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "macs2/detection.h"
#include "macs2/events.h"
#include "macs2/script/scriptexecutor.h"
#include <common/memstream.h>

namespace Macs2 {

struct Macs2GameDescription;

// enum class CursorMode { Talk = 0, Look = 1, Touch = 2, Walk = 3};
class Adlib;



struct GlyphData {
	byte* Data;
	char ASCII;
	uint16 Width;
	uint16 Height;

	void ReadFromeFile(Common::File &file);
	void ReadFromMemory(Common::MemoryReadStream* stream);
};

struct AnimFrame {
	byte *Data;
	uint16 Width;
	uint16 Height;

	void ReadFromeFile(Common::File &file);
	void ReadFromStream(Common::MemoryReadStream *stream);
	bool PixelHit(const Common::Point &point) const;
	Common::Point GetBottomMiddleOffset() const;
};

struct BackgroundAnimation {
	uint16 numFrames;
	uint16 X;
	uint16 Y;
	AnimFrame *Frames;
	uint32 FrameIndex;
};

struct BackgroundAnimationBlob {
	uint16 X;
	uint16 Y;
	Common::Array<uint8> Blob;
	uint32 FrameIndex;
	AnimFrame GetFrame(uint32 index);
	AnimFrame GetCurrentFrame();
	static uint16 Func1480(Common::Array<uint8>& blob, bool bpp6, uint16 bpp8);
};

struct Sprite {
	uint16 Width;
	uint16 Height;
	Common::Array<uint8> Data;
};

enum DebugFlag {
	DEBUG_RLE = 1 << 10,
	DEBUG_SV = 1 << 11
};

struct PathfindingPoint {
	uint8 Index;
	Common::Point Position;
	Common::Array<uint8> adjacentPoints;
};

class Macs2Engine : public Engine, public Events {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	
	Adlib *_adlib;

protected:
	// Engine APIs
	Common::Error run() override;

	/**
	 * Returns true if the game should quit
	 */
	bool shouldQuit() const override {
		return Engine::shouldQuit();
	}

	// TODO: Switch stream to an LE stream
	Graphics::ManagedSurface readRLEImage(int64 offs, Common::MemoryReadStream *stream);

	void readResourceFile();

	// Assumes that the stream is at the location of the number of background animations
	void ReadBackgroundAnimations(Common::MemoryReadStream *stream);

public:
	Macs2Engine(OSystem *syst, const ADGameDescription *gameDesc);
	~Macs2Engine() override;

	void changeScene(uint32 newSceneIndex, bool executeScript = true);

	Script::ScriptExecutor *_scriptExecutor;
	struct Graphics::ManagedSurface _bgImageShip;
	Graphics::ManagedSurface _map;
	// Note: This is used both for pathfinding as well as for area IDs
	Graphics::ManagedSurface _pathfindingMap;

	// This is the depth map
	Graphics::ManagedSurface _depthMap;

	byte _pal[256 * 3] = { 0 };
	byte _palVanilla[256 * 3] = { 0 };

	Common::Array<Common::String> debugOutput;

	// fn0037_0E8C proc
	uint16 getWalkabilityAt(uint16 x, uint16 y);

	// fn0037_1196
	bool isPathWalkable(uint16 x1, uint16 y1, uint16 x2, uint16 y2);

	uint16 _pathfindingPoints[32];
	Common::Array<PathfindingPoint> pathfindingPoints;

	Common::Array<Common::Point> _path;

	void CalculatePath(const Common::Point &source, const Common::Point &destination);

	byte* _charData;
	char _charASCII;
	uint16 _charWidth;
	uint16 _charHeight;

	GlyphData _glyphs[256];
	// TODO: THis count could be read from the file as well
	uint16 numGlyphs = 79;

	AnimFrame _animFrames[6];
	// TODO: Figure out how the game knows that there are 6 frames - and confirm that there are only 6 frames

	bool FindGlyph(char c, GlyphData &out) const;

	byte** _cursorData;
	uint16* _cursorWidths;
	uint16* _cursorHeights;

	// TODO: Need a data structure for this by now or check if a bitmap with transparent pixels for blitting exists in ScummVM
	byte* _guyData;
	uint16 _guyWidth;
	uint16 _guyHeight;

	Sprite _borderSprite;
	byte* _borderData;
	uint16 _borderWidth;
	uint16 _borderHeight;

	byte* _shadingTable;

	byte* _borderHighlightData;
	uint16 _borderHighlightWidth;
	uint16 _borderHighlightHeight;

	byte** _flagData;
	uint16* _flagWidths;
	uint16* _flagHeights;

	uint16 _numBackgroundAnimations;
	BackgroundAnimation *_backgroundAnimations;
	Common::Array<BackgroundAnimationBlob> _backgroundAnimationsBlobs;

	byte* mapData;

	Common::MemoryReadStream *_fileStream;

	// Common::MemoryReadStream* _stringsStream;
	uint16 numBytesStrings;
	byte *stringsData;

	// CursorMode _cursorMode = CursorMode::Touch;

	void NextCursorMode();

	// Offset 5023h of current scene data
	// TODO: Consider moving somewhere else
	Common::Array<uint8> array5023;

	Common::Array<uint16> array50D5;

	// [51F7h]
	uint16 numPathfindingPoints;

	// Offset 50D3h - This is used in 0037:10C4 to terminate the loop
	uint16 word50D3;

	uint16 GetInteractedBackgroundHotspot(const Common::Point &p);



	
	AnimFrame _stick;
	

	
	void RunScriptExecutor(bool firstRun = false) {
		_scriptExecutor->Run(firstRun);
	}

	bool runScheduled = false;
	// TODO: Feels like this should be more elegantly solved, also check how the game does this
	// Is required for example after a scene change
	bool scheduledRunIsInitScene = false;

	// Schedules a run of the script the next time the executor is ticked
	void ScheduleRun(bool initScene = false);

	uint16 Func0E8C(const Common::Point &p);

	int MeasureString(Common::String &s);

	int MeasureStrings(Common::StringArray sa);

	Common::StringArray DecodeStrings(Common::MemoryReadStream *stream, int offset, int numStrings);

	void PlaySound();

	void OnTimer();

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently() override {
		return true;
	}
	bool canSaveGameStateCurrently() override {
		return true;
	}

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

	virtual bool tick();
};

extern Macs2Engine *g_engine;
#define SHOULD_QUIT ::Macs2::g_engine->shouldQuit()

} // End of namespace Macs2

#endif // MACS2_H
