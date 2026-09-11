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

#include "ultima/ultima1/views/space_map.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

// EGA-mode colors for the sector view's baked-in raw bitmaps and the ship
// outlines - the disassembly's own hardcoded non-CGA fallback values for
// drawStarGraphic/drawStationGraphic, and (for the ships) the closest
// equivalents already established elsewhere in this port's space UI
constexpr int STAR_COLOR_1 = 12;
constexpr int STAR_COLOR_2 = 14;
constexpr int ANCHOR_COLOR = 15;
constexpr int STATION_COLOR_HILIGHT = 1;
constexpr int STATION_COLOR_BORDER = 10;
constexpr int STATION_COLOR_EDGE = 7;
constexpr int EXHAUST_COLOR_MAIN = 12;
constexpr int EXHAUST_COLOR_HILIGHT = 14;

// Bounds plotShipPixelBounded clips the exhaust trail's individual pixels to
static bool withinShipView(int x, int y) {
	return x > 20 && x < 300 && y > 18 && y < 141;
}

// The exhaust trail is plotted pixel-by-pixel (each already clipped to
// withinShipView), so a second whole-shape pass at a shifted anchor is safe
// - any pixels that land outside the viewport are simply skipped. Returns
// the extra anchor position(s) needed (not including the ship's own true
// position), up to 3 for a corner case where both axes overflow
static int wrappedPositions(int x, int y, int w, int h, Common::Point extra[3]) {
	int dxs[2] = { 0 }, dxCount = 1;
	if (x < Data::SPACE_SECTOR_MIN_X)
		dxs[dxCount++] = Data::SPACE_SECTOR_WRAP_WIDTH;
	else if (x + w > Data::SPACE_SECTOR_MAX_X)
		dxs[dxCount++] = -Data::SPACE_SECTOR_WRAP_WIDTH;

	int dys[2] = { 0 }, dyCount = 1;
	if (y < Data::SPACE_SECTOR_MIN_Y)
		dys[dyCount++] = Data::SPACE_SECTOR_WRAP_HEIGHT;
	else if (y + h > Data::SPACE_SECTOR_MAX_Y)
		dys[dyCount++] = -Data::SPACE_SECTOR_WRAP_HEIGHT;

	int count = 0;
	for (int dyi = 0; dyi < dyCount; ++dyi) {
		for (int dxi = 0; dxi < dxCount; ++dxi) {
			if (dxi == 0 && dyi == 0)
				continue;
			extra[count++] = Common::Point(x + dxs[dxi], y + dys[dyi]);
		}
	}
	return count;
}

// The ship tile is a single rectangular XOR blit, so (unlike the exhaust)
// there's no per-pixel clipping to fall back on - xorBlitFrom doesn't clip
// its rect either, so blitting the whole tile at a shifted, partly
// off-surface position would corrupt unrelated pixels rather than just get
// cropped. Instead, split the tile at the sector's wrap edge and draw each
// piece at its own fully-in-bounds position, wrapping the clipped part
// around to the opposite edge - up to 4 pieces at a corner
static void drawWrappedTile(Shared::Gfx::GfxSurface &s, const Graphics::ManagedSurface &tile, int x, int y) {
	int w = tile.w, h = tile.h;
	int splitX = (x + w > Data::SPACE_SECTOR_MAX_X) ? (Data::SPACE_SECTOR_MAX_X - x) : w;
	int splitY = (y + h > Data::SPACE_SECTOR_MAX_Y) ? (Data::SPACE_SECTOR_MAX_Y - y) : h;

	s.xorBlitFrom(tile, Common::Rect(0, 0, splitX, splitY), Common::Point(x, y));
	if (splitX < w)
		s.xorBlitFrom(tile, Common::Rect(splitX, 0, w, splitY), Common::Point(Data::SPACE_SECTOR_MIN_X, y));
	if (splitY < h)
		s.xorBlitFrom(tile, Common::Rect(0, splitY, splitX, h), Common::Point(x, Data::SPACE_SECTOR_MIN_Y));
	if (splitX < w && splitY < h)
		s.xorBlitFrom(tile, Common::Rect(splitX, splitY, w, h), Common::Point(Data::SPACE_SECTOR_MIN_X, Data::SPACE_SECTOR_MIN_Y));
}

// Star/heat hazard bitmap (18 rows x 19 cols), matching SPACE.EXE's own raw
// table for drawStarGraphic. 1 -> STAR_COLOR_1, 2 -> STAR_COLOR_2
static const int8 STAR_BITMAP[18][19] = {
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0 },
	{ 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 },
	{ 1, 1, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 1, 1, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 1, 1, 0, 0 },
	{ 1, 1, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 1, 1 },
	{ 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 },
};

