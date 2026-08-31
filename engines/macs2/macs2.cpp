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

#include "macs2/macs2.h"
#include "audio/audiostream.h"
#include "audio/fmopl.h"
#include "audio/mixer.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/ptr.h"
#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/types.h"
#include "common/util.h"
#include "engines/util.h"
#include "engines/enhancements.h"
#include "gameobjects.h"
#include "graphics/cursorman.h"
#include "graphics/hotspot_renderer.h"
#include "graphics/paletteman.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "gui/debugger.h"
#include "macs2/amiga_decode.h"
#include "macs2/debugtools.h"
#include "macs2/detection.h"
#include "macs2/hotspot_names.h"
#include "macs2/music.h"
#include "macs2/view1.h"

namespace Macs2 {

static constexpr const char *kGameSpeedModeConfigKey = "macs2_game_speed_mode";

namespace {

static constexpr uint16 kMaxSceneObjects = 0x200;

Common::U32String hotspotLabelToU32(const Common::String &name) {
	if (name.empty())
		return Common::U32String();
	return Common::U32String(name.c_str(), Common::kDos850);
}

bool isMapModeActive() {
	if (g_events == nullptr)
		return false;
	View1 *view = (View1 *)g_events->findView("View1");
	return view != nullptr && view->_currentMode == ViewMode::VM_HELP;
}

Common::Point getSceneObjectHotspotPosition(View1 *view, GameObject *obj) {
	if (view != nullptr) {
		Character *character = view->getCharacterByIndex(obj->_index);
		if (character != nullptr && !character->_markedForDeletion)
			return character->getPosition();
	}
	return obj->_position;
}

} // namespace

void resetCharacterWalkPath(Character *character) {
	if (character == nullptr || character->_gameObject == nullptr)
		return;
	const Common::Point pos = character->_gameObject->_position;
	character->_path.clear();
	character->_currentPathIndex = 0;
	character->_targetPosition = pos;
	character->_pathFinalDestination = pos;
	character->_stepDeltaX = 0;
	character->_stepDeltaY = 0;
	character->_stepError = 0;
	character->_stepDirectionSet = false;
}

Macs2Engine *g_engine;

Graphics::ManagedSurface Macs2Engine::readRLEImage(int64 offs, Common::SeekableReadStream *stream) {
	stream->seek(offs);

	Graphics::ManagedSurface result;
	result.create(kScreenWidth, kGameHeight, Graphics::PixelFormat::createFormatCLUT8());

	// Max RLE row size: kScreenWidth pixels uncompressed, but with escape sequences
	// the encoded form could be slightly larger. 1024 is more than sufficient.
	uint8 *data = new uint8[1024];

	for (int y = 0; y < kGameHeight; y++) {
		uint16 length = stream->readUint16LE();
		stream->read(data, length);
		// Signed, matching the original decodeRLERows (1008:0666): a final RLE run that
		// overshoots the row width drives this negative and terminates the row. Using an
		// unsigned counter here underflows and runs away, over-reading the encoded data and
		// corrupting this row and every row after it (spurious walkability values).
		int16 remainingPixels = kScreenWidth;
		uint8 *dataPointer = data;
		uint16 x = 0;
		while (remainingPixels > 0) {
			const uint8 &value = dataPointer[0];
			dataPointer++;
			if (value != 0xF0) {
				result.setPixel(x, y, value);
				remainingPixels--;
				x++;
			} else {
				// We need to decode the RLE data
				const uint8 &runlength = dataPointer[0];
				dataPointer++;
				const uint8 &encodedValue = dataPointer[0];
				dataPointer++;
				for (int i = 0; i < runlength && x < kScreenWidth; i++) {
					result.setPixel(x++, y, encodedValue);
				}
				remainingPixels -= runlength;
			}
		}
	}

	delete[] data;
	return result;
}

Macs2Engine::McsFileVersion Macs2Engine::detectMcsFileVersion(Common::SeekableReadStream &stream) const {
	if (stream.size() < (int64)kMcsMagicSize)
		return McsFileVersion::Unknown;

	const int64 pos = stream.pos();
	byte magic[kMcsMagicSize];
	stream.seek(0, SEEK_SET);
	if (stream.read(magic, kMcsMagicSize) != kMcsMagicSize) {
		stream.seek(pos, SEEK_SET);
		return McsFileVersion::Unknown;
	}
	stream.seek(pos, SEEK_SET);

	if (memcmp(magic, kMcsMagicV1, kMcsMagicSize) == 0)
		return McsFileVersion::V1;
	if (memcmp(magic, kMcsMagicV2, kMcsMagicSize) == 0)
		return McsFileVersion::V2;
	return McsFileVersion::Unknown;
}

const char *Macs2Engine::getResourceMcsFilename() const {
	return "RESOURCE.MCS";
}

void Macs2Engine::readResourceFile() {
	const char *mcsName = getResourceMcsFilename();
	{
		Common::File *file = new Common::File();
		if (!file->open(mcsName)) {
			delete file;
			error("readResourceFile(): Error reading MCS file %s", mcsName);
		}

		_mcsFileVersion = detectMcsFileVersion(*file);
		if (_mcsFileVersion == McsFileVersion::V1) {
			_mcsDirectoryOffset = kMcsV1DirectoryOffset;
			debugC(1, kDebugFilePath, "MCS %s: AHFFMACS0100, directory @ 0x%x", mcsName, _mcsDirectoryOffset);
			const int64 size = file->size();
			byte *fileData = (byte *)malloc((size_t)size);
			file->seek(0, SEEK_SET);
			file->read(fileData, size);
			delete file;
			_fileStream = new Common::MemoryReadStream(fileData, (uint32)size, DisposeAfterUse::YES);
			loadResourceFileV1();
		} else if (_mcsFileVersion == McsFileVersion::V2) {
			_mcsDirectoryOffset = kMcsV2DirectoryOffset;
			debugC(1, kDebugFilePath, "MCS %s: AHFFMACS0200, directory @ 0x%x", mcsName, _mcsDirectoryOffset);
			file->seek(0, SEEK_SET);
			_fileStream = file; // large archives stay file-backed
			loadResourceFileV2();
			_scriptExecutor->setOpcodeTable(Script::ScriptExecutor::kV2OpcodeTable,
											Script::ScriptExecutor::kV2OpcodeTableSize);
		} else {
			delete file;
			error("readResourceFile(): unrecognized MCS magic in %s", mcsName);
		}

		// Initialize border sprites from cursor image array entries at fixed indices.
		// Original loadResourceFile (1008:2e8d) calls changeScene(g_wCurrentSceneIndex) before
		// returning, ensuring all scene data (pathfinding maps, depth map, palette, background)
		// is loaded before the game loop processes any input.
		// The original allocates the 0x75E0-byte scene data buffer (which includes space for
		// all RLE-decoded maps) before calling changeScene. Create the surfaces here.
		const int sw = screenWidth();
		const int gh = gameHeight();
		_sceneBackground.create(sw, gh, Graphics::PixelFormat::createFormatCLUT8());
		_depthMap.create(sw, gh, Graphics::PixelFormat::createFormatCLUT8());
		_pathfindingMap.create(sw, gh, Graphics::PixelFormat::createFormatCLUT8());
		_shadowMap.create(sw, gh, Graphics::PixelFormat::createFormatCLUT8());
		_hotspotMap.create(sw, gh, Graphics::PixelFormat::createFormatCLUT8());
		changeScene(Scenes::instance()._currentSceneIndex);
	}
}

void Macs2Engine::loadResourceFileV1() {
	// File layout (loadResourceFile @ 1008:2e8d, magic AHFFMACS0100):
	//   +0x00  12-byte magic
	//   +0x0C  actor index, +0x0E initial scene index
	//   +0x10  directory 0x3000 (512 x 12); object DATA/SCRIPT ptrs at +0x17F4/+0x17F8
	//   +0x3010 vanilla palette 0x300
	//   +0x3310 shading table 0x800 -> scene+0x53D3
	//   then 0x21 cursor/icon images, Font1, Font2, 0x400 map scene offsets -> scene+0x5DDB
	// Global vanilla palette is overwritten per-scene in changeScene; skip to shading.
	_fileStream->seek(kMcsV1ShadingTableOffset, SEEK_SET);
	_shadingTable.resize(kMcsV1ShadingTableSize);
	_fileStream->read(_shadingTable.data(), kMcsV1ShadingTableSize);
	readImageResources(_fileStream);
	// Font 1 follows immediately after the 33 image resource entries.
	// Original: 4-byte size field (skipped) + 2-byte glyph count + glyph data.
	uint32 font1SizeField = _fileStream->readUint32LE(); // skip size field
	(void)font1SizeField;
	uint16 font1GlyphCount = _fileStream->readUint16LE();
	_maxGlyphHeight = 0;
	for (uint i = 0; i < font1GlyphCount; i++) {
		_glyphs[i].readFromMemory(_fileStream);
		_maxGlyphHeight = MAX(_glyphs[i]._height, _maxGlyphHeight);
	}
	_numGlyphs = font1GlyphCount;

	// Font 2: clean sans-serif font used by save/load panel (scene data offset 0x1044)
	uint32 font2SizeField = _fileStream->readUint32LE();
	(void)font2SizeField;
	uint16 font2GlyphCount = _fileStream->readUint16LE();
	maxPanelGlyphHeight = 0;
	for (uint i = 0; i < font2GlyphCount && i < 256; i++) {
		_panelGlyphs[i].readFromMemory(_fileStream);
		maxPanelGlyphHeight = MAX(maxPanelGlyphHeight, _panelGlyphs[i]._height);
	}
	numPanelGlyphs = font2GlyphCount;

	// Map scene offsets -> scene+0x5DDB. First entry is the help screen image offset.
	for (uint i = 0; i < kMcsV1MapSceneOffsetCount; i++) {
		_mapSceneOffsets[i] = _fileStream->readUint32LE();
	}

	_fileStream->seek(kMcsV1ActorIndexOffset, SEEK_SET);
	bootstrapMcsActorsObjectsAndScene();
}

void Macs2Engine::loadResourceFileV2() {
	// File layout after Directory:
	//   0x300 sprite palette
	//   4 words -> ShowText recolor colors
	//   panelTopY + panelHeight
	//   6 megapics (flag word; if nonzero, panelH rows of RLE)
	//   button count + per-button metadata + inline anim blobs
	//   inventory grid + text layout words
	//   TalkVol + Font1 + SysFont + 0x400 map offsets
	_shadingTable.clear();
	_shadingTable.resize(0x800, 0);
	_numGlyphs = 0;
	numPanelGlyphs = 0;
	memset(_mapSceneOffsets, 0, sizeof(_mapSceneOffsets));
	_imageResources.clear();
	_imageResources.resize(33);
	for (int i = 0; i < ARRAYSIZE(_cursorHotspots); i++) {
		_cursorHotspots[i] = Common::Point(0, 0);
	}
	_hudButtons.clear();
	for (int i = 0; i < ARRAYSIZE(_hudMegapicLoaded); i++) {
		_hudMegapicLoaded[i] = false;
	}
	for (int i = 0; i < ARRAYSIZE(_hudMegapics); i++) {
		_hudMegapics[i].free();
	}
	_panelTopY = 0;
	_panelHeight = 0;
	_menuMode = MenuMode::Main;
	_optionsSubMode = OptionsSubMode::None;
	_savedMenuCursorMode = Script::MouseMode::Walk;
	_inventScroll = 1;
	memset(_hudTextLayout, 0, sizeof(_hudTextLayout));
	memset(_hudTextRecolor, 0, sizeof(_hudTextRecolor));
	_talkVol = 0;

	if (_fileStream == nullptr)
		return;

	_fileStream->seek(_mcsDirectoryOffset + 0x3000, SEEK_SET);
	readPalette(_fileStream, _palVanilla);
	_pal = _palVanilla;

	for (int i = 0; i < ARRAYSIZE(_hudTextRecolor); i++) {
		_hudTextRecolor[i] = _fileStream->readUint16LE();
	}

	_panelTopY = _fileStream->readUint16LE();
	_panelHeight = _fileStream->readUint16LE();
	if (_panelTopY == 0 || _panelHeight == 0) {
		warning("readGlobalAssetsV2: invalid panel geometry %u+%u", _panelTopY, _panelHeight);
		_panelTopY = 280;
		_panelHeight = 146;
	}

	for (int i = 0; i < ARRAYSIZE(_hudMegapicLoaded); i++) {
		const uint16 flag = _fileStream->readUint16LE();
		if (flag == 0)
			continue;
		if (!readMegaPicImage(_fileStream, kWinScreenWidth, _panelHeight, _hudMegapics[i])) {
			warning("readGlobalAssetsV2: failed loading UI megapic %d", i);
			return;
		}
		_hudMegapicLoaded[i] = true;
	}

	const uint16 buttonCount = _fileStream->readUint16LE();
	struct CursorMap {
		uint16 cid;
		uint16 mouseNr;
		bool active;
	};
	static const CursorMap kCursorMap[] = {
		{0x6E, 0x16, true},
		{0x6F, 0x16, false},
		{0x69, 0x13, true},
		{0x6A, 0x13, false},
		{0x64, 0x14, true},
		{0x65, 0x14, false},
		{0x6B, 0x15, true},
		{0x6C, 0x15, false},
		{0x66, 0x17, true},
		{0x67, 0x17, false},
		{0x68, 0x19, true},
		{0x6D, 0x1A, true},
	};

	auto extractAnimStepFrame = [](Common::Array<uint8> blob, uint16 step, AnimFrame &out) -> bool {
		if (blob.empty() || step == 0)
			return false;
		const uint32 offset = BackgroundAnimationBlob::advanceAnimFrame(blob, true, (uint16)(step + 0x64));
		if (offset == 0 || offset + 10 > blob.size())
			return false;
		const uint32 frameOff = offset + 6;
		out._width = READ_LE_UINT16(&blob[frameOff]);
		out._height = READ_LE_UINT16(&blob[frameOff + 2]);
		if (out._width == 0 || out._width > 640 || out._height == 0 || out._height > 400)
			return false;
		const uint32 pix = (uint32)out._width * (uint32)out._height;
		if (frameOff + 4 + pix > blob.size())
			return false;
		out._data.resize(pix);
		memcpy(out._data.data(), &blob[frameOff + 4], pix);
		return true;
	};

	auto loadBigAnimFirstFrame = [&](int64 animStart, AnimFrame &out) -> bool {
		_fileStream->seek(animStart + 10, SEEK_SET);
		const uint16 local8 = _fileStream->readUint16LE();
		_fileStream->seek(animStart + local8 + 0x0E, SEEK_SET);
		_fileStream->skip(6);
		out._width = _fileStream->readUint16LE();
		out._height = _fileStream->readUint16LE();
		if (out._width == 0 || out._width > 640 || out._height == 0 || out._height > 400)
			return false;
		out._data.resize((uint)out._width * (uint)out._height);
		return _fileStream->read(out._data.data(), out._data.size()) == out._data.size();
	};

	for (uint16 b = 1; b <= buttonCount && !_fileStream->eos(); b++) {
		HudButton button;
		button.x = (int16)_fileStream->readUint16LE();
		button.y = (int16)_fileStream->readUint16LE();
		button.inactiveStep = _fileStream->readUint16LE();
		button.activeStep = _fileStream->readUint16LE();
		button.hoverStep = _fileStream->readUint16LE();
		button.buttonId = _fileStream->readUint16LE();
		button.menuId = _fileStream->readUint16LE();
		const uint32 animSize = _fileStream->readUint32LE();
		if (animSize == 0 || animSize > 0x1000000 || _fileStream->eos())
			break;

		const int64 animStart = _fileStream->pos();
		Common::Array<uint8> animBlob;
		animBlob.resize(animSize);
		if (_fileStream->read(animBlob.data(), animSize) != animSize)
			break;

		AnimFrame frame;
		bool gotFrame = extractAnimStepFrame(animBlob, button.inactiveStep ? button.inactiveStep : 1, frame);
		if (!gotFrame)
			gotFrame = loadBigAnimFirstFrame(animStart, frame);

		AnimFrame activeFrame;
		const bool gotActive = (button.activeStep != 0 && button.activeStep != button.inactiveStep) && extractAnimStepFrame(animBlob, button.activeStep, activeFrame);
		AnimFrame hoverFrame;
		const bool gotHover = (button.hoverStep != 0 && button.hoverStep != button.inactiveStep) && extractAnimStepFrame(animBlob, button.hoverStep, hoverFrame);

		if (button.menuId == 7 && gotFrame) {
			uint16 mouseNr = 0;
			bool prefer = false;
			for (const CursorMap &entry : kCursorMap) {
				if (entry.cid == button.buttonId) {
					mouseNr = entry.mouseNr;
					prefer = entry.active;
					break;
				}
			}
			const int slot = (int)mouseNr - 1;
			if (mouseNr != 0 && slot >= 0 && slot < ARRAYSIZE(_cursorHotspots)) {
				const bool empty = _imageResources[slot]._data.empty();
				if (empty || prefer) {
					_imageResources[slot] = prefer && gotActive ? activeFrame : frame;
					_cursorHotspots[slot] = Common::Point(button.x, button.y);
				}
			}
		} else if (gotFrame) {
			button.animBlob = Common::move(animBlob);
			button.frame = Common::move(frame);
			if (gotActive)
				button.activeFrame = Common::move(activeFrame);
			if (gotHover)
				button.hoverFrame = Common::move(hoverFrame);
			_hudButtons.push_back(Common::move(button));
		}

		_fileStream->seek(animStart + (int64)animSize, SEEK_SET);
	}

	_inventOriginX = _fileStream->readUint16LE();
	_inventOriginY = _fileStream->readUint16LE();
	_inventCols = _fileStream->readUint16LE();
	_inventRows = _fileStream->readUint16LE();
	_inventSlotW = _fileStream->readUint16LE();
	_inventSlotH = _fileStream->readUint16LE();
	_inventLayoutMode = _fileStream->readUint16LE();
	for (int i = 0; i < ARRAYSIZE(_hudTextLayout); i++) {
		_hudTextLayout[i] = _fileStream->readUint16LE();
	}

	if (_inventCols == 0)
		_inventCols = 4;
	if (_inventRows == 0)
		_inventRows = 2;

	_talkVol = _fileStream->readUint16LE();
	auto loadSizedFont = [&](GlyphData *out, uint16 &outCount, uint16 &outMaxH) -> bool {
		outCount = 0;
		outMaxH = 0;
		const uint32 fontSize = _fileStream->readUint32LE();
		if (fontSize == 0 || fontSize > 0x100000)
			return false;
		const int64 fontStart = _fileStream->pos();
		const uint16 glyphCount = _fileStream->readUint16LE();
		if (glyphCount == 0 || glyphCount > 256) {
			_fileStream->seek(fontStart + (int64)fontSize, SEEK_SET);
			return false;
		}
		for (uint16 i = 0; i < glyphCount; i++) {
			out[i].readFromMemory(_fileStream);
			outMaxH = MAX(outMaxH, out[i]._height);
		}
		outCount = glyphCount;
		_fileStream->seek(fontStart + (int64)fontSize, SEEK_SET);
		return true;
	};
	if (!loadSizedFont(_glyphs, _numGlyphs, _maxGlyphHeight))
		warning("readGlobalAssetsV2: failed loading Font1");
	if (!loadSizedFont(_panelGlyphs, numPanelGlyphs, maxPanelGlyphHeight))
		warning("readGlobalAssetsV2: failed loading SysFont");

	for (int i = 0; i < ARRAYSIZE(_mapSceneOffsets); i++)
		_mapSceneOffsets[i] = _fileStream->readUint32LE();

	_saveListScroll = 1;
	_saveSlotNames.clear();

	uint installed = 0;
	for (uint i = 0; i < _imageResources.size(); i++) {
		if (!_imageResources[i]._data.empty())
			installed++;
	}
	uint megas = 0;
	for (int i = 0; i < ARRAYSIZE(_hudMegapicLoaded); i++) {
		if (_hudMegapicLoaded[i])
			megas++;
	}
	debugC(1, kDebugFilePath,
		   "readGlobalAssetsV2: panel=%u+%u megapics=%u buttons=%u cursors=%u invent=%ux%u @(%u,%u) fonts=%u/%u",
		   _panelTopY, _panelHeight, megas, (uint)_hudButtons.size(), installed,
		   _inventCols, _inventRows, _inventOriginX, _inventOriginY,
		   _numGlyphs, numPanelGlyphs);
	_fileStream->seek(kMcsV2ActorIndexOffset, SEEK_SET);
	bootstrapMcsActorsObjectsAndScene();
}

void Macs2Engine::bootstrapMcsActorsObjectsAndScene() {
	Scenes &scenes = Scenes::instance();
	scenes._currentActorIndex = _fileStream->readUint16LE();
	uint16 firstSceneIndex = _fileStream->readUint16LE();
	scenes._currentSceneIndex = firstSceneIndex;
	scenes._currentSceneScript = scenes.readSceneScript(firstSceneIndex, _fileStream);
	scenes._currentSceneStrings = scenes.readSceneStrings(firstSceneIndex, _fileStream);
	scenes._currentSceneSpecialAnimOffsets = scenes.readSpecialAnimsOffsets(firstSceneIndex, _fileStream);
	_scriptExecutor->setScript(scenes._currentSceneScript);

	// Load object data (512 entries max, matching original loadResourceFile)
	// Original allocates all 512 slots, then frees unused ones. We pre-fill with nullptr.
	const uint32 dir = getMcsDirectoryOffset();
	GameObjects::instance()._objects.resize(0x200, nullptr);
	for (int i = 1; i <= 0x200; i++) {
		// Directory object DATA dword: file+kMcsV1DirectoryOffset+kMcsV1ObjectDataPtrRel+i*12
		const uint32 addressOffset = dir + kMcsV1ObjectDataPtrRel + (uint32)i * 0xC;
		_fileStream->seek(addressOffset, SEEK_SET);
		uint32 objectOffset = _fileStream->readUint32LE();
		if (objectOffset == 0) {
			continue;
		}

		_fileStream->seek(objectOffset, SEEK_SET);
		GameObject *gameObject = new GameObject();
		gameObject->_index = i;
		gameObject->_dataOffset = objectOffset;

		// Object header (ReadyObject / initGameObject): x, y, scene, orientation, vertical scale
		uint16 x = _fileStream->readUint16LE(); // TODO: use _engine->scaleScriptCoord
		uint16 y = _fileStream->readUint16LE();
		if (isV2()) {
			x = (uint16)(x << 1);
			y = (uint16)(y << 1);
		}
		gameObject->_position = Common::Point(x, y);
		gameObject->_sceneIndex = _fileStream->readUint16LE();
		gameObject->_orientation = _fileStream->readUint16LE();
		gameObject->_verticalOffsetScale = _fileStream->readUint16LE();

		const uint16 animSlotCount = maxAnimSlots();
		if (isV2()) {
			// ReadyObject: lead word, then slots; payload filled later by loadObjectData.
			_fileStream->readUint16LE();
			for (int j = 0; j < (int)animSlotCount; j++) {
				_fileStream->readUint16LE(); // animID
				_fileStream->readUint16LE(); // sourceKey
				uint32 dataSize = _fileStream->readUint32LE();
				if (dataSize > 0)
					_fileStream->skip(dataSize);
				_fileStream->readUint16LE(); // speed
				_fileStream->readByte();     // mirror
				_fileStream->readByte();     // pad
				gameObject->_blobs.push_back(Common::Array<uint8>());
				gameObject->_blobSourceKeys.push_back(0);
				gameObject->_blobWalkSpeeds.push_back(0);
				gameObject->_blobMirrorFlags.push_back(false);
			}
			_fileStream->readByte();
			gameObject->_hasShading = _fileStream->readByte() != 0;
			gameObject->_hasScaling = _fileStream->readByte() != 0;
			gameObject->_hasDoubleResAnim = _fileStream->readByte() != 0;
		} else {
			for (int j = 1; j <= (int)animSlotCount; j++) {
				_fileStream->readUint16LE(); // animID
				uint16 blobSourceKey = _fileStream->readUint16LE();
				uint32 dataSize = _fileStream->readUint32LE();
				uint8 *data = new uint8[dataSize];
				_fileStream->read(data, dataSize);
				gameObject->_blobs.push_back(Common::Array<uint8>(data, dataSize));
				delete[] data;
				gameObject->_blobSourceKeys.push_back(blobSourceKey);
				uint16 blobSpeed = _fileStream->readUint16LE();
				gameObject->_blobWalkSpeeds.push_back(blobSpeed);
				uint16 blobMirrorFlag = _fileStream->readByte();
				_fileStream->readByte();
				gameObject->_blobMirrorFlags.push_back(blobMirrorFlag != 0);
				if (blobMirrorFlag != 0 && dataSize > 0)
					BackgroundAnimationBlob::mirrorAnimBlob(gameObject->_blobs.back());
			}
			_fileStream->readByte();
			gameObject->_hasShading = _fileStream->readByte() != 0;
			gameObject->_hasScaling = _fileStream->readByte() != 0;
		}

		const uint32 scriptPtrOffset = dir + kMcsV1ObjectScriptPtrRel + (uint32)i * 0xC;
		_fileStream->seek(scriptPtrOffset, SEEK_SET);

		objectOffset = _fileStream->readUint32LE();
		// Binary loadResourceFile prunes an object slot ONLY when its DATA offset
		// (scene table +0x17F4) is zero (handled by the `continue` above). A zero
		// SCRIPT offset (+0x17F8) does NOT remove the object - it simply has no
		// script/resource table. The original keeps the slot non-null so that the
		// object set (used implicitly by save/load record ordering) stays correct.
		// Previously this did `break`, which leaked this object, left it null, and
		// aborted loading every higher-index object - corrupting the object set
		// and shifting the save-file object section.
		if (objectOffset == 0) {
			GameObjects::instance()._objects[i - 1] = gameObject;
			continue;
		}
		_fileStream->seek(objectOffset, SEEK_SET);
		// Resource offset table (32 dwords).
		const uint maxObjRes = maxObjectResources();
		for (uint r = 0; r < maxObjRes; r++) {
			gameObject->_resourceOffsets[r] = _fileStream->readUint32LE();
		}
		if (isV2()) {
			_fileStream->skip(0x200 - maxObjRes * 4);
			_fileStream->readUint16LE();
			_fileStream->readUint16LE();
		}
		uint16 scriptLength = _fileStream->readUint16LE();
		gameObject->_script.resize(scriptLength);
		_fileStream->read(gameObject->_script.data(), scriptLength);

		GameObjects::instance()._objects[i - 1] = gameObject;
	}
}

void Macs2Engine::readExecutable() {
	inventoryIconIndices.resize(6);
	containerInventoryIconIndices.resize(6);

	if (isAmiga() || isV2()) {
		for (uint i = 0; i < 6; i++) {
			inventoryIconIndices[i] = (uint16)(i + 1);
			containerInventoryIconIndices[i] = (uint16)(i + 1);
		}
		return;
	}

	Common::ScopedPtr<Common::MemoryReadStream> exeFileStream;
	{
		// Extra scope in order to make sure no code tries to read from the file directly.
		Common::File file;
		if (!file.open("MCSEXEC.EXE"))
			error("readExecutable(): Error reading executable file");

		int64 size = file.size();
		byte *fileData = (byte *)malloc(size);
		file.read(fileData, size);

		exeFileStream.reset(new Common::MemoryReadStream(fileData, size, DisposeAfterUse::YES));
	}

	// Full MCSEXEC.EXE and demo MCSEXEC.EXE are different binaries (different MD5, ~12k differing bytes),
	// but the whole Data5 segment is identical (1020:0000...1020:3787)
	// TODO: if there are ever other games using different versions of MCSEXEC.EXE, we should check the checksum here

	_music->readDataFromExecutable(exeFileStream.get());

	exeFileStream->seek(0x0001B610, SEEK_SET);
	exeFileStream->read(inventoryIconIndices.data(), 12);

	exeFileStream->seek(0x0001B61C, SEEK_SET);
	exeFileStream->read(containerInventoryIconIndices.data(), 12);
}

void Macs2Engine::softRestart() {
	getMusic()->stopMusic();
	stopSample();
	stopSpeech();
	clearDeltaAnim();
	_skipSpeed = 1;
	_menuMode = MenuMode::Main;
	_optionsSubMode = OptionsSubMode::None;
	_inventScroll = 1;
	_saveListScroll = 1;

	if (_scriptExecutor != nullptr) {
		_scriptExecutor->_waitForDeltaAnim = false;
		_scriptExecutor->_waitForDeltaSpeed = false;
		_scriptExecutor->_waitForPcmSound = false;
		_scriptExecutor->_waitForMusicControl = false;
		_scriptExecutor->_waitForAdlibReady = false;
		_scriptExecutor->_waitForObjectAnimStep = false;
		_scriptExecutor->_waitForSpecialAnimStep = false;
		_scriptExecutor->_waitingForUiClick = false;
		_scriptExecutor->endFrameWait();
		_scriptExecutor->releaseObjectStream();
	}

	View1 *currentView = (View1 *)findView("View1");
	if (currentView != nullptr) {
		for (Character *c : currentView->_characters)
			delete c;
		currentView->_characters.clear();
		currentView->flushPendingCharacterDeletes();
		currentView->_inventoryItems.clear();
		currentView->_activeInventoryItem = nullptr;
		currentView->_isShowingDialoguePanel = false;
		currentView->_isDialogueChoiceInputActive = false;
		currentView->_isShowingTextBox = false;
		currentView->currentSpeechActData = SpeechActData();
	}

	for (uint i = 0; i < GameObjects::instance()._objects.size(); i++)
		delete GameObjects::instance()._objects[i];
	GameObjects::instance()._objects.clear();

	delete Scenes::instance()._currentSceneScript;
	delete Scenes::instance()._currentSceneStrings;
	Scenes::instance()._currentSceneScript = nullptr;
	Scenes::instance()._currentSceneStrings = nullptr;
	Scenes::instance()._currentSceneSpecialAnimOffsets.clear();

	_backgroundAnimations.clear();
	_backgroundAnimationsBlobs.clear();
	clearDeltaAnim();

	delete _fileStream;
	_fileStream = nullptr;

	readResourceFile();

	if (currentView != nullptr) {
		currentView->_backgroundSurface.copyFrom(_sceneBackground);
		currentView->_paletteDirty = true;
		currentView->redraw();
	}
	runScriptExecutor();
}

void Macs2Engine::loadBootstrapResources() {
	if (isAmiga())
		readAmigaResources();
	else
		readResourceFile();
}

void Macs2Engine::readBackgroundAnimations(Common::SeekableReadStream *stream) {
	// changeScene (1008:2574): background animation loading at scene+0x50F5.
	// Per-entry runtime struct (0x10 bytes stride):
	//   +0x00: X position (word)
	//   +0x02: Y position (word)
	//   +0x04: blob data size (dword)
	//   +0x08: blob data pointer (far ptr, allocated at runtime)
	//   +0x0C: unknown word (read from file, not referenced at runtime)
	//   +0x0E: unknown byte (read from file, not referenced at runtime)
	//   +0x0F: unknown byte (read from file, not referenced at runtime)
	uint16 numBackgroundAnimations = stream->readUint16LE();

	_backgroundAnimations.resize(numBackgroundAnimations);
	_backgroundAnimationsBlobs.resize(numBackgroundAnimations);

	for (int i = 0; i < numBackgroundAnimations; i++) {
		BackgroundAnimationBlob &currentBlob = _backgroundAnimationsBlobs[i];
		BackgroundAnimation &current = _backgroundAnimations[i];

		// X position (+0x50E7 in scene data for entry 1)
		current._x = stream->readUint16LE();
		currentBlob._x = current._x;
		// Y position (+0x50E9)
		current._y = stream->readUint16LE();
		currentBlob._y = current._y;
		// Blob data size (+0x50EB, 4 bytes)
		uint32 numBytes = stream->readUint32LE();
		// Read raw blob data (+0x50EF points to this in runtime)
		currentBlob._blob.resize(numBytes);
		stream->read(currentBlob._blob.data(), numBytes);
		// Trailing per-animation fields (stored but not read at runtime by binary)
		currentBlob._unknown0C = stream->readUint16LE(); // +0x50F3: unknown word
		currentBlob._unknown0E = stream->readByte();     // +0x50F5: unknown byte
		currentBlob._unknown0F = stream->readByte();     // +0x50F6: unknown byte

		// Parse frames for the legacy BackgroundAnimation struct
		AnimBlobView blobView(currentBlob._blob);
		// Original uses sequence length (blob[0xA]+1) as numFrames for background animations
		uint16 numFrames = blobView.sequenceLength();
		current._frameIndex = 0;
		current._frames.resize(numFrames);
		uint16 actualFrameCount = blobView.frameCount();
		for (int j = 0; j < (int)actualFrameCount; j++) {
			AnimBlobView::FrameInfo fi;
			if (!blobView.getFrameInfo(j, fi))
				break;
			current._frames[j]._width = fi.width;
			current._frames[j]._height = fi.height;
			const uint32 pix = (uint32)fi.width * (uint32)fi.height;
			current._frames[j]._data.resize(pix);
			memcpy(current._frames[j]._data.data(), fi.pixels, pix);
		}

		// The blob's internal frame pointer is stored in the resource file data.
		// Do NOT reinitialize it - the original binary just reads the raw data.
	}
}

void Macs2Engine::readImageResources(Common::SeekableReadStream *stream) {
	// l0037_3355: Read 33 entries, preserving index alignment (zero-length = empty placeholder).
	// Binary uses g_pCursorImageArray[index] directly; indices must match.
	for (int i = 0; i < 0x21; i++) {
		uint32 length = stream->readUint32LE();
		if (length == 0) {
			AnimFrame empty;
			_imageResources.push_back(empty);
			continue;
		}
		AnimFrame frame;
		// Move forward to skip the first word
		stream->seek(0x2, SEEK_CUR);
		frame.readFromStream(stream);
		_imageResources.push_back(frame);
	}
}

Macs2Engine::Macs2Engine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																			 _gameDescription(gameDesc) {
	g_engine = this;
	_scriptExecutor = new Script::ScriptExecutor(this);
	_music = new Music();

	_hotspotOverrides.resize(0x21);
	for (uint i = 0; i < _hotspotOverrides.size(); i++) {
		_hotspotOverrides[i] = 0xFFFF;
	}
}

Macs2Engine::~Macs2Engine() {
#ifdef USE_IMGUI
	_system->setImGuiCallbacks(ImGuiCallbacks());
#endif
	stopInputRecording();
	clearCurrentSoundData();
	_music->deinit();
	delete _music;
	if (_amigaArchive) {
		SearchMan.remove("macs2amiga");
		delete _amigaArchive;
		_amigaArchive = nullptr;
	}
	delete _fileStream;
	_fileStream = nullptr;
	delete _scriptExecutor;
	for (uint i = 0; i < GameObjects::instance()._objects.size(); i++) {
		delete GameObjects::instance()._objects[i];
	}
	GameObjects::instance()._objects.clear();
	delete Scenes::instance()._currentSceneScript;
	delete Scenes::instance()._currentSceneStrings;
	Scenes::instance()._currentSceneScript = nullptr;
	Scenes::instance()._currentSceneStrings = nullptr;
}

void Macs2Engine::sayText(const Common::String &text, Common::TextToSpeechManager::Action action) const {
#ifdef USE_TTS
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan && ConfMan.getBool("tts_enabled")) {
		ttsMan->say(text, action);
	}
#endif
}

