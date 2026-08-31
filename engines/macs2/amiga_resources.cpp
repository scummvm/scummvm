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

#include "common/archive.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"

#include "macs2/amiga_decode.h"
#include "macs2/detection.h"
#include "macs2/gameobjects.h"
#include "macs2/macs2_constants.h"
#include "macs2/view1.h"

namespace Macs2 {

static void blitMap(Graphics::ManagedSurface &dest, const Common::Array<byte> &src) {
	if (src.size() != (uint)kScreenWidth * kGameHeight)
		return;
	for (int y = 0; y < kGameHeight; y++) {
		for (int x = 0; x < kScreenWidth; x++)
			dest.setPixel(x, y, src[(uint)y * kScreenWidth + x]);
	}
}

bool Macs2Engine::loadAmigaSceneBackground(uint32 sceneResourceId) {
	if (!_amigaArchive || sceneResourceId == 0 || sceneResourceId > 0xFFFF)
		return false;
	if (!_amigaArchive->hasResource(kAmigaResMM, (uint16)sceneResourceId))
		return false;

	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_amigaArchive->createReadStreamForResource(kAmigaResMM, (uint16)sceneResourceId));
	if (!stream)
		return false;

	const uint32 size = (uint32)stream->size();
	Common::Array<byte> mxmm;
	mxmm.resize(size);
	if (stream->read(mxmm.data(), size) != size)
		return false;

	Common::Array<byte> pixels;
	Graphics::Palette paletteRgb(Graphics::PALETTE_COUNT);
	uint colorCount = 0;
	Common::Array<byte> lineCopperPal;
	if (!decodeAmigaMxmmSceneBackground(mxmm.data(), size, pixels, paletteRgb, colorCount, lineCopperPal))
		return false;
	if (pixels.size() != (uint)kScreenWidth * kGameHeight || colorCount == 0)
		return false;

	for (int y = 0; y < kGameHeight; y++) {
		for (int x = 0; x < kScreenWidth; x++)
			_sceneBackground.setPixel(x, y, pixels[(uint)y * kScreenWidth + x]);
	}

	// _palVanilla holds raw 6-bit VGA values (fade math subtracts from these).
	// _pal is the 8-bit display palette - must be expanded via applyPaletteDarkening().
	_pal = Graphics::Palette(Graphics::PALETTE_COUNT);
	_palVanilla = Graphics::Palette(Graphics::PALETTE_COUNT);
	for (uint i = 0; i < colorCount && i < Graphics::PALETTE_COUNT; i++) {
		byte r8, g8, b8;
		paletteRgb.get(i, r8, g8, b8);
		_palVanilla.set(i, PALETTE_8BIT_TO_6BIT(r8), PALETTE_8BIT_TO_6BIT(g8), PALETTE_8BIT_TO_6BIT(b8));
	}

	// Keep Info UI chrome colors in the high VGA indices used by panel drawing.
	if (_amigaArchive->getInfo().loaded) {
		const AmigaInfoData &info = _amigaArchive->getInfo();
		for (uint i = 0; i < ARRAYSIZE(info.uiPaletteAmiga); i++) {
			const uint idx = 0xF0 + i;
			if (idx >= 256)
				break;
			uint16 rgb = info.uiPaletteAmiga[i];
			byte r6;
			byte g6;
			byte b6;
			amiga12ToVga6(rgb, r6, g6, b6);
			_palVanilla.set(idx, r6, g6, b6);
		}
	}
	_amigaNativePlayfieldPalette = true;
	_amigaLineCopperPal = Common::move(lineCopperPal);
	// Portraits share playfield COLOR17..31 (copper high bank). Native copper
	// already filled those slots; installAmigaPortraitPalette is a no-op here.
	installAmigaPortraitPalette(true);
	buildAmigaPanelRemapTable();
	applyPaletteDarkening();

	Common::Rect screenRect(0, 0, kScreenWidth, kGameHeight);
	_depthMap.fillRect(screenRect, 0);
	_pathfindingMap.fillRect(screenRect, 0);
	_shadowMap.fillRect(screenRect, 0);
	_hotspotMap.fillRect(screenRect, 0);

