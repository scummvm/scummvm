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

#include "hollywood/gameplay/panel_art.h"

#include "common/endian.h"
#include "common/file.h"
#include "common/formats/winexe.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/textconsole.h"
#include "graphics/surface.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kPanelExecutableName = "MONSTERS.EXE";
const char *const kPanelResource000Name = "RESOURCE.000";
const char *const kPanelDialogueMenuResourceType = "PANEL_CONVERSACIONES";
const char *const kPanelDialogueMenuResourceName = "panconve";
const char *const kPanelPaletteResourceType = "PALETA_OBJ_INT";
const char *const kPanelPaletteResourceName = "obj_pal";
const uint kPanelResource000HeaderByteCount = 1;
const uint kPanelResource000OffsetTableSize = 400;
const uint kPanelResource000SizeTableSize = 400;
const uint kPanelInventoryItemPagesResourceEntry = 0x2b;
const uint kPanelStartupResourceEntry = 0x2c;
const uint kPanelDialogueMenuResource000Entry = 0x2f;
const uint kPanelObjectPaletteResource000Entry = 0x31;
const uint kPanelStartupPrecedingBlockCount = 3;
const uint kPanelBottomBufferWidth = 640;
const uint kPanelBottomBufferRows = 0xbd;
const uint kPanelResourceSourceStride = 1024;
const uint kPanelDialogueMenuTopRows = 12;
const uint kPanelDialogueMenuLineRows = 21;
const uint kPanelDialogueMenuBottomRows = 12;
const uint kPanelDialogueMenuTopOffset = 0;
const uint kPanelDialogueMenuLineOffset = kPanelDialogueMenuTopRows * kPanelBottomBufferWidth;
const uint kPanelDialogueMenuBottomOffset = kPanelDialogueMenuLineOffset +
	kPanelDialogueMenuLineRows * kPanelBottomBufferWidth;
const uint kPanelDialogueMenuResourceSize = kPanelDialogueMenuBottomOffset +
	kPanelDialogueMenuBottomRows * kPanelBottomBufferWidth;
const uint kPanelInventoryTilePageCount = 0x100;
const uint kPanelInventoryTilePageSize = 0x1000;
const uint16 kPanelInventoryGridLeft = 0x32;
const uint16 kPanelInventoryGridTop = 0x152;
const uint16 kPanelInventoryTileSize = 0x40;
const uint16 kPanelInventoryTileStride = 0x44;
const uint kPanelObjectPaletteOffset = 0x210;
const uint kPanelObjectPaletteSize = 0xf0;
const uint kPanelObjectPaletteObjectOnlySize = 0x60;
const uint16 kPanelCaptionBandHeight = 0x15;
const uint16 kPanelVerbCaptionY = 0x19f;
const uint16 kPanelVerbContentY = 0x1b5;
const uint16 kPanelDialogueCaptionY = 0x10d;
const uint16 kPanelDialogueContentY = 0x123;
const uint16 kPanelDialogueMenuPanelTopBaseY = 0x1c8;
const uint16 kPanelDialogueMenuLineHeight = 0x15;
const uint16 kPanelDialogueMenuTextLeft = 0x0f;
const uint16 kPanelDialogueMenuFirstTextLeft = 7;
const uint16 kPanelDialogueMenuTextTopOffset = 0x0c;
const uint16 kPanelDialogueMenuTextWidth = 0x266;
const uint16 kPanelVerbStripTopInBuffer = 8;
const uint16 kPanelVerbStripTextYInBuffer = 0x0b;
const uint16 kPanelVerbStripHeight = 0x1b;
const uint16 kPanelVerbStripWidth = 0x58;
const byte kPanelCaptionColor = 0xfc;
const byte kPanelVerbLabelColor = 0xf1;
const byte kPanelDialogueMenuNormalColor = 0xf2;
const byte kPanelDialogueMenuHighlightColor = 0xf9;
const byte kPanelSelectedColorDelta = 9;
const uint16 kPanelVerbStripXOffsets[9] = {
	0xff, 0, 8, 97, 186, 276, 366, 456, 545
};
const uint16 kPanelVerbStripLabelCenters[9] = {
	0, 0, 0x33, 0x8c, 0xe5, 0x13f, 0x199, 499, 0x24c
};
const char *const kPanelVerbStripLabels[9] = {
	"", "", "Hablar", "Coger", "Mirar", "Usar", "Abrir", "Cerrar", "Dar"
};

