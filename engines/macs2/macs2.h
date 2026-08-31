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
#include "graphics/palette.h"
#include "macs2/amiga_archive.h"
#include "macs2/events.h"
#include "macs2/macs2_constants.h"
#include "macs2/scriptexecutor.h"

namespace Macs2 {

class MacsAudioStream : public Audio::SeekableAudioStream {
public:
	Common::Array<byte> _data;
	int64 _pos = 0;
	/** Playback rate in Hz (DOS SB samples are 8000; Amiga MXOS uses Paula period). */
	int _rate = 0x1F40;

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
	void readFromMemory(Common::SeekableReadStream *stream);
};

struct AnimFrame : public Sprite {
	int16 _offsetX = 0;
	int16 _offsetY = 0;

	void readFromeFile(Common::File &file);
	void readFromStream(Common::SeekableReadStream *stream);
	bool pixelHit(const Common::Point &point) const;
	Common::Point getBottomMiddleOffset(uint16 scale = 100) const;
};

enum class MenuMode : uint16 {
	Hidden = 0,
	Main = 1,
	Options = 2,
	DialogueList = 4
};

enum class OptionsSubMode : uint16 {
	None = 0,
	Save = 1,
	Load = 2
};

/** Persistent native HUD button (megapic panel skin). */
struct HudButton {
	int16 x = 0;
	int16 y = 0;
	uint16 inactiveStep = 0;
	uint16 activeStep = 0;
	uint16 hoverStep = 0;
	uint16 buttonId = 0; // 1=Walk, 2=Look, 3=Talk, 4=Use, 0x33=Options, ...
	uint16 menuId = 0;
	AnimFrame frame;
	AnimFrame activeFrame;
	AnimFrame hoverFrame;
	Common::Array<uint8> animBlob;
};

struct BackgroundAnimation {
	uint16 _x = 0;
	uint16 _y = 0;
	Common::Array<AnimFrame> _frames;
	uint32 _frameIndex = 0;
};

struct BackgroundAnimationBlob {
	uint16 _x = 0;
	uint16 _y = 0;
	Common::Array<uint8> _blob;
	/** Dialect-v2 special-anim extra slots (1..8); primary remains _blob. */
	Common::Array<uint8> _extraBlobs[8];
	uint16 _activeExtraSlot = 0; // 0 = primary _blob; 1..8 = _extraBlobs[slot-1]
	uint16 _unknown0C = 0; // +0x50F3: purpose unknown (word, read from file, not used at runtime)
	uint8 _unknown0E = 0;  // +0x50F5: purpose unknown (byte, read from file, not used at runtime)
	uint8 _unknown0F = 0;  // +0x50F6: purpose unknown (byte, read from file, not used at runtime)
	Common::Array<uint8> &activeBlob() {
		if (_activeExtraSlot >= 1 && _activeExtraSlot <= 8)
			return _extraBlobs[_activeExtraSlot - 1];
		return _blob;
	}
	const Common::Array<uint8> &activeBlob() const {
		if (_activeExtraSlot >= 1 && _activeExtraSlot <= 8)
			return _extraBlobs[_activeExtraSlot - 1];
		return _blob;
	}
	AnimFrame getCurrentFrame();
	static uint16 advanceAnimFrame(Common::Array<uint8> &blob, bool bpp6, uint16 bpp8);
	/** 1-based pixel frame index from the current sequence position (advanceAnimFrame cx). */
	static uint16 getCurrentPixelFrameNumber(const Common::Array<uint8> &blob);
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

	Music *_music = nullptr;

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
	Graphics::ManagedSurface readRLEImage(int64 offs, Common::SeekableReadStream *stream);

