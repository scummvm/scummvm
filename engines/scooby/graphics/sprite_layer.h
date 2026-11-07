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

#ifndef SCOOBY_SPRITE_LAYER_H
#define SCOOBY_SPRITE_LAYER_H

#include "common/scummsys.h"

#include "common/array.h"

#include "indexed_frame.h"
#include "scooby/common/span.h"
#include "sprite_instance.h"

// Owns linked sprite state and resolves sprite overlap before the scene
// applies plane priority.

namespace Scooby {

class SpriteLayer {
public:
	SpriteLayer();

	// Removes every sprite from the logical scene.
	void clear();

	// Replaces the linked sprites from one packed attribute table.
	void load(Span<const uint8> packedEntries);

	// Replaces the linked sprite result with backend-neutral sprite instances.
	void replace(Span<const SpriteInstance> sprites);

	// Resolves the highest-priority nontransparent sprite pixel at every
	// output position.
	void compose(Span<const uint8> tilePixels, int contentWidth, int verticalOffset,
				 Span<const SpriteInstance> dynamicSprites);

	// Composites sprite pixels from one side of the plane-priority boundary.
	void drawPass(IndexedFrame &frame, bool highPriority);

private:
	static const int kPixelsPerTile = 64;
	static const int kTileSize = 8;

	void composeSprite(Span<const uint8> tilePixels, int contentWidth, int outputLeft,
					   const SpriteInstance &sprite, int verticalOffset);

	Common::Array<bool> _highPriority;
	Common::Array<uint8> _pixels;
	Common::Array<SpriteInstance> _sprites;
};
} // namespace Scooby

#endif // SCOOBY_SPRITE_LAYER_H