struct InventoryItemPageMapEntry {
	byte itemId;
	byte pageIndex;
};

const InventoryItemPageMapEntry kSueInventoryItemPageMap[] = {
	{ 0x01, 0x6f },
	{ 0x02, 0x7c },
	{ 0x05, 0x7a },
	{ 0x06, 0x78 },
	{ 0x07, 0x06 },
	{ 0x08, 0x76 },
	{ 0x09, 0x77 },
	{ 0x0b, 0x74 },
	{ 0x0c, 0x71 },
	{ 0x0d, 0x72 },
	{ 0x0f, 0x70 },
	{ 0x10, 0x75 },
	{ 0x11, 0x73 },
	{ 0x13, 0x79 },
	{ 0x14, 0x65 },
	{ 0x15, 0x6b },
	{ 0x16, 0x55 },
	{ 0x17, 0x03 },
	{ 0x18, 0x61 },
	{ 0x19, 0x26 },
	{ 0x1a, 0x68 },
	{ 0x1b, 0x2f },
	{ 0x1c, 0x3f },
	{ 0x1d, 0x14 },
	{ 0x1e, 0x29 },
	{ 0x1f, 0x4a },
	{ 0x20, 0x52 },
	{ 0x21, 0x4b },
	{ 0x22, 0x7b }
};

GameplayPanelArt::GameplayPanelArt() :
		_inventoryItemPageBaseOffset(0),
		_loaded(false) {
}

bool GameplayPanelArt::load() {
	if (_loaded)
		return true;

	if (!loadBottomPanelBuffer())
		return false;

	if (!loadDialogueMenuPanelBuffer())
		return false;

	if (!loadObjectPalette())
		warning("Failed to load Hollywood bottom-panel object palette from %s", kPanelExecutableName);

	_loaded = true;
	return true;
}

bool GameplayPanelArt::loadBottomPanelBuffer() {
	Common::File file;
	if (!file.open(Common::Path(kPanelResource000Name))) {
		warning("Failed to open %s for Hollywood bottom-panel art", kPanelResource000Name);
		return false;
	}

	const uint32 startupTablesSize = kPanelResource000HeaderByteCount +
		kPanelResource000OffsetTableSize + kPanelResource000SizeTableSize;
	if ((uint32)file.size() < startupTablesSize) {
		warning("%s is too small for Hollywood startup tables", kPanelResource000Name);
		return false;
	}

	Common::Array<byte> offsetTable;
	Common::Array<byte> sizeTable;
	offsetTable.resize(kPanelResource000OffsetTableSize);
	sizeTable.resize(kPanelResource000SizeTableSize);

	file.seek(kPanelResource000HeaderByteCount);
	if (file.read(offsetTable.data(), offsetTable.size()) != offsetTable.size() ||
			file.read(sizeTable.data(), sizeTable.size()) != sizeTable.size()) {
		warning("Failed to read %s startup tables for Hollywood bottom-panel art", kPanelResource000Name);
		return false;
	}

	const uint tableEntryOffset = kPanelStartupResourceEntry * 4;
	if (tableEntryOffset + (kPanelStartupPrecedingBlockCount + 1) * 4 > offsetTable.size() ||
			tableEntryOffset + kPanelStartupPrecedingBlockCount * 4 > sizeTable.size() ||
			kPanelInventoryItemPagesResourceEntry * 4 + 4 > offsetTable.size()) {
		warning("%s startup table is too short for Hollywood bottom-panel art", kPanelResource000Name);
		return false;
	}

	_inventoryItemPageBaseOffset =
		READ_LE_UINT32(offsetTable.data() + kPanelInventoryItemPagesResourceEntry * 4);
	_inventoryItemTilePages.resize(kPanelInventoryTilePageCount);

	uint32 panelOffset = READ_LE_UINT32(offsetTable.data() + tableEntryOffset);
	for (uint i = 0; i < kPanelStartupPrecedingBlockCount; ++i)
		panelOffset += READ_LE_UINT32(sizeTable.data() + tableEntryOffset + i * 4);

	const uint32 sourceByteCount = (kPanelBottomBufferRows - 1) * kPanelResourceSourceStride +
		kPanelBottomBufferWidth;
	if (panelOffset > (uint32)file.size() || sourceByteCount > (uint32)file.size() - panelOffset) {
		warning("%s bottom-panel art has invalid bounds", kPanelResource000Name);
		return false;
	}

	_bottomPanelBuffer.resize(kPanelBottomBufferRows * kPanelBottomBufferWidth);
	file.seek(panelOffset);
	for (uint row = 0; row < kPanelBottomBufferRows; ++row) {
		byte *destination = _bottomPanelBuffer.data() + row * kPanelBottomBufferWidth;
		if (file.read(destination, kPanelBottomBufferWidth) != kPanelBottomBufferWidth) {
			warning("Failed to read Hollywood bottom-panel row %u", row);
			return false;
		}
		if (row + 1 < kPanelBottomBufferRows)
			file.seek(kPanelResourceSourceStride - kPanelBottomBufferWidth, SEEK_CUR);
	}

	return true;
}