void Macs2Engine::syncSoundSettings() {
	Engine::syncSoundSettings();

	if (_music && _scriptExecutor) {
		int musicVolume = ConfMan.getInt("music_volume");
		// OPL emulator is registered as kPlainSoundType; mute it at mixer level
		// when user sets music volume to 0 (OPL attenuation 0x3F is not true silence).
		_mixer->muteSoundType(Audio::Mixer::kPlainSoundType,
							  (musicVolume == 0) || (ConfMan.hasKey("mute") && ConfMan.getBool("mute")));
		_music->setVolume(scaledMusicVolume(_scriptExecutor->_musicControlVolume));
		_music->setSmfVolumeFromAttenuation(_scriptExecutor->_musicControlVolume);
	}

	// TalkVol (setWaveVolume): percent of speech loudness when set.
	if (_talkVol > 0 && _talkVol <= 100) {
		const int speechVolume = ConfMan.getInt("speech_volume");
		const int combined = MIN(255, (speechVolume * (int)_talkVol) / 100);
		_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, combined);
	}
}

uint16 Macs2Engine::scaledMusicVolume(uint16 gameAttenuation) const {
	// _masterVolume in Adlib is OPL register-level attenuation: 0 = loudest, 0x3F = silent.
	// _musicControlVolume (gameAttenuation) is also attenuation (0 = loud, 0x3F = silent).
	// Convert user's music_volume (0-255) to an attenuation (0x3F..0) and add both.
	bool mute = ConfMan.hasKey("mute") && ConfMan.getBool("mute");
	if (mute)
		return 0x3F;
	int musicVolume = ConfMan.getInt("music_volume");
	uint16 userAttenuation = 0x3F - (musicVolume * 0x3F / 255);
	uint16 total = gameAttenuation + userAttenuation;
	return (total > 0x3F) ? 0x3F : total;
}

bool Macs2Engine::loadSceneGraphics(uint32 sceneIndex) {
	if (isAmiga()) {
		uint32 sceneResourceId = 0;
		if (sceneIndex > 0)
			sceneResourceId = sceneIndex - 1;

		if (sceneResourceId != 0 && loadAmigaSceneBackground(sceneResourceId))
			return true;

		_sceneBackground.fillRect(Common::Rect(0, 0, kScreenWidth, kGameHeight), 0);
		_depthMap.fillRect(Common::Rect(0, 0, kScreenWidth, kGameHeight), 0);
		_pathfindingMap.fillRect(Common::Rect(0, 0, kScreenWidth, kGameHeight), 0);
		_shadowMap.fillRect(Common::Rect(0, 0, kScreenWidth, kGameHeight), 0);
		_hotspotMap.fillRect(Common::Rect(0, 0, kScreenWidth, kGameHeight), 0);
		_numHotspots = 0;
		_numPathfindingPoints = 0;
		_walkDepthThresholdY = 100;
		_walkDepthScaleFactor = 100;
		_walkBaseSpeedPct = 100;
		_scenePaletteMode = 1;
		_paletteDarkenPercent = 0;
		applyAmigaUiPalette();
		warning("Amiga: no MM_%04u in DataA (script scene %u)",
				(uint)sceneResourceId, (uint)sceneIndex);
		return true;
	}

	if (isV2())
		return loadSceneGraphicsV2(sceneIndex);

	return loadSceneGraphicsV1(sceneIndex);
}

