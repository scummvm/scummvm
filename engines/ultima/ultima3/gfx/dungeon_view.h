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

#include "common/array.h"
#include "ultima/ultima3/data/dungeon.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

/**
 * The first person view down the corridors of a dungeon: the walls, doors,
 * ladders and chests in sight drawn as lines, which are all white in the
 * original and have colors in the upgrade's EGA and VGA renderers. The VGA
 * renderer can instead use pictures of them, which are layers of the view
 * that are drawn over each other
 */
class DungeonView {
private:
	// The picture is built a pixel at a time at the position it has on the
	// screen, which leaves the view with a margin on all sides
	static const int WIDTH = 256;
	static const int HEIGHT = 200;
	static const int OFFSET = 0x20;

	// The pictures are layers the size of the view, and 0 in them is clear
	static const int PICTURE_SIZE = 0x80;
	static const int FACES = 64;
	static const int PLACES = 21;

	struct Picture {
		bool _present;
		byte _pixels[PICTURE_SIZE * PICTURE_SIZE];
	};

	// Something that is drawn once all the walls have been, so that it's over them
	struct Thing {
		bool _chest;
		bool _down;
		int _place;
	};

	static Picture _faces[FACES];
	static Picture _ladders[PLACES];
	static Picture _chests[PLACES];
	static bool _loaded;
	static bool _enabled;

	byte _pixels[WIDTH * HEIGHT];
	int _facing = 0;
	Common::Array<Thing> _things;
	bool _usePictures = false;

	static bool loadPicture(Picture &picture, const Common::String &name);

	/**
	 * Forgets any pictures, which are kept in plain static arrays so that they don't
	 * need constructing
	 */
	static void resetPictures();

	void drawPicture(const Picture &picture, bool flip, bool erase);

	/**
	 * Draws everything in view that isn't a ladder or a chest
	 */
	void drawPlaces(const Data::Dungeon &dungeon, int level, int x, int y);

	enum Sight {
		OPEN,
		WALL,
		DOOR
	};

	void plot(int x, int y);
	void line(int x0, int y0, int x1, int y1);
	void column(int x, int top, int bottom, bool erase, byte color);
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
		resetPictures();
	}

	/**
	 * Reads the pictures that are drawn in place of lines, which any of can be missing
	 */
	static void loadPictures();

	/**
	 * Sets whether the pictures are drawn, if they've been found and it's the VGA renderer
	 */
	static void setPicturesEnabled(bool enabled) {
		_enabled = enabled;
	}

	/**
	 * Returns true if the view is made of pictures
	 */
	static bool usesPictures();

	/**
	 * Draws the view from a square in a direction, for later reading back
	 */
	void draw(const Data::Dungeon &dungeon, int level, int x, int y, int facing);

	/**
	 * Returns the color of a pixel of the picture, whose position is that on
	 * the screen, which is zero if it's not lit
	 */
	byte color(int x, int y) const {
		return _pixels[y * WIDTH + x];
	}
};

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima

#endif
