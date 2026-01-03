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

#ifndef ULTIMA0_GFX_MAP_H
#define ULTIMA0_GFX_MAP_H

#include "graphics/managed_surface.h"
#include "ultima/ultima0/data/data.h"

namespace Ultima {
namespace Ultima0 {
namespace Gfx {

class Dungeon {
private:
	// Slanted drawing constants
	static int _xLeft, _xRight, _yBottom, _yDiffLeft, _yDiffRight;

	/**
	 * Calculate display rectangle
	 */
	static void calcRect(Graphics::ManagedSurface *s, Common::Rect *r, double _level);

	/**
	 * Rotate a direction left
	 */
	static void rotateLeft(Common::Point *Dir);

	/**
	 * Draw part of dungeon
	 */
	static void drawDungeon(Graphics::ManagedSurface *s, Common::Rect *rOut,
		Common::Rect *rIn, int Left, int Centre, int Right, int Room, int Monster);

	/**
	 * Set the oblique drawing routine
	 */
	static void setRange(int x1, int x2, int y, int yd1, int yd2);

	/**
	 * Draw wall object using current setting
	 */
	static void drawWall(Graphics::ManagedSurface *s, int n);

	/**
	 * Convert coordinates from oblique to logical
	 */
	static void drawConvert(int *px, int *py);

	/**
	 * Draw the pits/ladder hole
	 */
	static void _DRAWPit(Graphics::ManagedSurface *s, Common::Rect *r, int Dir);

public:
	static void draw(Graphics::ManagedSurface *s);
};

} // namespace Gfx
} // namespace Ultima0
} // namespace Ultima

#endif