	Common::Array<byte> pathMap, depthMap, shadowMap;
	if (extractAmigaMxmmSceneMaps(mxmm.data(), size, pathMap, depthMap, shadowMap)) {
		blitMap(_pathfindingMap, pathMap);
		blitMap(_depthMap, depthMap);
		blitMap(_shadowMap, shadowMap);
	}

	_numHotspots = 0;
	_hotspotColorTable.clear();
	uint16 numHotspots = 0;
	Common::Array<uint16> hotspotColors;
	if (extractAmigaMxmmSceneHotspotColors(mxmm.data(), size, numHotspots, hotspotColors)) {
		_numHotspots = numHotspots;
		_hotspotColorTable = Common::move(hotspotColors);
	}
	Common::Array<byte> hotspotMap;
	if (extractAmigaMxmmMxccHotspotMap(mxmm.data(), size, hotspotMap) &&
		hotspotMap.size() == (uint)kScreenWidth * kGameHeight) {
		for (int y = 0; y < kGameHeight; y++) {
			for (int x = 0; x < kScreenWidth; x++)
				_hotspotMap.setPixel(x, y, hotspotMap[(uint)y * kScreenWidth + x]);
		}
	}

	_backgroundAnimations.clear();
	_backgroundAnimationsBlobs.clear();
	_mapImageFileOffset = 0;
	_mapSubSceneTableFilePos = 0;
	if (amigaMxmmHasMxaaOverlay(mxmm.data(), size)) {
		debugC(1, kDebugFilePath, "Amiga: MM_%04u has MXAA overlay data (not loaded yet)",
			   (uint)sceneResourceId);
	}

	_pathfindingPoints.clear();
	_numPathfindingPoints = 0;
	uint16 numPfPoints = 0;
	Common::Array<AmigaPathfindingNode> pfNodes;
	if (extractAmigaMxmmScenePathfinding(mxmm.data(), size, numPfPoints, pfNodes)) {
		_numPathfindingPoints = numPfPoints;
		for (uint i = 0; i < pfNodes.size(); i++) {
			PathfindingPoint current;
			current._index = (uint16)i;
			current._position.x = (int16)pfNodes[i].x;
			current._position.y = (int16)pfNodes[i].y;
			current._adjacentPoints.clear();
			const uint16 nConn = MIN<uint16>(pfNodes[i].numConnections, 4);
			for (uint j = 0; j < nConn; j++) {
				if (pfNodes[i].adjacent[j] != 0)
					current._adjacentPoints.push_back(pfNodes[i].adjacent[j]);
			}
			_pathfindingPoints.push_back(current);
		}
	}

	uint16 walkThreshY = 0, walkScale = 0, walkBasePct = 0, palMode = 0, darken = 0;
	if (extractAmigaMxmmSceneWalkParams(mxmm.data(), size, walkThreshY, walkScale, walkBasePct, palMode,
										darken)) {
		_walkDepthThresholdY = walkThreshY;
		_walkDepthScaleFactor = walkScale;
		_walkBaseSpeedPct = walkBasePct;
		_scenePaletteMode = palMode != 0 ? palMode : 1;
		_paletteDarkenPercent = darken;
	} else {
		_walkDepthThresholdY = 100;
		_walkDepthScaleFactor = 100;
		_walkBaseSpeedPct = 100;
		_scenePaletteMode = 1;
		_paletteDarkenPercent = 0;
	}

	_amigaPendingSceneScript.clear();
	_amigaPendingSceneStrings.clear();
	extractAmigaMxmmSceneScript(mxmm.data(), size, _amigaPendingSceneScript, _amigaPendingSceneStrings);