bool Macs2Engine::loadSceneGraphicsV1(uint32 sceneIndex) {
	const uint32 newSceneIndex = sceneIndex;

	// Background image
	// [0752h] is pointing to 3000h bytes data starting at Ch + 4h in the file
	// Addressing the background image starts at l0037_25A9
	_fileStream->seek(0xC + 0x4 + 0xC * newSceneIndex - 0xC, SEEK_SET);
	uint32 bgImageOffset = _fileStream->readUint32LE();
	uint32 sceneTableEntry2 = _fileStream->readUint32LE();
	uint32 sceneTableEntry3 = _fileStream->readUint32LE();
	(void)sceneTableEntry3; // strings offset, not used here
	_mapSubSceneTableFilePos = 0;
	_mapImageFileOffset = 0;
	// The map image file offset is stored in the scene data block at offset +0x3C0.
	// (sceneDataOffset2 + 0x3C0 = resource_offsets(0x80) + 0x340 of additional data).
	if (sceneTableEntry2 != 0 && sceneTableEntry2 < (uint32)_fileStream->size()) {
		_fileStream->seek(sceneTableEntry2 + 0x3C0, SEEK_SET);
		uint32 mapOffset = _fileStream->readUint32LE();
		if (mapOffset != 0 && mapOffset < (uint32)_fileStream->size()) {
			// Validate it's actually RLE data for a kScreenWidth-wide image
			_fileStream->seek(mapOffset, SEEK_SET);
			uint16 rowLen = _fileStream->readUint16LE();
			if (rowLen >= 50 && rowLen <= 640) {
				_mapImageFileOffset = mapOffset;
				_mapSubSceneTableFilePos = sceneTableEntry2 + 0x3C0;
			}
		}
	}
	_fileStream->seek(bgImageOffset, SEEK_SET);

	// TODO: Copy-pasted code here

	uint8 data[0x320];

	for (int y = 0; y < gameHeight(); y++) {
		// TODO: Use the proper read function, it seems to be available
		uint16 length = _fileStream->readUint16LE();
		_fileStream->read(data, length);
		int16 remainingPixels = screenWidth(); // signed: see readRLEImage (matches decodeRLERows 1008:0666)
		uint8 *dataPointer = data;
		uint16 x = 0;
		while (remainingPixels > 0) {
			const uint8 &value = dataPointer[0];
			dataPointer++;
			if (value != 0xF0) {
				_sceneBackground.setPixel(x, y, value);
				remainingPixels--;
				x++;
			} else {
				// We need to decode the RLE data
				const uint8 &runlength = dataPointer[0];
				dataPointer++;
				const uint8 &encodedValue = dataPointer[0];
				dataPointer++;
				for (int i = 0; i < runlength && x < screenWidth(); i++) {
					_sceneBackground.setPixel(x++, y, encodedValue);
				}
				remainingPixels -= runlength;
			}
		}
	}

	// Palette is 0x300 bytes (256 RGB triples). Keep an uncorrected copy for fades.
	readPalette(_fileStream, _palVanilla);
	_pal = _palVanilla;
	expandPalette6To8(_pal);

	// changeScene @ 1008:2574: 0x100-byte panel remap table (scene+0x1006 area, NOT shading table)
	if (_panelRemapTable.size() != 0x100)
		_panelRemapTable.resize(0x100);
	_fileStream->read(_panelRemapTable.data(), 0x100);

	_fileStream->readByte(); // unknownByte1
	_fileStream->readByte(); // unknownByte2
	_fileStream->readByte(); // unknownByte3

	// Offset 1013h
	Graphics::ManagedSurface depthRLE = readRLEImage(_fileStream->pos(), _fileStream);
	// Confirmed: depth map at scene offset 0x1013
	_depthMap.blitFrom(depthRLE);
	_sceneDepthMap.copyFrom(_depthMap);

	// Offset 2017h
	Graphics::ManagedSurface pathfindingRLE = readRLEImage(_fileStream->pos(), _fileStream);
	// Walkability/pathfinding map at scene offset 0x2017
	_pathfindingMap.blitFrom(pathfindingRLE);

	// Offset 301Bh - Shadow/shading intensity map for character rendering
	Graphics::ManagedSurface shadowRLE = readRLEImage(_fileStream->pos(), _fileStream);
	_shadowMap.blitFrom(shadowRLE);

	// Offset 401Fh - Hotspot/interaction map (320x200, pixel value = hotspot color index)
	Graphics::ManagedSurface bgMap = readRLEImage(_fileStream->pos(), _fileStream);
	_hotspotMap.copyFrom(bgMap);

	// Pretty sure that this is the pathfinding points. We address them starting
	// Load pathfinding nodes (16 entries x 10 bytes at scene+0x5023)
	_pathfindingPoints.clear();
	for (int i = 0; i < 16; i++) {
		PathfindingPoint current;
		current._index = i;
		current._position.x = _fileStream->readUint16LE();
		current._position.y = _fileStream->readUint16LE();
		uint8 adj[4];
		_fileStream->read(adj, 4);
		uint16 numConnections = _fileStream->readUint16LE();
		current._adjacentPoints.clear();
		for (uint16 j = 0; j < numConnections && j < 4; j++)
			current._adjacentPoints.push_back(adj[j]);
		_pathfindingPoints.push_back(current);
	}

	_numHotspots = _fileStream->readUint16LE();

	_hotspotColorTable.clear();
	_hotspotColorTable.resize(0x20 / sizeof(uint16));
	_fileStream->read(_hotspotColorTable.data(), 0x20);

	// TODO: Remove the now superfluous one
	readBackgroundAnimations(_fileStream);
	updateAllBackgroundAnimationDepthMaps();

	// Offset 51F7h
	_numPathfindingPoints = _fileStream->readUint16LE();

	// Offset 51F9h
	_fileStream->readUint16LE();

	// Offset 51FBh
	_fileStream->readUint16LE();

	// Offset 51FDh - 5201h
	_walkDepthThresholdY = _fileStream->readUint16LE();
	_walkDepthScaleFactor = _fileStream->readUint16LE();
	_walkBaseSpeedPct = _fileStream->readUint16LE();

	_scenePaletteMode = _fileStream->readUint16LE();
	_paletteDarkenPercent = _fileStream->readUint16LE();

	// Seek to next place
	// TODO: Duplicated seek address calculation code
	// This addressing can be found in the l0037_2856 code block

	uint16 sceneDataOffset = newSceneIndex * 0xC;
	// Offset of the data in [0752h] global
	constexpr uint16 globalDataOffset = 0xC + 0x4;
	sceneDataOffset += globalDataOffset;
	_fileStream->seek(sceneDataOffset - 0x8);
	uint32 sceneDataOffset2 = _fileStream->readUint32LE();
	_fileStream->seek(sceneDataOffset2, SEEK_SET);

	// We read 80h bytes
	_sceneResourceOffsets.resize(0x80 / 4);
	_fileStream->read(_sceneResourceOffsets.data(), 0x80);

	// TODO: There are some more data points missing from the function

	// Apply palette darkening if this scene has it (binary: sceneData+0x5203 != 1)
	applyPaletteDarkening();

	return true;
}

bool Macs2Engine::loadSceneGraphicsV2(uint32 sceneIndex) {
	if (_fileStream == nullptr)
		return false;

	Common::SeekableReadStream *stream = _fileStream;
	stream->seek(_mcsDirectoryOffset + 0xC * sceneIndex - 0xC, SEEK_SET);
	const uint32 bgImageOffset = stream->readUint32LE();
	(void)stream->readUint32LE();
	(void)stream->readUint32LE();

	if (bgImageOffset == 0 || bgImageOffset >= (uint32)stream->size())
		return false;

	stream->seek(bgImageOffset, SEEK_SET);
	if (!readMegaPicImage(stream, kWinScreenWidth, kWinGameHeight, _sceneBackground))
		return false;

	readPalette(stream, _palVanilla);
	_pal = _palVanilla;
	expandPalette6To8(_pal);

	if (_panelRemapTable.size() != 0x100)
		_panelRemapTable.resize(0x100);
	stream->read(_panelRemapTable.data(), 0x100);
	stream->readByte();
	stream->readByte();
	stream->readByte();
	_shadingTable.clear();
	_shadingTable.resize(0x2000, 0);
	if (stream->read(_shadingTable.data(), 0x2000) != 0x2000)
		return false;

	Graphics::ManagedSurface depthFull;
	if (!readMegaPicImage(stream, kWinScreenWidth, kWinGameHeight, depthFull))
		return false;
	_depthMap.copyFrom(depthFull);

	auto upscaleHalfRes = [](const Graphics::ManagedSurface &half, Graphics::ManagedSurface &full) {
		full.create(kWinScreenWidth, kWinGameHeight, Graphics::PixelFormat::createFormatCLUT8());
		for (int y = 0; y < half.h; y++) {
			for (int x = 0; x < half.w; x++) {
				const byte p = half.getPixel(x, y);
				const int dx = x * 2;
				const int dy = y * 2;
				full.setPixel(dx, dy, p);
				full.setPixel(dx + 1, dy, p);
				full.setPixel(dx, dy + 1, p);
				full.setPixel(dx + 1, dy + 1, p);
			}
		}
	};

	Graphics::ManagedSurface half;
	if (!readMegaPicImage(stream, kScreenWidth, kGameHeight, half))
		return false;
	upscaleHalfRes(half, _pathfindingMap);

	if (!readMegaPicImage(stream, kScreenWidth, kGameHeight, half))
		return false;
	upscaleHalfRes(half, _shadowMap);

	if (!readMegaPicImage(stream, kScreenWidth, kGameHeight, half))
		return false;
	upscaleHalfRes(half, _hotspotMap);

	_pathfindingPoints.clear();
	for (int i = 0; i < 16; i++) {
		PathfindingPoint current;
		current._index = i;
		current._position.x = (int16)(stream->readUint16LE() << 1);
		current._position.y = (int16)(stream->readUint16LE() << 1);
		uint8 adj[8];
		stream->read(adj, 8);
		stream->skip(8);
		const uint16 numConnections = stream->readUint16LE();
		current._adjacentPoints.clear();
		for (uint16 j = 0; j < numConnections && j < 4; j++)
			current._adjacentPoints.push_back(adj[j]);
		_pathfindingPoints.push_back(current);
	}
	stream->skip(0x2c0 - 0x160);

	_numHotspots = stream->readUint16LE();
	_hotspotColorTable.clear();
	_hotspotColorTable.resize(0x40 / sizeof(uint16));
	stream->read(_hotspotColorTable.data(), 0x40);

	const uint16 numBackgroundAnimations = stream->readUint16LE();
	_backgroundAnimations.clear();
	_backgroundAnimationsBlobs.clear();
	_backgroundAnimations.resize(numBackgroundAnimations);
	_backgroundAnimationsBlobs.resize(numBackgroundAnimations);
	for (uint16 i = 0; i < numBackgroundAnimations; i++) {
		BackgroundAnimationBlob &currentBlob = _backgroundAnimationsBlobs[i];
		BackgroundAnimation &current = _backgroundAnimations[i];
		const uint16 halfX = stream->readUint16LE();
		const uint16 halfY = stream->readUint16LE();
		const uint32 animSize = stream->readUint32LE();
		currentBlob._blob.clear();
		if (animSize > 0 && animSize < 0x1000000) {
			currentBlob._blob.resize(animSize);
			if (stream->read(currentBlob._blob.data(), animSize) != animSize)
				return false;
		}
		currentBlob._unknown0C = stream->readUint16LE();
		(void)stream->readByte();
		const uint8 flagX = stream->readByte();
		const uint8 flagY = stream->readByte();
		currentBlob._unknown0E = stream->readByte();
		(void)stream->readByte();

		uint16 x = (uint16)(halfX << 1);
		uint16 y = (uint16)(halfY << 1);
		if (flagX)
			x = (uint16)(x + 1);
		if (flagY)
			y = (uint16)(y + 1);
		current._x = x;
		current._y = y;
		currentBlob._x = x;
		currentBlob._y = y;

		AnimBlobView blobView(currentBlob._blob);
		const uint16 numFrames = blobView.isValid() ? blobView.sequenceLength() : 0;
		current._frameIndex = 0;
		current._frames.resize(numFrames);
		const uint16 actualFrameCount = blobView.isValid() ? blobView.frameCount() : 0;
		for (uint16 j = 0; j < actualFrameCount && j < numFrames; j++) {
			AnimBlobView::FrameInfo fi;
			if (!blobView.getFrameInfo(j, fi))
				break;
			current._frames[j]._width = fi.width;
			current._frames[j]._height = fi.height;
			current._frames[j]._data.resize((uint)fi.width * (uint)fi.height);
			memcpy(current._frames[j]._data.data(), fi.pixels, (uint)fi.width * (uint)fi.height);
		}
	}

	_numPathfindingPoints = stream->readUint16LE();
	if (_numPathfindingPoints == 0 || _numPathfindingPoints > 16)
		_numPathfindingPoints = 16;
	(void)stream->readUint16LE();
	(void)stream->readUint16LE();
	_walkDepthThresholdY = (uint16)(stream->readUint16LE() << 1);
	_walkDepthScaleFactor = stream->readUint16LE();
	_walkBaseSpeedPct = stream->readUint16LE();
	_scenePaletteMode = stream->readUint16LE();
	_paletteDarkenPercent = stream->readUint16LE();

	_mapImageFileOffset = 0;
	_mapSubSceneTableFilePos = 0;

	stream->seek(_mcsDirectoryOffset + 0xC * sceneIndex - 0x8, SEEK_SET);
	const uint32 scriptBlobOffset = stream->readUint32LE();
	_sceneResourceOffsets.clear();
	clearDeltaAnim();
	if (scriptBlobOffset != 0 && scriptBlobOffset < (uint32)stream->size()) {
		const int64 saved = stream->pos();
		stream->seek(scriptBlobOffset, SEEK_SET);
		_sceneResourceOffsets.resize(0x200 / 4);
		if (stream->read(_sceneResourceOffsets.data(), 0x200) != 0x200)
			_sceneResourceOffsets.clear();
		stream->seek(saved, SEEK_SET);
	}

	applyPaletteDarkening();
	return true;
}