bool GameplayPanelArt::loadDialogueMenuPanelBuffer() {
	Common::ScopedPtr<Common::WinResources> exe(Common::WinResources::createFromEXE(Common::Path(kPanelExecutableName)));
	if (exe) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(exe->getResource(
			Common::WinResourceID(kPanelDialogueMenuResourceType),
			Common::WinResourceID(kPanelDialogueMenuResourceName)));
		if (stream && stream->size() >= kPanelDialogueMenuResourceSize) {
			_dialogueMenuPanelBuffer.resize(kPanelDialogueMenuResourceSize);
			return stream->read(_dialogueMenuPanelBuffer.data(), _dialogueMenuPanelBuffer.size()) ==
				_dialogueMenuPanelBuffer.size();
		}
	}

	return loadDialogueMenuPanelBufferFromResource000();
}

bool GameplayPanelArt::loadDialogueMenuPanelBufferFromResource000() {
	Common::File file;
	if (!file.open(Common::Path(kPanelResource000Name))) {
		warning("Failed to open %s for Hollywood dialogue panel art", kPanelResource000Name);
		return false;
	}

	const uint tableOffset = kPanelResource000HeaderByteCount + kPanelDialogueMenuResource000Entry * 4;
	if ((uint32)file.size() < tableOffset + 4) {
		warning("%s is too small for Hollywood dialogue panel table entry", kPanelResource000Name);
		return false;
	}

	file.seek(tableOffset);
	const uint32 panelOffset = file.readUint32LE();
	const uint32 sourceByteCount = (kPanelDialogueMenuTopRows + kPanelDialogueMenuLineRows +
		kPanelDialogueMenuBottomRows - 1) * kPanelResourceSourceStride + kPanelBottomBufferWidth;
	if (panelOffset > (uint32)file.size() || sourceByteCount > (uint32)file.size() - panelOffset) {
		warning("%s dialogue panel art has invalid bounds", kPanelResource000Name);
		return false;
	}

	_dialogueMenuPanelBuffer.resize(kPanelDialogueMenuResourceSize);
	file.seek(panelOffset);
	for (uint row = 0; row < kPanelDialogueMenuTopRows + kPanelDialogueMenuLineRows +
			kPanelDialogueMenuBottomRows; ++row) {
		byte *destination = _dialogueMenuPanelBuffer.data() + row * kPanelBottomBufferWidth;
		if (file.read(destination, kPanelBottomBufferWidth) != kPanelBottomBufferWidth) {
			warning("Failed to read Hollywood dialogue panel row %u", row);
			return false;
		}
		if (row + 1 < kPanelDialogueMenuTopRows + kPanelDialogueMenuLineRows +
				kPanelDialogueMenuBottomRows)
			file.seek(kPanelResourceSourceStride - kPanelBottomBufferWidth, SEEK_CUR);
	}

	return true;
}