	/** Open primary MCS archive, check magic, load v1 or v2 layout. */
	void readResourceFile();
	/** MCS dialect from the 12-byte file magic. */
	enum class McsFileVersion {
		Unknown = 0,
		V1, // AHFFMACS0100
		V2  // AHFFMACS0200
	};
	McsFileVersion detectMcsFileVersion(Common::SeekableReadStream &stream) const;
	/** Load AHFFMACS0100 layout (loadResourceFile @ 1008:2e8d). */
	void loadResourceFileV1();
	/** Load AHFFMACS0200 layout */
	void loadResourceFileV2();
	/** Shared actor/scene/object directory load after dialect-specific globals */
	void bootstrapMcsActorsObjectsAndScene();
	/** Soft restart (options button 0x20 / Macs2PretReInit). */
	void softRestart();
	const char *getResourceMcsFilename() const;
	/** Amiga: open DataA/Mdir, load OO objects as GameObjects, cursors, and scene stubs. */
	void readAmigaResources();
	void applyAmigaUiPalette();
	/**
	 * Ensure Amiga dialogue portraits can use playfield COLOR17..31.
	 * After native MM copper load those slots are already correct; otherwise
	 * seed them from MXIN chrome (copper base16 layout).
	 */
	void installAmigaPortraitPalette(bool copyFromPlayfield);
	/**
	 * Build _panelRemapTable from luminance buckets (Ghidra fill_ui_panel_darken_remap
	 * @ 002221fe). Outputs into private UI bank 0xF0.. so playfield/intro colors
	 * at MXIN darken indices are never overwritten.
	 */
	void buildAmigaPanelRemapTable();
	bool loadAmigaCursorResource(uint16 resourceId, AnimFrame &out);
	/** Load FF_0000 MXFF into `_glyphs` (Ghidra drawText / g_pFont1Data). */
	bool loadAmigaMxffFont();
	/** Opcode 0x38 overlay font: FF_* from DataA, else copy the main MXFF glyphs. */
	bool loadAmigaOverlayFont(uint8 resourceIndex);
	/** Load one FF_* MXFF into `_overlayGlyphs`. Returns false if missing/undecodable. */
	bool loadAmigaOverlayFontResource(uint16 ffId);
	/**
	 * Amiga: load native MM_* MXMM package by resource id (not script scene id).
	 * Script-visible scene ids are resourceId+1 (Ghidra FUN_002215fa / load_scene_mxmm).
	 * Also extracts trailer script/strings into _amigaPendingScene* for changeScene.
	 * Palette indices 0..31 stay Amiga COLOR registers for OO sprite compatibility.
	 */
	bool loadAmigaSceneBackground(uint32 sceneResourceId);

	// We also need some data from the executable, specifically embedded
	// Adlib data
	void readExecutable();

	// Assumes that the stream is at the location of the number of background animations
	void readBackgroundAnimations(Common::SeekableReadStream *stream);

	// Assumes that the stream is at the start of the right section
	void readImageResources(Common::SeekableReadStream *stream);

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
	// Scene-load snapshot used to restore depth under background animation frame 0.
	Graphics::ManagedSurface _sceneDepthMap;

	// Shadow/shading intensity map (scene+0x301B). Per-pixel values 0-32
	// control character sprite darkening via the shading table.
	// Only scenes with shadow regions have non-zero data.
	Graphics::ManagedSurface _shadowMap;

	// _palVanilla: raw 6-bit VGA source. _pal: 8-bit display (darkened + expanded).
	Graphics::Palette _pal{Graphics::PALETTE_COUNT};
	Graphics::Palette _palVanilla{Graphics::PALETTE_COUNT};

	Common::Array<Common::String> _debugOutput;
	Common::Array<Common::String> _textLog;

	// Note: This is used both for pathfinding as well as for area IDs
	Graphics::ManagedSurface _pathfindingMap;

	Common::Array<PathfindingAreaOverride> _pathfindingOverrides;
	// Area override table at scene+value*5+0x4EA8 (for getAreaAtPoint)
	uint16 _areaOverrides[AREA_OVERRIDE_COUNT] = {0};
	Common::Array<PathfindingPoint> _pathfindingPoints;
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
	/** Sync depth map with the current background animation frame (v1 gate fix). */
	void updateBackgroundAnimationDepthMap(size_t animIndex);
	void updateAllBackgroundAnimationDepthMaps();
	bool isPathWalkable(int16 y1, int16 x1, int16 y2, int16 x2);
	void snapToWalkablePosition(int16 *pTargetY, int16 *pTargetX, int16 charY, int16 charX);
	int getPathfindingNodeCount() const { return (int)_numPathfindingPoints; }
	int euclideanDistance(const Common::Point &a, const Common::Point &b);
	int walkableDistance(int nodeA, int nodeB);
	int computeMinCostToReachable(int nodeIndex, int prevNode, uint16 actorIndex, const bool *reachable, int nodeCount, const Common::Point &finalDest);

