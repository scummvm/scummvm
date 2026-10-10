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

#ifndef ULTIMA3_GFX_DUNGEON_VIEW_H
#define ULTIMA3_GFX_DUNGEON_VIEW_H

#include "ultima/ultima3/data/dungeon.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

/**
 * The first person view down the corridors of a dungeon: the walls, doors,
 * ladders and chests in sight drawn as white lines
 */
class DungeonView {
private:
	// The picture is built a pixel at a time at the position it has on the
	// screen, which leaves the view with a margin on all sides
	static const int WIDTH = 256;
	static const int HEIGHT = 200;
	static const int OFFSET = 0x20;

	byte _pixels[WIDTH * HEIGHT];
	int _facing = 0;

	enum Sight {
		OPEN,
		WALL,
		DOOR
	};

	void plot(int x, int y);
	void line(int x0, int y0, int x1, int y1);
	void column(int x, int top, int bottom, bool erase);
	void drawWall(int face);
	void drawLadder(bool down, int place);
	void drawChest(int place);

	/**
	 * Draws whatever is at a place in view, and says what kind of obstacle it is
	 */
	Sight look(const Data::Dungeon &dungeon, int level, int x, int y, int place);

	/**
	 * Looks at a series of places, going no further down the series once
	 * something blocks the way
	 */
	void lookAlong(const Data::Dungeon &dungeon, int level, int x, int y, const byte *places, int count);

public:
	DungeonView() {
		memset(_pixels, 0, sizeof(_pixels));
	}

	/**
	 * Draws the view from a square in a direction, for later reading back
	 */
	void draw(const Data::Dungeon &dungeon, int level, int x, int y, int facing);

	/**
	 * Returns true if a pixel of the picture, whose position is that on the
	 * screen, is lit
	 */
	bool isLit(int x, int y) const {
		return _pixels[y * WIDTH + x] != 0;
	}
};

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima

#endif
