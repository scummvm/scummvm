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

#ifndef ULTIMA1_GFX_DUNGEON_PICTURES_H
#define ULTIMA1_GFX_DUNGEON_PICTURES_H

#include "common/str.h"
#include "graphics/palette.h"
#include "graphics/views/gfx_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace Gfx {

/**
 * The pictures that the EGA renderer can use for the dungeons, in place of lines.
 * Each is a layer the size of the view of the dungeon, with 0 as clear. They're
 * drawn with a standard set of colors that fills the part of the palette that the
 * game doesn't use
 */
class DungeonPictures {
public:
	/**
	 * The parts of the corridor, which are drawn at one of the planes of
	 * its perspective, numbered from the party outwards
	 */
	enum Kind {
		FRONT, LEFT, RIGHT, LEFT_OPEN, RIGHT_OPEN, LEFT_EDGE, RIGHT_EDGE, FRONT_DOOR, LEFT_DOOR,
		RIGHT_DOOR, DOORWAY, BEAMS, KIND_COUNT
	};

	/**
	 * Things in the corridor, which are drawn at one of the sizes of the
	 * game's own shapes
	 */
	enum Widget {
		LADDER_UP_HOLE, LADDER_DOWN_HOLE, LADDER_UP_FACE_ON, LADDER_DOWN_FACE_ON,
		LADDER_UP_SIDEWAYS, LADDER_DOWN_SIDEWAYS, CHEST, COFFIN, WIDGET_COUNT
	};

	static constexpr int DEPTHS = 6;
	static constexpr int TIERS = 5;
	static constexpr int MONSTER_TYPES = 25;
	static constexpr int MONSTER_DISTANCES = 5;

	/**
	 * The first of the palette entries of the standard colors, which run to the end
	 */
	static constexpr int PALETTE_BASE = 48;

	/**
	 * Where the pictures are, in the screen
	 */
	static constexpr int VIEW_LEFT = 16;
	static constexpr int VIEW_TOP = 8;

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
	static Picture _widgets[WIDGET_COUNT][TIERS];
	static Picture _monsters[MONSTER_TYPES][MONSTER_DISTANCES];
	static bool _enabled;
	static bool _loaded;
	static bool _paletteLoaded;

	static void freePictures();
	static bool loadPicture(Picture &picture, const Common::String &name);
	static void draw(Graphics::Views::GfxSurface &s, const Picture &picture);

public:
	/**
	 * Sets whether the pictures are used, if they've been found and it's the EGA renderer
	 */
	static void setEnabled(bool enabled) {
		_enabled = enabled;
	}

	/**
	 * Returns true if the dungeons are drawn from pictures
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
	 * Adds the standard colors to a palette, if they're in the game's data
	 */
	static void loadPalette(Graphics::Palette &palette);

	/**
	 * Draws a picture of a part of the corridor
	 * @returns		False if there's no picture of it
	 */
	static bool draw(Graphics::Views::GfxSurface &s, Kind kind, int depth);

	/**
	 * Draws a picture of something in the corridor, at the size that the game gives it
	 * @returns		False if there's no picture of it
	 */
	static bool drawWidget(Graphics::Views::GfxSurface &s, Widget widget, int tier);

	/**
	 * Draws a picture of a monster, from the number it has in the game
	 * @param distance	How many squares away it is, from 1
	 * @returns		False if there's no picture of it
	 */
	static bool drawMonster(Graphics::Views::GfxSurface &s, int type, int distance);
};

} // namespace Gfx
} // namespace Ultima1
} // namespace Ultima

#endif