void Macs2Engine::changeScene(uint32 newSceneIndex, bool executeScript) {
	// Release old scene resources
	_backgroundAnimations.clear();
	_backgroundAnimationsBlobs.clear();
	memset(_areaOverrides, 0, sizeof(_areaOverrides));

	if (isAmiga()) {
		// Amiga scripts use scene ids = MM_resource_id + 1 (Ghidra FUN_002215fa
		// subtracts 1 before load_scene_mxmm; load_scene sets curScene = mmId+1).
		_amigaPendingSceneScript.clear();
		_amigaPendingSceneStrings.clear();

		if (!loadSceneGraphics(newSceneIndex))
			error("changeScene(): Failed to load scene graphics for scene %u", newSceneIndex);

		_menuMode = MenuMode::Main;
		_optionsSubMode = OptionsSubMode::None;
		_bottomHudVisible = true;

		View1 *currentView = (View1 *)findView("View1");
		if (currentView != nullptr) {
			// Do not push _pal here: scriptChangeScene fades the previous
			// palette to black, then fades the new scene in.
			for (auto currentCharacter : currentView->_characters) {
				if (currentCharacter->_gameObject != nullptr)
					_scriptExecutor->saveWalkRuntime(currentCharacter, currentCharacter->_gameObject);
				delete currentCharacter;
			}
			currentView->_characters.clear();
			currentView->flushPendingCharacterDeletes();

			GameObject *actorObject = GameObjects::getObjectByIndex(Scenes::instance()._currentActorIndex);
			if (actorObject != nullptr && actorObject->_sceneIndex == newSceneIndex) {
				Character *actorChar = new Character();
				actorChar->_gameObject = actorObject;
				currentView->_characters.push_back(actorChar);
				_scriptExecutor->restoreWalkRuntime(actorChar, actorObject);
				resetCharacterWalkPath(actorChar);
				_scriptExecutor->saveWalkRuntime(actorChar, actorObject);
			}
			for (auto currentObject : GameObjects::instance()._objects) {
				if (currentObject == nullptr)
					continue;
				if (currentObject->_sceneIndex == newSceneIndex &&
					currentObject->_index != Scenes::instance()._currentActorIndex &&
					currentObject->_dataOffset != 0) {
					Character *c = new Character();
					c->_gameObject = currentObject;
					currentView->_characters.push_back(c);
					_scriptExecutor->restoreWalkRuntime(c, currentObject);
					resetCharacterWalkPath(c);
					_scriptExecutor->saveWalkRuntime(c, currentObject);
				}
			}
			currentView->rebuildCharacterLookupTable();
			currentView->_backgroundSurface.copyFrom(_sceneBackground);
			if (executeScript)
				currentView->_paletteDirty = true;
		}

		Scenes::instance()._lastSceneIndex = Scenes::instance()._currentSceneIndex;
		Scenes::instance()._currentSceneIndex = newSceneIndex;
		_scriptExecutor->releaseObjectStream();
		delete Scenes::instance()._currentSceneScript;
		delete Scenes::instance()._currentSceneStrings;

		byte *scriptCopy = nullptr;
		uint32 scriptSize = 0;
		byte *stringCopy = nullptr;
		uint32 stringSize = 0;

		if (!_amigaPendingSceneScript.empty()) {
			scriptSize = _amigaPendingSceneScript.size();
			scriptCopy = (byte *)malloc(scriptSize);
			if (scriptCopy)
				memcpy(scriptCopy, _amigaPendingSceneScript.data(), scriptSize);
			else
				scriptSize = 0;
		}
		if (!_amigaPendingSceneStrings.empty()) {
			stringSize = _amigaPendingSceneStrings.size();
			stringCopy = (byte *)malloc(stringSize);
			if (stringCopy)
				memcpy(stringCopy, _amigaPendingSceneStrings.data(), stringSize);
			else
				stringSize = 0;
		}
		_amigaPendingSceneScript.clear();
		_amigaPendingSceneStrings.clear();

		// Fallback: global scene_table MXOO stub (usually just opcode 0x18 in the demo).
		if (scriptSize == 0 && _fileStream != nullptr) {
			AmigaMxooInfo sceneInfo;
			const uint32 tableSize = (uint32)_fileStream->size();
			Common::Array<byte> table;
			table.resize(tableSize);
			_fileStream->seek(0);
			if (_fileStream->read(table.data(), tableSize) == tableSize &&
				parseAmigaMxoo(table.data(), tableSize, sceneInfo)) {
				Common::Array<byte> script;
				Common::Array<byte> strings;
				if (extractAmigaScript(table.data(), tableSize, script) && !script.empty()) {
					scriptSize = script.size();
					scriptCopy = (byte *)malloc(scriptSize);
					if (scriptCopy)
						memcpy(scriptCopy, script.data(), scriptSize);
					else
						scriptSize = 0;
				}
				if (stringSize == 0 && extractAmigaStringBlock(table.data(), tableSize, strings)) {
					stringSize = strings.size();
					stringCopy = (byte *)malloc(stringSize);
					if (stringCopy && stringSize)
						memcpy(stringCopy, strings.data(), stringSize);
					else
						stringSize = 0;
				}
			}
		}

		Scenes::instance()._currentSceneScript = new Common::MemoryReadStream(scriptCopy, scriptSize, DisposeAfterUse::YES);
		Scenes::instance()._currentSceneStrings = new Common::MemoryReadStream(stringCopy, stringSize, DisposeAfterUse::YES);
		Scenes::instance()._currentSceneSpecialAnimOffsets.clear();
		_scriptExecutor->setScript(Scenes::instance()._currentSceneScript);

		_pathfindingOverrides.clear();
		for (uint i = 0; i < _hotspotOverrides.size(); i++)
			_hotspotOverrides[i] = 0xFFFF;

		// Match DOS changeScene: when View1 is not up yet (first call from
		// readAmigaResources), only load scene data. Entry init/repeat - including
		// intro frameWait/changeScene - must run from View1::tick so waits can
		// complete in the game loop.
		if (executeScript && currentView != nullptr)
			_scriptExecutor->runSceneEntryScriptPasses();
		return;
	}

	if (!loadSceneGraphics(newSceneIndex))
		error("changeScene(): Failed to load scene graphics for scene %u", newSceneIndex);

	// Scene change starts with the main HUD shown. v2 scripts may hide it
	// during init (overview map). v1 has no hide/show opcodes; kEnhUIUX uses
	// the same flag so a scene change restores the strip.
	_menuMode = MenuMode::Main;
	_optionsSubMode = OptionsSubMode::None;
	_bottomHudVisible = true;

	// Refresh characters
	View1 *currentView = (View1 *)findView("View1");
	if (!currentView) {
		// View system not yet initialized (first call from readResourceFile).
		// Scene data is loaded; view refresh will happen on first tick.
		return;
	}

	// Refresh the surface
	currentView->_backgroundSurface.copyFrom(_sceneBackground);
	// scriptChangeScene (1008:ad6e) manages palette via fade/instant-cut paths when
	// executeScript==false; avoid pushing _pal via draw() before fadePaletteToBlack.
	if (executeScript) {
		currentView->_paletteDirty = true;
	}
	currentView->handleTextBoxInput();
	currentView->_drawnStringBox.clear();
	currentView->_continueScriptAfterUI = false;
	currentView->currentSpeechActData = SpeechActData();
	currentView->_pendingPanelRequest = View1::kPanelRequestNone;
	currentView->_activeInventoryItem = nullptr;
	currentView->_uiPanelState = View1::kUiPanelNone;
	currentView->clearOverlayTextEntries();
	_scriptExecutor->_inventoryActionFlag = false;
	_scriptExecutor->_inventoryCombineFlag = false;

	for (auto currentCharacter : currentView->_characters) {
		if (currentCharacter->_gameObject != nullptr)
			_scriptExecutor->saveWalkRuntime(currentCharacter, currentCharacter->_gameObject);
		delete currentCharacter;
	}
	currentView->_characters.clear();
	currentView->flushPendingCharacterDeletes();
	// Binary changeScene (1008:2574): loadObjectData for scene objects except current actor.
	GameObject *actorObject = GameObjects::getObjectByIndex(Scenes::instance()._currentActorIndex);
	if (actorObject != nullptr && actorObject->_sceneIndex == newSceneIndex) {
		if (isV2())
			loadObjectData(actorObject);
		Character *actorChar = new Character();
		actorChar->_gameObject = actorObject;
		currentView->_characters.push_back(actorChar);
		_scriptExecutor->restoreWalkRuntime(actorChar, actorObject);
		resetCharacterWalkPath(actorChar);
		_scriptExecutor->saveWalkRuntime(actorChar, actorObject);
	}
	for (auto currentObject : GameObjects::instance()._objects) {
		if (currentObject == nullptr)
			continue;
		if (currentObject->_sceneIndex == newSceneIndex &&
			currentObject->_index != Scenes::instance()._currentActorIndex &&
			currentObject->_dataOffset != 0 &&
			loadObjectData(currentObject)) {
			Character *c = new Character();
			c->_gameObject = currentObject;
			currentView->_characters.push_back(c);
			_scriptExecutor->restoreWalkRuntime(c, currentObject);
			resetCharacterWalkPath(c);
			_scriptExecutor->saveWalkRuntime(c, currentObject);
		}
	}

	currentView->rebuildCharacterLookupTable();

	// Load the script and execute it
	Scenes::instance()._lastSceneIndex = Scenes::instance()._currentSceneIndex;
	Scenes::instance()._currentSceneIndex = newSceneIndex;
	// Free any object-specific stream before deleting the scene script
	_scriptExecutor->releaseObjectStream();
	delete Scenes::instance()._currentSceneScript;
	delete Scenes::instance()._currentSceneStrings;
	Scenes::instance()._currentSceneScript = Scenes::instance().readSceneScript(newSceneIndex, _fileStream);
	Scenes::instance()._currentSceneStrings = Scenes::instance().readSceneStrings(newSceneIndex, _fileStream);
	Scenes::instance()._currentSceneSpecialAnimOffsets = Scenes::instance().readSpecialAnimsOffsets(newSceneIndex, _fileStream);
	_scriptExecutor->setScript(Scenes::instance()._currentSceneScript);

	// Reset overrides before running the new scene's script (original placement:
	// memsetBytes(0, 200, sceneData+0x528D) and memsetBytes(0xffff, 0x20, sceneData+0x5BD3)
	// happen after all scene data is loaded, before script execution)
	_pathfindingOverrides.clear();
	for (uint i = 0; i < _hotspotOverrides.size(); i++) {
		_hotspotOverrides[i] = 0xFFFF;
	}

	saveAutosaveIfEnabled();

	if (executeScript) {
		_scriptExecutor->runSceneEntryScriptPasses();
	}
}

bool Macs2Engine::resolveResourceFileOffset(uint8 resourceIndex, uint16 executingObjectId, uint32 &outOffset) const {
	outOffset = 0;
	if (resourceIndex == 0 || _fileStream == nullptr)
		return false;

	if (executingObjectId == 0) {
		if (resourceIndex > _sceneResourceOffsets.size())
			return false;
		outOffset = _sceneResourceOffsets[resourceIndex - 1];
	} else {
		GameObject *object = GameObjects::getObjectByIndex(executingObjectId);
		if (object == nullptr || object->_dataOffset == 0)
			return false;
		if ((uint)(resourceIndex - 1) >= maxObjectResources())
			return false;
		outOffset = object->_resourceOffsets[resourceIndex - 1];
	}
	return outOffset != 0 && outOffset < (uint32)_fileStream->size();
}

bool Macs2Engine::loadSizedResourcePayload(uint8 resourceIndex, uint16 executingObjectId,
										   Common::Array<uint8> &outPayload) {
	outPayload.clear();
	uint32 address = 0;
	if (!resolveResourceFileOffset(resourceIndex, executingObjectId, address))
		return false;

	const int64 oldPos = _fileStream->pos();
	_fileStream->seek(address, SEEK_SET);
	const uint32 size = _fileStream->readUint32LE();
	if (size == 0 || size > 0x1000000) {
		_fileStream->seek(oldPos, SEEK_SET);
		return false;
	}
	outPayload.resize(size);
	if (_fileStream->read(outPayload.data(), size) != size) {
		outPayload.clear();
		_fileStream->seek(oldPos, SEEK_SET);
		return false;
	}
	_fileStream->seek(oldPos, SEEK_SET);
	return !outPayload.empty();
}

bool Macs2Engine::loadAhffAnimResource(uint8 resourceIndex, uint16 executingObjectId,
									   Common::Array<uint8> &outBlob) {
	Common::Array<uint8> payload;
	if (!loadSizedResourcePayload(resourceIndex, executingObjectId, payload))
		return false;
	if (payload.size() < 12 || memcmp(payload.data(), "AHFFANIM0100", 12) != 0)
		return false;
	outBlob.clear();
	outBlob.resize(payload.size() - 12);
	if (!outBlob.empty())
		memcpy(outBlob.data(), payload.data() + 12, outBlob.size());
	return !outBlob.empty();
}

bool Macs2Engine::readMegaPicImage(Common::SeekableReadStream *stream, int width, int height,
								   Graphics::ManagedSurface &out) {
	if (stream == nullptr || width <= 0 || height <= 0)
		return false;

	out.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	Common::Array<byte> rowBuf;
	rowBuf.resize(3000);

	for (int y = 0; y < height; y++) {
		uint16 packedLen = stream->readUint16LE();
		if (packedLen == 0 || packedLen > 2999)
			return false;
		if (stream->read(rowBuf.data(), packedLen) != packedLen)
			return false;

		int x = 0;
		uint i = 0;
		while (x < width && i < packedLen) {
			const byte code = rowBuf[i++];
			if (code < 0x80) {
				const uint run = code;
				for (uint n = 0; n < run && x < width; n++) {
					if (i >= packedLen)
						return false;
					out.setPixel(x++, y, rowBuf[i++]);
				}
			} else {
				if (i >= packedLen)
					return false;
				const byte value = rowBuf[i++];
				const uint run = code & 0x7F;
				for (uint n = 0; n < run && x < width; n++)
					out.setPixel(x++, y, value);
			}
		}
	}
	return true;
}

bool Macs2Engine::loadMaskFromResource(uint8 resourceIndex, uint16 executingObjectId,
									   Graphics::ManagedSurface &dest, int megapicW, int megapicH,
									   bool upscaleHalfRes) {
	uint32 address = 0;
	if (!resolveResourceFileOffset(resourceIndex, executingObjectId, address))
		return false;

	const int64 oldPos = _fileStream->pos();
	_fileStream->seek(address, SEEK_SET);
	(void)_fileStream->readUint32LE(); // size header skipped by Load*Mask
	Graphics::ManagedSurface half;
	Graphics::ManagedSurface &target = upscaleHalfRes ? half : dest;
	if (!readMegaPicImage(_fileStream, megapicW, megapicH, target)) {
		_fileStream->seek(oldPos, SEEK_SET);
		return false;
	}
	if (upscaleHalfRes) {
		dest.create(megapicW * 2, megapicH * 2, Graphics::PixelFormat::createFormatCLUT8());
		for (int y = 0; y < half.h; y++) {
			for (int x = 0; x < half.w; x++) {
				const byte p = half.getPixel(x, y);
				const int dx = x * 2;
				const int dy = y * 2;
				dest.setPixel(dx, dy, p);
				dest.setPixel(dx + 1, dy, p);
				dest.setPixel(dx, dy + 1, p);
				dest.setPixel(dx + 1, dy + 1, p);
			}
		}
	}
	_fileStream->seek(oldPos, SEEK_SET);
	return true;
}

void Macs2Engine::clearDeltaAnim() {
	_deltaAnim.clear(screenWidth(), gameHeight());
}

bool Macs2Engine::loadDeltaAnimResource(uint8 resourceIndex, uint16 executingObjectId, bool forceSkipSpeed1) {
	uint32 address = 0;
	if (!resolveResourceFileOffset(resourceIndex, executingObjectId, address))
		return false;

	const int64 oldPos = _fileStream->pos();
	_fileStream->seek(address, SEEK_SET);
	const uint32 size = _fileStream->readUint32LE();
	char magic[8];
	if (_fileStream->read(magic, 8) != 8 || memcmp(magic, "AHFFDLTA", 8) != 0) {
		_fileStream->seek(oldPos, SEEK_SET);
		return false;
	}
	_fileStream->skip(4); // remainder of 16-byte header after size

	// LoadDeltaAnim SkipSpeed layouts:
	//   1: frameCount, 0x1000 offset table, skip 0x2000, palette, frames
	//   2: frameCount, skip 0x1000, 0x1000 table, skip 0x1000; frame counts halved
	//   else: frameCount, skip 0x2000, 0x1000 table; frame counts / 3
	// CheckDeltaSpeed always uses layout 1 regardless of SkipSpeed.
	uint16 frameCount = _fileStream->readUint16LE();
	if (frameCount == 0 || frameCount > 512) {
		_fileStream->seek(oldPos, SEEK_SET);
		return false;
	}

	uint16 skipSpeed = (_skipSpeed >= 1 && _skipSpeed <= 4) ? _skipSpeed : 1;
	if (forceSkipSpeed1)
		skipSpeed = 1;
	Common::Array<uint32> relOffsets;
	relOffsets.resize(512);
	// FBlockRead(0x1000): 512 uint32 offsets (0x800) plus 0x800 trailing bytes.
	auto readOffsetTable1000 = [&]() {
		for (uint i = 0; i < 512; i++)
			relOffsets[i] = _fileStream->readUint32LE();
		_fileStream->skip(0x800);
	};
	if (skipSpeed == 1) {
		readOffsetTable1000();
		_fileStream->skip(0x2000);
	} else if (skipSpeed == 2) {
		_fileStream->skip(0x1000);
		readOffsetTable1000();
		_fileStream->skip(0x1000);
		frameCount = (uint16)(((uint32)frameCount + 1) >> 1);
		if (frameCount > 0)
			frameCount--;
	} else {
		_fileStream->skip(0x2000);
		readOffsetTable1000();
		frameCount = (uint16)(((uint32)frameCount + 1) / 3);
		if (frameCount > 0)
			frameCount--;
	}
	if (frameCount == 0 || frameCount > 512) {
		_fileStream->seek(oldPos, SEEK_SET);
		return false;
	}
	const uint16 numFrames = frameCount;

	// Scripts call addDeltaSfx before playDiskDelta; keep the pending SFX list.
	Common::Array<DeltaSfxEvent> savedSfx = Common::move(_deltaAnim.sfxEvents);
	clearDeltaAnim();
	_deltaAnim.sfxEvents = Common::move(savedSfx);
	readPalette(_fileStream, _deltaAnim.palette);
	_deltaAnim.frames.resize(numFrames);
	_deltaAnim.frameCount = numFrames;
	_deltaAnim.loaded = true;

	const uint32 base = address + 4;
	for (uint16 fi = 0; fi < numFrames; fi++) {
		const uint32 absOff = relOffsets[fi] + base;
		if (absOff >= (uint32)_fileStream->size())
			continue;
		_fileStream->seek(absOff, SEEK_SET);
		const uint16 stripCount = _fileStream->readUint16LE();
		DeltaFrame &frame = _deltaAnim.frames[fi];
		frame.strips.clear();
		if (stripCount == 0 || stripCount > 400)
			continue;
		frame.strips.resize(stripCount);
		for (uint16 si = 0; si < stripCount; si++) {
			frame.strips[si].y = _fileStream->readUint16LE();
			const uint16 rleSize = _fileStream->readUint16LE();
			if (rleSize == 0 || rleSize > 0x8000)
				break;
			frame.strips[si].rle.resize(rleSize);
			if (_fileStream->read(frame.strips[si].rle.data(), rleSize) != rleSize) {
				frame.strips[si].rle.clear();
				break;
			}
		}
	}

	(void)size;
	_fileStream->seek(oldPos, SEEK_SET);
	return _deltaAnim.loaded;
}

void Macs2Engine::applyDeltaFrameToBackground(const DeltaFrame &frame) {
	if (_sceneBackground.w <= 0 || _sceneBackground.h <= 0)
		return;

	for (const DeltaStrip &strip : frame.strips) {
		const int y = (int)strip.y;
		if (y < (int)_deltaAnim.clipMiY || y > (int)_deltaAnim.clipMaY)
			continue;
		if (y < 0 || y >= _sceneBackground.h)
			continue;
		if (strip.rle.empty())
			continue;

		const uint8 *p = strip.rle.data();
		const uint8 *end = p + strip.rle.size();
		int x = 0;
		while (p + 4 <= end) {
			const int16 skip = (int16)READ_LE_UINT16(p);
			p += 2;
			uint16 runLen = READ_LE_UINT16(p);
			p += 2;
			x += skip;
			if (runLen == 0)
				break;
			while (runLen != 0 && p < end) {
				uint8 code = *p++;
				if (code < 0x80) {
					uint16 n = code;
					if (n > runLen)
						n = runLen;
					for (uint16 i = 0; i < n && p < end; i++, x++) {
						if (x >= (int)_deltaAnim.clipMiX && x <= (int)_deltaAnim.clipMaX &&
							x >= 0 && x < _sceneBackground.w)
							_sceneBackground.setPixel(x, y, *p);
						p++;
					}
					runLen = (uint16)(runLen - n);
				} else {
					uint16 n = (uint16)(code - 0x80);
					if (n > runLen)
						n = runLen;
					if (p >= end)
						break;
					const uint8 val = *p++;
					for (uint16 i = 0; i < n; i++, x++) {
						if (x >= (int)_deltaAnim.clipMiX && x <= (int)_deltaAnim.clipMaX &&
							x >= 0 && x < _sceneBackground.w)
							_sceneBackground.setPixel(x, y, val);
					}
					runLen = (uint16)(runLen - n);
				}
			}
		}
	}
}

void Macs2Engine::playDeltaFrameSfx(uint16 displayFrame) {
	for (const DeltaSfxEvent &ev : _deltaAnim.sfxEvents) {
		if (ev.frameIndex != displayFrame || ev.fileName.empty())
			continue;
		if (ev.duckMusic)
			getMusic()->setSmfDucked(true, _talkVol);
		const Common::String base = Script::ScriptExecutor::stripAudioExtension(ev.fileName);
		playDigitalAudioFile(Common::Path("SOUNDFX").join(base), false);
	}
}

bool Macs2Engine::startDeltaPlayback(uint16 startFrame, uint16 endFrame, uint16 speedTicks, bool applyPalette) {
	if (!_deltaAnim.loaded || _deltaAnim.frameCount == 0)
		return false;
	uint16 start = startFrame ? startFrame : 1;
	uint16 end = endFrame;
	if (end == 0 || end > _deltaAnim.frameCount)
		end = _deltaAnim.frameCount;
	if (start > end)
		start = end;
	_deltaAnim.startFrame = (uint16)(start - 1);
	_deltaAnim.endFrame = (uint16)(end - 1);
	_deltaAnim.currentFrame = _deltaAnim.startFrame;
	_deltaAnim.speedTicks = speedTicks ? speedTicks : 1;
	_deltaAnim.tickCounter = 0;
	_deltaAnim.playing = true;
	_deltaAnim.applyPaletteOnStart = applyPalette;
	if (applyPalette || _deltaAnim.currentFrame == 0) {
		_palVanilla = _deltaAnim.palette;
		_pal = _deltaAnim.palette;
		expandPalette6To8(_pal);
		g_system->getPaletteManager()->setPalette(_pal);
	}
	const uint16 displayFrame = _deltaAnim.currentFrame;
	playDeltaFrameSfx(displayFrame);
	if (displayFrame < _deltaAnim.frames.size())
		applyDeltaFrameToBackground(_deltaAnim.frames[displayFrame]);
	_deltaAnim.currentFrame++;
	if (_deltaAnim.currentFrame > _deltaAnim.endFrame)
		_deltaAnim.playing = false;
	return true;
}