	// This is the override list living at [5BD1]
	// Savegames sync 16 words into indices 1..16 (array size 0x11 during sync).
	// Runtime capacity is 0x21 for later growth after load.
	Common::Array<uint16> _hotspotOverrides;

	Common::Array<Macs2::AnimFrame> _imageResources;
	/** Per-cursor hotspot from native HUD button metadata (v2); (0,0) = use center. */
	Common::Point _cursorHotspots[33];

	GlyphData _glyphs[256];
	GlyphData _panelGlyphs[256]; // Font 2: clean sans-serif font used by save/load panel
	GlyphData _overlayGlyphs[256];
	uint16 numOverlayGlyphs = 0;
	uint16 maxOverlayGlyphHeight = 0;
	uint16 numPanelGlyphs = 0;
	uint16 maxPanelGlyphHeight = 0;
	bool loadOverlayFont(uint8 resourceIndex, uint16 executingObjectID);
	/**
	 * Resolve scene/object resource table entry to an absolute MCS file offset.
	 * Shared by sized-resource loads, AHFFANIM, AHFFDLTA, and MegaPic masks.
	 */
	bool resolveResourceFileOffset(uint8 resourceIndex, uint16 executingObjectId, uint32 &outOffset) const;
	/** Read size-prefixed resource payload (size dword excluded from outPayload). */
	bool loadSizedResourcePayload(uint8 resourceIndex, uint16 executingObjectId, Common::Array<uint8> &outPayload);
	/** Decode AHFFANIM0100 body into a runtime anim blob. */
	bool loadAhffAnimResource(uint8 resourceIndex, uint16 executingObjectId, Common::Array<uint8> &outBlob);
	/** Decode MegaPic row-RLE into dest (width x height). */
	bool readMegaPicImage(Common::SeekableReadStream *stream, int width, int height, Graphics::ManagedSurface &out);
	/**
	 * Load a MegaPic mask resource into dest.
	 * If upscaleHalfRes, decode at (width x height) then nearest-neighbor 2x into dest
	 * sized (width*2 x height*2). Otherwise dest is created at width x height.
	 */
	bool loadMaskFromResource(uint8 resourceIndex, uint16 executingObjectId, Graphics::ManagedSurface &dest,
							  int megapicW, int megapicH, bool upscaleHalfRes = false);

	/** Dialect-v2 AHFFDLTA cutscene state (load/play delta opcodes). */
	struct DeltaStrip {
		uint16 y = 0;
		Common::Array<uint8> rle;
	};
	struct DeltaFrame {
		Common::Array<DeltaStrip> strips;
	};
	struct DeltaSfxEvent {
		uint16 frameIndex = 0;
		Common::String fileName;
		bool duckMusic = false;
	};
	struct DeltaAnimState {
		bool loaded = false;
		bool playing = false;
		uint16 frameCount = 0;
		uint16 startFrame = 0;
		uint16 endFrame = 0;
		uint16 speedTicks = 1;
		uint16 tickCounter = 0;
		uint16 currentFrame = 0;
		uint16 clipMiX = 0;
		uint16 clipMiY = 0;
		uint16 clipMaX = 0;
		uint16 clipMaY = 0;
		Graphics::Palette palette{Graphics::PALETTE_COUNT};
		bool applyPaletteOnStart = false;
		Common::Array<DeltaFrame> frames;
		Common::Array<DeltaSfxEvent> sfxEvents;
		void clear(int screenW, int screenH) {
			loaded = false;
			playing = false;
			frameCount = 0;
			startFrame = endFrame = currentFrame = 0;
			speedTicks = 1;
			tickCounter = 0;
			applyPaletteOnStart = false;
			frames.clear();
			sfxEvents.clear();
			clipMiX = clipMiY = 0;
			clipMaX = (uint16)MAX(0, screenW - 1);
			clipMaY = (uint16)MAX(0, screenH - 1);
			palette = Graphics::Palette(Graphics::PALETTE_COUNT);
		}
	};
	DeltaAnimState _deltaAnim;
	bool loadDeltaAnimResource(uint8 resourceIndex, uint16 executingObjectId, bool forceSkipSpeed1 = false);
	void clearDeltaAnim();
	bool startDeltaPlayback(uint16 startFrame, uint16 endFrame, uint16 speedTicks, bool applyPalette);
	bool tickDeltaPlayback();
	void applyDeltaFrameToBackground(const DeltaFrame &frame);
	void playDeltaFrameSfx(uint16 displayFrame);
	// Font glyph count (79 glyphs in the resource file's font data)
	uint16 _numGlyphs = 79;
	uint16 _maxGlyphHeight;

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

