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

#include "common/util.h"
#include "ultima/ultima1/logic/space_map_logic.h"
#include "ultima/ultima1/logic/space_cockpit_logic.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

constexpr int MAX_DRIFT = 9;

// The fastest the cockpit's speed can be set from the overhead view's drift
constexpr int MAX_DRIFT_SPEED = 8;

// Flying within this many units (on both axes) of a sector's star/heat
// hazard is fatal (checkStarProximity)
constexpr int STAR_DEATH_RANGE = 25;

// Touching anything within this range (on both axes) of the station's docking
// bay entrance lands the ship back on the planet - or destroys it, if it isn't
// a shuttle
constexpr int LANDING_RANGE = 25;

// Rotating Left/Right cycles the ship's facing a quarter-turn counter-
// clockwise/clockwise (word_17EA2), indexed by the ship's current facing
constexpr Data::SpaceShipFacing ROTATE_CCW[4] = {
	Data::FACING_DOWN, Data::FACING_UP, Data::FACING_LEFT, Data::FACING_RIGHT
};
constexpr Data::SpaceShipFacing ROTATE_CW[4] = {
	Data::FACING_UP, Data::FACING_DOWN, Data::FACING_RIGHT, Data::FACING_LEFT
};

static int wrapCoord(int value, int minVal, int maxVal, int wrapSize) {
	if (value < minVal)
		return value + wrapSize;
	if (value > maxVal)
		return value - wrapSize;
	return value;
}

// True if the given pixel of a row-major bitmap is set. Positions outside it are not
static bool bitmapPixel(const int8 *bitmap, int width, int height, int x, int y) {
	return x >= 0 && x < width && y >= 0 && y < height && bitmap[y * width + x] != 0;
}

static bool shipTilePixel(const Graphics::ManagedSurface &tile, int x, int y) {
	return x >= 0 && x < tile.w && y >= 0 && y < tile.h && *(const byte *)tile.getBasePtr(x, y) != 0;
}

static const Graphics::ManagedSurface &shipTile(const Data::SpaceMapShip &ship) {
	return _G(map).spaceShipTiles()[ship._shipType * 4 + ship._facing];
}

// True if anything other than the given ship is drawn on the given pixel of the
// sector view: the station, the star, the docking anchor, or another ship
static bool pixelDrawn(const Data::SpaceMapCell &cell, int shipIndex, bool isStationSector, int px, int py) {
	if (isStationSector && bitmapPixel(&Data::SPACE_STATION_BITMAP[0][0], Data::SPACE_STATION_WIDTH,
			Data::SPACE_STATION_HEIGHT, px - Data::SPACE_STATION_SCREEN_X, py - Data::SPACE_STATION_SCREEN_Y))
		return true;

	if (cell._hazardX != 0 && bitmapPixel(&Data::SPACE_STAR_BITMAP[0][0], Data::SPACE_STAR_WIDTH,
			Data::SPACE_STAR_HEIGHT, px - cell._hazardX, py - cell._hazardY))
		return true;

	if (cell._anchorX != 0 && bitmapPixel(&Data::SPACE_PLANET_BITMAP[0][0], Data::SPACE_PLANET_WIDTH,
			Data::SPACE_PLANET_HEIGHT, px - cell._anchorX, py - cell._anchorY))
		return true;

	for (int i = 0; i < Data::SPACE_SHIPS_PER_SECTOR; ++i) {
		const Data::SpaceMapShip &other = cell._ships[i];
		if (i == shipIndex || other._shipType == Data::SHIP_NONE)
			continue;

		// A ship drawn across the sector edge has its far part wrapped around
		for (int wrapX = 0; wrapX <= Data::SPACE_SECTOR_WRAP_WIDTH; wrapX += Data::SPACE_SECTOR_WRAP_WIDTH) {
			for (int wrapY = 0; wrapY <= Data::SPACE_SECTOR_WRAP_HEIGHT; wrapY += Data::SPACE_SECTOR_WRAP_HEIGHT) {
				if (shipTilePixel(shipTile(other), px + wrapX - other._x, py + wrapY - other._y))
					return true;
			}
		}
	}

	return false;
}

// True if the player's ship, were it at (x,y), would have any of its pixels
// land on something already drawn in the sector. This is the only thing that
// triggers a collision; the sector edges just wrap
static bool shipCollides(const Data::SpaceMapCell &cell, int shipIndex, bool isStationSector, int x, int y) {
	const Graphics::ManagedSurface &tile = shipTile(cell._ships[shipIndex]);

	for (int row = 0; row < tile.h; ++row) {
		for (int col = 0; col < tile.w; ++col) {
			if (!shipTilePixel(tile, col, row))
				continue;

			// The part of the ship past the sector edge is drawn wrapped
			int px = x + col, py = y + row;
			if (px >= Data::SPACE_SECTOR_MAX_X)
				px = Data::SPACE_SECTOR_MIN_X + px - Data::SPACE_SECTOR_MAX_X;
			if (py >= Data::SPACE_SECTOR_MAX_Y)
				py = Data::SPACE_SECTOR_MIN_Y + py - Data::SPACE_SECTOR_MAX_Y;

			if (pixelDrawn(cell, shipIndex, isStationSector, px, py))
				return true;
		}
	}

	return false;
}