bool Macs2Engine::tickDeltaPlayback() {
	if (!_deltaAnim.playing)
		return false;
	_deltaAnim.tickCounter++;
	if (_deltaAnim.tickCounter < _deltaAnim.speedTicks)
		return true;
	_deltaAnim.tickCounter = 0;

	const uint16 displayFrame = _deltaAnim.currentFrame;
	playDeltaFrameSfx(displayFrame);
	if (displayFrame < _deltaAnim.frames.size())
		applyDeltaFrameToBackground(_deltaAnim.frames[displayFrame]);
	_deltaAnim.currentFrame++;
	if (_deltaAnim.currentFrame > _deltaAnim.endFrame) {
		_deltaAnim.playing = false;
		getMusic()->setSmfDucked(false);
		return false;
	}
	return true;
}

bool Macs2Engine::loadOverlayFont(uint8 resourceIndex, uint16 executingObjectID) {
	if (isAmiga())
		return loadAmigaOverlayFont(resourceIndex);

	// Original (1008:d749): resource table offset, then seek address+0x10 and loadFontData.
	uint32 address = 0;
	if (!resolveResourceFileOffset(resourceIndex, executingObjectID, address))
		return false;

	const int64 oldPos = _fileStream->pos();
	_fileStream->seek(address + 0x10, SEEK_SET);
	const uint16 glyphCount = _fileStream->readUint16LE();
	if (glyphCount == 0 || glyphCount > 256) {
		_fileStream->seek(oldPos, SEEK_SET);
		return false;
	}

	numOverlayGlyphs = glyphCount;
	maxOverlayGlyphHeight = 0;
	for (uint i = 0; i < glyphCount; i++) {
		_overlayGlyphs[i].readFromMemory(_fileStream);
		maxOverlayGlyphHeight = MAX(maxOverlayGlyphHeight, _overlayGlyphs[i]._height);
	}
	_fileStream->seek(oldPos, SEEK_SET);
	return true;
}

bool Macs2Engine::findGlyph(char c, GlyphData &out) const {
	for (int i = 0; i < _numGlyphs; i++) {
		if (_glyphs[i]._ascii == c) {
			out = _glyphs[i];
			return true;
		}
	}
	return false;
}

// getWalkabilityAt (1008:0e8c)
// Params: (param_1=y, param_2=x)
// Bounds: x<0 || x>=screenWidth || y<0 || y>=gameHeight -> return 0
// Lookup: scene[y*4 + 0x2017] -> row pointer, then byte at [rowPtr + x]
// Values 0xC8..0xEF: override range - checks scene[value*5 + 0x4EA5]:
//   If override disabled (flag==0): returns 0xFF
//   If override enabled (flag!=0): returns scene[value*5 + 0x4EA6]
uint16 Macs2Engine::getWalkabilityAt(int16 y, int16 x) {
	if (x < 0 || x >= screenWidth() || y < 0 || y >= gameHeight() || _pathfindingMap.w == 0) {
		return 0;
	}
	uint16 value = _pathfindingMap.getPixel(x, y);
	if (value >= 0xC8 && value <= 0xEF) {
		uint16 overrideResult;
		if (getPathfindingOverride(value, overrideResult)) {
			return overrideResult;
		}
		return 0xFF;
	}
	return value;
}

void Macs2Engine::updateBackgroundAnimationDepthMap(size_t animIndex) {
	if (isV2() || _sceneDepthMap.w == 0 || animIndex >= _backgroundAnimations.size())
		return;

	BackgroundAnimation &anim = _backgroundAnimations[animIndex];
	BackgroundAnimationBlob &blobEntry = _backgroundAnimationsBlobs[animIndex];
	Common::Array<uint8> &blob = blobEntry.activeBlob();
	if (blob.empty())
		return;

	const uint32 frameStart = BackgroundAnimationBlob::advanceAnimFrame(blob, false, 0);
	if (frameStart == 0 || frameStart + 10 > blob.size())
		return;

	const uint16 pixelFrameNum = BackgroundAnimationBlob::getCurrentPixelFrameNumber(blob);
	const int16 frameOffsetX = (int16)READ_LE_UINT16(&blob[frameStart]);
	const int16 frameOffsetY = (int16)READ_LE_UINT16(&blob[frameStart + 2]);
	const uint16 width = READ_LE_UINT16(&blob[frameStart + 6]);
	const uint16 height = READ_LE_UINT16(&blob[frameStart + 8]);
	if (width == 0 || height == 0 || frameStart + 10 + (uint32)width * height > blob.size())
		return;

	const int16 baseX = (int16)anim._x + 1 + frameOffsetX;
	const int16 baseY = (int16)anim._y + frameOffsetY;
	const byte *pixels = &blob[frameStart + 10];

	if (pixelFrameNum <= 1) {
		// First pixel frame (closed gate): restore authored depth under opaque pixels.
		for (uint16 yy = 0; yy < height; yy++) {
			for (uint16 xx = 0; xx < width; xx++) {
				if (pixels[yy * width + xx] == 0)
					continue;
				const int px = baseX + (int)xx;
				const int py = baseY + (int)yy;
				if (px >= 0 && px < _depthMap.w && py >= 0 && py < _depthMap.h)
					_depthMap.setPixel(px, py, _sceneDepthMap.getPixel(px, py));
			}
		}
		return;
	}

	// Later pixel frames (open gate): walkable tiles under opaque pixels use path height.
	for (uint16 yy = 0; yy < height; yy++) {
		for (uint16 xx = 0; xx < width; xx++) {
			if (pixels[yy * width + xx] == 0)
				continue;
			const int px = baseX + (int)xx;
			const int py = baseY + (int)yy;
			if (px < 0 || px >= _depthMap.w || py < 0 || py >= _depthMap.h)
				continue;
			const uint16 walkVal = getWalkabilityAt((int16)py, (int16)px);
			if (isWalkabilityWalkable(walkVal))
				_depthMap.setPixel(px, py, (byte)walkVal);
		}
	}
}

void Macs2Engine::updateAllBackgroundAnimationDepthMaps() {
	for (size_t i = 0; i < _backgroundAnimations.size(); i++)
		updateBackgroundAnimationDepthMap(i);
}

// snapToWalkablePosition (1008:9be2)
// Params: (pTargetY, pTargetX, charY, charX)
// Modifies *pTargetY and *pTargetX in place.
void Macs2Engine::snapToWalkablePosition(int16 *pTargetY, int16 *pTargetX, int16 charY, int16 charX) {
	int16 savedX = *pTargetX;
	int16 savedY = *pTargetY;
	const int16 maxY = (int16)gameHeightLast();
	const int16 maxX = (int16)screenWidthLast();

	// Phase 1: Scan downward with depth constraint
	// Condition: walkability >= 200 OR (targetY - walkability) < savedY
	while (true) {
		uint16 w = getWalkabilityAt(*pTargetY, savedX);
		if (isWalkabilityWalkable(w) && (*pTargetY - (int16)w >= savedY)) {
			break;
		}
		if (*pTargetY >= maxY) {
			break;
		}
		*pTargetY = *pTargetY + 1;
	}

	// Phase 2: Continue scanning to bottom for best depth match
	int16 scanY = *pTargetY;
	while (scanY <= maxY) {
		uint16 w = getWalkabilityAt(scanY, *pTargetX);
		if (scanY - (int16)w == savedY) {
			*pTargetY = scanY;
		}
		if (scanY == maxY) {
			break;
		}
		scanY++;
	}

	// Phase 3: If at screen bottom and still non-walkable, scan upward
	if (*pTargetY == maxY) {
		uint16 w = getWalkabilityAt(*pTargetY, *pTargetX);
		if (isWalkabilityBlocking(w)) {
			while (isWalkabilityBlocking(w) && *pTargetY > 0) {
				*pTargetY = *pTargetY - 1;
				w = getWalkabilityAt(*pTargetY, *pTargetX);
			}
		}
	}

	// Phase 4: If still non-walkable, scan X toward character
	uint16 w = getWalkabilityAt(*pTargetY, *pTargetX);
	if (isWalkabilityBlocking(w)) {
		*pTargetY = savedY;
		if (charX < *pTargetX) {
			while (true) {
				uint16 w2 = getWalkabilityAt(*pTargetY, *pTargetX);
				if (isWalkabilityWalkable(w2))
					break;
				if (*pTargetX <= 0)
					break;
				*pTargetX = *pTargetX - 1;
			}
		} else {
			while (true) {
				uint16 w2 = getWalkabilityAt(*pTargetY, *pTargetX);
				if (isWalkabilityWalkable(w2))
					break;
				if (*pTargetX >= maxX)
					break;
				*pTargetX = *pTargetX + 1;
			}
		}
		// Phase 5: If all failed, fall back to character position
		uint16 w2 = getWalkabilityAt(*pTargetY, *pTargetX);
		if (isWalkabilityBlocking(w2)) {
			*pTargetX = charX;
			*pTargetY = charY;
		}
	}

	// Phase 6: Gradient-based wall push
	int16 pushX = 0;
	int16 pushY = 0;
	if (isWalkabilityBlocking(getWalkabilityAt(*pTargetY, *pTargetX + 1)))
		pushX--;
	if (isWalkabilityBlocking(getWalkabilityAt(*pTargetY, *pTargetX - 1)))
		pushX++;
	if (isWalkabilityBlocking(getWalkabilityAt(*pTargetY + 1, *pTargetX)))
		pushY--;
	if (isWalkabilityBlocking(getWalkabilityAt(*pTargetY - 1, *pTargetX)))
		pushY++;
	if (isWalkabilityBlocking(getWalkabilityAt(*pTargetY, *pTargetX + 2)))
		pushX--;
	if (isWalkabilityBlocking(getWalkabilityAt(*pTargetY, *pTargetX - 2)))
		pushX++;
	if (isWalkabilityBlocking(getWalkabilityAt(*pTargetY + 2, *pTargetX)))
		pushY--;
	if (isWalkabilityBlocking(getWalkabilityAt(*pTargetY - 2, *pTargetX)))
		pushY++;

	while (pushX != 0 || pushY != 0) {
		if (pushX < 0) {
			if (isWalkabilityWalkable(getWalkabilityAt(*pTargetY, *pTargetX - 1))) {
				*pTargetX = *pTargetX - 1;
			}
			pushX++;
		}
		if (pushX > 0) {
			if (isWalkabilityWalkable(getWalkabilityAt(*pTargetY, *pTargetX + 1))) {
				*pTargetX = *pTargetX + 1;
			}
			pushX--;
		}
		if (pushY < 0) {
			if (isWalkabilityWalkable(getWalkabilityAt(*pTargetY - 1, *pTargetX))) {
				*pTargetY = *pTargetY - 1;
			}
			pushY++;
		}
		if (pushY > 0) {
			if (isWalkabilityWalkable(getWalkabilityAt(*pTargetY + 1, *pTargetX))) {
				*pTargetY = *pTargetY + 1;
			}
			pushY--;
		}
	}
}

bool Macs2Engine::getPathfindingOverride(uint16 index, uint16 &result) {
	for (auto current : _pathfindingOverrides) {
		if (current._index == index && current._active) {
			result = current._overrideValue;
			return true;
		}
	}
	return false;
}
void Macs2Engine::setPathfindingOverride(uint16 index, uint16 overrideValue) {
	removePathfindingOverride(index);
	PathfindingAreaOverride override;
	override._active = true;
	override._index = index;
	override._overrideValue = overrideValue;
	_pathfindingOverrides.push_back(override);
}

uint16 Macs2Engine::getPathfindingOverride2(uint16 index) {
	if (index < AREA_OVERRIDE_MIN || index > AREA_OVERRIDE_MAX) {
		return 0;
	}
	return _areaOverrides[index - AREA_OVERRIDE_MIN];
}

void Macs2Engine::removePathfindingOverride(uint16 index) {
	for (uint i = 0; i < _pathfindingOverrides.size(); i++) {
		PathfindingAreaOverride &current = _pathfindingOverrides[i];
		if (current._index == index) {
			_pathfindingOverrides.remove_at(i);
			return;
		}
	}
};

// isPathWalkable (1008:1196)
// Params: (param_1=y1, param_2=x1, param_3=y2, param_4=x2)
// Traces from (x2,y2) toward (x1,y1). Checks walkability only on major-axis steps.
// Uses unsigned 16-bit error accumulator with wrapping arithmetic.
// Returns true if entire line is walkable (all sampled pixels < 0xC8).
bool Macs2Engine::isPathWalkable(int16 y1, int16 x1, int16 y2, int16 x2) {
	uint16 error = 0;
	int16 curX = x2;
	int16 curY = y2;
	uint16 absDx = (uint16)abs((int)(x2 - x1));
	uint16 absDy = (uint16)abs((int)(y2 - y1));
	bool result = true;

	do {
		bool steppedX;
		if (error >= absDx) {
			if (y1 < y2)
				curY--;
			if (y2 < y1)
				curY++;
			error -= absDx;
			steppedX = false;
		} else {
			if (x1 < x2)
				curX--;
			if (x2 < x1)
				curX++;
			error += absDy;
			steppedX = true;
		}

		if (absDx > absDy && steppedX) {
			if (isWalkabilityBlocking(getWalkabilityAt(curY, curX)))
				result = false;
		}
		if (absDx <= absDy && !steppedX) {
			if (isWalkabilityBlocking(getWalkabilityAt(curY, curX)))
				result = false;
		}
	} while (curX != x1 || curY != y1);

	return result;
}

// Binary euclideanDistance (1008:1390): integer Euclidean distance approximation.
// Iterates i from 0 until i^2 >= dx^2 + dy^2. Capped at 0x500.
int Macs2Engine::euclideanDistance(const Common::Point &a, const Common::Point &b) {
	int32 dx = abs((int)(b.x - a.x));
	int32 dy = abs((int)(b.y - a.y));
	int32 distSq = dx * dx + dy * dy;
	int i = 0;
	while (i < 0x500 && (int32)i * i < distSq)
		i++;
	return i;
}

// Binary walkableDistance (1008:1293): distance between two nodes IF walkable, else 0x500.
// Uses binary search on precomputed squared-distance table (scene+0x61DC) for O(log n) sqrt.
int Macs2Engine::walkableDistance(int nodeA, int nodeB) {
	const Common::Point &a = _pathfindingPoints[nodeA - 1]._position;
	const Common::Point &b = _pathfindingPoints[nodeB - 1]._position;
	if (!isPathWalkable(a.y, a.x, b.y, b.x))
		return 0x500;
	// Binary search for integer sqrt(dx^2 + dy^2), matching binary at 1008:1293
	int32 dx = abs((int)(b.x - a.x));
	int32 dy = abs((int)(b.y - a.y));
	int32 distSq = dx * dx + dy * dy;
	int result = 0x280;
	int step = 0x280;
	do {
		step = step >> 1;
		if ((int32)result * result >= distSq) {
			result -= step;
		} else {
			result += step;
		}
	} while (step > 1);
	return result;
}

// Binary buildPathFromNodes (1008:15a8): recursive DFS cost to reach a reachable node.
// Full recursive DFS with visited-stack cycle detection matching binary exactly.
// Terminal: returns walkableDistance(node, finalDest) when node is reachable.
// Recursive: min(computeMinCostToReachable(adj)) + walkableDistance(bestAdj, current).
int Macs2Engine::computeMinCostToReachable(int nodeIndex, int prevNode, uint16 actorIndex, const bool *reachable, int nodeCount, const Common::Point &finalDest) {
	// Static visited stack (matches binary's stack-frame approach, max 16 nodes)
	static int visitedStack[17];
	static int visitedCount = 0;

	// Push current node to visited stack
	visitedCount++;
	visitedStack[visitedCount] = nodeIndex;

	int result;
	const Common::Point &nodePos = _pathfindingPoints[nodeIndex - 1]._position;

	if (reachable[nodeIndex]) {
		// Terminal: return walkable distance from this node to finalDest
		// Binary calls walkableDistance(nodePos, finalDest) = findPathNode(1008:1293)
		if (!isPathWalkable(nodePos.y, nodePos.x, finalDest.y, finalDest.x)) {
			result = 0x500;
		} else {
			int32 dx = abs((int)(finalDest.x - nodePos.x));
			int32 dy = abs((int)(finalDest.y - nodePos.y));
			int32 distSq = dx * dx + dy * dy;
			int dist = 0x280;
			int step = 0x280;
			do {
				step = step >> 1;
				if ((int32)dist * dist >= distSq) {
					dist -= step;
				} else {
					dist += step;
				}
			} while (step > 1);
			result = dist;
		}
		visitedCount--;
		return result;
	}

	int bestCost = 0x7777;
	int bestAdj = 0;
	const PathfindingPoint &pt = _pathfindingPoints[nodeIndex - 1];
	int adjCount = (int)pt._adjacentPoints.size();

	if (adjCount > 0) {
		for (int i = 0; i < adjCount; i++) {
			int adj = pt._adjacentPoints[i];
			if (adj == prevNode)
				continue;

			// Check visited stack
			bool alreadyVisited = false;
			for (int j = 1; j < visitedCount; j++) {
				if (visitedStack[j] == adj) {
					alreadyVisited = true;
					break;
				}
			}
			if (alreadyVisited)
				continue;

			// Recursive call
			int cost = computeMinCostToReachable(adj, nodeIndex, actorIndex, reachable, nodeCount, finalDest);
			if (cost < bestCost) {
				bestAdj = adj;
				bestCost = cost;
			}
		}
	}

	if (bestCost < 0x7777) {
		// Add edge cost: walkable distance from bestAdj to current node
		result = bestCost + walkableDistance(bestAdj, nodeIndex);
	} else {
		result = 0x7777;
	}

	// Pop visited stack
	visitedCount--;
	return result;
}

void Macs2Engine::nextCursorMode() {
	switch (_scriptExecutor->_cursorMode) {
	case Script::MouseMode::Talk:
		setCursorMode(Script::MouseMode::Look);
		break;
	case Script::MouseMode::Look:
		setCursorMode(Script::MouseMode::Use);
		break;
	case Script::MouseMode::Use:
	case Script::MouseMode::UseInventory:
		setCursorMode(Script::MouseMode::Walk);
		break;
	default:
		setCursorMode(Script::MouseMode::Talk);
		break;
	}
}

void Macs2Engine::setBottomHudVisible(bool visible) {
	// hide/show opcodes toggle Hidden vs Main. Cursor mode alone never hides
	// the HUD. Native skin restores the cursor saved on hide.
	if (hasNativeHudAssets()) {
		if (visible) {
			if (_menuMode == MenuMode::Hidden) {
				_menuMode = MenuMode::Main;
				if (_scriptExecutor)
					setCursorMode(_savedMenuCursorMode);
			}
		} else {
			if (_menuMode == MenuMode::Main && _scriptExecutor)
				_savedMenuCursorMode = _scriptExecutor->_cursorMode;
			_menuMode = MenuMode::Hidden;
		}
	}
	_bottomHudVisible = visible;
}