	Common::SeekableReadStream *_fileStream = nullptr;
	McsFileVersion _mcsFileVersion = McsFileVersion::Unknown;
	/** Absolute file offset of the 0x3000-byte scene/object directory. */
	uint32 _mcsDirectoryOffset = kMcsV1DirectoryOffset;

	/** Amiga MXFF line pitch: measureTextWidth @ 00224420 uses (font[+8] - 1). */
	uint16 amigaTextLinePitch = 0;
	/** True after loadAmigaSceneBackground installed copper colors in 0..31. */
	bool _amigaNativePlayfieldPalette = false;
	Common::Array<byte> _amigaLineCopperPal;
	/** Amiga DataA/Mdir archive (owned). Null on DOS. */
	Macs2AmigaArchive *_amigaArchive = nullptr;
	/** Filled by loadAmigaSceneBackground; consumed by Amiga changeScene. */
	Common::Array<byte> _amigaPendingSceneScript;
	Common::Array<byte> _amigaPendingSceneStrings;

	/** Bottom HUD visible (showActionBar/hideActionBar); default shown. */
	bool _bottomHudVisible = true;

	/**
	 * Native persistent HUD button table and megapic panels.
	 * Populated when dialect-v2 panel assets are loaded.
	 */
	Graphics::ManagedSurface _hudMegapics[6];
	bool _hudMegapicLoaded[6] = {};
	Common::Array<HudButton> _hudButtons;
	/** Y where the bottom HUD starts; scene above this is interactive. */
	uint16 _panelTopY = 0;
	uint16 _panelHeight = 0;
	MenuMode _menuMode = MenuMode::Main;
	OptionsSubMode _optionsSubMode = OptionsSubMode::None;
	Script::MouseMode _savedMenuCursorMode = Script::MouseMode::Walk;
	uint16 _inventScroll = 1;
	uint16 _inventOriginX = 0;
	uint16 _inventOriginY = 0;
	uint16 _inventCols = 4;
	uint16 _inventRows = 2;
	uint16 _inventSlotW = 64;
	uint16 _inventSlotH = 52;
	uint16 _inventLayoutMode = 0;
	/**
	 * Text layout after invent grid:
	 * [0..4] = options list X/Y/maxW/rows/pitch
	 * [5..6] = dialogue list X/Y
	 */
	uint16 _hudTextLayout[7] = {};
	uint16 _hudTextRecolor[4] = {};
	uint16 _saveListScroll = 1;
	Common::Array<Common::String> _saveSlotNames;
	/** Cutscene skip-speed preference (1..4) from options HUD. */
	uint16 _skipSpeed = 1;

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
	void readPalette(Common::SeekableReadStream *stream, Graphics::Palette &dest);
	void expandPalette6To8(Graphics::Palette &pal);
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

