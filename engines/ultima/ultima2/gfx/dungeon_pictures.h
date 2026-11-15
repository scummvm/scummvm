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

#ifndef ULTIMA2_GFX_DUNGEON_PICTURES_H
#define ULTIMA2_GFX_DUNGEON_PICTURES_H

#include "graphics/palette.h"
#include "graphics/views/gfx_surface.h"
#include "ultima/ultima2/data/data.h"

namespace Ultima {
namespace Ultima2 {
namespace Gfx {

/**
 * The pictures that the EGA renderers can use for the corridors of the dungeons,
 * in place of lines, and for the monsters in them. Each is a layer the size of the
 * corridor, with 0 as clear. They're drawn with a standard set of colors that
 * fills the part of the palette that the game doesn't use
 */
class DungeonPictures {
public:
	enum Kind {
		FRONT, LEFT, RIGHT, LEFT_OPEN, RIGHT_OPEN, FRONT_DOOR, LEFT_DOOR, RIGHT_DOOR,
		LADDER_UP, LADDER_DOWN, CHEST, KIND_COUNT
	};

	static constexpr int DEPTHS = 8;
	static constexpr int MONSTER_TYPES = 8;

	/**
	 * The first of the palette entries of the standard colors, which run to the end
	 */
	static constexpr int PALETTE_BASE = 48;

private:
	// Plain data, so that the static pictures need no constructors
	struct Picture {
		int16 _left, _top, _width, _height;
		byte *_pixels;

		bool isPresent() const {
			return _pixels != nullptr;
		}
	};

	static Picture _pictures[KIND_COUNT][DEPTHS];
	static Picture _doorway;
	static Picture _monsters[MONSTER_TYPES][DEPTHS];
	static bool _enabled;
	static bool _loaded;
	static bool _paletteLoaded;
	static byte _inverse[256];

	static void freePictures();
	static bool loadPicture(Picture &picture, const Common::String &name);
	static void draw(Graphics::Views::GfxSurface &s, const Picture &picture, int offset);

public:
	/**
	 * Sets whether the pictures are used, if they've been found and it's one of the EGA renderers
	 */
	static void setEnabled(bool enabled) {
		_enabled = enabled;
	}

	/**
	 * Returns true if the corridors are drawn from pictures
	 */
	static bool isActive();

	/**
	 * Reads the pictures, any of which can be missing
	 */
	static void load();

	/**
	 * Releases the pictures
	 */
	static void unload();

	/**
	 * Adds the standard colors to a palette, if they're in the game's data, and works
	 * out the colors that are the opposite of them
	 */
	static void loadPalette(Graphics::Palette &palette);

	/**
	 * Draws a picture of a part of the corridor
	 * @returns		False if there's no picture of it
	 */
	static bool draw(Graphics::Views::GfxSurface &s, Kind kind, int depth, int offset);

	/**
	 * Draws the wall seen from a doorway that the party is in
	 */
	static bool drawDoorway(Graphics::Views::GfxSurface &s, int offset);

	/**
	 * Draws a picture of a monster, which has a number from 1
	 * @returns		False if there's no picture of it
	 */
	static bool drawMonster(Graphics::Views::GfxSurface &s, int type, int depth, int offset);

	/**
	 * Returns the color that a pixel becomes when the view is inverted, as when the party is hit
	 */
	static byte invert(byte color);
};

} // namespace Gfx
} // namespace Ultima2
} // namespace Ultima

#endif