void Macs2Engine::setCursorMode(Script::MouseMode newMode) {
	// setCursorMode (1008:3ea5): when the cursor image changes, keep the hotspot
	// fixed on screen by compensating for the old/new image half-extents, clamp,
	// refresh the cursor graphic, and flag the clip rect dirty.
	const Script::MouseMode oldMode = _scriptExecutor->_cursorMode;
	const bool cursorVisible = CursorMan.isVisible();

	auto cursorHalfSize = [this](Script::MouseMode mode, uint16 &halfW, uint16 &halfH) {
		halfW = halfH = 0;
		const int index = (int)mode - 1;
		if (index < 0 || index >= (int)_imageResources.size())
			return;
		halfW = _imageResources[index]._width / 2;
		halfH = _imageResources[index]._height / 2;
	};

	auto isGameplayVerb = [](Script::MouseMode mode) {
		return mode == Script::MouseMode::Talk || mode == Script::MouseMode::Look ||
			   mode == Script::MouseMode::Use || mode == Script::MouseMode::Walk;
	};

	View1 *view = (View1 *)findView("View1");
	const bool persistentBar = view && view->hasPersistentActionBar();
	const int barTopY = view ? view->actionBarTopY() : gameHeight();

	uint16 oldHalfW = 0, oldHalfH = 0, newHalfW = 0, newHalfH = 0;
	cursorHalfSize(oldMode, oldHalfW, oldHalfH);

	Common::Point mouse = g_system->getEventManager()->getMousePos();
	const bool mouseInUiPanel = persistentBar && mouse.y >= barTopY;

	_scriptExecutor->_cursorMode = newMode;

	// Keep the pointer on the verb/inventory panel when selecting verbs there, and
	// skip hotspot compensation when the SCUMM UI shows the same walk cursor for all verbs.
	if (!mouseInUiPanel && !(persistentBar && isGameplayVerb(oldMode) && isGameplayVerb(newMode))) {
		mouse.x += oldHalfW;
		mouse.y += oldHalfH;

		cursorHalfSize(newMode, newHalfW, newHalfH);
		mouse.x -= newHalfW;
		mouse.y -= newHalfH;

		const int maxY = persistentBar ? (kScreenHeightLast - (int)newHalfH)
									: (gameHeightLast() - (int)newHalfH);
		mouse.x = CLIP<int>(mouse.x, (int)newHalfW, screenWidthLast() - (int)newHalfW);
		mouse.y = CLIP<int>(mouse.y, (int)newHalfH, maxY);
		g_system->warpMouse(mouse.x, mouse.y);
	}

	_clipRectDirty = true;

	if (view)
		view->updateCursor();

	if (cursorVisible)
		_needsRedraw = true;

	debugC(kDebugInput, "Cursor mode set to %i (%s)", (int)newMode,
		   newMode == Script::MouseMode::Talk ? "Talk" : newMode == Script::MouseMode::Look       ? "Look"
													 : newMode == Script::MouseMode::Use          ? "Use"
													 : newMode == Script::MouseMode::Walk         ? "Walk"
													 : newMode == Script::MouseMode::UseInventory ? "UseInventory"
													 : newMode == Script::MouseMode::PanelUse     ? "PanelUse"
													 : newMode == Script::MouseMode::PanelCursor  ? "PanelCursor"
													 : newMode == Script::MouseMode::Disabled     ? "Disabled"
																								  : "Unknown");
}

uint16 Macs2Engine::getHotspotAtPoint(const Common::Point &p) {
	uint16 result = 0;
	if (p.x < 0 || p.x >= screenWidth() || p.y < 0 || p.y >= gameHeight() || _hotspotMap.w == 0) {
		return result;
	}

	uint8 firstLookup = _hotspotMap.getPixel(p.x, p.y);
	uint16 numHotspots = _numHotspots;

	uint8 i = 1;
	if (i > numHotspots) {
		return result;
	}

	Common::Array<uint16> a = _hotspotColorTable;

	do {
		if ((uint)(i - 1) >= a.size())
			break;
		// Binary compares only the low byte: *(char*)(scene + i*2 + 0x50D3)
		uint8 lookup = (uint8)a[i - 1];
		if (lookup == firstLookup) {
			if (_hotspotOverrides[i] != 0xFFFF) {
				return 0x800 + _hotspotOverrides[i];
			}
			return 0x800 + i;
		}
		i++;
	} while (i <= numHotspots);
	return 0;
}

Common::String getObjectHotspotName(uint16 objectIndex) {
	const GameObjects &objects = GameObjects::instance();
	if (objectIndex > 0 && objectIndex < objects._objectNames.size() && !objects._objectNames[objectIndex].empty()) {
		if (g_engine != nullptr)
			return g_engine->translateHotspotLabel(objects._objectNames[objectIndex]);
		return objects._objectNames[objectIndex];
	}
	return Common::String();
}

Common::String lookupInteractionDisplayName(uint16 interactionId) {
	if (interactionId >= 0x800)
		return lookupSceneHotspotName((uint16)Scenes::instance()._currentSceneIndex, (uint16)(interactionId - 0x800));
	if (interactionId >= 0x400)
		return getObjectHotspotName((uint16)(interactionId - 0x400));
	return Common::String();
}

void Macs2Engine::rebuildHotspotSnapshot() const {
	_hotspotSnapshot.currentSceneIndex = Scenes::instance()._currentSceneIndex;
	_hotspotSnapshot.numHotspots = _numHotspots;
	_hotspotSnapshot.hotspotColorTable = _hotspotColorTable;
	_hotspotSnapshot.hotspotOverrides = _hotspotOverrides;
	_hotspotSnapshot.mapModeActive = isMapModeActive();
	_hotspotSnapshot.sceneHotspots.clear();
	_hotspotSnapshot.sceneObjects.clear();

	if (_hotspotMap.w > 0 && _numHotspots > 0) {
		Common::Array<int32> sumX(_numHotspots, 0);
		Common::Array<int32> sumY(_numHotspots, 0);
		Common::Array<int32> count(_numHotspots, 0);

		for (int y = 0; y < _hotspotMap.h; ++y) {
			for (int x = 0; x < _hotspotMap.w; ++x) {
				const uint8 pixel = _hotspotMap.getPixel(x, y);
				if (pixel == 0)
					continue;

				for (uint16 i = 0; i < _numHotspots; ++i) {
					if ((uint8)_hotspotColorTable[i] == pixel) {
						sumX[i] += x;
						sumY[i] += y;
						count[i]++;
						break;
					}
				}
			}
		}

		for (uint16 i = 0; i < _numHotspots; ++i) {
			if (count[i] == 0)
				continue;

			HotspotSnapshot::SceneHotspotEntry entry;
			entry.index = i + 1;
			if ((uint)(i + 1) < _hotspotOverrides.size() && _hotspotOverrides[i + 1] != 0xFFFF)
				entry.index = _hotspotOverrides[i + 1];
			entry.center = Common::Point(sumX[i] / count[i], sumY[i] / count[i]);
			_hotspotSnapshot.sceneHotspots.push_back(entry);
		}
	}

	View1 *view = g_events ? (View1 *)g_events->findView("View1") : nullptr;
	const uint16 sceneIndex = (uint16)Scenes::instance()._currentSceneIndex;
	for (uint16 objectIndex = 1; objectIndex <= kMaxSceneObjects; ++objectIndex) {
		GameObject *obj = GameObjects::getObjectByIndex(objectIndex);
		if (obj == nullptr || obj->_dataOffset == 0)
			continue;
		if ((int16)obj->_sceneIndex < 0 || obj->_sceneIndex != sceneIndex)
			continue;

		HotspotSnapshot::SceneObjectEntry entry;
		entry.index = objectIndex;
		entry.orientation = obj->_orientation;
		entry.position = getSceneObjectHotspotPosition(view, obj);
		_hotspotSnapshot.sceneObjects.push_back(entry);
	}
}

bool Macs2Engine::hotspotDirty() const {
	const bool mapMode = isMapModeActive();

	if (Scenes::instance()._currentSceneIndex != _hotspotSnapshot.currentSceneIndex ||
		_numHotspots != _hotspotSnapshot.numHotspots ||
		_hotspotColorTable != _hotspotSnapshot.hotspotColorTable ||
		_hotspotOverrides != _hotspotSnapshot.hotspotOverrides ||
		mapMode != _hotspotSnapshot.mapModeActive) {
		rebuildHotspotSnapshot();
		return true;
	}

	if (mapMode)
		return false;

	View1 *view = g_events ? (View1 *)g_events->findView("View1") : nullptr;
	const uint16 sceneIndex = (uint16)Scenes::instance()._currentSceneIndex;
	uint snapshotIdx = 0;
	for (uint16 objectIndex = 1; objectIndex <= kMaxSceneObjects; ++objectIndex) {
		GameObject *obj = GameObjects::getObjectByIndex(objectIndex);
		if (obj == nullptr || obj->_dataOffset == 0)
			continue;
		if ((int16)obj->_sceneIndex < 0 || obj->_sceneIndex != sceneIndex)
			continue;

		const Common::Point pos = getSceneObjectHotspotPosition(view, obj);
		if (snapshotIdx >= _hotspotSnapshot.sceneObjects.size()) {
			rebuildHotspotSnapshot();
			return true;
		}

		const HotspotSnapshot::SceneObjectEntry &snap = _hotspotSnapshot.sceneObjects[snapshotIdx];
		if (snap.index != objectIndex || snap.position != pos || snap.orientation != obj->_orientation) {
			rebuildHotspotSnapshot();
			return true;
		}
		snapshotIdx++;
	}

	if (snapshotIdx != _hotspotSnapshot.sceneObjects.size()) {
		rebuildHotspotSnapshot();
		return true;
	}

	return false;
}

void Macs2Engine::getHotspotPositions(Common::Array<Graphics::HotspotInfo> &hotspots) {
	if (isMapModeActive())
		return;

	for (const HotspotSnapshot::SceneHotspotEntry &entry : _hotspotSnapshot.sceneHotspots) {
		if (entry.index == 0)
			continue;

		const Common::Point &center = entry.center;
		if (center.x < 0 || center.x >= screenWidth() || center.y < 0 || center.y >= gameHeight())
			continue;

		const uint16 sceneIndex = (uint16)Scenes::instance()._currentSceneIndex;
		const Common::String name = lookupSceneHotspotName(sceneIndex, entry.index);
		const Graphics::HotspotType type = lookupSceneHotspotType(sceneIndex, entry.index);
		hotspots.push_back(Graphics::HotspotInfo(center, hotspotLabelToU32(name), type));
	}

	View1 *view = g_events ? (View1 *)g_events->findView("View1") : nullptr;
	const uint16 currentActorIndex = (uint16)Scenes::instance()._currentActorIndex;
	for (const HotspotSnapshot::SceneObjectEntry &entry : _hotspotSnapshot.sceneObjects) {
		if (entry.index == currentActorIndex)
			continue;

		const Common::Point &screenPos = entry.position;
		if (screenPos.x < 0 || screenPos.x >= screenWidth() || screenPos.y < 0 || screenPos.y >= gameHeight())
			continue;

		Character *character = view ? view->getCharacterByIndex(entry.index) : nullptr;
		const bool isCharacter = character != nullptr && !character->_markedForDeletion;
		Graphics::HotspotType hotspotType = Graphics::kHotspotObject;
		if (isCharacter && GameObjects::isNpcIndex(entry.index))
			hotspotType = Graphics::kHotspotNPC;

		const Common::String &name = getObjectHotspotName(entry.index);
		hotspots.push_back(Graphics::HotspotInfo(screenPos, hotspotLabelToU32(name), hotspotType));
	}
}

void Macs2Engine::scheduleRun(bool initScene) {
	_runScheduled = true;
	_scheduledRunIsInitScene = initScene;
}

void Macs2Engine::startInputRecording(const Common::Path &filename) {
	Common::DumpFile *f = new Common::DumpFile();
	if (!f->open(filename)) {
		warning("Failed to open recording file %s", filename.toString().c_str());
		delete f;
		return;
	}
	// Write header matching original format: 12-byte magic "AHFFMCSR0100"
	f->write("AHFFMCSR0100", 12);
	_inputRecordStream = f;
	_inputMode = InputMode::Record;
	_inputFrameCounter = 0;
	debug("Input recording started: %s", filename.toString().c_str());
}

void Macs2Engine::startInputPlayback(const Common::Path &filename) {
	Common::File *f = new Common::File();
	if (!f->open(filename)) {
		warning("Failed to open playback file %s", filename.toString().c_str());
		delete f;
		return;
	}
	// Skip 12-byte header
	f->skip(12);
	// Read first record's frame counter to prime the playback target
	_inputPlaybackEndFrame = f->readUint16LE();
	_inputPlaybackStream = f;
	_inputMode = InputMode::Playback;
	_inputFrameCounter = 0;
	debug("Input playback started: %s (first event at frame %u)", filename.toString().c_str(), _inputPlaybackEndFrame);
}

void Macs2Engine::stopInputRecording() {
	if (_inputRecordStream) {
		_inputRecordStream->finalize();
		delete _inputRecordStream;
		_inputRecordStream = nullptr;
	}
	if (_inputPlaybackStream) {
		delete _inputPlaybackStream;
		_inputPlaybackStream = nullptr;
	}
	_inputMode = InputMode::None;
}

void Macs2Engine::recordInputFrame(uint16 mouseX, uint16 mouseY, uint16 buttons) {
	if (_inputRecordStream) {
		_inputFrameCounter++;
		_inputRecordStream->writeUint16LE(_inputFrameCounter);
		_inputRecordStream->writeUint16LE(mouseX);
		_inputRecordStream->writeUint16LE(mouseY);
		_inputRecordStream->writeUint16LE(buttons);
	}
}

bool Macs2Engine::readInputFrame(uint16 &mouseX, uint16 &mouseY, uint16 &buttons) {
	if (!_inputPlaybackStream || _inputPlaybackStream->eos())
		return false;
	// Format: each record is [frameCounter(2), mouseX(2), mouseY(2), buttons(2)]
	// Playback waits until current frame >= next record's frame counter
	if (_inputFrameCounter < _inputPlaybackEndFrame)
		return false;
	mouseX = _inputPlaybackStream->readUint16LE();
	mouseY = _inputPlaybackStream->readUint16LE();
	buttons = _inputPlaybackStream->readUint16LE();
	if (_inputPlaybackStream->eos())
		return false;
	// Read next record's frame counter (or detect end)
	_inputPlaybackEndFrame = _inputPlaybackStream->readUint16LE();
	return !_inputPlaybackStream->eos();
}

uint16 Macs2Engine::getWalkabilityAt(const Common::Point &p) {
	return getWalkabilityAt((int16)p.y, (int16)p.x);
}

int Macs2Engine::measureString(const Common::String &s) {
	int sum = 0;
	GlyphData currentGlyph;
	bool found = false;
	uint16 widestGlyph = 0;
	for (auto current = s.begin(); current != s.end(); current++) {
		found = findGlyph(*current, currentGlyph);
		if (found) {
			widestGlyph = MAX(widestGlyph, currentGlyph._width);
		}
	}

	for (auto current = s.begin(); current != s.end(); current++) {
		found = findGlyph(*current, currentGlyph);
		if (!found) {
			sum += widestGlyph;
		} else {
			sum += currentGlyph._width + 1;
		}
	}
	return sum;
}

int Macs2Engine::measureStringsVertically(const Common::StringArray &sa) {
	// DOS l0037_B318: maxGlyphHeight + 2. Amiga uses absolute MXFF line pitch.
	return (int)sa.size() * dialogLineHeight();
}

int Macs2Engine::measureStrings(const Common::StringArray &sa) {
	int max = -1;
	for (auto iter = sa.begin(); iter != sa.end(); iter++) {
		max = MAX(measureString(*iter), max);
	}
	return max;
}

int Macs2Engine::computeStringIndex(Common::MemoryReadStream *stream, int targetOffset) {
	stream->seek(0);
	int index = 0;
	while (stream->pos() < targetOffset && !stream->eos()) {
		// DOS: u16LE + XOR ciphertext. Amiga: u16BE + plaintext.
		const uint16 len = isAmiga() ? stream->readUint16BE() : stream->readUint16LE();
		if (len == 0)
			break;
		stream->skip(len);
		index++;
	}
	return index;
}

void Macs2Engine::loadTranslation() {
	Common::SeekableReadStream *f = SearchMan.createReadStreamForMember("macs2_translation.dat");
	if (!f) {
		warning("Cannot open macs2_translation.dat");
		return;
	}

	// Read and verify header
	char magic[4];
	f->read(magic, 4);
	if (memcmp(magic, "MCS2", 4) != 0) {
		warning("Invalid macs2_translation.dat magic");
		delete f;
		return;
	}

	uint16 version = f->readUint16LE();
	if (version != 1) {
		warning("Unsupported macs2_translation.dat version %u", version);
		delete f;
		return;
	}

	uint16 numScenes = f->readUint16LE();
	uint16 numObjects = f->readUint16LE();
	uint16 numHotspotLabels = f->readUint16LE();
	uint16 numUiLabels = f->readUint16LE();

	// Read index tables
	struct IndexEntry {
		uint16 id;
		uint16 numStrings;
		uint32 dataOffset;
	};

	Common::Array<IndexEntry> sceneIndex(numScenes);
	for (uint16 i = 0; i < numScenes; i++) {
		sceneIndex[i].id = f->readUint16LE();
		sceneIndex[i].numStrings = f->readUint16LE();
		sceneIndex[i].dataOffset = f->readUint32LE();
	}

	Common::Array<IndexEntry> objectIndex(numObjects);
	for (uint16 i = 0; i < numObjects; i++) {
		objectIndex[i].id = f->readUint16LE();
		objectIndex[i].numStrings = f->readUint16LE();
		objectIndex[i].dataOffset = f->readUint32LE();
	}

	uint32 stringDataEnd = 0;

	// Read string data for scenes
	for (uint16 i = 0; i < numScenes; i++) {
		f->seek(sceneIndex[i].dataOffset);
		TranslationEntry entry;
		for (uint16 j = 0; j < sceneIndex[i].numStrings; j++) {
			uint16 len = f->readUint16LE();
			Common::String s;
			for (uint16 k = 0; k < len; k++)
				s += (char)f->readByte();
			entry.strings.push_back(s);
		}
		stringDataEnd = MAX(stringDataEnd, (uint32)f->pos());
		_sceneTranslations[sceneIndex[i].id] = entry;
	}

	// Read string data for objects
	for (uint16 i = 0; i < numObjects; i++) {
		f->seek(objectIndex[i].dataOffset);
		TranslationEntry entry;
		for (uint16 j = 0; j < objectIndex[i].numStrings; j++) {
			uint16 len = f->readUint16LE();
			Common::String s;
			for (uint16 k = 0; k < len; k++)
				s += (char)f->readByte();
			entry.strings.push_back(s);
		}
		stringDataEnd = MAX(stringDataEnd, (uint32)f->pos());
		_objectTranslations[objectIndex[i].id] = entry;
	}

	auto readLabelMap = [f](uint16 count, Common::HashMap<Common::String, Common::String> &out) {
		for (uint16 i = 0; i < count; i++) {
			uint16 keyLen = f->readUint16LE();
			Common::String key;
			for (uint16 k = 0; k < keyLen; k++)
				key += (char)f->readByte();
			uint16 valLen = f->readUint16LE();
			Common::String val;
			for (uint16 k = 0; k < valLen; k++)
				val += (char)f->readByte();
			if (!key.empty() && !val.empty())
				out[key] = val;
		}
	};

	_hotspotLabelTranslations.clear();
	_uiLabelTranslations.clear();
	if (numHotspotLabels > 0 || numUiLabels > 0)
		f->seek(stringDataEnd);
	readLabelMap(numHotspotLabels, _hotspotLabelTranslations);
	readLabelMap(numUiLabels, _uiLabelTranslations);

	delete f;
	debug("Loaded macs2_translation.dat: %u scenes, %u objects, %u overlay labels, %u UI labels",
		  numScenes, numObjects, (uint)_hotspotLabelTranslations.size(), (uint)_uiLabelTranslations.size());
}

