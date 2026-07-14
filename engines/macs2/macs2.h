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

#ifndef MACS2_MACS2_H
#define MACS2_MACS2_H

#include "advancedDetector.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/array.h"
#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/hashmap.h"
#include "common/memstream.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/text-to-speech.h"
#include "common/util.h"
#include "engines/engine.h"
#include "macs2/events.h"
#include "macs2/macs2_constants.h"
#include "macs2/scriptexecutor.h"

namespace Macs2 {

class MacsAudioStream : public Audio::SeekableAudioStream {
public:
	Common::Array<byte> _data;
	int64 _pos;

	virtual ~MacsAudioStream() {}

	/**
	 * Fill the given buffer with up to @p numSamples samples.
	 *
	 * Data must be in native endianness, 16 bits per sample, signed. For stereo
	 * stream, the buffer will be filled with interleaved left and right channel
	 * samples, starting with the left sample. Furthermore, the samples in the
	 * left and right are summed up. So if you request 4 samples from a stereo
	 * stream, you will get a total of two left channel and two right channel
	 * samples.
	 *
	 * @return The actual number of samples read, or -1 if a critical error occurred.
	 *
	 * @note You *must* check whether the returned value is less than what you requested.
	 *       This indicates that the stream is fully used up.
	 *
	 */
	virtual int readBuffer(int16 *buffer, const int numSamples);

	/** Check whether this is a stereo stream. */
	virtual bool isStereo() const;

	/** Sample rate of the stream. */
	virtual int getRate() const;

	/**
	 * Check whether end of data has been reached.
	 *
	 * If this returns true, it indicates that at this time there is no data
	 * available in the stream. However, there might be more data in the future.
	 *
	 * This is used by e.g. a rate converter to decide whether to keep on
	 * converting data or to stop.
	 */
	virtual bool endOfData() const;

	virtual bool seek(const Audio::Timestamp &where);

	virtual Audio::Timestamp getLength() const;
};

struct Macs2GameDescription;

// enum class CursorMode { Talk = 0, Look = 1, Touch = 2, Walk = 3};
class Music;

struct Sprite {
	uint16 _width = 0;
	uint16 _height = 0;
	Common::Array<uint8> _data;
};

struct GlyphData : public Sprite {
	char _ascii = 0;

	void readFromeFile(Common::File &file);
	void readFromMemory(Common::MemoryReadStream *stream);
};

struct AnimFrame : public Sprite {
	int16 _offsetX = 0;
	int16 _offsetY = 0;

	void readFromeFile(Common::File &file);
	void readFromStream(Common::MemoryReadStream *stream);
	bool pixelHit(const Common::Point &point) const;
	Common::Point getBottomMiddleOffset(uint16 scale = 100) const;
};

struct BackgroundAnimation {
	uint16 _x;
	uint16 _y;
	Common::Array<AnimFrame> _frames;
	uint32 _frameIndex;
};

struct BackgroundAnimationBlob {
	uint16 _x;
	uint16 _y;
	Common::Array<uint8> _blob;
	uint16 _unknown0C = 0; // +0x50F3: purpose unknown (word, read from file, not used at runtime)
	uint8 _unknown0E = 0;  // +0x50F5: purpose unknown (byte, read from file, not used at runtime)
	uint8 _unknown0F = 0;  // +0x50F6: purpose unknown (byte, read from file, not used at runtime)
	AnimFrame getCurrentFrame();
	static uint16 advanceAnimFrame(Common::Array<uint8> &blob, bool bpp6, uint16 bpp8);
	static uint16 getAnimFrameCount(Common::Array<uint8> &blob);
	// Mirrors (horizontally flips) all frames in an animation blob in-place.
	// Matches binary decodeAnimBlob (1010:184d) which calls the row-flip at 1010:1319.
	static void mirrorAnimBlob(Common::Array<uint8> &blob);
};

/**
 * Read-only structured view over an animation blob's raw byte array.
 *
 * Blob binary layout (from advanceAnimFrame at 1010:1480):
 *   Header (12 bytes):
 *     +0x00: uint16 unknown (preserved across save/load)
 *     +0x02: uint16 sequencePosition (1-based index into sequence table)
 *     +0x04: uint16 repeatCounter
 *     +0x06: uint16 loopStartPosition
 *     +0x08: uint16 delayCounter
 *     +0x0A: uint16 sequenceLengthMinusOne (actual length = value + 1)
 *   Sequence table (sequenceLength - 1 bytes at offset 0x0C):
 *     Commands: 1=repeat(N), 2=delay(N), 3=jump(pos), >=10: frame index (value-10)
 *   Frame data (at offset 0x0B + sequenceLength):
 *     uint16 frameCount
 *     Per frame: int16 offsetX, int16 offsetY, uint16 unknown, uint16 width, uint16 height, byte[w*h] pixels
 */
struct AnimBlobView {
	const Common::Array<uint8> &_blob;

