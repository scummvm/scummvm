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

#include "tile_scene.h"

#include "genesis_asset_decoder.h"
#include "scooby/common/contracts.h"
#include "common/algorithm.h"

namespace Scooby {

TileScene::TileScene()
	: _backgroundLayer(kDefaultLayerColumns * kDefaultLayerRows),
	  _foregroundLayer(kDefaultLayerColumns * kDefaultLayerRows),
	  _interfaceLayer(kDefaultLayerColumns * kDefaultLayerRows),
	  _windowLayer(kWindowLayerColumns * kWindowLayerRows), _palette(64),
	  _backgroundHorizontalOffset(0), _backgroundVerticalOffset(0),
	  _horizontalDisplayMode(HorizontalDisplayMode::H32),
	  _foregroundHorizontalOffset(0), _foregroundVerticalOffset(0), _layerColumns(kDefaultLayerColumns),
	  _layerRows(kDefaultLayerRows), _roomInterfaceVisible(false), _spriteVerticalOffset(0),
	  _spritesVisible(true),
	  _windowVerticalPositionBits(0), _interfaceHorizontalScroll(0) {
	Common::fill(_palette.begin(), _palette.end(), PaletteColor());
}

Common::Array<TileCell> &TileScene::layerFor(TileLayer layer) {
	switch (layer) {
	case TileLayer::Background:
		return _backgroundLayer;
	case TileLayer::Foreground:
		return _foregroundLayer;
	case TileLayer::Interface:
		return _interfaceLayer;
	}
	SDM_UNREACHABLE("Unhandled tile layer.");
	return _foregroundLayer; // unreachable, silences -Wreturn-type
}

void TileScene::reset(HorizontalDisplayMode horizontalDisplayMode) {
	clearContent();
	Common::fill(_palette.begin(), _palette.end(), PaletteColor());
	_horizontalDisplayMode = horizontalDisplayMode;
}

void TileScene::clearContent() {
	_layerColumns = kDefaultLayerColumns;
	_layerRows = kDefaultLayerRows;
	_backgroundLayer = Common::Array<TileCell>(
		static_cast<std::size_t>(kDefaultLayerColumns) * static_cast<std::size_t>(kDefaultLayerRows),
		TileCell());
	_foregroundLayer = Common::Array<TileCell>(
		static_cast<std::size_t>(kDefaultLayerColumns) * static_cast<std::size_t>(kDefaultLayerRows),
		TileCell());
	_interfaceLayer = Common::Array<TileCell>(
		static_cast<std::size_t>(kDefaultLayerColumns) * static_cast<std::size_t>(kDefaultLayerRows),
		TileCell());
	_windowLayer = Common::Array<TileCell>(
		static_cast<std::size_t>(kWindowLayerColumns) * static_cast<std::size_t>(kWindowLayerRows),
		TileCell());
	_spriteLayer.clear();
	_dynamicSprites.clear();
	_tilePixels.clear();
	_backgroundHorizontalOffset = 0;
	_backgroundLineHorizontalOffsets.clear();
	_backgroundVerticalOffset = 0;
	_foregroundHorizontalOffset = 0;
	_foregroundLineHorizontalOffsets.clear();
	_foregroundVerticalOffset = 0;
	_roomInterfaceVisible = false;
	_spriteVerticalOffset = 0;
	_spritesVisible = true;
	_windowVerticalPositionBits = 0;
}

void TileScene::loadTiles(Span<const uint8> packedTiles) {
	_tilePixels = decodeTiles(packedTiles);
}

void TileScene::loadTilesAt(Span<const uint8> packedTiles, int startTileIndex) {
	Common::Array<uint8> pixels = decodeTiles(packedTiles);
	std::size_t destinationOffset = static_cast<std::size_t>(startTileIndex) * static_cast<std::size_t>(
																				   kPixelsPerTile);
	std::size_t requiredLength = destinationOffset + pixels.size();
	if (_tilePixels.size() < requiredLength) {
		_tilePixels.resize(requiredLength);
	}

	Common::copy(pixels.begin(), pixels.end(), _tilePixels.begin() + static_cast<long>(destinationOffset));
}

void TileScene::overwritePackedPatternBytes(Span<const uint8> packedTileBytes,
											int destinationByteOffset) {
	std::size_t destinationOffset = static_cast<std::size_t>(destinationByteOffset) * 2;
	std::size_t requiredLength = destinationOffset + packedTileBytes.size() * 2;
	if (_tilePixels.size() < requiredLength) {
		_tilePixels.resize(requiredLength);
	}

	for (std::size_t index = 0; index < packedTileBytes.size(); ++index) {
		uint8 packedByte = packedTileBytes[index];
		std::size_t pixelOffset = destinationOffset + index * 2;
		_tilePixels[pixelOffset] = static_cast<uint8>(packedByte >> 4);
		_tilePixels[pixelOffset + 1] = static_cast<uint8>(packedByte & 0x0F);
	}
}

TileScene::ContentReplacementGuard::ContentReplacementGuard(TileScene *scene)
	: _scene(scene), _backgroundLayer(scene->_backgroundLayer),
	  _backgroundHorizontalOffset(scene->_backgroundHorizontalOffset),
	  _backgroundLineHorizontalOffsets(scene->_backgroundLineHorizontalOffsets),
	  _backgroundVerticalOffset(scene->_backgroundVerticalOffset),
	  _horizontalDisplayMode(scene->_horizontalDisplayMode),
	  _dynamicSprites(scene->_dynamicSprites), _foregroundLayer(scene->_foregroundLayer),
	  _foregroundHorizontalOffset(scene->_foregroundHorizontalOffset),
	  _foregroundLineHorizontalOffsets(scene->_foregroundLineHorizontalOffsets),
	  _foregroundVerticalOffset(scene->_foregroundVerticalOffset), _interfaceLayer(scene->_interfaceLayer),
	  _layerColumns(scene->_layerColumns), _layerRows(scene->_layerRows),
	  _roomInterfaceVisible(scene->_roomInterfaceVisible),
	  _spriteVerticalOffset(scene->_spriteVerticalOffset), _spritesVisible(scene->_spritesVisible),
	  _tilePixels(scene->_tilePixels), _windowLayer(scene->_windowLayer),
	  _windowVerticalPositionBits(scene->_windowVerticalPositionBits),
	  _interfaceHorizontalScroll(scene->_interfaceHorizontalScroll) {
	scene->_spritesVisible = false;
}

TileScene::ContentReplacementGuard::ContentReplacementGuard(ContentReplacementGuard &&other) noexcept
	: _scene(other._scene), _backgroundLayer(std::move(other._backgroundLayer)),
	  _backgroundHorizontalOffset(other._backgroundHorizontalOffset),
	  _backgroundLineHorizontalOffsets(std::move(other._backgroundLineHorizontalOffsets)),
	  _backgroundVerticalOffset(other._backgroundVerticalOffset),
	  _horizontalDisplayMode(other._horizontalDisplayMode),
	  _dynamicSprites(std::move(other._dynamicSprites)), _foregroundLayer(std::move(other._foregroundLayer)),
	  _foregroundHorizontalOffset(other._foregroundHorizontalOffset),
	  _foregroundLineHorizontalOffsets(std::move(other._foregroundLineHorizontalOffsets)),
	  _foregroundVerticalOffset(other._foregroundVerticalOffset),
	  _interfaceLayer(std::move(other._interfaceLayer)),
	  _layerColumns(other._layerColumns), _layerRows(other._layerRows),
	  _roomInterfaceVisible(other._roomInterfaceVisible),
	  _spriteVerticalOffset(other._spriteVerticalOffset), _spritesVisible(other._spritesVisible),
	  _tilePixels(std::move(other._tilePixels)), _windowLayer(std::move(other._windowLayer)),
	  _windowVerticalPositionBits(other._windowVerticalPositionBits),
	  _interfaceHorizontalScroll(other._interfaceHorizontalScroll) {
	other._scene = nullptr;
}

TileScene::ContentReplacementGuard &TileScene::ContentReplacementGuard::operator=(
	ContentReplacementGuard &&other) noexcept {
	if (this != &other) {
		_scene = other._scene;
		_backgroundLayer = std::move(other._backgroundLayer);
		_backgroundHorizontalOffset = other._backgroundHorizontalOffset;
		_backgroundLineHorizontalOffsets = std::move(other._backgroundLineHorizontalOffsets);
		_backgroundVerticalOffset = other._backgroundVerticalOffset;
		_horizontalDisplayMode = other._horizontalDisplayMode;
		_dynamicSprites = std::move(other._dynamicSprites);
		_foregroundLayer = std::move(other._foregroundLayer);
		_foregroundHorizontalOffset = other._foregroundHorizontalOffset;
		_foregroundLineHorizontalOffsets = std::move(other._foregroundLineHorizontalOffsets);
		_foregroundVerticalOffset = other._foregroundVerticalOffset;
		_interfaceLayer = std::move(other._interfaceLayer);
		_layerColumns = other._layerColumns;
		_layerRows = other._layerRows;
		_roomInterfaceVisible = other._roomInterfaceVisible;
		_spriteVerticalOffset = other._spriteVerticalOffset;
		_spritesVisible = other._spritesVisible;
		_tilePixels = std::move(other._tilePixels);
		_windowLayer = std::move(other._windowLayer);
		_windowVerticalPositionBits = other._windowVerticalPositionBits;
		_interfaceHorizontalScroll = other._interfaceHorizontalScroll;
		other._scene = nullptr;
	}
	return *this;
}

TileScene::ContentReplacementGuard::~ContentReplacementGuard() {
	if (_scene == nullptr) {
		return;
	}

	_scene->_backgroundLayer = _backgroundLayer;
	_scene->_backgroundHorizontalOffset = _backgroundHorizontalOffset;
	_scene->_backgroundLineHorizontalOffsets = _backgroundLineHorizontalOffsets;
	_scene->_backgroundVerticalOffset = _backgroundVerticalOffset;
	_scene->_horizontalDisplayMode = _horizontalDisplayMode;
	_scene->_dynamicSprites = _dynamicSprites;
	_scene->_foregroundLayer = _foregroundLayer;
	_scene->_foregroundHorizontalOffset = _foregroundHorizontalOffset;
	_scene->_foregroundLineHorizontalOffsets = _foregroundLineHorizontalOffsets;
	_scene->_foregroundVerticalOffset = _foregroundVerticalOffset;
	_scene->_interfaceLayer = _interfaceLayer;
	_scene->_layerColumns = _layerColumns;
	_scene->_layerRows = _layerRows;
	_scene->_roomInterfaceVisible = _roomInterfaceVisible;
	_scene->_spriteVerticalOffset = _spriteVerticalOffset;
	_scene->_spritesVisible = _spritesVisible;
	_scene->_tilePixels = _tilePixels;
	_scene->_windowLayer = _windowLayer;
	_scene->_windowVerticalPositionBits = _windowVerticalPositionBits;
	_scene->_interfaceHorizontalScroll = _interfaceHorizontalScroll;
	_scene = nullptr;
}

TileScene::ContentReplacementGuard TileScene::beginTemporaryContentReplacement() {
	return ContentReplacementGuard(this);
}

void TileScene::loadSprites(Span<const uint8> packedEntries) {
	_spriteLayer.load(packedEntries);
	_dynamicSprites.clear();
}

void TileScene::replaceSprites(Span<const SpriteInstance> sprites) {
	_spriteLayer.replace(sprites);
	_dynamicSprites.clear();
}

void TileScene::setDynamicSprites(Span<const SpriteInstance> sprites) {
	_dynamicSprites = sprites.toArray();
}

void TileScene::setSpriteVerticalOffset(int offset) { _spriteVerticalOffset = offset; }

void TileScene::loadLayerRows(Span<const uint8> packedCells, TileLayer layer, int startColumn,
							  int startRow, int columnCount, int rowCount) {
	Common::Array<TileCell> &destination = layerFor(layer);
	int destinationColumns = layer == TileLayer::Interface ? kDefaultLayerColumns : _layerColumns;
	_roomInterfaceVisible = _roomInterfaceVisible || layer == TileLayer::Interface;
	for (int row = 0; row < rowCount; ++row) {
		for (int column = 0; column < columnCount; ++column) {
			int sourceOffset = (row * columnCount + column) * static_cast<int>(sizeof(uint16));
			std::size_t destinationIndex =
				static_cast<std::size_t>((startRow + row) * destinationColumns + startColumn + column);
			destination[destinationIndex] = decodeTileCell(packedCells, sourceOffset);
		}
	}
}

void TileScene::fillLayer(const TileCell &cell, TileLayer layer) {
	Common::Array<TileCell> &destination = layerFor(layer);
	_roomInterfaceVisible = _roomInterfaceVisible || layer == TileLayer::Interface;
	Common::fill(destination.begin(), destination.end(), cell);
}

void TileScene::fillWindow(const TileCell &cell) { Common::fill(_windowLayer.begin(), _windowLayer.end(), cell); }

void TileScene::setWindowCell(const TileCell &cell, int column, int row) {
	_windowLayer[static_cast<std::size_t>(row * kWindowLayerColumns + column)] = cell;
}

void TileScene::setWindowVerticalPosition(uint8 positionBits) {
	_windowVerticalPositionBits = positionBits;
}

void TileScene::copyPackedLayerRows(Span<uint8> destination, TileLayer layer, int startColumn,
									int startRow, int columnCount, int rowCount) {
	Common::Array<TileCell> &source = layerFor(layer);
	int sourceColumns = layer == TileLayer::Interface ? kDefaultLayerColumns : _layerColumns;
	for (int row = 0; row < rowCount; ++row) {
		for (int column = 0; column < columnCount; ++column) {
			uint16 packedCell = encodeTileCell(
				source[static_cast<std::size_t>((startRow + row) * sourceColumns + startColumn + column)]);
			std::size_t destinationOffset =
				static_cast<std::size_t>((row * columnCount + column) * static_cast<int>(sizeof(
																			uint16)));
			destination[destinationOffset] = static_cast<uint8>(packedCell >> 8);
			destination[destinationOffset + 1] = static_cast<uint8>(packedCell);
		}
	}
}

void TileScene::fillLayerRows(const TileCell &cell, TileLayer layer, int startColumn, int startRow,
							  int columnCount,
							  int rowCount) {
	Common::Array<TileCell> &destination = layerFor(layer);
	int destinationColumns = layer == TileLayer::Interface ? kDefaultLayerColumns : _layerColumns;
	_roomInterfaceVisible = _roomInterfaceVisible || layer == TileLayer::Interface;
	for (int row = 0; row < rowCount; ++row) {
		std::size_t rowStart = static_cast<std::size_t>((startRow + row) * destinationColumns + startColumn);
		Common::fill(destination.begin() + static_cast<long>(rowStart),
				  destination.begin() + static_cast<long>(rowStart) + columnCount, cell);
	}
}

void TileScene::replaceCompleteLayers(Span<const uint8> backgroundCells,
									  Span<const uint8> foregroundCells, int columnCount,
									  int rowCount) {
	_layerColumns = columnCount;
	_layerRows = rowCount;
	_backgroundLayer = decodeLayer(backgroundCells, columnCount, rowCount);
	_foregroundLayer = decodeLayer(foregroundCells, columnCount, rowCount);
	_interfaceLayer = Common::Array<TileCell>(
		static_cast<std::size_t>(kDefaultLayerColumns) * static_cast<std::size_t>(kDefaultLayerRows),
		TileCell());
	_windowLayer = Common::Array<TileCell>(
		static_cast<std::size_t>(kWindowLayerColumns) * static_cast<std::size_t>(kWindowLayerRows),
		TileCell());
	_backgroundHorizontalOffset = 0;
	_backgroundLineHorizontalOffsets.clear();
	_foregroundHorizontalOffset = 0;
	_foregroundLineHorizontalOffsets.clear();
	_roomInterfaceVisible = false;
	_windowVerticalPositionBits = 0;
}

void TileScene::replaceLayerRow(Span<const TileCell> cells, TileLayer layer, int startColumn, int row) {
	Common::Array<TileCell> &destination = layerFor(layer);
	int destinationColumns = layer == TileLayer::Interface ? kDefaultLayerColumns : _layerColumns;
	_roomInterfaceVisible = _roomInterfaceVisible || layer == TileLayer::Interface;
	std::size_t start = static_cast<std::size_t>(row * destinationColumns + startColumn);
	for (std::size_t i = 0; i < cells.size(); ++i) {
		destination[start + i] = cells[i];
	}
}

void TileScene::replacePaletteRange(Span<const PaletteColor> colors, int startIndex) {
	for (std::size_t i = 0; i < colors.size(); ++i) {
		_palette[static_cast<std::size_t>(startIndex) + i] = colors[i];
	}
}

void TileScene::rotatePaletteRangeLeft(int startIndex, int colorCount) {
	std::size_t start = static_cast<std::size_t>(startIndex);
	PaletteColor first = _palette[start];
	for (int i = 0; i < colorCount - 1; ++i) {
		_palette[start + static_cast<std::size_t>(i)] = _palette[start + static_cast<std::size_t>(i) + 1];
	}
	_palette[start + static_cast<std::size_t>(colorCount) - 1] = first;
}

void TileScene::advancePaletteToward(Span<const PaletteColor> target) {
	for (std::size_t index = 0; index < _palette.size(); ++index) {
		PaletteColor current = _palette[index];
		PaletteColor desired = target[index];
		_palette[index] = PaletteColor(stepLevel(current.redLevel, desired.redLevel),
									   stepLevel(current.greenLevel, desired.greenLevel),
									   stepLevel(current.blueLevel, desired.blueLevel));
	}
}

void TileScene::advancePaletteTowardBlack() {
	for (std::size_t index = 0; index < _palette.size(); ++index) {
		PaletteColor current = _palette[index];
		_palette[index] =
			PaletteColor(stepLevel(current.redLevel, 0), stepLevel(current.greenLevel, 0),
						 stepLevel(current.blueLevel, 0));
	}
}

void TileScene::rotateFirstLogoRamp() {
	PaletteColor last = _palette[10];
	for (int index = 10; index > 3; --index) {
		_palette[static_cast<std::size_t>(index)] = _palette[static_cast<std::size_t>(index) - 1];
	}
	_palette[3] = last;
}

void TileScene::setHorizontalOffsets(int foreground, int background) {
	_foregroundHorizontalOffset = foreground;
	_backgroundHorizontalOffset = background;
	_foregroundLineHorizontalOffsets.clear();
	_backgroundLineHorizontalOffsets.clear();
}

void TileScene::setLineHorizontalOffsets(Span<const int16> foreground,
										 Span<const int16> background) {
	_foregroundLineHorizontalOffsets = foreground.toArray();
	_backgroundLineHorizontalOffsets = background.toArray();
}

void TileScene::setVerticalOffsets(int foreground, int background) {
	_foregroundVerticalOffset = foreground;
	_backgroundVerticalOffset = background;
}

void TileScene::render(IndexedFrame &frame) {
	Common::fill(frame._pixels.begin(), frame._pixels.end(), static_cast<uint8>(0));
	Common::copy(_palette.begin(), _palette.end(), frame._palette.begin());
	if (_spritesVisible) {
		_spriteLayer.compose(MakeSpan(_tilePixels), contentWidth(), _spriteVerticalOffset,
							 MakeSpan(_dynamicSprites));
	}

	int roomBottom = _roomInterfaceVisible ? kRoomInterfaceSplitY : IndexedFrame::Height;
	drawLayer(frame, _backgroundLayer, _layerColumns, _layerRows, _backgroundHorizontalOffset,
			  MakeSpan(_backgroundLineHorizontalOffsets), _backgroundVerticalOffset, false, 0,
			  roomBottom);
	drawForegroundAndWindow(frame, false, roomBottom);
	if (_roomInterfaceVisible) {
		drawLayer(frame, _interfaceLayer, kDefaultLayerColumns, kDefaultLayerRows, _interfaceHorizontalScroll,
				  Span<const int16>(),
				  0, false, kRoomInterfaceSplitY, IndexedFrame::Height);
	}

	if (_spritesVisible) {
		_spriteLayer.drawPass(frame, false);
	}

	drawLayer(frame, _backgroundLayer, _layerColumns, _layerRows, _backgroundHorizontalOffset,
			  MakeSpan(_backgroundLineHorizontalOffsets), _backgroundVerticalOffset, true, 0,
			  roomBottom);
	drawForegroundAndWindow(frame, true, roomBottom);
	if (_roomInterfaceVisible) {
		drawLayer(frame, _interfaceLayer, kDefaultLayerColumns, kDefaultLayerRows, _interfaceHorizontalScroll,
				  Span<const int16>(),
				  0, true, kRoomInterfaceSplitY, IndexedFrame::Height);
	}

	if (_spritesVisible) {
		_spriteLayer.drawPass(frame, true);
	}
}

int TileScene::mod(int value, int modulus) {
	int result = value % modulus;
	return result < 0 ? result + modulus : result;
}

uint8 TileScene::stepLevel(uint8 current, uint8 target) {
	if (current == target) {
		return current;
	}
	int diff = static_cast<int>(target) - static_cast<int>(current);
	int sign = (diff > 0) - (diff < 0);
	return static_cast<uint8>(static_cast<int>(current) + sign);
}

int TileScene::contentWidth() const {
	switch (_horizontalDisplayMode) {
	case HorizontalDisplayMode::H32:
		return 32 * kTileSize;
	case HorizontalDisplayMode::H40:
		return IndexedFrame::Width;
	}
	SDM_UNREACHABLE("Unhandled horizontal display mode.");
	return IndexedFrame::Width;
}

Common::Array<TileCell> TileScene::decodeLayer(Span<const uint8> packedCells, int columnCount,
											   int rowCount) {
	Common::Array<TileCell> cells(static_cast<std::size_t>(columnCount) * static_cast<std::size_t>(rowCount));
	for (std::size_t index = 0; index < cells.size(); ++index) {
		cells[index] = decodeTileCell(packedCells,
									  static_cast<int>(index) * static_cast<int>(sizeof(
																	uint16)));
	}
	return cells;
}

void TileScene::drawForegroundAndWindow(IndexedFrame &frame, bool highPriority, int roomBottom) {
	int windowBoundary = MIN((_windowVerticalPositionBits & 0x1F) * kTileSize, roomBottom);
	if ((_windowVerticalPositionBits & 0x80) == 0) {
		drawLayer(frame, _windowLayer, kWindowLayerColumns, kWindowLayerRows, 0,
				  Span<const int16>(), 0,
				  highPriority, 0, windowBoundary);
		drawLayer(frame, _foregroundLayer, _layerColumns, _layerRows, _foregroundHorizontalOffset,
				  MakeSpan(_foregroundLineHorizontalOffsets), _foregroundVerticalOffset, highPriority,
				  windowBoundary, roomBottom);
		return;
	}

	drawLayer(frame, _foregroundLayer, _layerColumns, _layerRows, _foregroundHorizontalOffset,
			  MakeSpan(_foregroundLineHorizontalOffsets), _foregroundVerticalOffset, highPriority, 0,
			  windowBoundary);
	drawLayer(frame, _windowLayer, kWindowLayerColumns, kWindowLayerRows, 0, Span<const int16>(),
			  0,
			  highPriority, windowBoundary, roomBottom);
}

void TileScene::drawLayer(IndexedFrame &frame, const Common::Array<TileCell> &layer, int columnCount,
						  int rowCount,
						  int horizontalOffset, Span<const int16> lineHorizontalOffsets,
						  int verticalOffset, bool highPriority, int startY, int endY) {
	int contentWidth = this->contentWidth();
	int outputLeft = (IndexedFrame::Width - contentWidth) / 2;
	int layerWidth = columnCount * kTileSize;
	int layerHeight = rowCount * kTileSize;
	for (int y = startY; y < endY; ++y) {
		int sourceY = mod(y - verticalOffset, layerHeight);
		int cellY = sourceY / kTileSize;
		int pixelY = sourceY % kTileSize;
		int lineHorizontalOffset = lineHorizontalOffsets.isEmpty()
									   ? horizontalOffset
									   : lineHorizontalOffsets[static_cast<std::size_t>(y)];
		for (int x = 0; x < contentWidth; ++x) {
			int sourceX = mod(x - lineHorizontalOffset, layerWidth);
			int cellX = sourceX / kTileSize;
			const TileCell &cell = layer[static_cast<std::size_t>(cellY * columnCount + cellX)];
			if (cell.highPriority != highPriority) {
				continue;
			}

			int pixelX = sourceX % kTileSize;
			int tilePixelY = pixelY;
			if (cell.flipHorizontally) {
				pixelX = kTileSize - 1 - pixelX;
			}

			if (cell.flipVertically) {
				tilePixelY = kTileSize - 1 - tilePixelY;
			}

			int tilePixelOffset = cell.tileIndex * kPixelsPerTile + tilePixelY * kTileSize + pixelX;
			if (static_cast<unsigned>(tilePixelOffset) >= static_cast<unsigned>(_tilePixels.size())) {
				continue;
			}

			uint8 color = _tilePixels[static_cast<std::size_t>(tilePixelOffset)];
			if (color != 0) {
				frame._pixels[static_cast<std::size_t>(y * IndexedFrame::Width + outputLeft + x)] =
					static_cast<uint8>(cell.paletteIndex * 16 + color);
			}
		}
	}
}
} // namespace Scooby