Common::String Macs2Engine::translateHotspotLabel(const Common::String &cp850Name) const {
	if (cp850Name.empty() || !(getFeatures() & GF_TRANSLATED))
		return cp850Name;
	auto it = _hotspotLabelTranslations.find(cp850Name);
	if (it != _hotspotLabelTranslations.end())
		return it->_value;
	return cp850Name;
}

Common::String Macs2Engine::translateUiLabel(const Common::String &source) const {
	if (source.empty() || !(getFeatures() & GF_TRANSLATED))
		return source;
	auto it = _uiLabelTranslations.find(source);
	if (it != _uiLabelTranslations.end())
		return it->_value;
	return source;
}

Common::StringArray Macs2Engine::decodeStrings(Common::MemoryReadStream *stream, int offset, int numStrings, int sceneId, int objectId) {
	Common::StringArray result(numStrings);
	stream->seek(offset);

	if (isAmiga()) {
		// Amiga strings: u16BE length + plaintext (Latin-1), no XOR cipher.
		for (int i = 0; i < numStrings; i++) {
			Common::String currentLine;
			const uint16 length = stream->readUint16BE();
			for (uint16 index = 0; index < length; index++)
				currentLine += (char)stream->readByte();
			result[i] = currentLine;
		}
	} else {
		for (int i = 0; i < numStrings; i++) {
			Common::String currentLine;
			uint16 length = stream->readUint16LE();
			for (int index = 1; index < length + 1; index++) {
				const byte currentByte = stream->readByte();
				const byte x = (byte)(index * index * 0x0c);
				const byte y = (byte)(currentByte ^ index);
				const byte r = (byte)(x ^ y);
				currentLine += (char)r;
			}
			result[i] = currentLine;
		}
	}

	// Apply translation if available
	if (getFeatures() & GF_TRANSLATED) {
		int baseIndex = computeStringIndex(stream, offset);
		const TranslationEntry *entry = nullptr;
		if (objectId != 0 && _objectTranslations.contains(objectId)) {
			entry = &_objectTranslations[objectId];
		} else if (sceneId != 0 && _sceneTranslations.contains(sceneId)) {
			entry = &_sceneTranslations[sceneId];
		}
		if (entry) {
			for (int i = 0; i < numStrings; i++) {
				int idx = baseIndex + i;
				if (idx >= 0 && idx < (int)entry->strings.size() && !entry->strings[idx].empty()) {
					result[i] = entry->strings[idx];
				}
			}
		}
	}

	return result;
}

uint32 Macs2Engine::getFeatures() const {
	return _gameDescription->flags;
}

bool Macs2Engine::loadAnimationFromSceneData(uint16 objectIndex, uint16 slotIndex, uint8 arrayIndex, bool shouldMirror, uint16 executingScriptObjectId) {
	GameObject *go = GameObjects::instance().getObjectByIndex(objectIndex);
	if (go == nullptr) {
		_scriptExecutor->setScriptError(0x19);
		return false;
	}

	uint32 address = 0;
	if (executingScriptObjectId == 0) {
		if (arrayIndex == 0 || arrayIndex > _sceneResourceOffsets.size()) {
			_scriptExecutor->setScriptError(1);
			return false;
		}
		address = _sceneResourceOffsets[arrayIndex - 1];
	} else {
		GameObject *execObj = GameObjects::getObjectByIndex(executingScriptObjectId);
		if (execObj == nullptr || arrayIndex == 0 || arrayIndex > maxObjectResources()) {
			_scriptExecutor->setScriptError(1);
			return false;
		}
		address = execObj->_resourceOffsets[arrayIndex - 1];
	}
	if (address == 0) {
		_scriptExecutor->setScriptError(1);
		return false;
	}

	_fileStream->seek(address);
	uint32 size = _fileStream->readUint32LE();
	_fileStream->seek(address + 0x10);
	Common::Array<uint8> data;
	data.resize(size);
	_fileStream->read(data.data(), size);

	const uint16 minSlots = maxAnimSlots();
	const uint16 overloadSlot = overloadAnimSlot();
	while (go->_blobs.size() < minSlots)
		go->_blobs.push_back(Common::Array<uint8>());
	while (go->_blobSourceKeys.size() < minSlots)
		go->_blobSourceKeys.push_back(0);
	while (go->_blobMirrorFlags.size() < minSlots)
		go->_blobMirrorFlags.push_back(false);

	Common::Array<uint8> *targetBlob = nullptr;
	if (slotIndex == overloadSlot) {
		while (go->_blobs.size() <= (uint)(overloadSlot - 1))
			go->_blobs.push_back(Common::Array<uint8>());
		targetBlob = &go->_blobs[overloadSlot - 1];
		go->_overloadAnimationSourceKey = static_cast<uint16>(address >> 16);
		go->_overloadAnimationMirrored = shouldMirror;
	} else {
		targetBlob = &go->_blobs[slotIndex - 1];
		// Binary scriptLoadObjectAnim (1008:cb45) does NOT modify slot+0x0C (wAnimSpeed).
		go->_blobSourceKeys[slotIndex - 1] = static_cast<uint16>(address >> 16);
		go->_blobMirrorFlags[slotIndex - 1] = shouldMirror;
	}

	// Binary: memFree old blob if bSlotLoaded, then alloc + read; sets slot+0x33 = 1.
	*targetBlob = data;
	if (slotIndex == overloadSlot)
		go->_overloadAnimation = data;
	if (shouldMirror) {
		BackgroundAnimationBlob::mirrorAnimBlob(*targetBlob);
	}
	return true;
}

void Macs2Engine::sortObjectsByDepth(uint16 objectIndex) {
	if (objectIndex < 1 || objectIndex > 0x200)
		return;

	GameObject *obj = GameObjects::getObjectByIndex(objectIndex);
	if (obj == nullptr || obj->_dataOffset == 0)
		return;

	View1 *currentView = (View1 *)findView("View1");
	if (currentView != nullptr && currentView->_activeInventoryItem != nullptr &&
		currentView->_activeInventoryItem->_index == objectIndex) {
		currentView->_activeInventoryItem = nullptr;
		if (currentView->_savedCursorMode == Script::MouseMode::UseInventory) {
			currentView->_savedCursorMode = Script::MouseMode::Use;
		}
		if (_scriptExecutor->_cursorModeBeforeWait == Script::MouseMode::UseInventory) {
			_scriptExecutor->_cursorModeBeforeWait = Script::MouseMode::Use;
		}
		if (_scriptExecutor->_cursorMode == Script::MouseMode::UseInventory) {
			setCursorMode(Script::MouseMode::Use);
			currentView->updateCursor();
		}
	}

	clearObjectRuntime(obj);
}

void Macs2Engine::clearObjectRuntime(GameObject *obj) {
	if (obj == nullptr)
		return;

	obj->_blobs.clear();
	obj->_blobSourceKeys.clear();
	obj->_blobWalkSpeeds.clear();
	obj->_blobMirrorFlags.clear();
	obj->_script.clear();
	memset(obj->_resourceOffsets, 0, sizeof(obj->_resourceOffsets));
	obj->_overloadAnimation.clear();
	obj->_useOverloadAnimation = false;
	obj->_overloadAnimationMirrored = false;
	obj->_storedWalkRuntime = GameObject::StoredWalkRuntime();
	obj->resetDrawBounds();
}

bool Macs2Engine::loadObjectData(GameObject *obj) {
	if (obj == nullptr) {
		_scriptExecutor->setScriptError(0x19);
		return false;
	}
	// Amiga: OO_* animation/script payloads stay resident after readAmigaResources().
	// _dataOffset is a non-zero sentinel (not an MCS file offset). Opcode 0x0b
	// (moveObject) and checkObjectData call this when an object enters the scene;
	// seeking into RESOURCE.MCS would set script error 0x01.
	if (isAmiga()) {
		if (obj->_dataOffset == 0) {
			_scriptExecutor->setScriptError(0x19);
			return false;
		}
		return true;
	}
	if (obj->_dataOffset == 0) {
		_scriptExecutor->setScriptError(0x19);
		return false;
	}
	if (_fileStream == nullptr) {
		_scriptExecutor->setScriptError(0x19);
		return false;
	}
	if (!_fileStream->seek(obj->_dataOffset + 10, SEEK_SET)) {
		_scriptExecutor->setScriptError(0x19);
		return false;
	}

	const Common::Array<Common::Array<uint8>> blobsBackup = obj->_blobs;
	const Common::Array<uint16> keysBackup = obj->_blobSourceKeys;
	const Common::Array<uint16> speedsBackup = obj->_blobWalkSpeeds;
	const Common::Array<bool> mirrorsBackup = obj->_blobMirrorFlags;
	const auto rollbackPartialLoad = [&]() {
		obj->_blobs = blobsBackup;
		obj->_blobSourceKeys = keysBackup;
		obj->_blobWalkSpeeds = speedsBackup;
		obj->_blobMirrorFlags = mirrorsBackup;
	};

	const uint16 animSlotCount = maxAnimSlots();
	if (isV2())
		_fileStream->readUint16LE(); // ReadyObject lead word before anim slots
	for (int j = 0; j < (int)animSlotCount; j++) {
		_fileStream->readUint16LE(); // animID (editor metadata, unused at runtime)
		uint16 blobSourceKey = _fileStream->readUint16LE();
		uint32 dataSize = _fileStream->readUint32LE();

		if (_fileStream->eos() && dataSize != 0) {
			rollbackPartialLoad();
			_scriptExecutor->setScriptError(1);
			return false;
		}

		Common::Array<uint8> data;
		if (dataSize > 0) {
			if (dataSize > 0x1000000) {
				rollbackPartialLoad();
				_scriptExecutor->setScriptError(1);
				return false;
			}
			data.resize(dataSize);
			const uint32 bytesRead = _fileStream->read(data.data(), dataSize);
			if (bytesRead != dataSize) {
				rollbackPartialLoad();
				_scriptExecutor->setScriptError(1);
				return false;
			}
		}

		if (j < (int)obj->_blobs.size()) {
			obj->_blobs[j] = data;
		} else {
			obj->_blobs.push_back(data);
		}
		if (j < (int)obj->_blobSourceKeys.size()) {
			obj->_blobSourceKeys[j] = blobSourceKey;
		} else {
			obj->_blobSourceKeys.push_back(blobSourceKey);
		}

		uint16 blobSpeed = _fileStream->readUint16LE();
		if (j < (int)obj->_blobWalkSpeeds.size()) {
			obj->_blobWalkSpeeds[j] = blobSpeed;
		} else {
			obj->_blobWalkSpeeds.push_back(blobSpeed);
		}

		uint16 blobMirrorFlag = _fileStream->readByte();
		_fileStream->readByte(); // discarded byte
		if (j < (int)obj->_blobMirrorFlags.size()) {
			obj->_blobMirrorFlags[j] = blobMirrorFlag != 0;
		} else {
			obj->_blobMirrorFlags.push_back(blobMirrorFlag != 0);
		}

		if (blobMirrorFlag != 0 && dataSize > 0 && j < (int)obj->_blobs.size()) {
			BackgroundAnimationBlob::mirrorAnimBlob(obj->_blobs[j]);
		}
	}

	// Binary loadObjectData (1008:08ec): flags, runtime reset, then script/resource table.
	_fileStream->readByte(); // runtime+0x184 hasInventoryIcon (derived from slot 0x13 in C++)
	obj->_hasShading = _fileStream->readByte() != 0;
	obj->_hasScaling = _fileStream->readByte() != 0;
	if (isV2())
		obj->_hasDoubleResAnim = _fileStream->readByte() != 0;
	else
		obj->_hasDoubleResAnim = false;

	if (obj->_blobs.size() > 0x11 && !obj->_blobs[0x11].empty()) {
		const uint16 frameCount = BackgroundAnimationBlob::getAnimFrameCount(obj->_blobs[0x11]);
		obj->_pickupFrameStart = (frameCount >> 1) + 1;
		obj->_pickupFrameEnd = frameCount + 1;
	} else {
		obj->_pickupFrameStart = 1;
		obj->_pickupFrameEnd = 2;
	}

	obj->_overloadAnimTriggerDirection = 0x7FFF;
	for (uint i = 0; i < ARRAYSIZE(obj->_specialAnimTriggers); i++)
		obj->_specialAnimTriggers[i] = 0x7FFF;
	obj->_useOverloadAnimation = false;
	obj->_overloadAnimation.clear();
	obj->_snapToTarget = false;
	obj->_hasBoundsAttachment = false;
	obj->_boundsAttachmentObjectID = 0;
	obj->_boundsAttachmentValue1 = 0;
	obj->_boundsAttachmentValue2 = 0;
	obj->_boundsAttachmentValue3 = 0;
	// Binary loadObjectData (1008:08ec): runtime+0x21D = object vertical offset.
	obj->_storedWalkRuntime.motionTargetVerticalOffset = obj->_verticalOffsetScale;

	const uint32 scriptTableOffset = getMcsDirectoryOffset() + kMcsV1ObjectScriptPtrRel + obj->_index * 0xC;
	_fileStream->seek(scriptTableOffset, SEEK_SET);
	const uint32 scriptOffset = _fileStream->readUint32LE();
	if (scriptOffset != 0) {
		_fileStream->seek(scriptOffset, SEEK_SET);
		const uint maxObjRes = maxObjectResources();
		for (uint r = 0; r < maxObjRes; r++) {
			obj->_resourceOffsets[r] = _fileStream->readUint32LE();
		}
		if (isV2()) {
			_fileStream->skip(0x200 - maxObjRes * 4);
			_fileStream->readUint16LE();
			_fileStream->readUint16LE();
		}
		const uint16 scriptLength = _fileStream->readUint16LE();
		obj->_script.resize(scriptLength);
		if (scriptLength > 0) {
			_fileStream->read(obj->_script.data(), scriptLength);
		}
	}
	return true;
}

void Macs2Engine::setCurrentSoundData(const Common::Array<uint8> &data, int rateHz, int headerSkip) {
	stopSample();
	_currentSoundData = data;
	_currentSoundRate = rateHz > 0 ? rateHz : 0x1F40;
	_currentSoundHeaderSkip = headerSkip >= 0 ? headerSkip : 0;
}

void Macs2Engine::clearCurrentSoundData() {
	stopSample();
	_currentSoundData.clear();
	_currentSoundRate = 0x1F40;
	_currentSoundHeaderSkip = 2;
}

void Macs2Engine::playSample() {
	if (_currentSoundData.empty())
		return;

	stopSample();
	MacsAudioStream *audioStream = new MacsAudioStream();
	audioStream->_rate = _currentSoundRate;
	// DOS: skip 2-byte size header. Amiga MXOS extract is raw PCM (skip 0).
	audioStream->_pos = _currentSoundHeaderSkip;
	if (audioStream->_pos > (int64)_currentSoundData.size())
		audioStream->_pos = (int64)_currentSoundData.size();
	audioStream->_data = _currentSoundData;
	g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_currentSoundHandle, audioStream);
}

void Macs2Engine::stopSample() {
	Audio::Mixer *mixer = g_system->getMixer();
	if (mixer->isSoundHandleActive(_currentSoundHandle))
		mixer->stopHandle(_currentSoundHandle);
}

bool Macs2Engine::isSamplePlaying() const {
	return g_system->getMixer()->isSoundHandleActive(_currentSoundHandle);
}

void Macs2Engine::stopSpeech() {
	Audio::Mixer *mixer = g_system->getMixer();
	if (mixer->isSoundHandleActive(_speechSoundHandle))
		mixer->stopHandle(_speechSoundHandle);
}

bool Macs2Engine::isSpeechPlaying() const {
	return g_system->getMixer()->isSoundHandleActive(_speechSoundHandle);
}

void Macs2Engine::playDigitalAudioFile(const Common::Path &basename, bool speechBus) {
	Audio::SeekableAudioStream *stream = Audio::SeekableAudioStream::openStreamFile(basename);
	if (stream == nullptr) {
		debugC(kDebugScript, "playDigitalAudioFile: no audio for %s",
			   basename.toString().c_str());
		return;
	}

	if (speechBus) {
		stopSpeech();
		g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_speechSoundHandle, stream);
	} else {
		stopSample();
		g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_currentSoundHandle, stream);
	}
}

Common::String Macs2Engine::getGameId() const {
	return _gameDescription->gameId;
}

uint16 Macs2Engine::specialAnimSlotToAnimSlot(uint16 specialSlot) {
	static const uint16 kMap[5] = {0x15, 0x11, 0x16, 0x17, 0x18};
	if (specialSlot < 1 || specialSlot > 5)
		return 0;
	return kMap[specialSlot - 1];
}

uint16 Macs2Engine::resolveAnimSlotIndex(const GameObject *obj) const {
	if (obj == nullptr)
		return 0;
	if (isV2()) {
		for (uint i = 0; i < ARRAYSIZE(obj->_specialAnimTriggers); i++) {
			const uint16 trig = obj->_specialAnimTriggers[i];
			if ((int16)trig >= 0 && trig == obj->_orientation)
				return specialAnimSlotToAnimSlot(i + 1);
		}
		return obj->_orientation;
	}
	if ((int16)obj->_overloadAnimTriggerDirection < 0 ||
		obj->_overloadAnimTriggerDirection != obj->_orientation) {
		return obj->_orientation;
	}
	return overloadAnimSlot();
}

void Macs2Engine::setGameSpeedMode(uint16 mode) {
	_gameSpeedMode = mode % 3;
	ConfMan.setInt(kGameSpeedModeConfigKey, _gameSpeedMode);
}

Common::Error Macs2Engine::run() {
	GameObjects::instance().init();
	setGameSpeedMode(ConfMan.getInt(kGameSpeedModeConfigKey));
	loadBootstrapResources();
	readExecutable();

	// Load translation data if available
	if (getFeatures() & GF_TRANSLATED) {
		loadTranslation();
	}

	// Initialize graphics mode (taller framebuffer when action bar verb UI is enabled)
	int gfxH = screenHeight();
	initGraphics(screenWidth(), gfxH);

	CursorMan.showMouse(false);

	_music->init();
	syncSoundSettings();
	setDebugger(new GUI::Debugger());

#ifdef USE_IMGUI
	ImGuiCallbacks callbacks;
	callbacks.init = onImGuiInit;
	callbacks.render = onImGuiRender;
	callbacks.cleanup = onImGuiCleanup;
	_system->setImGuiCallbacks(callbacks);
#endif

	runGame();

	return Common::kNoError;
}

Common::Error Macs2Engine::loadGameState(int slot) {
	if (slot >= 100 && slot < 110) {
		// Load original DOS save file (SAVEGAME.N)
		int dosSlot = slot - 100;
		Common::String name = Common::String::format("SAVEGAME.%d", dosSlot);
		Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(name);
		if (!f) {
			name = Common::String::format("savegame.%d", dosSlot);
			f = g_system->getSavefileManager()->openForLoading(name);
		}
		if (!f)
			return Common::kReadingFailed;
		Common::Serializer s(f, nullptr);
		Common::Error err = syncGame(s);
		delete f;
		return err;
	}
	return Engine::loadGameState(slot);
}

