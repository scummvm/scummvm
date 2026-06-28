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
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kPanelExecutableName = "MONSTERS.EXE";
const char *const kPanelResource000Name = "RESOURCE.000";
const char *const kPanelPaletteResourceType = "PALETA_OBJ_INT";
const char *const kPanelPaletteResourceName = "obj_pal";
const uint kPanelResource000HeaderByteCount = 1;
const uint kPanelResource000OffsetTableSize = 400;
const uint kPanelResource000SizeTableSize = 400;
const uint kPanelStartupResourceEntry = 0x2c;
const uint kPanelStartupPrecedingBlockCount = 3;
const uint kPanelBottomBufferWidth = 640;
const uint kPanelBottomBufferRows = 0xbd;
const uint kPanelResourceSourceStride = 1024;
const uint kPanelObjectPaletteOffset = 0x210;
const uint kPanelObjectPaletteSize = 0xf0;
const uint kPanelObjectPaletteObjectOnlySize = 0x60;
const uint16 kPanelCaptionBandHeight = 0x15;
const uint16 kPanelVerbCaptionY = 0x19f;
const uint16 kPanelVerbContentY = 0x1b5;
const uint16 kPanelDialogueCaptionY = 0x10d;
const uint16 kPanelDialogueContentY = 0x123;
const uint16 kPanelVerbStripTopInBuffer = 8;
const uint16 kPanelVerbStripTextYInBuffer = 0x0b;
const uint16 kPanelVerbStripHeight = 0x1b;
const uint16 kPanelVerbStripWidth = 0x58;
const byte kPanelCaptionColor = 0xfc;
const byte kPanelVerbLabelColor = 0xf1;
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

GameplayPanelArt::GameplayPanelArt() :
		_loaded(false) {
}

bool GameplayPanelArt::load() {
	if (_loaded)
		return true;

	if (!loadBottomPanelBuffer())
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
			tableEntryOffset + kPanelStartupPrecedingBlockCount * 4 > sizeTable.size()) {
		warning("%s startup table is too short for Hollywood bottom-panel art", kPanelResource000Name);
		return false;
	}

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

bool GameplayPanelArt::loadObjectPalette() {
	Common::ScopedPtr<Common::WinResources> exe(Common::WinResources::createFromEXE(Common::Path(kPanelExecutableName)));
	if (!exe)
		return false;

	Common::ScopedPtr<Common::SeekableReadStream> stream(exe->getResource(
		Common::WinResourceID(kPanelPaletteResourceType), Common::WinResourceID(kPanelPaletteResourceName)));
	if (!stream || stream->size() < kPanelObjectPaletteSize)
		return false;

	_objectPaletteTriples.resize(kPanelObjectPaletteSize);
	return stream->read(_objectPaletteTriples.data(), _objectPaletteTriples.size()) ==
		_objectPaletteTriples.size();
}

bool GameplayPanelArt::applyPalette(Common::Array<byte> &palette) const {
	if (_objectPaletteTriples.size() != kPanelObjectPaletteSize ||
			palette.size() < kPanelObjectPaletteOffset + kPanelObjectPaletteObjectOnlySize)
		return false;

	memcpy(palette.data() + kPanelObjectPaletteOffset, _objectPaletteTriples.data(),
		kPanelObjectPaletteObjectOnlySize);
	return true;
}

void GameplayPanelArt::drawVerbPanel(Graphics::Surface &surface, const Common::Array<byte> &savedFramebuffer,
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
		const Common::Array<byte> &savedFramebuffer, uint16 viewportXOffset, uint16 viewportYOffset,
		const GameplayPanelState &panelState, HollywoodFont *font) const {
	if (!_loaded)
		return;

	copySavedCaptionBand(surface, savedFramebuffer, viewportXOffset, viewportYOffset, kPanelDialogueCaptionY);
	copyBottomPanelRows(surface, 0, kPanelDialogueContentY,
		HollywoodEngine::kScreenHeight - kPanelDialogueContentY);
	drawVerbStripLabels(surface, kPanelDialogueContentY, font);
	applySelectedVerbStrip(surface, kPanelDialogueContentY, panelState.currentStrip);
	drawCaptionText(surface, panelState.captionText, kPanelDialogueCaptionY, font);
}

void GameplayPanelArt::copySavedCaptionBand(Graphics::Surface &surface,
		const Common::Array<byte> &savedFramebuffer, uint16 viewportXOffset, uint16 viewportYOffset,
		uint16 screenY) const {
	if (surface.format.bytesPerPixel != 1)
		return;

	for (uint row = 0; row < kPanelCaptionBandHeight; ++row) {
		const uint sceneY = viewportYOffset + screenY + row;
		const uint sourceOffset = sceneY * HollywoodEngine::kSceneBufferWidth + viewportXOffset;
		if (sceneY >= HollywoodEngine::kSceneBufferHeight ||
				sourceOffset + HollywoodEngine::kScreenWidth > savedFramebuffer.size() ||
				screenY + row >= (uint)surface.h)
			continue;

		memcpy(surface.getBasePtr(0, screenY + row), savedFramebuffer.data() + sourceOffset,
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

} // End of namespace Hollywood