// Docking/encounter anchor bitmap (20 rows x 20 cols), matching SPACE.EXE's
// own raw table for drawPlanetGraphic. Any nonzero value -> ANCHOR_COLOR
static const int8 PLANET_BITMAP[20][20] = {
	{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 },
	{ 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0 },
	{ 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
};

// Space station bitmap (18 rows x 21 cols), matching SPACE.EXE's own raw
// table for drawStationGraphic. 1/2 -> STATION_COLOR_HILIGHT,
// 4 -> STATION_COLOR_EDGE, 3 -> STATION_COLOR_BORDER
static const int8 STATION_BITMAP[18][21] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 4, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 4, 1, 1, 1, 2, 3, 2, 3, 2, 3, 1, 1, 4, 0, 0, 0, 0 },
	{ 0, 0, 0, 4, 1, 1, 2, 3, 2, 2, 2, 3, 2, 3, 2, 1, 1, 4, 0, 0, 0 },
	{ 0, 0, 4, 1, 1, 3, 2, 2, 2, 2, 2, 3, 2, 3, 2, 2, 1, 1, 4, 0, 0 },
	{ 0, 4, 1, 1, 3, 3, 2, 2, 2, 2, 2, 3, 2, 2, 2, 3, 2, 1, 1, 4, 0 },
	{ 0, 4, 1, 1, 3, 3, 3, 2, 2, 3, 3, 3, 2, 3, 3, 3, 3, 1, 1, 4, 0 },
	{ 4, 1, 1, 2, 3, 2, 2, 2, 2, 3, 3, 3, 3, 2, 3, 3, 3, 2, 1, 1, 4 },
	{ 4, 1, 1, 1, 3, 2, 2, 3, 2, 2, 3, 3, 3, 3, 3, 3, 2, 2, 1, 1, 4 },
	{ 4, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 2, 1, 1, 4 },
	{ 4, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 4 },
	{ 0, 4, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 4, 0 },
	{ 0, 4, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 4, 0 },
	{ 0, 0, 4, 1, 1, 2, 2, 3, 2, 3, 3, 3, 3, 2, 2, 2, 1, 1, 4, 0, 0 },
	{ 0, 0, 0, 4, 1, 1, 2, 3, 2, 3, 2, 3, 2, 2, 2, 1, 1, 4, 0, 0, 0 },
	{ 0, 0, 0, 0, 4, 1, 1, 1, 2, 3, 2, 3, 2, 1, 1, 1, 4, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 4, 1, 1, 1, 1, 1, 1, 1, 4, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
};

bool SpaceMap::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_SPACE);
	return Map::msgFocus(msg);
}

bool SpaceMap::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return Map::msgUnfocus(msg);
}

void SpaceMap::draw() {
	Map::draw();

	auto s = getSurface();
	const Data::SpaceMapCell &cell =
		_G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY];

	// The station graphic only ever appears in its own fixed sector, at the
	// docking-bay entrance point
	if (_G(savegame)._sectorX == Data::SPACE_STATION_X && _G(savegame)._sectorY == Data::SPACE_STATION_Y)
		drawStationGraphic(s, Data::SPACE_STATION_SCREEN_X, Data::SPACE_STATION_SCREEN_Y);

	if (cell._hazardX != 0)
		drawStarGraphic(s, cell._hazardX, cell._hazardY);

	if (cell._anchorX != 0)
		drawPlanetGraphic(s, cell._anchorX, cell._anchorY);

	for (int i = 0; i < Data::SPACE_SHIPS_PER_SECTOR; ++i) {
		const Data::SpaceMapShip &ship = cell._ships[i];
		if (ship._shipType != Data::SHIP_NONE)
			drawShipOutline(s, ship);
	}

	// Only the player's own ship thrusts, so only it ever trails exhaust
	if (_G(shipExhaustCountdown) > 0) {
		const Data::SpaceMapShip &playerShip = cell._ships[_G(savegame)._shipIndex];
		drawShipExhaust(s, playerShip);

		Common::Point extra[3];
		int count = wrappedPositions(playerShip._x, playerShip._y,
			Data::SPACE_SHIP_TILE_WIDTH, Data::SPACE_SHIP_TILE_HEIGHT, extra);
		for (int i = 0; i < count; ++i) {
			Data::SpaceMapShip wrapped = playerShip;
			wrapped._x = (int16)extra[i].x;
			wrapped._y = (int16)extra[i].y;
			drawShipExhaust(s, wrapped);
		}
	}
}