bool SpaceMapLogic::move(Data::Direction dir) {
	writeString("%s\n", Data::SPACE_DIRECTION_NAMES[dir]);

	int fuelCost = (dir == Data::DIR_LEFT || dir == Data::DIR_RIGHT) ? 2 : 5;

	if (shipFuel() == 0) {
		writeString("No fuel!!\n");
		writeString("Wilt thou drift forever?!?\n");
		return true;
	}
	if (shipFuel() < fuelCost)
		fuelCost = shipFuel();
	subtractFuel(fuelCost);

	Data::SpaceMapShip &ship =
		_G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY]._ships[_G(savegame)._shipIndex];

	switch (dir) {
	case Data::DIR_LEFT:
		ship._facing = ROTATE_CCW[ship._facing];
		_G(shipExhaustCountdown) = 0;
		break;
	case Data::DIR_RIGHT:
		ship._facing = ROTATE_CW[ship._facing];
		_G(shipExhaustCountdown) = 0;
		break;
	case Data::DIR_UP:
		_G(sectorDriftX) = CLIP(_G(sectorDriftX) + Data::SPACE_FACING_DX[ship._facing], -MAX_DRIFT, MAX_DRIFT);
		_G(sectorDriftY) = CLIP(_G(sectorDriftY) + Data::SPACE_FACING_DY[ship._facing], -MAX_DRIFT, MAX_DRIFT);
		_G(shipExhaustCountdown) += 10;
		break;
	case Data::DIR_DOWN:
		// Retro-thrust - a real thrust in the opposite direction, not a
		// full stop. From rest it starts the ship moving backwards, and
		// against existing forward drift it only decelerates it by one
		// increment (mirrors Up, just subtracted instead of added). It does
		// immediately clear any exhaust trail still showing from a
		// previous Up, rather than extending it further
		_G(sectorDriftX) = CLIP(_G(sectorDriftX) - Data::SPACE_FACING_DX[ship._facing], -MAX_DRIFT, MAX_DRIFT);
		_G(sectorDriftY) = CLIP(_G(sectorDriftY) - Data::SPACE_FACING_DY[ship._facing], -MAX_DRIFT, MAX_DRIFT);
		_G(shipExhaustCountdown) = 0;
		break;
	default:
		break;
	}

	redrawMap();
	return true;
}

bool SpaceMapLogic::attack(Data::Direction) {
	if (_G(savegame)._sectorX != Data::SPACE_STATION_X || _G(savegame)._sectorY != Data::SPACE_STATION_Y) {
		writeString("Autopilot: only in home sector.\n");
		return true;
	}

	Data::SpaceMapCell &cell = _G(savegame)._starmap._sectors[Data::SPACE_STATION_X][Data::SPACE_STATION_Y];
	int freeEdge = -1;
	for (int edge = 0; edge < Data::SPACE_DOCK_EDGE_COUNT; ++edge) {
		if (cell.shipAtEdge(edge) == -1) {
			freeEdge = edge;
			break;
		}
	}
	if (freeEdge == -1)
		// Should never happen
		return true;

	Data::SpaceMapShip &ship = cell._ships[_G(savegame)._shipIndex];
	ship._x = (int16)(cell._anchorX + Data::SPACE_DOCK_DELTA_X[freeEdge][ship._shipType]);
	ship._y = (int16)(cell._anchorY + Data::SPACE_DOCK_DELTA_Y[freeEdge][ship._shipType]);
	ship._facing = Data::SPACE_DOCK_EDGE_FACING[freeEdge];

	_G(sectorDriftX) = 0;
	_G(sectorDriftY) = 0;
	_G(shipExhaustCountdown) = 0;

	g_engine->addView("SpaceStation");
	return false;
}

bool SpaceMapLogic::hyperjump() {
	writeString("HyperJump?\n");
	writeString("Hyperjump can only be made\n");
	writeString("from front view.\n");
	return true;
}

