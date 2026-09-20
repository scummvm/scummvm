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

#ifndef ULTIMA2_VIEWS_LOCATION_MAP_H
#define ULTIMA2_VIEWS_LOCATION_MAP_H

#include "ultima/ultima2/views/map.h"
#include "ultima/ultima2/data/tiles.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

/**
 * Renders the 20x10-tile viewport centered on the player for a village,
 * town, or castle map. Like OverworldMap, but cells outside the map's
 * 0-63 bounds show as grass (there's no wraparound - walking off the
 * edge instead exits back to the overworld, see CityCastleLogic::move)
 */
class LocationMap : public Map {
private:
	Graphics::Surface _tiles[Data::TILE_COUNT];
	Graphics::ManagedSurface _attackSprite;

protected:
	Graphics::Surface *tileGraphics() override {
		return _tiles;
	}

public:
	LocationMap();
	~LocationMap() override;

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	bool msgAttackTile(const AttackTileMessage &msg) override;
	void draw() override;
};

} // namespace Views
} // namespace Ultima2
} // namespace Ultima

#endif