	Music *getMusic() const { return _music; }
	// Returns the Music volume (0-63) scaled by the user's music_volume setting
	uint16 scaledMusicVolume(uint16 gameAttenuation) const;
	/**
	 * TalkVol / setWaveVolume percent (0..100). Used to duck SMF while speech plays.
	 * 0 means unset (duck uses a 50% default).
	 */
	uint16 _talkVol = 0;
	/**
	 * Install PCM for opcode 0x3E / playSample.
	 * @param rateHz sample rate (DOS Sound Blaster path uses 8000)
	 * @param headerSkip bytes to skip at start of buffer (DOS resources have a 2-byte size header;
	 *                   Amiga MXOS extract is raw PCM and uses 0)
	 */
	void setCurrentSoundData(const Common::Array<uint8> &data, int rateHz = 0x1F40, int headerSkip = 2);
	void clearCurrentSoundData();
	bool hasCurrentSoundData() const { return !_currentSoundData.empty(); }
	void playSample();
	void stopSample();
	bool isSamplePlaying() const;
	/**
	 * Play digital audio by basename (no extension). Tries flac/ogg/mp3/m4a/wav
	 * via SeekableAudioStream::openStreamFile (codec #ifdefs live in audio/).
	 * SPEECH paths use the speech mixer handle; others use the SFX handle.
	 */
	void playDigitalAudioFile(const Common::Path &basename, bool speechBus);
	void stopSpeech();
	bool isSpeechPlaying() const;

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
	void setGameSpeedMode(uint16 mode);

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
	int _currentSoundRate = 0x1F40;
	int _currentSoundHeaderSkip = 2;
	/** One-shot PCM / SOUNDFX WAV (script SFX). */
	Audio::SoundHandle _currentSoundHandle;
	/** Optional SPEECH WAV dialogue (independent of environment SFX). */
	Audio::SoundHandle _speechSoundHandle;

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
	Common::HashMap<Common::String, Common::String> _uiLabelTranslations;
	void loadTranslation();
	Common::String translateHotspotLabel(const Common::String &cp850Name) const;
	/** Action-bar / HUD chrome; German source key, same lookup rules as hotspot labels. */
	Common::String translateUiLabel(const Common::String &source) const;
	// Compute the sequential string index at the given byte offset in a string blob
	int computeStringIndex(Common::MemoryReadStream *stream, int targetOffset);

	uint32 getFeatures() const;
	Common::Platform getPlatform() const { return _gameDescription->platform; }
	bool isAmiga() const { return getPlatform() == Common::kPlatformAmiga; }
	Macs2AmigaArchive *getAmigaArchive() const { return _amigaArchive; }

	bool isDemo() const { return getFeatures() & ADGF_DEMO; }

	/** AHFFMACS0200 dialect (directory at 0x212) vs AHFFMACS0100 (0x10). */
	bool isV2() const { return _mcsFileVersion == McsFileVersion::V2; }

	/** MCS directory base. */
	uint32 getMcsDirectoryOffset() const { return _mcsDirectoryOffset; }
	McsFileVersion getMcsFileVersion() const { return _mcsFileVersion; }
	int screenWidth() const { return isV2() ? kWinScreenWidth : kScreenWidth; }
	int screenWidthLast() const { return screenWidth() - 1; }
	int gameHeight() const { return isV2() ? kWinGameHeight : kGameHeight; }
	int gameHeightLast() const { return gameHeight() - 1; }
	/**
	 * Full framebuffer height (playfield + bottom HUD / Scumm strip).
	 * Matches initGraphics height used at startup.
	 */
	int screenHeight() {
		if (isV2()) {
			if (_panelTopY == 0 || _panelHeight == 0)
				return kWinGameHeight;
			return (int)_panelTopY + (int)_panelHeight;
		}
		if (enhancementEnabled(kEnhUIUX))
			return gameHeight() + kUIHeight;
		return gameHeight();
	}
	int screenHeightLast() { return screenHeight() - 1; }

	/**
	 * Bottom HUD / action-bar visibility (dialect-neutral).
	 * hide/show opcodes toggle MenuMode Hidden vs Main and restore the cursor saved
	 * on hide. The Scumm kEnhUIUX strip uses the same flag; cursor mode alone
	 * never hides the bar.
	 */
	bool isBottomHudVisible() const { return _bottomHudVisible; }
	void setBottomHudVisible(bool visible);

	/** True when dialect-v2 panel geometry/assets are available for native HUD. */
	bool hasNativeHudAssets() const {
		return _panelTopY != 0 && _panelHeight != 0;
	}

