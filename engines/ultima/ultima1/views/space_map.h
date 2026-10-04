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

#ifndef ULTIMA1_VIEWS_SPACE_MAP_H
#define ULTIMA1_VIEWS_SPACE_MAP_H

#include "ultima/ultima1/views/map.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/data/space_map.h"
#include "graphics/views/gfx_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

using namespace Graphics::Views;

/**
 * The overhead view of the player's current galaxy sector - the station
 * and/or star hazard and/or docking anchor (if any are present), and every
 * active ship, matching the original SPACE.EXE's setupView (overhead branch)
 */
class SpaceMap : public Map {
private:
	/**
	 * Draws the station's raw bitmap (drawStationGraphic), only shown in
	 * sector (3,3)
	 */
	void drawStationGraphic(Graphics::Views::GfxSurface &s, int x, int y);

	/**
	 * Draws a sector's star/heat hazard raw bitmap (drawStarGraphic)
	 */
	void drawStarGraphic(Graphics::Views::GfxSurface &s, int x, int y);

	/**
	 * Draws a sector's docking/encounter anchor raw bitmap (drawPlanetGraphic)
	 */
	void drawPlanetGraphic(Graphics::Views::GfxSurface &s, int x, int y);

	/**
	 * XOR-blits a ship's glyph (egaspace.bin, via Data::Map::spaceShipTiles,
	 * indexed by shipType*4+facing) at its current position
	 * (blitShipSpriteXor)
	 */
	void drawShipOutline(Graphics::Views::GfxSurface &s, const Data::SpaceMapShip &ship);

	/**
	 * Draws the player ship's engine exhaust trail while
	 * _G(shipExhaustCountdown) is active, dispatching by ship type
	 * (drawShipSprite, vector-plotted rather than a sprite tile)
	 */
	void drawShipExhaust(Graphics::Views::GfxSurface &s, const Data::SpaceMapShip &ship);
	void drawShuttleExhaust(Graphics::Views::GfxSurface &s, int x, int y, int facing);
	void drawLargeFighterExhaust(Graphics::Views::GfxSurface &s, int x, int y, int facing);
	void drawSmallFighterExhaust(Graphics::Views::GfxSurface &s, int x, int y, int facing);

public:
	SpaceMap() : Map("SpaceMap") {}
	~SpaceMap() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	void draw() override;
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