	explicit AnimBlobView(const Common::Array<uint8> &blob) : _blob(blob) {}

	bool isValid() const { return _blob.size() >= 14; } // header(12) + at least 2 bytes frame count

	// Header fields
	uint16 sequencePosition() const { return READ_LE_UINT16(&_blob[0x02]); }
	uint16 repeatCounter() const { return READ_LE_UINT16(&_blob[0x04]); }
	uint16 loopStartPosition() const { return READ_LE_UINT16(&_blob[0x06]); }
	uint16 delayCounter() const { return READ_LE_UINT16(&_blob[0x08]); }
	uint16 sequenceLength() const { return READ_LE_UINT16(&_blob[0x0A]) + 1; }

	// Derived offsets
	uint32 frameDataOffset() const { return 0x0B + sequenceLength(); }
	uint16 frameCount() const {
		uint32 off = frameDataOffset();
		if (off + 2 > _blob.size())
			return 0;
		return READ_LE_UINT16(&_blob[off]);
	}

	// Get frame info at index (0-based). Returns false if out of bounds.
	struct FrameInfo {
		int16 offsetX;
		int16 offsetY;
		uint16 unknown;
		uint16 width;
		uint16 height;
		const byte *pixels; // pointer into blob data
	};

	bool getFrameInfo(uint16 index, FrameInfo &out) const {
		uint32 pos = frameDataOffset() + 2; // skip frame count word
		for (uint16 i = 0; i <= index; i++) {
			if (pos + 10 > _blob.size())
				return false;
			int16 ox = (int16)READ_LE_UINT16(&_blob[pos]);
			int16 oy = (int16)READ_LE_UINT16(&_blob[pos + 2]);
			uint16 unk = READ_LE_UINT16(&_blob[pos + 4]);
			uint16 w = READ_LE_UINT16(&_blob[pos + 6]);
			uint16 h = READ_LE_UINT16(&_blob[pos + 8]);
			pos += 10;
			if (w == 0 || h == 0 || pos + (uint32)w * h > _blob.size())
				return false;
			if (i == index) {
				out = {ox, oy, unk, w, h, &_blob[pos]};
				return true;
			}
			pos += (uint32)w * h;
		}
		return false;
	}
};

struct PathfindingPoint {
	uint8 _index;
	Common::Point _position;
	Common::Array<uint8> _adjacentPoints;
};

struct PathfindingAreaOverride {
	bool _active;
	uint16 _index;
	uint16 _overrideValue;
};

// Area override table at scene+0x4EA8 (indexed by pathfinding value 0xC8..0xEF)
// Set by opcode 0x4D, read by getAreaAtPoint (1008:101d)
#define AREA_OVERRIDE_MIN 200
#define AREA_OVERRIDE_MAX 239
#define AREA_OVERRIDE_COUNT (AREA_OVERRIDE_MAX - AREA_OVERRIDE_MIN + 1)

class Macs2Engine : public Engine, public Events {
private:
	const ADGameDescription *_gameDescription;

	Music *_adlib = nullptr;

protected:
	// Engine APIs
	Common::Error run() override;

	/**
	 * Returns true if the game should quit
	 */
	bool shouldQuit() const override {
		return Engine::shouldQuit();
	}

public:
	Graphics::ManagedSurface readRLEImage(int64 offs, Common::MemoryReadStream *stream);

	void readResourceFile();

	// We also need some data from the executable, specifically embedded
	// Adlib data
	void readExecutable();

	// Assumes that the stream is at the location of the number of background animations
	void readBackgroundAnimations(Common::MemoryReadStream *stream);

	// Assumes that the stream is at the start of the right section
	void readImageResources(Common::MemoryReadStream *stream);

public:
	Macs2Engine(OSystem *osystem, const ADGameDescription *gameDesc);
	~Macs2Engine() override;

	void changeScene(uint32 newSceneIndex, bool executeScript = true);

	Script::ScriptExecutor *_scriptExecutor = nullptr;
	Graphics::ManagedSurface _sceneBackground;
	Graphics::ManagedSurface _hotspotMap;