bool GameplayPanelArt::loadObjectPalette() {
	Common::ScopedPtr<Common::WinResources> exe(Common::WinResources::createFromEXE(Common::Path(kPanelExecutableName)));
	if (exe) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(exe->getResource(
			Common::WinResourceID(kPanelPaletteResourceType), Common::WinResourceID(kPanelPaletteResourceName)));
		if (stream && stream->size() >= kPanelObjectPaletteSize) {
			_objectPaletteTriples.resize(kPanelObjectPaletteSize);
			return stream->read(_objectPaletteTriples.data(), _objectPaletteTriples.size()) ==
				_objectPaletteTriples.size();
		}
	}

	return loadObjectPaletteFromResource000();
}

bool GameplayPanelArt::loadObjectPaletteFromResource000() {
	Common::File file;
	if (!file.open(Common::Path(kPanelResource000Name)))
		return false;

	const uint offsetTableOffset = kPanelResource000HeaderByteCount + kPanelObjectPaletteResource000Entry * 4;
	const uint sizeTableOffset = kPanelResource000HeaderByteCount + kPanelResource000OffsetTableSize +
		kPanelObjectPaletteResource000Entry * 4;
	if ((uint32)file.size() < sizeTableOffset + 4)
		return false;

	file.seek(offsetTableOffset);
	const uint32 paletteOffset = file.readUint32LE();
	file.seek(sizeTableOffset);
	const uint32 paletteSize = file.readUint32LE();
	if (paletteSize < kPanelObjectPaletteObjectOnlySize ||
			paletteOffset > (uint32)file.size() ||
			kPanelObjectPaletteObjectOnlySize > (uint32)file.size() - paletteOffset)
		return false;

	_objectPaletteTriples.resize(kPanelObjectPaletteSize);
	memset(_objectPaletteTriples.data(), 0, _objectPaletteTriples.size());
	file.seek(paletteOffset);
	return file.read(_objectPaletteTriples.data(), kPanelObjectPaletteObjectOnlySize) ==
		kPanelObjectPaletteObjectOnlySize;
}

bool GameplayPanelArt::loadInventoryItemTilePage(byte pageIndex, Common::Array<byte> &page) const {
	if (_inventoryItemPageBaseOffset == 0)
		return false;
	if (!page.empty())
		return page.size() == kPanelInventoryTilePageSize;

	Common::File file;
	if (!file.open(Common::Path(kPanelResource000Name))) {
		warning("Failed to open %s for Hollywood inventory item art", kPanelResource000Name);
		return false;
	}

	const uint32 pageOffset = _inventoryItemPageBaseOffset + (uint32)pageIndex * kPanelInventoryTilePageSize;
	if (pageOffset > (uint32)file.size() ||
			kPanelInventoryTilePageSize > (uint32)file.size() - pageOffset) {
		warning("%s inventory item page %u is out of range", kPanelResource000Name, pageIndex);
		return false;
	}

	page.resize(kPanelInventoryTilePageSize);
	file.seek(pageOffset);
	if (file.read(page.data(), page.size()) != page.size()) {
		warning("Failed to read Hollywood inventory item page %u", pageIndex);
		page.clear();
		return false;
	}

	return true;
}

bool GameplayPanelArt::applyPalette(Common::Array<byte> &palette) const {
	return applyInteractiveObjectPalette(palette);
}