	debugC(1, kDebugFilePath,
		   "Amiga: loaded native MM_%04u (script %u bytes, strings %u bytes, pfNodes %u, "
		   "hotspots %u, walk %u/%u/%u)",
		   (uint)sceneResourceId, (uint)_amigaPendingSceneScript.size(),
		   (uint)_amigaPendingSceneStrings.size(), (uint)_numPathfindingPoints, (uint)_numHotspots,
		   (uint)_walkDepthThresholdY, (uint)_walkDepthScaleFactor, (uint)_walkBaseSpeedPct);
	return true;
}

bool Macs2Engine::loadAmigaMxffFont() {
	if (!_amigaArchive || !_amigaArchive->hasResource(kAmigaResFF, 0))
		return false;

	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_amigaArchive->createReadStreamForResource(kAmigaResFF, 0));
	if (!stream)
		return false;

	const uint32 size = (uint32)stream->size();
	Common::Array<byte> mxff;
	mxff.resize(size);
	if (stream->read(mxff.data(), size) != size)
		return false;

	Common::Array<AmigaMxffGlyph> glyphs;
	if (!decodeAmigaMxffFont(mxff.data(), size, glyphs) || glyphs.empty())
		return false;

	_numGlyphs = 0;
	_maxGlyphHeight = 0;
	amigaTextLinePitch = 0;
	if (size >= 0x0A) {
		const uint16 atlasRows = READ_BE_UINT16(mxff.data() + 8);
		if (atlasRows > 1)
			amigaTextLinePitch = (uint16)(atlasRows - 1);
	}
	for (uint i = 0; i < glyphs.size() && i < 256; i++) {
		_glyphs[i]._ascii = glyphs[i].ascii;
		_glyphs[i]._width = glyphs[i].width;
		_glyphs[i]._height = glyphs[i].height;
		_glyphs[i]._data = Common::move(glyphs[i].pixels);
		// MXFF dialogue glyphs use copper COLOR23 (black) + COLOR27 (near-white
		// outline). Remap into private UI bank 0xF0.. so outdoor copper cannot
		// recolor text. COLOR17+i -> 0xF1+i (MXIN ui[1+i]); COLOR23->0xF7=ui[7]=0,
		// COLOR27->0xFB=ui[11]=EEE. drawText @ 00224492 blits via drawSprite.
		for (uint p = 0; p < _glyphs[i]._data.size(); p++) {
			const byte c = _glyphs[i]._data[p];
			if (c == 0)
				continue;
			if (c >= 17 && c <= 31)
				_glyphs[i]._data[p] = (byte)(0xF0 + (c - 16));
			else if (c < 16)
				_glyphs[i]._data[p] = (byte)(0xF0 + c);
		}
		_maxGlyphHeight = MAX(_maxGlyphHeight, _glyphs[i]._height);
		_numGlyphs++;
	}
	if (amigaTextLinePitch == 0 && _maxGlyphHeight > 1)
		amigaTextLinePitch = (uint16)(_maxGlyphHeight - 1);
	// Reuse dialogue font for panel/save UI until a second MXFF exists.
	numPanelGlyphs = _numGlyphs;
	maxPanelGlyphHeight = _maxGlyphHeight;
	for (uint i = 0; i < _numGlyphs; i++)
		_panelGlyphs[i] = _glyphs[i];

	debugC(1, kDebugFilePath, "Amiga: loaded MXFF font FF_0000 (%u glyphs, height %u, linePitch %u)",
		   _numGlyphs, _maxGlyphHeight, amigaTextLinePitch);
	return _numGlyphs > 0;
}

