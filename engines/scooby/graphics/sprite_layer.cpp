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

#include "sprite_layer.h"

// #include "common/algorithm.h"

#include "genesis_sprite_table_decoder.h"

namespace Scooby {

SpriteLayer::SpriteLayer()
	: _highPriority(
		  static_cast<std::size_t>(IndexedFrame::Width) * static_cast<std::size_t>(IndexedFrame::Height),
		  false),
	  _pixels(static_cast<std::size_t>(IndexedFrame::Width) * static_cast<std::size_t>(IndexedFrame::Height), 0) {
}

void SpriteLayer::clear() { _sprites.clear(); }

void SpriteLayer::load(Span<const uint8> packedEntries) {
	_sprites = decode(packedEntries);
}

void SpriteLayer::replace(Span<const SpriteInstance> sprites) { _sprites = sprites.toArray(); }

void SpriteLayer::compose(Span<const uint8> tilePixels, int contentWidth, int verticalOffset,
						  Span<const SpriteInstance> dynamicSprites) {
	Common::fill(_pixels.begin(), _pixels.end(), 0);
	Common::fill(_highPriority.begin(), _highPriority.end(), false);
	int outputLeft = (IndexedFrame::Width - contentWidth) / 2;
	for (std::size_t i = 0; i < _sprites.size(); ++i) {
		composeSprite(tilePixels, contentWidth, outputLeft, _sprites[i], verticalOffset);
	}

	for (std::size_t i = 0; i < dynamicSprites.size(); ++i) {
		composeSprite(tilePixels, contentWidth, outputLeft, dynamicSprites[i], 0);
	}
}

void SpriteLayer::drawPass(IndexedFrame &frame, bool highPriority) {
	for (std::size_t index = 0; index < _pixels.size(); ++index) {
		if (_pixels[index] != 0 && _highPriority[index] == highPriority) {
			frame._pixels[index] = _pixels[index];
		}
	}
}

void SpriteLayer::composeSprite(Span<const uint8> tilePixels, int contentWidth, int outputLeft,
								const SpriteInstance &sprite, int verticalOffset) {
	int width = sprite.widthInTiles * kTileSize;
	int height = sprite.heightInTiles * kTileSize;
	for (int pixelY = 0; pixelY < height; ++pixelY) {
		int targetY = sprite.y + verticalOffset + pixelY;
		if (static_cast<unsigned>(targetY) >= static_cast<unsigned>(IndexedFrame::Height)) {
			continue;
		}

		for (int pixelX = 0; pixelX < width; ++pixelX) {
			int logicalX = sprite.x + pixelX;
			if (static_cast<unsigned>(logicalX) >= static_cast<unsigned>(contentWidth)) {
				continue;
			}

			std::size_t targetOffset =
				static_cast<std::size_t>(targetY) * static_cast<std::size_t>(IndexedFrame::Width) +
				static_cast<std::size_t>(outputLeft) + static_cast<std::size_t>(logicalX);
			if (_pixels[targetOffset] != 0) {
				continue;
			}

			int sourceX = sprite.flipHorizontally ? width - 1 - pixelX : pixelX;
			int sourceY = sprite.flipVertically ? height - 1 - pixelY : pixelY;
			int tileIndex = sprite.tileIndex + (sourceX / kTileSize) * sprite.heightInTiles + sourceY / kTileSize;
			int tilePixelOffset =
				tileIndex * kPixelsPerTile + (sourceY % kTileSize) * kTileSize + sourceX % kTileSize;
			if (static_cast<unsigned>(tilePixelOffset) >= static_cast<unsigned>(tilePixels.size())) {
				continue;
			}

			uint8 color = tilePixels[static_cast<std::size_t>(tilePixelOffset)];
			if (color == 0) {
				continue;
			}

			_pixels[targetOffset] = static_cast<uint8>(sprite.paletteIndex * 16 + color);
			_highPriority[targetOffset] = sprite.highPriority;
		}
	}
}
} // namespace Scooby