bool GameplayPanelArt::applyInteractiveObjectPalette(Common::Array<byte> &palette) const {
	if (_objectPaletteTriples.size() != kPanelObjectPaletteSize ||
			palette.size() < kPanelObjectPaletteOffset + kPanelObjectPaletteObjectOnlySize)
		return false;

	memcpy(palette.data() + kPanelObjectPaletteOffset, _objectPaletteTriples.data(),
		kPanelObjectPaletteObjectOnlySize);
	return true;
}

void GameplayPanelArt::drawVerbPanel(Graphics::Surface &surface, const Graphics::Surface &savedFramebuffer,
		uint16 viewportXOffset, uint16 viewportYOffset, const GameplayPanelState &panelState,
		HollywoodFont *font) const {
	if (!_loaded)
		return;

	copySavedCaptionBand(surface, savedFramebuffer, viewportXOffset, viewportYOffset, kPanelVerbCaptionY);
	copyBottomPanelRows(surface, 0, kPanelVerbContentY, HollywoodEngine::kScreenHeight - kPanelVerbContentY);
	drawVerbStripLabels(surface, kPanelVerbContentY, font);
	applySelectedVerbStrip(surface, kPanelVerbContentY, panelState.currentStrip);
	drawCaptionText(surface, panelState.captionText, kPanelVerbCaptionY, font);
}

void GameplayPanelArt::drawDialogueInventoryPanel(Graphics::Surface &surface,
		const Graphics::Surface &savedFramebuffer, uint16 viewportXOffset, uint16 viewportYOffset,
		const GameplayPanelState &panelState, const GameplayState &gameState, HollywoodFont *font) const {
	if (!_loaded)
		return;

	copySavedCaptionBand(surface, savedFramebuffer, viewportXOffset, viewportYOffset, kPanelDialogueCaptionY);
	copyBottomPanelRows(surface, 0, kPanelDialogueContentY,
		HollywoodEngine::kScreenHeight - kPanelDialogueContentY);
	drawInventoryItems(surface, gameState);
	drawVerbStripLabels(surface, kPanelDialogueContentY, font);
	applySelectedVerbStrip(surface, kPanelDialogueContentY, panelState.currentStrip);
	drawCaptionText(surface, panelState.captionText, kPanelDialogueCaptionY, font);
}

void GameplayPanelArt::drawDialogueMenuPanel(Graphics::Surface &surface,
		const DialogueMenuState &menuState, HollywoodFont *font) const {
	if (!_loaded || !menuState.visible() ||
			_dialogueMenuPanelBuffer.size() < kPanelDialogueMenuResourceSize ||
			surface.format.bytesPerPixel != 1)
		return;

	const uint16 panelTop = kPanelDialogueMenuPanelTopBaseY -
		(uint16)menuState.lineCount * kPanelDialogueMenuLineHeight;
	for (uint row = 0; row < kPanelDialogueMenuTopRows; ++row) {
		const uint targetY = panelTop + row;
		if (targetY >= (uint)surface.h)
			continue;

		memcpy(surface.getBasePtr(0, targetY),
			_dialogueMenuPanelBuffer.data() + kPanelDialogueMenuTopOffset +
				row * kPanelBottomBufferWidth,
			kPanelBottomBufferWidth);
	}
	for (byte lineIndex = 0; lineIndex < menuState.lineCount; ++lineIndex) {
		for (uint row = 0; row < kPanelDialogueMenuLineRows; ++row) {
			const uint targetY = panelTop + kPanelDialogueMenuTopRows +
				(uint)lineIndex * kPanelDialogueMenuLineRows + row;
			if (targetY >= (uint)surface.h)
				continue;

			memcpy(surface.getBasePtr(0, targetY),
				_dialogueMenuPanelBuffer.data() + kPanelDialogueMenuLineOffset +
					row * kPanelBottomBufferWidth,
				kPanelBottomBufferWidth);
		}
	}
	for (uint row = 0; row < kPanelDialogueMenuBottomRows; ++row) {
		const uint targetY = panelTop + kPanelDialogueMenuTopRows +
			(uint)menuState.lineCount * kPanelDialogueMenuLineRows + row;
		if (targetY >= (uint)surface.h)
			continue;

		memcpy(surface.getBasePtr(0, targetY),
			_dialogueMenuPanelBuffer.data() + kPanelDialogueMenuBottomOffset +
				row * kPanelBottomBufferWidth,
			kPanelBottomBufferWidth);
	}
	drawDialogueMenuRows(surface, menuState, panelTop, font);
}