bool Macs2Engine::loadAmigaOverlayFontResource(uint16 ffId) {
	if (_amigaArchive == nullptr || !_amigaArchive->hasResource(kAmigaResFF, ffId))
		return false;

	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_amigaArchive->createReadStreamForResource(kAmigaResFF, ffId));
	if (!stream)
		return false;

	const uint32 size = (uint32)stream->size();
	Common::Array<byte> mxff;
	mxff.resize(size);
	if (stream->read(mxff.data(), size) != size)
		return false;

	Common::Array<AmigaMxffGlyph> glyphs;
	if (!decodeAmigaMxffFont(mxff.data(), size, glyphs) || glyphs.empty())
		return false;

	numOverlayGlyphs = 0;
	maxOverlayGlyphHeight = 0;
	for (uint i = 0; i < glyphs.size() && i < 256; i++) {
		_overlayGlyphs[i]._ascii = glyphs[i].ascii;
		_overlayGlyphs[i]._width = glyphs[i].width;
		_overlayGlyphs[i]._height = glyphs[i].height;
		_overlayGlyphs[i]._data = Common::move(glyphs[i].pixels);
		// Same copper->UI-bank remap as loadAmigaMxffFont.
		for (uint p = 0; p < _overlayGlyphs[i]._data.size(); p++) {
			const byte c = _overlayGlyphs[i]._data[p];
			if (c == 0)
				continue;
			if (c >= 17 && c <= 31)
				_overlayGlyphs[i]._data[p] = (byte)(0xF0 + (c - 16));
			else if (c < 16)
				_overlayGlyphs[i]._data[p] = (byte)(0xF0 + c);
		}
		maxOverlayGlyphHeight = MAX(maxOverlayGlyphHeight, _overlayGlyphs[i]._height);
		numOverlayGlyphs++;
	}
	return numOverlayGlyphs > 0;
}

bool Macs2Engine::loadAmigaOverlayFont(uint8 resourceIndex) {
	if (resourceIndex != 0 && _amigaArchive != nullptr) {
		uint16 ffId = resourceIndex;
		if (!_amigaArchive->hasResource(kAmigaResFF, ffId) && resourceIndex > 0)
			ffId = (uint16)(resourceIndex - 1);
		if (loadAmigaOverlayFontResource(ffId))
			return true;
	}

	// Fall back to the already-loaded main MXFF dialogue font.
	if (_numGlyphs == 0)
		return false;

	numOverlayGlyphs = _numGlyphs;
	maxOverlayGlyphHeight = _maxGlyphHeight;
	for (uint i = 0; i < _numGlyphs; i++)
		_overlayGlyphs[i] = _glyphs[i];
	return true;
}

bool Macs2Engine::loadAmigaCursorResource(uint16 resourceId, AnimFrame &out) {
	out = AnimFrame();
	if (!_amigaArchive || resourceId == 0)
		return false;

	Common::ScopedPtr<Common::SeekableReadStream> stream(_amigaArchive->createReadStreamForResource(kAmigaResOO, resourceId));
	if (!stream)
		return false;

	const uint32 size = (uint32)stream->size();
	Common::Array<byte> data;
	data.resize(size);
	if (stream->read(data.data(), size) != size)
		return false;

	uint16 width = 0, height = 0;
	Common::Array<byte> pixels;
	if (!Macs2AmigaArchive::decodePlanarSprite(data.data(), size, width, height, pixels))
		return false;

	out._width = width;
	out._height = height;
	out._offsetX = 0;
	out._offsetY = 0;
	out._data = Common::move(pixels);
	return true;
}

void Macs2Engine::installAmigaPortraitPalette(bool copyFromPlayfield) {
	static const uint16 kHighBankFallback[15] = {
		0x0BBA, 0x0EB8, 0x0C96, 0x0A74, 0x0963, 0x0741, 0x0000, 0x049E,
		0x0C00, 0x0DDC, 0x0EEE, 0x0887, 0x0776, 0x0006, 0x0520
	};

	const uint16 *highSrc = kHighBankFallback;
	if (_amigaArchive && _amigaArchive->getInfo().loaded)
		highSrc = &_amigaArchive->getInfo().uiPaletteAmiga[1];

	if (!_amigaNativePlayfieldPalette) {
		// Copper base16: COLOR00 + COLOR17..31 == MXIN ui[0..15].
		for (uint i = 0; i < ARRAYSIZE(kHighBankFallback); i++) {
			byte r6, g6, b6;
			amiga12ToVga6(highSrc[i], r6, g6, b6);
			_palVanilla.set(17 + i, r6, g6, b6);
		}
	}

	for (uint i = 0; i < 16; i++) {
		byte r6, g6, b6;
		_palVanilla.get(0xF0 + i, r6, g6, b6);
		_palVanilla.set(0xE0 + i, (byte)(r6 / 2), (byte)(g6 / 2), (byte)(b6 / 2));
	}
}

