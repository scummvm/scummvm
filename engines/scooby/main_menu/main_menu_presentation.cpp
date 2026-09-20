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

#include "main_menu_presentation.h"

#include "common/scummsys.h"

#include "common/array.h"

#include "reveal/menu_reveal_compositor.h"
#include "scooby/compression/byte_rle_decoder.h"
#include "scooby/compression/ring_lz_decoder.h"
#include "scooby/graphics/binary_mask_tile_generator.h"
#include "scooby/graphics/genesis_asset_decoder.h"
#include "scooby/graphics/horizontal_display_mode.h"
#include "scooby/graphics/tile_cell.h"
#include "scooby/graphics/tile_layer.h"

namespace Scooby {
const Common::String MainMenuPresentation::kBlankTileRow(static_cast<std::size_t>(kMenuPlaneColumns), ' ');

int MainMenuPresentation::loadInitialAssets() {
	_scene.reset(HorizontalDisplayMode::H32);
	_menuAnimation.reset();
	_textAttributes = kBaseTextAttributes;
	Common::Array<uint8> initialTiles = decompressRingLz(_rom, 0x1155A);
	_scene.loadTiles(MakeSpan(initialTiles));
	int initialTileCount = static_cast<int>(initialTiles.size()) / kPackedBytesPerTile;

	Common::Array<Common::Array<uint8>> additionalTileChunks{decompressByteRle(_rom, 0x16D38),
															 decompressByteRle(_rom, 0x16E6A),
															 decompressByteRle(_rom, 0x16F76)};

	_fixedBackgroundMap = _rom.readBytes(0x165B8, 0x700).toArray();
	_menuLayout = _rom.readBytes(0x85F8, 0x68).toArray();

	int assetTableBase = 0x10136;
	uint16 selector = _rom.readUInt16(assetTableBase + 4);
	uint16 firstPointerIndex = _rom.readUInt16(assetTableBase + selector);
	uint16 secondPointerIndex = _rom.readUInt16(assetTableBase + 2 + selector);
	_wideRevealSource = decompressByteRle(
		_rom, assetTableBase + static_cast<int>(_rom.readUInt32(assetTableBase + 6 + firstPointerIndex)));
	_narrowRevealSource = decompressByteRle(
		_rom, assetTableBase + static_cast<int>(_rom.readUInt32(assetTableBase + 6 + secondPointerIndex)));
	stageInitialMenuAssets(initialTileCount, additionalTileChunks, _fixedBackgroundMap);
	return initialTileCount;
}

void MainMenuPresentation::restoreBinaryMaskTiles() {
	int firstMaskTileIndex = (_textAttributes & 0x07FF) + 0x20;
	Common::Array<uint8> maskTiles = generate(_rom, 0x0F, 0);
	_scene.loadTilesAt(MakeSpan(maskTiles), firstMaskTileIndex);
}

bool MainMenuPresentation::fadePaletteIn(const Common::Functor0<void> &beforeRetrace) {
	Common::Array<PaletteColor> targetPalette = readPalette(
		_rom, 0x16CB8);
	Common::Array<PaletteColor> overlay = readPalette(_rom, 0x32482, 30);
	Common::copy(overlay.begin(), overlay.end(), targetPalette.begin() + 1);
	return _presenter.fadePaletteIn(MakeSpan(targetPalette), &beforeRetrace);
}

void MainMenuPresentation::stageByteRleData(int sourceOffset) {
	_currentRleData = decompressByteRle(_rom, sourceOffset);
}

void MainMenuPresentation::composeWideMenuRevealTiles(int16 rowMaskOffset, int16 columnMaskOffset) {
	_wideRevealTiles = ComposeWide(
		_rom, MakeSpan(_wideRevealSource), rowMaskOffset, columnMaskOffset,
		_menuAnimation.rowShiftStartIndex());
}

void MainMenuPresentation::composeNarrowMenuRevealTiles(int16 rowMaskOffset,
														int16 columnMaskOffset) {
	_narrowRevealTiles = ComposeNarrow(
		_rom, MakeSpan(_narrowRevealSource), rowMaskOffset, columnMaskOffset,
		_menuAnimation.rowShiftStartIndex());
}

void MainMenuPresentation::loadLightningPalette(int sourceOffset) {
	Common::Array<PaletteColor> palette =
		readPalette(_rom, sourceOffset, kColorsPerPaletteLine);
	_scene.replacePaletteRange(MakeSpan(palette), kLightningPaletteStartIndex);
}

void MainMenuPresentation::applyLightningPatch(int sourceOffset, int column, int row, int width, int height) {
	_scene.loadLayerRows(_rom.readBytes(sourceOffset, width * height * static_cast<int>(sizeof(uint16))),
						 TileLayer::Background, column, row, width, height);
}

void MainMenuPresentation::restoreLightningPatch(int column, int row, int width, int height) {
	for (int rowOffset = 0; rowOffset < height; rowOffset++) {
		int sourceOffset = ((row + rowOffset) * kMenuPlaneColumns + column) * static_cast<int>(sizeof(
																				  uint16));
		Span<const uint8> patch =
			MakeSpan(_fixedBackgroundMap)
				.slice(static_cast<std::size_t>(sourceOffset),
					   static_cast<std::size_t>(width * static_cast<int>(sizeof(uint16))));
		_scene.loadLayerRows(patch, TileLayer::Background, column, row + rowOffset, width, 1);
	}
}

void MainMenuPresentation::commitRevealSceneBuffers(bool includeNarrowTiles) {
	_scene.loadTilesAt(MakeSpan(_wideRevealTiles), kWideRevealTileIndex);
	if (includeNarrowTiles) {
		_scene.loadTilesAt(MakeSpan(_narrowRevealTiles), kNarrowRevealTileIndex);
	}
}

void MainMenuPresentation::commitStagedMenuPlane() {
	_scene.loadLayerRows(MakeSpan(_currentRleData), TileLayer::Foreground, 0, 0,
						 kMenuPlaneColumns,
						 kMenuPlaneRows);
}

void MainMenuPresentation::updateSelectionCursor(int verticalOffset, Optional<uint16> attributes) {
	if (!attributes.hasValue()) {
		_scene.setDynamicSprites(Span<const SpriteInstance>());
		return;
	}

	uint16 packedAttributes = attributes.value();
	Common::Array<SpriteInstance> cursor(1);
	cursor[0] = SpriteInstance(
		kSelectionCursorX, kSelectionCursorBaseY + verticalOffset, kSelectionCursorSizeInTiles,
		kSelectionCursorSizeInTiles, packedAttributes & 0x07FF,
		static_cast<uint8>((packedAttributes >> 13) & 3), (packedAttributes & 0x0800) != 0,
		(packedAttributes & 0x1000) != 0, (packedAttributes & 0x8000) != 0);
	_scene.setDynamicSprites(MakeSpan(cursor));
}

void MainMenuPresentation::updatePasswordGridCursors(int gridColumn, int gridRow, int displayPosition,
													 int passwordRow) {
	// Ghidra RunEpisodeTwoPasswordMenu 0x000091E6-0x00009243: replace the terminal table writes with
	// two ordered logical cursors. Entry 12 keeps entry 13 behind it in the original link order.
	Common::Array<SpriteInstance> cursors(2);
	cursors[0] = SpriteInstance(kPasswordGridCursorBaseX + gridColumn * 8,
								kPasswordGridCursorBaseY + gridRow * 16, 1,
								kPasswordGridCursorHeightInTiles, kPasswordGridCursorTileIndex, 0,
								false,
								false, false);
	cursors[1] = SpriteInstance(kPasswordEntryCursorBaseX + displayPosition * 8,
								kPasswordEntryCursorBaseY + passwordRow * 8, 1, 1,
								kPasswordGridCursorTileIndex, 0, false, false, false);
	_scene.setDynamicSprites(MakeSpan(cursors));
}

void MainMenuPresentation::writeIndexedTileRow(const Common::String &text, int columnIndex, int rowIndex) {
	int characterCount = getCharacterCount(text);
	Common::Array<TileCell> cells(static_cast<std::size_t>(characterCount));
	for (int index = 0; index < static_cast<int>(cells.size()); index++) {
		uint16 packedAttributes = static_cast<uint16>(
			_textAttributes + static_cast<uint8>(text[static_cast<std::size_t>(index)]));
		cells[static_cast<std::size_t>(index)] =
			decodeTileCell(packedAttributes);
	}

	int sourceOffset = 0;
	while (sourceOffset < static_cast<int>(cells.size())) {
		int rowLength = MIN(static_cast<int>(cells.size()) - sourceOffset,
							kMenuPlaneColumns - columnIndex);
		Span<const TileCell> rowCells = MakeSpan(cells).slice(
			static_cast<std::size_t>(sourceOffset), static_cast<std::size_t>(rowLength));
		_scene.replaceLayerRow(rowCells, TileLayer::Foreground, columnIndex, rowIndex);
		sourceOffset += rowLength;
		columnIndex = 0;
		rowIndex++;
	}
}

void MainMenuPresentation::writeIndexedTile(uint8 character, int columnIndex, int rowIndex) {
	Common::Array<TileCell> cell(1);
	cell[0] = decodeTileCell(
		static_cast<uint16>(_textAttributes + character));
	_scene.replaceLayerRow(MakeSpan(cell), TileLayer::Foreground, columnIndex, rowIndex);
}

void MainMenuPresentation::writePriorityTileRow(const Common::String &text, int columnIndex, int rowIndex) {
	uint16 savedAttributes = _textAttributes;
	setTextPriority(true);
	writeIndexedTileRow(text, columnIndex, rowIndex);
	_textAttributes = savedAttributes;
}

void MainMenuPresentation::updateCatalogSelectionHighlight(int rowIndex) {
	Common::Array<SpriteInstance> sprites(kCatalogSelectionHighlightSegmentCount);
	for (int index = 0; index < static_cast<int>(sprites.size()); index++) {
		sprites[static_cast<std::size_t>(index)] = SpriteInstance(
			index * kCatalogSelectionHighlightSegmentWidth * 8, kCatalogSelectionHighlightBaseY + rowIndex * 8,
			kCatalogSelectionHighlightSegmentWidth, 1, kCatalogSelectionHighlightTileIndex, 0, false, false,
			false);
	}

	_scene.setSpriteVerticalOffset(0);
	_scene.replaceSprites(MakeSpan(sprites));
}

void MainMenuPresentation::restoreMainMenuSpriteLayout() {
	_scene.setSpriteVerticalOffset(0);
	_scene.loadSprites(MakeSpan(_menuLayout));
}

void MainMenuPresentation::writeCenteredTileRow(const Common::String &text, int rowIndex) {
	// Ghidra 0x00009F30-0x00009F39: preserve A0 and count bytes through the NUL terminator.
	int characterCount = getCharacterCount(text);

	// Ghidra 0x00009F3A-0x00009F45: compute floor((32 - length) / 2) and tail-call the row writer.
	writeIndexedTileRow(text, (kMenuPlaneColumns - characterCount) / 2, rowIndex);
}

int MainMenuPresentation::getCharacterCount(const Common::String &text) {
	uint32 terminatorIndex = text.find('\0');
	return terminatorIndex == Common::String::npos
			   ? static_cast<int>(text.size())
			   : static_cast<int>(terminatorIndex);
}

void MainMenuPresentation::stageInitialMenuAssets(int firstAdditionalTileIndex,
												  const Common::Array<Common::Array<uint8>> &
													  additionalTileChunks,
												  const Common::Array<uint8> &fixedTileBlock) {
	int nextTileIndex = firstAdditionalTileIndex;
	for (const Common::Array<uint8> &tileChunk : additionalTileChunks) {
		_scene.loadTilesAt(MakeSpan(tileChunk), nextTileIndex);
		nextTileIndex += static_cast<int>(tileChunk.size()) / kPackedBytesPerTile;
	}

	restoreBinaryMaskTiles();
	Common::Array<uint8> solidTiles(static_cast<std::size_t>(4 * kPackedBytesPerTile), 0xAA);
	_scene.loadTilesAt(MakeSpan(solidTiles), 0x7D0);
	_scene.loadLayerRows(MakeSpan(fixedTileBlock), TileLayer::Background, 0, 0, 32, 28);
	_scene.loadSprites(MakeSpan(_menuLayout));
}
} // namespace Scooby