void GameplayPanelArt::copySavedCaptionBand(Graphics::Surface &surface,
		const Graphics::Surface &savedFramebuffer, uint16 viewportXOffset, uint16 viewportYOffset,
		uint16 screenY) const {
	if (surface.format.bytesPerPixel != 1 || savedFramebuffer.format.bytesPerPixel != 1)
		return;

	for (uint row = 0; row < kPanelCaptionBandHeight; ++row) {
		const uint sceneY = viewportYOffset + screenY + row;
		if (sceneY >= HollywoodEngine::kSceneBufferHeight ||
				sceneY >= (uint)savedFramebuffer.h ||
				viewportXOffset + HollywoodEngine::kScreenWidth > (uint)savedFramebuffer.w ||
				screenY + row >= (uint)surface.h)
			continue;

		memcpy(surface.getBasePtr(0, screenY + row),
			savedFramebuffer.getBasePtr(viewportXOffset, sceneY),
			HollywoodEngine::kScreenWidth);
	}
}

void GameplayPanelArt::copyBottomPanelRows(Graphics::Surface &surface, uint16 sourceRow, uint16 screenY,
		uint16 rowCount) const {
	if (surface.format.bytesPerPixel != 1)
		return;

	for (uint row = 0; row < rowCount; ++row) {
		const uint panelRow = sourceRow + row;
		const uint targetY = screenY + row;
		const uint sourceOffset = panelRow * kPanelBottomBufferWidth;
		if (panelRow >= kPanelBottomBufferRows || targetY >= (uint)surface.h ||
				sourceOffset + kPanelBottomBufferWidth > _bottomPanelBuffer.size())
			continue;

		memcpy(surface.getBasePtr(0, targetY), _bottomPanelBuffer.data() + sourceOffset,
			kPanelBottomBufferWidth);
	}
}

void GameplayPanelArt::drawInventoryItems(Graphics::Surface &surface, const GameplayState &gameState) const {
	if (surface.format.bytesPerPixel != 1)
		return;

	const byte owner = gameState.currentInventoryOwnerIndex;
	if (owner >= GameplayState::kInventoryOwnerCount)
		return;

	byte firstVisibleSlot = gameState.inventoryFirstVisibleSlotByOwner[owner];
	if (firstVisibleSlot == 0)
		firstVisibleSlot = GameplayState::kInventoryFirstSlot;

	for (byte visibleSlot = 0; visibleSlot < GameplayState::kInventoryVisibleSlotCount; ++visibleSlot) {
		const uint slot = firstVisibleSlot + visibleSlot;
		if (slot >= GameplayState::kInventoryOwnerSlotStride ||
				slot > gameState.inventoryItemCountByOwner[owner])
			continue;

		const byte itemId = gameState.inventorySlotItemIdByOwner[owner][slot];
		if (itemId == 0)
			continue;

		byte pageIndex = gameState.inventoryItemResourcePageByOwnerAndItemId[owner][itemId];
		if (owner == 1 && pageIndex == 0)
			pageIndex = sueInventoryItemPage(itemId);
		if (pageIndex == 0)
			continue;

		Common::Array<byte> &page = _inventoryItemTilePages[pageIndex];
		if (!loadInventoryItemTilePage(pageIndex, page))
			continue;

		const int targetX = kPanelInventoryGridLeft + (visibleSlot % 8) * kPanelInventoryTileStride;
		const int targetY = kPanelInventoryGridTop + (visibleSlot / 8) * kPanelInventoryTileStride;
		for (uint row = 0; row < kPanelInventoryTileSize; ++row) {
			const int y = targetY + row;
			if (y < 0 || y >= surface.h)
				continue;

			byte *destination = (byte *)surface.getBasePtr(targetX, y);
			const byte *source = page.data() + row * kPanelInventoryTileSize;
			for (uint column = 0; column < kPanelInventoryTileSize && targetX + (int)column < surface.w; ++column)
				destination[column] = source[column];
		}
	}
}