	// File offset to the map mode image for the current scene (scene table entry +8).
	// When 0, the map mode is unavailable for this scene.
	uint32 _mapImageFileOffset = 0;

	// Per-depth sub-scene file offsets for map mode preview (binary: scene+0x5DD7+depth*4).
	// File position where the sub-scene offset table starts (after map depth map).
	int64 _mapSubSceneTableFilePos = 0;

	// This is the depth map
	Graphics::ManagedSurface _depthMap;

	// Shadow/shading intensity map (scene+0x301B). Per-pixel values 0-32
	// control character sprite darkening via the shading table.
	// Only scenes with shadow regions have non-zero data.
	Graphics::ManagedSurface _shadowMap;

	// TODO: use Graphics::Palette for this
	byte _pal[256 * 3] = {0};
	byte _palVanilla[256 * 3] = {0};

	Common::Array<Common::String> _debugOutput;
	Common::Array<Common::String> _textLog;

	// Note: This is used both for pathfinding as well as for area IDs
	Graphics::ManagedSurface _pathfindingMap;

	Common::Array<PathfindingAreaOverride> _pathfindingOverrides;
	// Area override table at scene+value*5+0x4EA8 (for getAreaAtPoint)
	uint16 _areaOverrides[AREA_OVERRIDE_COUNT] = {0};
	uint16 _pathfindingPoints[32];
	Common::Array<PathfindingPoint> pathfindingPoints;
	Common::Array<Common::Point> _path;

	bool getPathfindingOverride(uint16 index, uint16 &result);
	void setPathfindingOverride(uint16 index, uint16 overrideValue);

	// Walkability threshold 0xC8 uses signed 16-bit comparison in the binary (JL/JGE).
	// Values with (int16)value < 0xC8 are walkable heights; e.g. -2 (0xFFFE) is walkable.
	static inline bool isWalkabilityBlocking(uint16 value) {
		return (int16)value >= 0xC8;
	}
	static inline bool isWalkabilityWalkable(uint16 value) {
		return (int16)value < 0xC8;
	}

	// This one implements the lookup relative to es:[di+4EA8h] vs. the other one at es:[di+4EA5h] and es:[di+4EA6h]
	uint16 getPathfindingOverride2(uint16 index);
	void removePathfindingOverride(uint16 index);

	uint16 getWalkabilityAt(int16 y, int16 x);
	bool isPathWalkable(int16 y1, int16 x1, int16 y2, int16 x2);
	void snapToWalkablePosition(int16 *pTargetY, int16 *pTargetX, int16 charY, int16 charX);
	int getPathfindingNodeCount() const { return (int)_numPathfindingPoints; }
	int euclideanDistance(const Common::Point &a, const Common::Point &b);
	int walkableDistance(int nodeA, int nodeB);
	int computeMinCostToReachable(int nodeIndex, int prevNode, uint16 actorIndex, const bool *reachable, int nodeCount, const Common::Point &finalDest);

	// This is the override list living at [5BD1]
	Common::Array<uint16> _hotspotOverrides;

	Common::Array<Macs2::AnimFrame> _imageResources;

	GlyphData _glyphs[256];
	GlyphData _panelGlyphs[256]; // Font 2: clean sans-serif font used by save/load panel
	GlyphData _overlayGlyphs[256];
	uint16 numOverlayGlyphs = 0;
	uint16 maxOverlayGlyphHeight = 0;
	uint16 numPanelGlyphs = 0;
	uint16 maxPanelGlyphHeight = 0;
	bool loadOverlayFont(uint8 resourceIndex, uint16 executingObjectID);
	// Font glyph count (79 glyphs in the resource file's font data)
	uint16 numGlyphs = 79;
	uint16 maxGlyphHeight;

	AnimFrame _animFrames[6];
	// 6 flag/decoration animation frames at fixed file offset 0x6A5941, each followed by 6 padding bytes

	bool findGlyph(char c, GlyphData &out) const;

	// Character shading remap (loadResourceFile @ 1008:2e8d -> scene+0x53D3).
	// Indexed as (color - 0xC0) * 0x20 + shadowIntensity (drawSpriteTransparent @ 1010:0ed1).
	Common::Array<byte> _shadingTable;
	// Per-scene 256-byte UI pixel remap (changeScene @ 1008:2574, drawAnimFrameScaled @ 1010:1399).
	Common::Array<byte> _panelRemapTable;