Common::Error Macs2Engine::saveOriginalGameState(int dosSlot) {
	if (dosSlot < 0 || dosSlot > 9)
		return Common::kWritingFailed;

	// Original DOS filename, uppercase first (matches loadGameState lookup order).
	Common::String name = Common::String::format("SAVEGAME.%d", dosSlot);
	Common::OutSaveFile *f = g_system->getSavefileManager()->openForSaving(name, false /*no compression*/);
	if (!f)
		return Common::kWritingFailed;

	Common::Serializer s(nullptr, f);
	Common::Error err = syncGame(s);
	f->finalize();
	delete f;
	return err;
}

bool Macs2Engine::tick() {
	_scriptExecutor->tick();
	if (_runScheduled) {
		_runScheduled = false;
		bool shouldRunInit = _scheduledRunIsInitScene;
		_scheduledRunIsInitScene = false;
		_scriptExecutor->_isRepeatRun = true;
		_scriptExecutor->run(shouldRunInit);
	}
	const bool result = Events::tick();
	drawHotspots();
	return result;
}

void GlyphData::readFromeFile(Common::File &file) {
	_ascii = file.readByte();
	_width = file.readUint16LE();
	_height = file.readUint16LE();
	_data.resize(_width * _height);
	file.read(_data.data(), _width * _height);
}

void GlyphData::readFromMemory(Common::SeekableReadStream *stream) {
	_ascii = stream->readByte();
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_data.resize(_width * _height);
	stream->read(_data.data(), _width * _height);
}

void AnimFrame::readFromeFile(Common::File &file) {
	_width = file.readUint16LE();
	_height = file.readUint16LE();
	_data.resize(_width * _height);
	file.read(_data.data(), _width * _height);
}

void AnimFrame::readFromStream(Common::SeekableReadStream *stream) {
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_data.resize(_width * _height);
	stream->read(_data.data(), _width * _height);
}

bool AnimFrame::pixelHit(const Common::Point &point) const {
	if (point.x < 0 || point.x >= _width || point.y < 0 || point.y >= _height) {
		return false;
	}
	return _data[point.y * _width + point.x] != 0;
}

Common::Point AnimFrame::getBottomMiddleOffset(uint16 scale) const {
	if (scale == 100) {
		return Common::Point(_width / 2, _height);
	}
	return Common::Point(
		_width * scale / 200, // scaled width / 2
		_height * scale / 100);
}

AnimFrame BackgroundAnimationBlob::getCurrentFrame() {
	// Mode 0: read current frame without advancing (draw path uses mode 2 in drawBackgroundAnimations)
	Common::Array<uint8> &blob = activeBlob();
	uint16 offset = advanceAnimFrame(blob, false, 0x0);
	// offset points to per-frame header: offsetX(2), offsetY(2), unknown(2), width(2), height(2), pixels
	offset += 6; // skip offsetX, offsetY, unknown
	AnimFrame result;
	result._width = READ_LE_UINT16(&blob[offset]);
	result._height = READ_LE_UINT16(&blob[offset + 2]);
	result._data.resize(result._width * result._height);
	memcpy(result._data.data(), &blob[offset + 4], result._width * result._height);
	return result;
}

// Animation blob frame sequencer (matches advanceAnimFrame at 1010:1480).
// bpp6: save flag (true=write state back to blob header)
// bpp8: mode (0=current frame, 1=reset to frame 1, 2=advance, 100+N=jump to frame N)
//
// Blob header (12 bytes):
//   +0: unknown (preserved), +2: sequence position, +4: repeat counter,
//   +6: loop start position, +8: delay counter, +10: sequence length - 1
//
// Sequence table (at blob+0xC, seqLen bytes):
//   Command 1: set repeat (next byte), record loop start
//   Command 2: set delay (next byte)
//   Command 3: jump to position (next byte)
//   Values >= 10: frame index (0-based = value - 10)
//
// Frame data (at blob+0xC+seqLen): frame count word, then per frame:
//   offsetX(2), offsetY(2), unknown(2), width(2), height(2), pixels(w*h)
//
// Returns byte offset to the start of the target frame within the blob.
uint16 BackgroundAnimationBlob::advanceAnimFrame(Common::Array<uint8> &blob, bool bpp6, uint16 bpp8) {
	Common::MemorySeekableReadWriteStream stream(blob.data(), blob.size());

	uint16 bp22 = stream.readUint16LE();     // +0: unknown (preserved on save)
	uint16 bp6 = stream.readUint16LE();      // +2: current sequence position
	uint16 bp8 = stream.readUint16LE();      // +4: repeat counter
	uint16 bp0A = stream.readUint16LE();     // +6: loop start position
	uint16 bp10 = stream.readUint16LE();     // +8: delay counter
	uint16 bp0E = stream.readUint16LE() + 1; // +10: sequence length (stored as len-1)

	if (bpp8 == 0x1) {
		// Mode 1: reset to frame 1
		bp8 = 0;
		bp10 = 0;
		bp6 = 1;
	} else if (bpp8 >= 0x65 && bpp8 <= 0xA4) {
		// Mode 100+N: jump to frame N
		bp6 = bpp8 - 0x64;
		bp8 = 0;
		bp10 = 0;
		if (bp6 > bp0E) {
			bp6 = 1;
		}
	}

	if (bp6 >= bp0E) {
		bp6 = 1;
	}

	// Parse sequence commands until we hit a frame index (>= 10)
	uint8 bp0C;
	while (true) {
		if (bp6 >= bp0E) {
			bp6 = 1;
		}
		stream.seek(0x0B + bp6, SEEK_SET);
		bp0C = stream.readByte();
		if (bp0C == 0x01) {
			// Set repeat counter, record loop start
			bp6++;
			bp8 = stream.readByte();
			bp6++;
			bp0A = bp6;
		} else if (bp0C == 0x02) {
			// Set delay counter
			bp6++;
			bp10 = stream.readByte();
			bp6++;
		} else if (bp0C == 0x03) {
			// Jump to position
			bp6 = stream.readByte();
		} else {
			break;
		}
	}

	// Seek to frame data table (past sequence table)
	uint16 cx = bp0C - 0xA;
	stream.seek(0xB + bp0E, SEEK_SET);
	uint16 frameCount = stream.readUint16LE();
	if (cx > frameCount) {
		cx = 1;
	}

	// Skip to target frame (cx is 1-based frame number)
	for (; cx > 1; cx--) {
		stream.seek(0x6, SEEK_CUR); // skip offsetX, offsetY, unknown
		uint16 w = stream.readUint16LE();
		uint16 h = stream.readUint16LE();
		stream.seek(w * h, SEEK_CUR);
	}

	uint16 bp12 = stream.pos();

	// Mode 2: advance sequence position after finding current frame
	if (bpp8 == 0x02 && bp0C >= 0xA) {
		bp6++;
		if (bp10 > 0) {
			bp10--;
		}
		if (bp10 == 0 && bp8 > 0) {
			bp8--;
			bp6 = bp0A;
		}
	}

	if (bp6 >= bp0E) {
		bp6 = 1;
	}

	// Save state back to blob header
	if (bpp6) {
		stream.seek(0, SEEK_SET);
		stream.writeUint16LE(bp22);
		stream.writeUint16LE(bp6);
		stream.writeUint16LE(bp8);
		stream.writeUint16LE(bp0A);
		stream.writeUint16LE(bp10);
	}

	return bp12;
}

uint16 BackgroundAnimationBlob::getCurrentPixelFrameNumber(const Common::Array<uint8> &blob) {
	if (blob.size() < 14)
		return 1;

	Common::MemorySeekableReadWriteStream stream(const_cast<byte *>(blob.data()), blob.size());
	stream.readUint16LE();       // unknown
	uint16 bp6 = stream.readUint16LE(); // sequence position
	stream.readUint16LE();       // repeat counter
	stream.readUint16LE();       // loop start
	stream.readUint16LE();       // delay counter
	const uint16 bp0E = stream.readUint16LE() + 1;

	if (bp6 >= bp0E)
		bp6 = 1;

	uint8 bp0C = 0;
	while (true) {
		if (bp6 >= bp0E)
			bp6 = 1;
		stream.seek(0x0B + bp6, SEEK_SET);
		bp0C = stream.readByte();
		if (bp0C == 0x01) {
			bp6++;
			stream.readByte();
			bp6++;
		} else if (bp0C == 0x02) {
			bp6++;
			stream.readByte();
			bp6++;
		} else if (bp0C == 0x03) {
			bp6 = stream.readByte();
		} else {
			break;
		}
	}

	uint16 cx = bp0C - 0xA;
	stream.seek(0xB + bp0E, SEEK_SET);
	const uint16 frameCount = stream.readUint16LE();
	if (cx == 0 || cx > frameCount)
		cx = 1;
	return cx;
}

// Matches binary decodeAnimBlob (1010:184d) + mirrorAnimFrame (1010:1319).
// Iterates each frame in the blob and horizontally flips its pixel data in-place.
void BackgroundAnimationBlob::mirrorAnimBlob(Common::Array<uint8> &blob) {
	if (blob.size() < 14)
		return;
	AnimBlobView blobView(blob);
	if (!blobView.isValid())
		return;
	uint16 frameCount = blobView.frameCount();
	uint32 framePos = blobView.frameDataOffset() + 2; // skip frame count word
	for (uint16 f = 0; f < frameCount; f++) {
		if (framePos + 10 > blob.size())
			break;
		uint16 width = READ_LE_UINT16(&blob[framePos + 6]);
		uint16 height = READ_LE_UINT16(&blob[framePos + 8]);
		uint32 pixelStart = framePos + 10;
		if (pixelStart + (uint32)width * height > blob.size())
			break;
		// Flip each row horizontally
		for (uint16 row = 0; row < height; row++) {
			uint8 *rowStart = &blob[pixelStart + row * width];
			for (uint16 col = 0; col < width / 2; col++) {
				SWAP(rowStart[col], rowStart[width - 1 - col]);
			}
		}
		framePos += 10 + (uint32)width * height;
	}
}

// Returns sequence length from blob header (matches getAnimFrameCount at 1010:168c).
// Note: this returns the sequence table length, NOT the actual frame count.
// Use AnimBlobView::frameCount() for the actual number of pixel frames.
uint16 BackgroundAnimationBlob::getAnimFrameCount(Common::Array<uint8> &blob) {
	AnimBlobView view(blob);
	return view.sequenceLength();
}

int MacsAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int numSamplesRead = 0;
	for (int i = 0; i < numSamples; i++) {
		if (_pos >= _data.size()) {
			return numSamplesRead;
		}
		buffer[i] = static_cast<int16>((_data[_pos] - 128) * 256);
		numSamplesRead++;
		_pos++;
	}
	return numSamplesRead;
}

bool MacsAudioStream::isStereo() const {
	return false;
}

int MacsAudioStream::getRate() const {
	return _rate > 0 ? _rate : 0x1F40;
}

bool MacsAudioStream::endOfData() const {
	return _pos >= _data.size();
}

bool MacsAudioStream::seek(const Audio::Timestamp &where) {
	const int64 targetPos = where.msecs() * getRate() / 1000;
	if (targetPos < 0 || targetPos > _data.size()) {
		return false;
	}

	_pos = targetPos;
	return true;
}

Audio::Timestamp MacsAudioStream::getLength() const {
	return Audio::Timestamp(0, _data.size(), getRate());
}

void Macs2Engine::readPalette(Common::SeekableReadStream *stream, Graphics::Palette &dest) {
	byte buf[Graphics::PALETTE_SIZE];
	stream->read(buf, Graphics::PALETTE_SIZE);
	if (dest.size() != Graphics::PALETTE_COUNT)
		dest.resize(Graphics::PALETTE_COUNT, false);
	dest.set(buf, 0, Graphics::PALETTE_COUNT);
}

void Macs2Engine::expandPalette6To8(Graphics::Palette &pal) {
	for (uint i = 0; i < pal.size(); i++) {
		byte r, g, b;
		pal.get(i, r, g, b);
		pal.set(i, (byte)((r * 259 + 33) >> 6),
				(byte)((g * 259 + 33) >> 6),
				(byte)((b * 259 + 33) >> 6));
	}
}

void Macs2Engine::applyPaletteDarkening() {
	// Binary: sceneData+0x5203 == 1 means copy source palette as-is to display;
	// otherwise darken: display[i] = source[i] * (100 - darkenPercent) / 100.
	// _palVanilla = raw 6-bit source palette (unchanged).
	// _pal = 8-bit display palette (darkened + expanded).
	uint16 darkenPercent = (_scenePaletteMode == 1) ? 0 : _paletteDarkenPercent;
	if (darkenPercent > 100)
		darkenPercent = 100;
	uint16 brightnessFactor = 100 - darkenPercent;
	for (uint i = 0; i < Graphics::PALETTE_COUNT; i++) {
		byte r, g, b;
		_palVanilla.get(i, r, g, b);
		if (isAmiga()) {
			_pal.set(i, (byte)((r * brightnessFactor / 100 * 255) / 63),
					 (byte)((g * brightnessFactor / 100 * 255) / 63),
					 (byte)((b * brightnessFactor / 100 * 255) / 63));
		} else {
			_pal.set(i, (byte)((r * brightnessFactor / 100 * 259 + 33) >> 6),
					 (byte)((g * brightnessFactor / 100 * 259 + 33) >> 6),
					 (byte)((b * brightnessFactor / 100 * 259 + 33) >> 6));
		}
	}
}

void Macs2Engine::applyScenePaletteEffect() {
	// applyScenePaletteEffect (1000:103e): reduce scene colors to 16 rarest indices
	// (0..0xBF) plus fixed UI palette 0xC0..0xFF, then nearest-color remap.
	uint32 histogram[256] = {};
	for (int y = 0; y < _sceneBackground.h; y++) {
		for (int x = 0; x < _sceneBackground.w; x++) {
			histogram[_sceneBackground.getPixel(x, y)]++;
		}
	}

	bool selected[256] = {};
	for (int pick = 0; pick < 16; pick++) {
		uint32 minCount = 0xFFFFFFFF;
		int minIndex = 0;
		for (int i = 0; i <= 0xBF; i++) {
			if (histogram[i] != 0xFFFFFFFF && histogram[i] < minCount) {
				minCount = histogram[i];
				minIndex = i;
			}
		}
		histogram[minIndex] = 0xFFFFFFFF;
		selected[minIndex] = true;
	}

	Graphics::Palette refPalette(Graphics::PALETTE_COUNT);
	int refSlot = 0x10;
	for (int i = 0; i <= 0xBF; i++) {
		if (selected[i]) {
			byte r, g, b;
			_palVanilla.get(i, r, g, b);
			refPalette.set(refSlot, r, g, b);
			refSlot++;
		}
	}
	for (int i = 0xC0; i <= 0xFF; i++) {
		byte r, g, b;
		_palVanilla.get(i, r, g, b);
		refPalette.set(i, r, g, b);
	}

	uint8 remap[256];
	for (int paletteIndex = 0; paletteIndex < 256; paletteIndex++) {
		byte srcR, srcG, srcB;
		_palVanilla.get(paletteIndex, srcR, srcG, srcB);
		uint32 bestDistance = 0x7FFF;
		uint8 bestIndex = 0x10;
		for (int candidate = 0x10; candidate <= 0xFF; candidate++) {
			byte candR, candG, candB;
			refPalette.get(candidate, candR, candG, candB);
			int dR = (int)srcR - (int)candR;
			int dG = (int)srcG - (int)candG;
			int dB = (int)srcB - (int)candB;
			if (dR < 0)
				dR = -dR;
			if (dG < 0)
				dG = -dG;
			if (dB < 0)
				dB = -dB;
			const uint32 distance = (uint32)(dR + dG + dB);
			if (distance < bestDistance) {
				bestDistance = distance;
				bestIndex = (uint8)candidate;
			}
		}
		remap[paletteIndex] = bestIndex;
	}

	auto remapIndex = [&](uint8 index) -> uint8 {
		return remap[index];
	};

	for (int y = 0; y < _sceneBackground.h; y++) {
		for (int x = 0; x < _sceneBackground.w; x++) {
			_sceneBackground.setPixel(x, y, remapIndex(_sceneBackground.getPixel(x, y)));
		}
	}

	for (auto &blobEntry : _backgroundAnimationsBlobs) {
		Common::Array<uint8> &blob = blobEntry._blob;
		AnimBlobView blobView(blob);
		if (!blobView.isValid())
			continue;
		const uint16 frameCount = blobView.frameCount();
		for (uint16 frame = 0; frame < frameCount; frame++) {
			AnimBlobView::FrameInfo frameInfo;
			if (!blobView.getFrameInfo(frame, frameInfo))
				break;
			const uint32 pixelCount = (uint32)frameInfo.width * frameInfo.height;
			byte *pixels = const_cast<byte *>(frameInfo.pixels);
			for (uint32 p = 0; p < pixelCount; p++) {
				pixels[p] = remapIndex(pixels[p]);
			}
		}
	}

	Graphics::Palette remappedVanilla(Graphics::PALETTE_COUNT);
	for (int i = 0; i < 256; i++) {
		byte r, g, b;
		refPalette.get(remap[i], r, g, b);
		remappedVanilla.set(i, r, g, b);
	}
	_palVanilla = remappedVanilla;
	applyPaletteDarkening();

	View1 *view = (View1 *)findView("View1");
	if (view != nullptr) {
		view->_backgroundSurface.copyFrom(_sceneBackground);
		view->_paletteDirty = true;
	}
}

// Gradual palette brighten for _scenePaletteMode == 2, matching the binary
// updateBackgroundAnimations (1008:2c05).
//
// The binary decrements sceneData+0x5205 (the darken percent) by 1 each call
// while it is above 60 (0x3C), then recomputes the display palette via the
// standard darkening formula and pushes it. The pushed range depends on the
// map/help-disabled flag (1020:23B4):
//   flag != 0 (map active): update palette entries 16..255 only, preserving
//             the UI palette entries 0..15 used by the map/help overlay
//             (setPaletteRange(0xF0, 0x10, dest+0x30)).
//   flag == 0:              update all 256 entries (setPaletteRange(0x100, 0, dest)).
//
// Only mode 2 acts here. Mode 3 calls this in the binary too, but its body is
// a no-op for mode 3 (its darkening is static), so we guard on mode 2.
void Macs2Engine::updateBackgroundAnimationPalette() {
	if (_scenePaletteMode != 2)
		return;

	if (_paletteDarkenPercent > 60)
		_paletteDarkenPercent--;

	// Recompute the 8-bit display palette from the vanilla source.
	applyPaletteDarkening();

	View1 *view = (View1 *)findView("View1");
	const bool mapActive = view && view->isHelpButtonDisabled();

	if (mapActive) {
		// Preserve entries 0..15 (UI), update 16..255.
		g_system->getPaletteManager()->setPalette(_pal.data() + 16 * 3, 16, 240);
	} else {
		g_system->getPaletteManager()->setPalette(_pal);
	}
}

} // End of namespace Macs2