void Macs2Engine::applyAmigaUiPalette() {
	if (!_amigaArchive || !_amigaArchive->getInfo().loaded)
		return;

	const AmigaInfoData &info = _amigaArchive->getInfo();

	byte r6, g6, b6;
	amiga12ToVga6(info.uiPaletteAmiga[0], r6, g6, b6);
	_palVanilla.set(0, 0, 0, 0);
	for (uint i = 1; i < 16; i++) {
		const byte v = (byte)((i * 63) / 15);
		_palVanilla.set(i, v, (byte)((v * 3) / 4), (byte)(v / 2));
	}
	for (uint i = 0; i < 15; i++) {
		amiga12ToVga6(info.uiPaletteAmiga[i + 1], r6, g6, b6);
		_palVanilla.set(17 + i, r6, g6, b6);
	}

	// Amiga UI colors also live in the high indices used by panel/chrome drawing.
	for (uint i = 0; i < ARRAYSIZE(info.uiPaletteAmiga); i++) {
		amiga12ToVga6(info.uiPaletteAmiga[i], r6, g6, b6);
		const uint idx = 0xF0 + i;
		if (idx >= Graphics::PALETTE_COUNT)
			break;
		_palVanilla.set(idx, r6, g6, b6);
	}
	_amigaNativePlayfieldPalette = false;
	_amigaLineCopperPal.clear();
	installAmigaPortraitPalette(false);
	buildAmigaPanelRemapTable();
	applyPaletteDarkening();
}

void Macs2Engine::buildAmigaPanelRemapTable() {
	if (_panelRemapTable.size() != 0x100)
		_panelRemapTable.resize(0x100);

	// MXIN UI[0..5] = wood ramp (BBA..741). UI[6] is 0x000 - never use it for fill.
	static const byte kUiWood[8] = {0, 1, 2, 3, 4, 5, 2, 3};
	for (uint i = 0; i < 256; i++) {
		byte r6, g6, b6;
		_palVanilla.get(i, r6, g6, b6);
		const uint r4 = (r6 * 15) / 63;
		const uint g4 = (g6 * 15) / 63;
		const uint b4 = (b6 * 15) / 63;
		const uint sum = r4 + g4 + b4;
		int bucket = 7 - (int)(sum / 0x18);
		if (bucket < 0)
			bucket = 0;
		if (bucket > 7)
			bucket = 7;
		_panelRemapTable[i] = (byte)(0xF0 + kUiWood[bucket]);
	}
}

