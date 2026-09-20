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


#ifndef ULTIMA2_VIEWS_DUNGEON_MAP_H
#define ULTIMA2_VIEWS_DUNGEON_MAP_H

#include "ultima/ultima2/views/map.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

/**
 * Renders the first-person wireframe corridor seen from inside a tower or
 * dungeon, along with the nearest monster in view
 */
class DungeonMap : public Map {
private:
	byte _monsterSprites[2048] = {};

	/**
	 * Draws the corridor ahead. Returns the depth it was drawn out to
	 */
	int drawCorridor(Shared::Gfx::GfxSurface &s);

	/**
	 * Draws the nearest monster in the corridor, out to the given depth
	 */
	void drawMonster(Shared::Gfx::GfxSurface &s, const byte *monsters, int depth);

protected:
	Graphics::Surface *tileGraphics() override {
		return nullptr;
	}

public:
	DungeonMap();
	~DungeonMap() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	void draw() override;
	bool tick() override;
};

} // namespace Views
} // namespace Ultima2
} // namespace Ultima

#endif
