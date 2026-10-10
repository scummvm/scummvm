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

#ifndef ULTIMA3_GFX_OVERVIEW_H
#define ULTIMA3_GFX_OVERVIEW_H

#include "graphics/views/gfx_surface.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

/**
 * The picture of the whole of a map or dungeon level that a gem or a vision
 * shows, with the position of the party flickering on it
 */
class Overview {
private:
	static const int MAP_SIZE = 64;
	static const int DUNGEON_SIZE = 16;

	enum Type {
		NONE,
		WORLD,
		LEVEL
	};

	Type _type = NONE;
	byte _pixels[MAP_SIZE * 2 * MAP_SIZE * 2] = {};
	byte _glyphs[DUNGEON_SIZE * DUNGEON_SIZE] = {};
	int _partyX = 0, _partyY = 0;
	int _step = 0;

	// Which of the four pixels the party flickers in have been lit
	bool _lit[4] = {};

	void plot(int x, int y, int value);

public:
	/**
	 * Builds the picture of the current map
	 */
	void showMap();

	/**
	 * Builds the picture of the current level of the dungeon
	 */
	void showLevel();

	void hide() {
		_type = NONE;
	}

	bool isActive() const {
		return _type != NONE;
	}

	/**
	 * Moves the flickering of the party's position on a step
	 */
	void tick();

	void draw(Graphics::Views::GfxSurface &s) const;
};

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima

#endif