	// Map scene offsets from resource file (scene+0x5DDB, 256 entries x 4 bytes).
	// Each entry is a file offset to a scene preview image for map mode.
	uint32 _mapSceneOffsets[256] = {0};

	Common::Array<BackgroundAnimation> _backgroundAnimations;
	Common::Array<BackgroundAnimationBlob> _backgroundAnimationsBlobs;

	Common::MemoryReadStream *_fileStream;

	void setCursorMode(Script::MouseMode newMode);
	void nextCursorMode();

	Common::Array<uint16> _hotspotColorTable;

	uint16 _numPathfindingPoints;
	uint16 _walkDepthThresholdY;
	uint16 _walkDepthScaleFactor;
	uint16 _walkBaseSpeedPct;

	// Scene palette/animation mode at sceneData+0x5203:
	//   1 = normal (no darkening, no bg animation)
	//   2 = slow bg animation + palette darkening
	//   3 = fast bg animation + palette darkening
	uint16 _scenePaletteMode;
	// Palette darken percentage (0-100) at sceneData+0x5205.
	// Formula: displayedPal[i] = sourcePal[i] * (100 - _paletteDarkenPercent) / 100
	uint16 _paletteDarkenPercent;

	void applyPaletteDarkening();
	// Palette quantization for g_wHelpButtonDisabled path (1000:103e).
	// Histograms scene pixels, keeps 16 rarest colors (0..0xBF) plus UI range
	// 0xC0..0xFF, remaps background + bg-anim blobs + palette via Manhattan RGB.
	void applyScenePaletteEffect();

	// Gradual palette brighten effect for _scenePaletteMode == 2, matching the
	// binary updateBackgroundAnimations (1008:2c05). Called from the game tick at
	// the mode-gated rate. Despite the name it does NOT advance animation frames
	// (that happens in the per-frame render); it decrements the darken percent
	// toward 60 and reapplies the palette darkening.
	void updateBackgroundAnimationPalette();

	// Binary g_bMovementFinishedFlag [1020:0000]: set by walkAlongPath on final arrival
	// (orientation < 9), checked after all characters processed in drawAllCharacters.
	bool _movementFinishedFlag = false;

	Common::Array<uint32> _sceneResourceOffsets;

	bool loadAnimationFromSceneData(uint16 objectIndex, uint16 slotIndex, uint8 arrayIndex, bool shouldMirror = false, uint16 executingScriptObjectId = 0);
	bool loadObjectData(GameObject *obj);
	void clearObjectRuntime(GameObject *obj);
	// sortObjectsByDepth @ 1008:0d79 - inventory cursor reset + free object runtime blobs.
	void sortObjectsByDepth(uint16 objectIndex);

	void loadSongFromSceneData(uint8 dataIndex);
	Music *getAdlib() const { return _adlib; }
	// Returns the Music volume (0-63) scaled by the user's music_volume setting
	uint16 scaledMusicVolume(uint16 gameAttenuation) const;
	void setCurrentSoundData(const Common::Array<uint8> &data);
	void clearCurrentSoundData();
	void playCurrentSound();
	void stopCurrentSound();
	bool hasCurrentSound() const { return !_currentSoundData.empty(); }
	bool isCurrentSoundPlaying() const;

	// Offset 50D3h - This is used in 0037:10C4 to terminate the loop
	uint16 _numHotspots;

	// Reserved/unused scene data fields at scene+0x53C3..+0x53CF (4 dwords).
	// Zeroed on scene change, saved/loaded, but never read or written with meaningful
	// values by any game logic. Kept only for save/load format compatibility.
	uint32 _sceneTimerParams[4] = {0};

	// Clip rect dirty flag [0xfec] - in the original DOS engine this signaled the VGA
	// blitter to reset the clip region to full screen before the next partial-region
	// updates (a dirty-rect optimization). Not used in ScummVM because we redraw the
	// full backbuffer each frame via ManagedSurface. Kept only for save/load compatibility.
	bool _clipRectDirty = false;

	uint16 getHotspotAtPoint(const Common::Point &p);

	Common::Array<uint16> inventoryIconIndices;
	Common::Array<uint16> containerInventoryIconIndices;

	void runScriptExecutor(bool firstRun = false) {
		_scriptExecutor->run(firstRun);
	}

	bool _runScheduled = false;
	// TODO: Feels like this should be more elegantly solved, also check how the game does this
	// Is required for example after a scene change
	bool _scheduledRunIsInitScene = false;