void GameplayPanelArt::drawDialogueMenuRows(Graphics::Surface &surface,
		const DialogueMenuState &menuState, uint16 screenY, HollywoodFont *font) const {
	if (!font || !font->isLoaded())
		return;

	const byte highlightedChoice = menuState.choiceForLine(menuState.highlightedLineIndex);
	font->setShadowColor(0);
	for (byte lineIndex = 0; lineIndex < menuState.lineCount && lineIndex < menuState.lines.size(); ++lineIndex) {
		const DialogueMenuLine &line = menuState.lines[lineIndex];
		const byte color = line.choiceIndex == highlightedChoice ?
			kPanelDialogueMenuHighlightColor : kPanelDialogueMenuNormalColor;
		const int x = line.firstLineOfChoice ? kPanelDialogueMenuFirstTextLeft :
			kPanelDialogueMenuTextLeft;
		const int y = screenY + lineIndex * kPanelDialogueMenuLineHeight +
			kPanelDialogueMenuTextTopOffset;
		font->drawString(&surface, line.text, x, y, kPanelDialogueMenuTextWidth, color,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void GameplayPanelArt::drawCaptionText(Graphics::Surface &surface, const Common::String &text, int y,
		HollywoodFont *font) const {
	if (!font || !font->isLoaded() || text.empty())
		return;

	font->setShadowColor(0);
	const int textWidth = font->getStringWidth(text) + 2;
	const int x = MAX<int>(0, (HollywoodEngine::kScreenWidth - textWidth) / 2);
	font->drawString(&surface, text, x, y, textWidth, kPanelCaptionColor, Graphics::kTextAlignLeft, 0,
		false, true);
}

void GameplayPanelArt::drawVerbStripLabels(Graphics::Surface &surface, int screenY, HollywoodFont *font) const {
	if (!font || !font->isLoaded())
		return;

	font->setShadowColor(0);
	const int textY = screenY + kPanelVerbStripTextYInBuffer;
	for (byte stripIndex = 2; stripIndex <= 8; ++stripIndex) {
		const Common::String text(kPanelVerbStripLabels[stripIndex]);
		const int textWidth = font->getStringWidth(text) + 2;
		const int x = MAX<int>(0, (int)kPanelVerbStripLabelCenters[stripIndex] - textWidth / 2);
		font->drawString(&surface, text, x, textY, textWidth, kPanelVerbLabelColor,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void GameplayPanelArt::applySelectedVerbStrip(Graphics::Surface &surface, int screenY, byte stripIndex) const {
	if (surface.format.bytesPerPixel != 1 || stripIndex < 2 || stripIndex > 8)
		return;

	const int left = kPanelVerbStripXOffsets[stripIndex];
	const int top = screenY + kPanelVerbStripTopInBuffer;
	for (int row = 0; row < kPanelVerbStripHeight; ++row) {
		const int y = top + row;
		if (y < 0 || y >= surface.h)
			continue;

		byte *pixels = (byte *)surface.getBasePtr(left, y);
		for (int column = 0; column < kPanelVerbStripWidth && left + column < surface.w; ++column) {
			if (pixels[column] != 0)
				pixels[column] += kPanelSelectedColorDelta;
		}
	}
}

byte GameplayPanelArt::sueInventoryItemPage(byte itemId) const {
	for (uint i = 0; i < ARRAYSIZE(kSueInventoryItemPageMap); ++i) {
		if (kSueInventoryItemPageMap[i].itemId == itemId)
			return kSueInventoryItemPageMap[i].pageIndex;
	}

	return 0;
}

} // End of namespace Hollywood