void SpaceMap::drawStarGraphic(Shared::Gfx::GfxSurface &s, int x, int y) {
	for (int row = 0; row < 18; ++row) {
		for (int col = 0; col < 19; ++col) {
			switch (STAR_BITMAP[row][col]) {
			case 1:
				s.setPixel(x + col, y + row, STAR_COLOR_1);
				break;
			case 2:
				s.setPixel(x + col, y + row, STAR_COLOR_2);
				break;
			default:
				break;
			}
		}
	}
}

void SpaceMap::drawPlanetGraphic(Shared::Gfx::GfxSurface &s, int x, int y) {
	for (int row = 0; row < 20; ++row) {
		for (int col = 0; col < 20; ++col) {
			if (PLANET_BITMAP[row][col] != 0)
				s.setPixel(x + col, y + row, ANCHOR_COLOR);
		}
	}
}

void SpaceMap::drawStationGraphic(Shared::Gfx::GfxSurface &s, int x, int y) {
	for (int row = 0; row < 18; ++row) {
		for (int col = 0; col < 21; ++col) {
			switch (STATION_BITMAP[row][col]) {
			case 1:
			case 2:
				s.setPixel(x + col, y + row, STATION_COLOR_HILIGHT);
				break;
			case 4:
				s.setPixel(x + col, y + row, STATION_COLOR_EDGE);
				break;
			case 3:
				s.setPixel(x + col, y + row, STATION_COLOR_BORDER);
				break;
			default:
				break;
			}
		}
	}
}

void SpaceMap::drawShipOutline(Shared::Gfx::GfxSurface &s, const Data::SpaceMapShip &ship) {
	int tileIndex = ship._shipType * 4 + ship._facing;
	const Graphics::ManagedSurface &tile = _G(map).spaceShipTiles()[tileIndex];
	drawWrappedTile(s, tile, ship._x, ship._y);
}

void SpaceMap::drawShipExhaust(Shared::Gfx::GfxSurface &s, const Data::SpaceMapShip &ship) {
	switch (ship._shipType) {
	case Data::SHIP_LARGE_FIGHTER:
		drawLargeFighterExhaust(s, ship._x, ship._y, ship._facing);
		break;
	case Data::SHIP_SMALL_FIGHTER:
		drawSmallFighterExhaust(s, ship._x, ship._y, ship._facing);
		break;
	case Data::SHIP_SHUTTLE:
	default:
		drawShuttleExhaust(s, ship._x, ship._y, ship._facing);
		break;
	}
}

void SpaceMap::drawShuttleExhaust(Shared::Gfx::GfxSurface &s, int x, int y, int facing) {
	auto plot = [&s](int px, int py, int color) {
		if (withinShipView(px, py))
			s.setPixel(px, py, color);
	};

	switch (facing) {
	case Data::FACING_LEFT:
		for (int i = 0; i < 3; ++i) {
			plot(x + 19, y + 4 + i, EXHAUST_COLOR_MAIN);
			plot(x + 19, y + 11 + i, EXHAUST_COLOR_MAIN);
		}
		plot(x + 20, y + 5, EXHAUST_COLOR_HILIGHT);
		plot(x + 20, y + 6, EXHAUST_COLOR_HILIGHT);
		plot(x + 20, y + 11, EXHAUST_COLOR_HILIGHT);
		plot(x + 20, y + 12, EXHAUST_COLOR_HILIGHT);
		plot(x + 21, y + 6, EXHAUST_COLOR_MAIN);
		plot(x + 21, y + 11, EXHAUST_COLOR_MAIN);
		break;

	case Data::FACING_RIGHT:
		for (int i = 0; i < 3; ++i) {
			plot(x - 1, y + 4 + i, EXHAUST_COLOR_MAIN);
			plot(x - 1, y + 11 + i, EXHAUST_COLOR_MAIN);
		}
		plot(x - 2, y + 5, EXHAUST_COLOR_HILIGHT);
		plot(x - 2, y + 6, EXHAUST_COLOR_HILIGHT);
		plot(x - 2, y + 11, EXHAUST_COLOR_HILIGHT);
		plot(x - 2, y + 12, EXHAUST_COLOR_HILIGHT);
		plot(x - 3, y + 6, EXHAUST_COLOR_MAIN);
		plot(x - 3, y + 11, EXHAUST_COLOR_MAIN);
		break;

	case Data::FACING_UP:
		for (int i = 0; i < 3; ++i) {
			plot(x + 5 + i, y + 19, EXHAUST_COLOR_MAIN);
			plot(x + 12 + i, y + 19, EXHAUST_COLOR_MAIN);
		}
		plot(x + 6, y + 20, EXHAUST_COLOR_HILIGHT);
		plot(x + 7, y + 20, EXHAUST_COLOR_HILIGHT);
		plot(x + 12, y + 20, EXHAUST_COLOR_HILIGHT);
		plot(x + 13, y + 20, EXHAUST_COLOR_HILIGHT);
		plot(x + 7, y + 21, EXHAUST_COLOR_MAIN);
		plot(x + 12, y + 21, EXHAUST_COLOR_MAIN);
		break;

	case Data::FACING_DOWN:
	default:
		for (int i = 0; i < 3; ++i) {
			plot(x + 5 + i, y - 1, EXHAUST_COLOR_MAIN);
			plot(x + 12 + i, y - 1, EXHAUST_COLOR_MAIN);
		}
		plot(x + 6, y - 2, EXHAUST_COLOR_HILIGHT);
		plot(x + 7, y - 2, EXHAUST_COLOR_HILIGHT);
		plot(x + 12, y - 2, EXHAUST_COLOR_HILIGHT);
		plot(x + 13, y - 2, EXHAUST_COLOR_HILIGHT);
		plot(x + 7, y - 3, EXHAUST_COLOR_MAIN);
		plot(x + 12, y - 3, EXHAUST_COLOR_MAIN);
		break;
	}
}