	// Game speed mode from original binary (g_wGameSpeedMode at 1020:0214).
	// Cycled by Ctrl+T: 0=normal, 1=fast (no frame wait), 2=slow (wait for tick>=0x12).
	uint16 _gameSpeedMode = 0;

	// Input record/playback system from original binary.
	// Original usage: MCSEXEC filename /rRecFile or /pPlayFile
	// Record writes per-frame: mouseX(2), mouseY(2), buttons(2)
	// Playback reads the same and also a leading frame-count word.
	enum class InputMode { None,
						   Record,
						   Playback };
	InputMode _inputMode = InputMode::None;
	Common::WriteStream *_inputRecordStream = nullptr;
	Common::SeekableReadStream *_inputPlaybackStream = nullptr;
	uint32 _inputFrameCounter = 0;
	uint32 _inputPlaybackEndFrame = 0;

	void startInputRecording(const Common::Path &filename);
	void startInputPlayback(const Common::Path &filename);
	void stopInputRecording();
	void recordInputFrame(uint16 mouseX, uint16 mouseY, uint16 buttons);
	bool readInputFrame(uint16 &mouseX, uint16 &mouseY, uint16 &buttons);

	Common::Array<uint8> _currentSoundData;
	Audio::SoundHandle _currentSoundHandle;

	// Schedules a run of the script the next time the executor is ticked
	void scheduleRun(bool initScene = false);

	uint16 getWalkabilityAt(const Common::Point &p);

	int measureString(const Common::String &s);

	int measureStrings(const Common::StringArray &sa);
	int measureStringsVertically(const Common::StringArray &sa);

	Common::StringArray decodeStrings(Common::MemoryReadStream *stream, int offset, int numStrings, int sceneId = 0, int objectId = 0);

	// --- Translation support ---
	struct TranslationEntry {
		Common::StringArray strings;
	};
	Common::HashMap<uint32, TranslationEntry> _sceneTranslations;
	Common::HashMap<uint32, TranslationEntry> _objectTranslations;
	Common::HashMap<Common::String, Common::String> _hotspotLabelTranslations;
	void loadTranslation();
	Common::String translateHotspotLabel(const Common::String &cp850Name) const;
	// Compute the sequential string index at the given byte offset in a string blob
	int computeStringIndex(Common::MemoryReadStream *stream, int targetOffset);

	uint32 getFeatures() const;

	bool isDemo() const { return getFeatures() & ADGF_DEMO; }

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsLoadingDuringRuntime) ||
			   (f == kSupportsSavingDuringRuntime) ||
			   (f == kSupportsReturnToLauncher) ||
			   (f == kSupportsChangingOptionsDuringRuntime);
	};

	void syncSoundSettings() override;

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}

	Common::Error loadGameState(int slot) override;

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method.
	 * Produces binary-compatible saves with the original DOS game.
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

	// Write a raw original-DOS-format save file ("SAVEGAME.N", N=0..9) with no
	// ScummVM wrapper, so it can be loaded by the original game executable.
	// The byte layout is produced directly by syncGame (binary-compatible with
	// saveGameToFile at 1008:6859). Mirrors loadGameState(slot 100..109).
	Common::Error saveOriginalGameState(int dosSlot);

	bool tick() override;

	void sayText(const Common::String &text, Common::TextToSpeechManager::Action action = Common::TextToSpeechManager::INTERRUPT_NO_REPEAT) const;

	void getHotspotPositions(Common::Array<Graphics::HotspotInfo> &hotspots) override;
	bool hotspotDirty() const override;
	void rebuildHotspotSnapshot() const;

	struct HotspotSnapshot {
		struct SceneHotspotEntry {
			uint16 index = 0;
			Common::Point center;
		};
		struct SceneObjectEntry {
			uint16 index = 0;
			Common::Point position;
			uint16 orientation = 0;
		};

		int currentSceneIndex = -1;
		uint16 numHotspots = 0;
		Common::Array<uint16> hotspotColorTable;
		Common::Array<uint16> hotspotOverrides;
		Common::Array<SceneHotspotEntry> sceneHotspots;
		Common::Array<SceneObjectEntry> sceneObjects;
		bool mapModeActive = false;
	};
	mutable HotspotSnapshot _hotspotSnapshot;
};

extern Macs2Engine *g_engine;
#define SHOULD_QUIT ::Macs2::g_engine->shouldQuit()

} // End of namespace Macs2

#endif // MACS2_MACS2H