void SpaceMapLogic::tick() {
	if (_G(sectorDriftX) == 0 && _G(sectorDriftY) == 0 && _G(shipExhaustCountdown) == 0)
		return;

	if (_G(sectorDriftX) != 0 || _G(sectorDriftY) != 0) {
		Data::SpaceMapCell &cell = _G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY];
		Data::SpaceMapShip &ship = cell._ships[_G(savegame)._shipIndex];

		int newX = wrapCoord(ship._x + _G(sectorDriftX), Data::SPACE_SECTOR_MIN_X, Data::SPACE_SECTOR_MAX_X, Data::SPACE_SECTOR_WRAP_WIDTH);
		int newY = wrapCoord(ship._y + _G(sectorDriftY), Data::SPACE_SECTOR_MIN_Y, Data::SPACE_SECTOR_MAX_Y, Data::SPACE_SECTOR_WRAP_HEIGHT);

		int oldX = ship._x, oldY = ship._y;
		ship._x = (int16)newX;
		ship._y = (int16)newY;

		bool isStationSector = _G(savegame)._sectorX == Data::SPACE_STATION_X && _G(savegame)._sectorY == Data::SPACE_STATION_Y;
		bool crunch = shipCollides(cell, _G(savegame)._shipIndex, isStationSector, newX, newY);

		// Coming to rest exactly in one of the station's docking slots (the
		// right position AND facing, for the ship's own type) docks safely
		if (!crunch && isStationSector && cell.dockedEdge(_G(savegame)._shipIndex) >= 0) {
			_G(sectorDriftX) = 0;
			_G(sectorDriftY) = 0;
			g_engine->addView("SpaceStation");
			redrawMap();
			return;
		}

		// Touching anything within range of the star is fatal, wherever the
		// ship was touching
		if (crunch && cell._hazardX != 0 && ABS(newX - cell._hazardX) < STAR_DEATH_RANGE &&
				ABS(newY - cell._hazardY) < STAR_DEATH_RANGE) {
			writeString("Thy ship melts near the star!\n");
			death();
			return;
		}

		if (crunch && isStationSector && ABS(newX - Data::SPACE_STATION_SCREEN_X) < LANDING_RANGE &&
				ABS(newY - Data::SPACE_STATION_SCREEN_Y) < LANDING_RANGE) {
			_G(sectorDriftX) = 0;
			_G(sectorDriftY) = 0;
			_G(shipExhaustCountdown) = 0;

			if (ship._shipType == Data::SHIP_SHUTTLE) {
				writeString("Thou hast landed safely!\n");
				_G(savegame)._shipFuel = ship._fuel;
				_G(savegame)._shipShield = ship._shield;

				// This logic is replaced by loading the overworld, so nothing
				// of it can be used afterwards
				prompt();
				_G(map).load(Data::MAP_OVERWORLD);
			} else {
				writeString("Only space shuttles are heat\n");
				writeString("shielded for landings!\n");
				ship._fuel = ship._shield = 0;
				_G(savegame)._shipFuel = _G(savegame)._shipShield = 0;
				death();
			}

			return;
		}

		if (crunch) {
			// Revert - stay where we were before this frame's drift
			ship._x = (int16)oldX;
			ship._y = (int16)oldY;

			writeString("Crunch!\n");
			playFX(0);
			subtractShields(shipShields() / 4 + 5);
			if (shipShields() == 0) {
				writeString("Thy shield is drained!\n");
				death();
				return;
			}

			// Bounce back rather than plough into it
			_G(sectorDriftX) = -_G(sectorDriftX);
			_G(sectorDriftY) = -_G(sectorDriftY);
		}
	}

	if (_G(shipExhaustCountdown) > 0)
		--_G(shipExhaustCountdown);

	redrawMap();
}

bool SpaceMapLogic::fire() {
	// Nothing to shoot at from the overhead view
	writeString("Fire?\n");
	playFX(1);
	return true;
}

// The original's integer square root, which rounds up
static int isqrt(int value) {
	int result = 0;
	while (value > 0)
		value -= 2 * ++result - 1;

	return result;
}

bool SpaceMapLogic::view() {
	writeString("View");

	Data::SpaceMapCell &cell = _G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY];
	if (cell.dockedEdge(_G(savegame)._shipIndex) >= 0) {
		writeString("?\n");
		writeString("View can't be changed\n");
		writeString("whilst in space dock.\n");
		return true;
	}

	writeString("\n");
	if (shipFuel() == 0) {
		writeString("Not enough fuel!\n");
		return true;
	}

	// The cockpit's speed is the overhead view's current drift. If the ship is
	// at rest, it starts at the slowest speed in the direction it's facing
	int speed = isqrt(_G(sectorDriftX) * _G(sectorDriftX) + _G(sectorDriftY) * _G(sectorDriftY));
	if (speed > MAX_DRIFT_SPEED) {
		speed = MAX_DRIFT_SPEED;
	} else if (speed == 0) {
		speed = 1;

		const Data::SpaceMapShip &ship = cell._ships[_G(savegame)._shipIndex];
		_G(sectorDriftX) = Data::SPACE_FACING_DX[ship._facing];
		_G(sectorDriftY) = Data::SPACE_FACING_DY[ship._facing];
	}

	_G(cockpitSpeed) = _G(cockpitEntrySpeed) = speed;

	Common::SharedPtr<SpaceCockpitLogic> cockpitLogic(new SpaceCockpitLogic());
	cockpitLogic->setupSectorEnemies();
	_G(logic) = cockpitLogic;
	g_engine->replaceView("SpaceCockpit");

	prompt();
	return false;
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
