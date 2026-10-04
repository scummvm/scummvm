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

#ifndef ULTIMA3_GFX_SHAPES_H
#define ULTIMA3_GFX_SHAPES_H

#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

constexpr int SHAPE_COUNT = 80;
constexpr int SHAPE_SIZE = 16;
constexpr int SHAPE_BYTES = 64;

/**
 * The map tile graphics from SHAPES.ULT: 80 tiles of 16x16 pixels at 2 bits
 * per pixel. Each tile holds its even scanlines first, then its odd ones.
 * The animated tiles are changed in place as the animation ticks.
 */
class Shapes {
private:
	byte _data[SHAPE_COUNT * SHAPE_BYTES] = {};

	// Ticks left before each animation step is next applied
	byte _scrollCounters[4];
	byte _flickerCounters[3];
	byte _swapCounter;

	byte *tile(int index) {
		return &_data[index * SHAPE_BYTES];
	}

	/**
	 * Moves every scanline of a tile down one pixel, wrapping the last to the top
	 */
	void scrollTile(int index);

	/**
	 * Swaps two scanlines of a tile
	 */
	void swapLines(int index, int line1, int line2);

	/**
	 * Swaps a run of whole tiles with another
	 */
	void swapTiles(int index1, int index2, int count);

public:
	/**
	 * Loads the graphics, restarting the animation
	 */
	void load();

	/**
	 * Advances the tile animations by one tick
	 */
	void animate();

	/**
	 * Draws a tile, overwriting all 256 pixels. The caller is responsible
	 * for flagging the changed area as dirty
	 */
	void drawTile(Graphics::ManagedSurface &dest, int x, int y, int index) const;
};

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima

#endif