	// --- Layout / dialect facades ---

	/** Game-loop timer quantum in milliseconds. */
	uint32 timerTickMs() const { return isV2() ? 55 : 46; }
	/** Normal-speed: game frames per that many timer ticks. */
	uint16 ticksPerGameFrame() const { return isV2() ? 1 : 2; }

	/** ReadyObject anim slots (1-based inclusive max). */
	uint16 maxAnimSlots() const { return isV2() ? 0x26 : 0x15; }
	/** Orientations that map to anim slots 1..N (inclusive). */
	uint16 maxOrientations() const { return isV2() ? 0x25 : 0x14; }
	/** Overload / special-anim slot index. */
	uint16 overloadAnimSlot() const { return maxAnimSlots(); }
	static uint16 specialAnimSlotToAnimSlot(uint16 specialSlot);
	/** Scene hotspot override table entries (1-based inclusive max). */
	/** Hotspot remap table indices (1-based). DOS scene+0x5BD1: 16; V2 ActModule+0x6161: 32. */
	uint16 maxHotspots() const { return isV2() ? 0x20 : 0x10; }
	/** Per-object resource offset table entries. */
	uint maxObjectResources() const { return 32; }
	/** Anim slot used for the current orientation (overload-direction rule). */
	uint16 resolveAnimSlotIndex(const GameObject *obj) const;

	/** Script stream: literals carry an extra high word after the value word. */
	bool scriptValuesHaveHighWord() const { return isV2(); }
	/** Script stream: variable index is followed by a padding word. */
	bool scriptVarIndexHasPaddingWord() const { return isV2(); }
	/** Script coordinates -> screen/runtime coordinates (x2 on v2). */
	int16 scaleScriptCoord(int16 coord) const { return isV2() ? (int16)(coord * 2) : coord; }

	/** Dialogue / text-box chrome (DOS l0037_B368 / B462). */
	int dialogPadW() const { return isAmiga() ? 0x08 : 0x12; }
	int dialogPadH() const { return isAmiga() ? 0x08 : 0x10; }
	int dialogTextInset() const { return isAmiga() ? 0x04 : 0x09; }
	int dialogLineGap() const { return 2; }
	int portraitBorderPad() const { return isAmiga() ? 2 : 0x0D; }
	int portraitContentInset() const { return isAmiga() ? 1 : 7; }
	int portraitTextGap() const { return isAmiga() ? 0x0A : 0x12; }
	/**
	 * Per-line step for dialogue layout.
	 * DOS: maxGlyphHeight + dialogLineGap(). Amiga: absolute MXFF pitch
	 * (amigaTextLinePitch) or maxGlyphHeight when pitch is unset.
	 */
	int dialogLineHeight() const {
		if (isAmiga())
			return amigaTextLinePitch ? (int)amigaTextLinePitch : (int)_maxGlyphHeight;
		return (int)_maxGlyphHeight + dialogLineGap();
	}

	/** Depth-map compare Y for sprite occlusion (halved on v2 full-res depth). */
	uint8 depthThresholdForY(int16 charY) const {
		return isV2() ? (uint8)((uint16)charY >> 1) : (uint8)charY;
	}

	/** Resource bootstrap (MCS or Amiga DataA/Mdir). */
	void loadBootstrapResources();
	/**
	 * Load scene background, maps, pathfinding, and related scene tables.
	 * Called from changeScene; Amiga uses native MXMM from DataA.
	 */
	bool loadSceneGraphics(uint32 sceneIndex);
	/** AHFFMACS0100 scene package (RLE maps). */
	bool loadSceneGraphicsV1(uint32 sceneIndex);
	/** AHFFMACS0200 ReadyModule scene package (MegaPic + half-res masks). */
	bool loadSceneGraphicsV2(uint32 sceneIndex);

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
Common::String getObjectHotspotName(uint16 objectIndex);
/** Display name for a hit id: 0x400+object or 0x800+scene hotspot. */
Common::String lookupInteractionDisplayName(uint16 interactionId);

} // End of namespace Macs2

#endif // MACS2_MACS2H