void SpaceMap::drawSmallFighterExhaust(Shared::Gfx::GfxSurface &s, int x, int y, int facing) {
	auto plot = [&s](int px, int py) {
		if (withinShipView(px, py))
			s.setPixel(px, py, EXHAUST_COLOR_MAIN);
	};

	switch (facing) {
	case Data::FACING_LEFT:
		for (int i = 0; i < 2; ++i) {
			plot(x + 17 + i, y + 1);
			plot(x + 17 + i, y + 2);
			plot(x + 17 + i, y + 15);
			plot(x + 17 + i, y + 16);
		}
		for (int i = 0; i < 4; ++i)
			plot(x + 17, y + 7 + i);
		plot(x + 18, y + 6);
		plot(x + 18, y + 11);
		break;

	case Data::FACING_RIGHT:
		for (int i = 0; i < 2; ++i) {
			plot(x - 1 - i, y + 1);
			plot(x - 1 - i, y + 2);
			plot(x - 1 - i, y + 15);
			plot(x - 1 - i, y + 16);
		}
		for (int i = 0; i < 4; ++i)
			plot(x - 1, y + 7 + i);
		plot(x - 2, y + 6);
		plot(x - 2, y + 11);
		break;

	case Data::FACING_UP:
		for (int i = 0; i < 2; ++i) {
			plot(x + i, y + 18);
			plot(x + i, y + 19);
			plot(x + 14 + i, y + 18);
			plot(x + 14 + i, y + 19);
		}
		for (int i = 0; i < 4; ++i)
			plot(x + 6 + i, y + 18);
		plot(x + 5, y + 19);
		plot(x + 10, y + 19);
		break;

	case Data::FACING_DOWN:
	default:
		for (int i = 0; i < 2; ++i) {
			plot(x + i, y);
			plot(x + i, y - 1);
			plot(x + 14 + i, y);
			plot(x + 14 + i, y - 1);
		}
		for (int i = 0; i < 4; ++i)
			plot(x + 6 + i, y);
		plot(x + 5, y - 1);
		plot(x + 10, y - 1);
		break;
	}
}

void SpaceMap::drawLargeFighterExhaust(Shared::Gfx::GfxSurface &s, int x, int y, int facing) {
	auto plot = [&s](int px, int py) {
		if (withinShipView(px, py))
			s.setPixel(px, py, EXHAUST_COLOR_MAIN);
	};

	switch (facing) {
	case Data::FACING_LEFT:
		for (int i = 0; i < 2; ++i) {
			plot(x + 17, y + 7 + i);
			plot(x + 17, y + 9 + i);
			plot(x + 18, y + 8 + i);
		}
		for (int i = 0; i < 6; ++i)
			plot(x + 19, y + 6 + i);
		break;

	case Data::FACING_RIGHT:
		for (int i = 0; i < 2; ++i) {
			plot(x - 1, y + 7 + i);
			plot(x - 1, y + 9 + i);
			plot(x - 2, y + 8 + i);
		}
		for (int i = 0; i < 6; ++i)
			plot(x - 3, y + 6 + i);
		break;

	case Data::FACING_UP:
		for (int i = 0; i < 2; ++i) {
			plot(x + 7 + i, y + 18);
			plot(x + 9 + i, y + 18);
			plot(x + 8 + i, y + 19);
		}
		for (int i = 0; i < 6; ++i)
			plot(x + 6 + i, y + 20);
		break;

	case Data::FACING_DOWN:
	default:
		for (int i = 0; i < 2; ++i) {
			plot(x + 7 + i, y);
			plot(x + 9 + i, y);
			plot(x + 8 + i, y - 1);
		}
		for (int i = 0; i < 6; ++i)
			plot(x + 6 + i, y - 2);
		break;
	}
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