// --- readAmigaResources (macs2.cpp:3020-3317) ---
void Macs2Engine::readAmigaResources() {
	_amigaArchive = new Macs2AmigaArchive();
	if (!_amigaArchive->open())
		error("readAmigaResources(): Failed to open Amiga DataA/Mdir archive");

	SearchMan.add("macs2amiga", _amigaArchive, 0, false);

	Common::ScopedPtr<Common::SeekableReadStream> sceneTable(_amigaArchive->createSceneTableStream());
	if (!sceneTable)
		error("readAmigaResources(): Failed to decompress Amiga scene table");

	// Keep the scene table resident for later Amiga scene loading work.
	{
		const uint32 size = (uint32)sceneTable->size();
		byte *data = (byte *)malloc(size);
		if (!data)
			error("readAmigaResources(): Out of memory for scene table");
		if (sceneTable->read(data, size) != size) {
			free(data);
			error("readAmigaResources(): Failed reading scene table");
		}
		_fileStream = new Common::MemoryReadStream(data, size, DisposeAfterUse::YES);
	}

	debugC(1, kDebugFilePath, "Amiga scene table loaded (%d bytes), %u scenes, %u archive members",
		   (int)_fileStream->size(), _amigaArchive->getSceneCount(), (uint)_amigaArchive->getResourceCount());

	// Cursor / UI icon slots: DOS uses 33 entries; modes 0x13.. map to index mode-1.
	_imageResources.clear();
	_imageResources.resize(33);

	const AmigaInfoData &info = _amigaArchive->getInfo();
	auto tryLoadCursor = [this](uint16 resourceId) {
		if (resourceId == 0 || resourceId > _imageResources.size())
			return;
		AnimFrame frame;
		if (loadAmigaCursorResource(resourceId, frame))
			_imageResources[resourceId - 1] = Common::move(frame);
	};

	if (info.loaded) {
		for (uint i = 0; i < 5; i++)
			tryLoadCursor(info.cursorResourceIds[i]);
		tryLoadCursor(info.useInventoryCursorId);
		// Panel / map cursor modes used by the engine (0x18, 0x19).
		tryLoadCursor(0x18);
		tryLoadCursor(0x19);
		tryLoadCursor(0x1A);
	} else {
		// Fallback: load every small OO sprite whose id fits the cursor table.
		for (uint16 id = 1; id <= 33; id++) {
			if (_amigaArchive->hasResource(kAmigaResOO, id))
				tryLoadCursor(id);
		}
	}

	// Some cursor mode ids in Info are not standalone OO sprites in the demo.
	// Duplicate the first loaded cursor into empty gameplay slots so the UI works.
	int fallbackIndex = -1;
	for (uint i = 0; i < _imageResources.size(); i++) {
		if (!_imageResources[i]._data.empty() && _imageResources[i]._width > 0 &&
			_imageResources[i]._height > 0) {
			fallbackIndex = (int)i;
			break;
		}
	}
	if (fallbackIndex >= 0) {
		static const uint16 kRequiredModes[] = {0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A};
		for (uint16 mode : kRequiredModes) {
			const uint idx = mode - 1;
			if (idx < _imageResources.size() && _imageResources[idx]._data.empty())
				_imageResources[idx] = _imageResources[fallbackIndex];
		}
	}

	applyAmigaUiPalette();

	const bool loadedMxff = loadAmigaMxffFont();

	// Load every OO resource as a GameObject. Object index = resource id + 1.
	GameObjects::instance()._objects.resize(0x200, nullptr);
	uint loadedObjects = 0;
	uint loadedAnims = 0;

	Common::ArchiveMemberList members;
	_amigaArchive->listMembers(members);
	for (const Common::ArchiveMemberPtr &member : members) {
		AmigaResourceType type = kAmigaResUnknown;
		uint16 resId = 0;
		if (!Macs2AmigaArchive::parseResourceName(member->getName(), type, resId) || type != kAmigaResOO)
			continue;

		Common::ScopedPtr<Common::SeekableReadStream> stream(_amigaArchive->createReadStreamForResource(type, resId));
		if (!stream)
			continue;

		const uint32 size = (uint32)stream->size();
		Common::Array<byte> mxoo;
		mxoo.resize(size);
		if (stream->read(mxoo.data(), size) != size)
			continue;

		const uint16 objectIndex = (uint16)(resId + 1);
		if (objectIndex == 0 || objectIndex > 0x200)
			continue;

		GameObject *gameObject = GameObjects::instance()._objects[objectIndex - 1];
		if (gameObject == nullptr) {
			gameObject = new GameObject();
			gameObject->_index = objectIndex;
			GameObjects::instance()._objects[objectIndex - 1] = gameObject;
		}
		// Non-zero marks the object as having resident Amiga data (skip DOS file reload).
		gameObject->_dataOffset = 1;
		gameObject->_position = Common::Point(0, 0);
		gameObject->_sceneIndex = 0;
		gameObject->_orientation = 11;
		gameObject->_verticalOffsetScale = 0;

		while (gameObject->_blobs.size() < 0x15)
			gameObject->_blobs.push_back(Common::Array<uint8>());
		while (gameObject->_blobSourceKeys.size() < 0x15)
			gameObject->_blobSourceKeys.push_back(0);
		while (gameObject->_blobWalkSpeeds.size() < 0x15)
			gameObject->_blobWalkSpeeds.push_back(0);
		while (gameObject->_blobMirrorFlags.size() < 0x15)
			gameObject->_blobMirrorFlags.push_back(false);

		AmigaMxooInfo mxooInfo;
		if (parseAmigaMxoo(mxoo.data(), size, mxooInfo)) {
			for (uint slot = 0; slot < 21; slot++) {
				Common::Array<byte> blob;
				if (!convertAmigaAnimSlotToDosBlob(mxoo.data(), size, mxooInfo.slotOffsets[slot], blob) || blob.empty())
					continue;
				if (slot < gameObject->_blobs.size())
					gameObject->_blobs[slot] = Common::move(blob);
				else
					gameObject->_blobs.push_back(Common::move(blob));
				loadedAnims++;
			}

			// Amiga MXOO omits mirrored walk/stand slots (Ghidra/demo OO_0000: slots
			// 6-8 and 14-16 empty). DOS stores those slots with bMirrorFlag=1 and
			// mirrorAnimBlob at load (MCSEXEC loadObjectData). Synthesize the same
			// post-load state so walkAlongPath DirAvail and drawAllCharacters work.
			// Walk speeds match DOS protagonist RESOURCE.MCS object 1:
			//   orients 1..8 -> 2,4,6,4,2,4,6,4
			static const uint16 kAmigaWalkSpeeds[8] = {2, 4, 6, 4, 2, 4, 6, 4};
			static const uint8 kMirrorOrientToSource[6][2] = {
				{6, 4}, {7, 3}, {8, 2}, {14, 12}, {15, 11}, {16, 10}};
			for (uint i = 0; i < ARRAYSIZE(kAmigaWalkSpeeds); i++) {
				if (i < gameObject->_blobWalkSpeeds.size())
					gameObject->_blobWalkSpeeds[i] = kAmigaWalkSpeeds[i];
			}
			for (uint m = 0; m < 6; m++) {
				const uint16 dstOrient = kMirrorOrientToSource[m][0];
				const uint16 srcOrient = kMirrorOrientToSource[m][1];
				const uint dstIdx = dstOrient - 1;
				const uint srcIdx = srcOrient - 1;
				if (dstIdx >= gameObject->_blobs.size() || srcIdx >= gameObject->_blobs.size())
					continue;
				if (!gameObject->_blobs[dstIdx].empty() || gameObject->_blobs[srcIdx].empty())
					continue;
				gameObject->_blobs[dstIdx] = gameObject->_blobs[srcIdx];
				BackgroundAnimationBlob::mirrorAnimBlob(gameObject->_blobs[dstIdx]);
				gameObject->_blobMirrorFlags[dstIdx] = true;
				if (dstOrient <= 8 && srcOrient <= 8)
					gameObject->_blobWalkSpeeds[dstIdx] = kAmigaWalkSpeeds[dstOrient - 1];
				loadedAnims++;
			}

			extractAmigaScript(mxoo.data(), size, gameObject->_script);
			extractAmigaStringBlock(mxoo.data(), size, gameObject->_stringData);

			// Dialogue portraits: 240x80x6 atlas (3x80x80) -> blobs[17]/[18].
			while (gameObject->_blobs.size() < 19)
				gameObject->_blobs.push_back(Common::Array<uint8>());
			Common::Array<byte> portraitBlob;
			if (convertAmigaPortraitAtlasToDosBlob(mxoo.data(), size, mxooInfo.extraOffset, portraitBlob) &&
				!portraitBlob.empty()) {
				gameObject->_blobs[17] = portraitBlob;
				gameObject->_blobs[18] = portraitBlob;
				loadedAnims++;
			}

			// DOS MCS stores hasShading/hasScaling after the 21 anim slots (obj1/6/12/...
			// = 1). Amiga MXOO omits those bytes; without them drawAllCharacters used
			// the non-depth blit and actors drew over foreground depth (brown rocks).
			// Match DOS character objects: walk anims and/or dialogue portrait.
			uint walkSlots = 0;
			for (uint s = 0; s < 8 && s < gameObject->_blobs.size(); s++) {
				if (!gameObject->_blobs[s].empty())
					walkSlots++;
			}
			const bool hasPortrait = !gameObject->_blobs[17].empty();
			if (walkSlots >= 2 || hasPortrait) {
				gameObject->_hasScaling = true;
				gameObject->_hasShading = true;
			}
		} else {
			// Simple planar sprite (cursor / inventory icon): expose as walk + inventory slots.
			Common::Array<byte> blob;
			if (convertAmigaSimpleSpriteToDosBlob(mxoo.data(), size, blob) && !blob.empty()) {
				gameObject->_blobs[0] = blob;
				gameObject->_blobs[0x13] = blob;
				loadedAnims++;
			}
		}

		loadedObjects++;
	}

	GameObject *protagonist = GameObjects::instance()._objects[0];
	if (protagonist == nullptr) {
		protagonist = new GameObject();
		protagonist->_index = 1;
		protagonist->_dataOffset = 1;
		GameObjects::instance()._objects[0] = protagonist;
	}
	protagonist->_sceneIndex = 0;
	protagonist->_position = Common::Point(0, 0);
	protagonist->_orientation = 11;

	Scenes::instance()._currentActorIndex = 1;

	uint16 startResourceId = _amigaArchive->getInfo().startSceneResourceId;
	if (startResourceId == 0 || !_amigaArchive->hasResource(kAmigaResMM, startResourceId)) {
		if (_amigaArchive->hasResource(kAmigaResMM, 40))
			startResourceId = 40;
		else if (_amigaArchive->hasResource(kAmigaResMM, 4))
			startResourceId = 4;
		else
			startResourceId = 1;
	}
	const uint16 startScriptScene = (uint16)(startResourceId + 1);
	Scenes::instance()._currentSceneIndex = startScriptScene;

	_sceneBackground.create(kScreenWidth, kGameHeight, Graphics::PixelFormat::createFormatCLUT8());
	_depthMap.create(kScreenWidth, kGameHeight, Graphics::PixelFormat::createFormatCLUT8());
	_pathfindingMap.create(kScreenWidth, kGameHeight, Graphics::PixelFormat::createFormatCLUT8());
	_shadowMap.create(kScreenWidth, kGameHeight, Graphics::PixelFormat::createFormatCLUT8());
	_hotspotMap.create(kScreenWidth, kGameHeight, Graphics::PixelFormat::createFormatCLUT8());
	_shadingTable.resize(0x800);
	Common::fill(_shadingTable.begin(), _shadingTable.end(), 0);
	_panelRemapTable.resize(0x100);
	buildAmigaPanelRemapTable();

	_numHotspots = 0;
	_numPathfindingPoints = 0;
	_scenePaletteMode = 1;
	_paletteDarkenPercent = 0;

	if (!loadedMxff)
		warning("Amiga: no MXFF font FF_0000 in DataA - text may be missing");

	auto ensureBorderTile = [this](uint index, byte color, uint16 w, uint16 h) {
		if (index >= _imageResources.size())
			return;
		if (!_imageResources[index]._data.empty() && _imageResources[index]._width > 0 &&
			_imageResources[index]._height > 0)
			return;
		AnimFrame tile;
		tile._width = w;
		tile._height = h;
		tile._data.resize((uint)w * h);
		Common::fill(tile._data.begin(), tile._data.end(), color);
		_imageResources[index] = Common::move(tile);
	};
	if (_imageResources.size() < 33)
		_imageResources.resize(33);
	ensureBorderTile(30, 0xF2, 8, 1);
	ensureBorderTile(31, 0xF1, 8, 8);
	ensureBorderTile(32, 0xF3, 8, 1);

	debugC(1, kDebugFilePath,
		   "Amiga objects loaded: %u resources, %u anim slots; start MM_%04u (script scene %u)",
		   loadedObjects, loadedAnims, startResourceId, startScriptScene);
	changeScene(Scenes::instance()._currentSceneIndex);
}

} // namespace Macs2
