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

// Flying within this many units (on both axes) of a sector's star/heat
// hazard is fatal (checkStarProximity)
constexpr int STAR_DEATH_RANGE = 25;

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

// The ship tile is a 32x19 canvas, but most of that is blank padding left
// over from the original's byte-alignment pre-shift copies (see
// loadShipTiles) - the actual ship silhouette drawn within it is much
// smaller. Using the full tile as the collision box was triggering "Crunch!"
// roughly a whole ship-width before the ship visually touched anything, so
// use a smaller, roughly-centred box instead
constexpr int SHIP_COLLISION_WIDTH = 16;
constexpr int SHIP_COLLISION_HEIGHT = 10;
constexpr int SHIP_COLLISION_OFFSET_X = (Data::SPACE_SHIP_TILE_WIDTH - SHIP_COLLISION_WIDTH) / 2;
constexpr int SHIP_COLLISION_OFFSET_Y = (Data::SPACE_SHIP_TILE_HEIGHT - SHIP_COLLISION_HEIGHT) / 2;

// True if a ship at (x,y) (its tile's top-left) would overlap the given
// rect - used to detect running into the station or another ship (the only
// thing that actually triggers "Crunch!"; the sector edges just wrap)
static bool overlapsRect(int x, int y, int rx, int ry, int rw, int rh) {
	x += SHIP_COLLISION_OFFSET_X;
	y += SHIP_COLLISION_OFFSET_Y;
	return x < rx + rw && x + SHIP_COLLISION_WIDTH > rx &&
		y < ry + rh && y + SHIP_COLLISION_HEIGHT > ry;
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

bool SpaceMapLogic::attack(Data::Direction dir) {
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

		if (cell._hazardX != 0 && ABS(newX - cell._hazardX) < STAR_DEATH_RANGE && ABS(newY - cell._hazardY) < STAR_DEATH_RANGE) {
			writeString("Thy ship melts near the star!\n");
			death();
			return;
		}

		int oldX = ship._x, oldY = ship._y;
		ship._x = (int16)newX;
		ship._y = (int16)newY;

		bool isStationSector = _G(savegame)._sectorX == Data::SPACE_STATION_X && _G(savegame)._sectorY == Data::SPACE_STATION_Y;

		// Coming to rest exactly in one of the station's docking slots (the
		// right position AND facing, for the ship's own type) docks safely;
		// anything else that touches the station or another ship is a crunch
		if (isStationSector && cell.dockedEdge(_G(savegame)._shipIndex) >= 0) {
			_G(sectorDriftX) = 0;
			_G(sectorDriftY) = 0;
			g_engine->addView("SpaceStation");
			redrawMap();
			return;
		}

		bool crunch = false;
		if (isStationSector) {
			crunch = overlapsRect(newX, newY, Data::SPACE_STATION_SCREEN_X, Data::SPACE_STATION_SCREEN_Y,
				Data::SPACE_STATION_WIDTH, Data::SPACE_STATION_HEIGHT);
		}
		for (int i = 0; !crunch && i < Data::SPACE_SHIPS_PER_SECTOR; ++i) {
			if (i == _G(savegame)._shipIndex)
				continue;
			const Data::SpaceMapShip &other = cell._ships[i];
			if (other._shipType != Data::SHIP_NONE)
				crunch = overlapsRect(newX, newY, other._x + SHIP_COLLISION_OFFSET_X, other._y + SHIP_COLLISION_OFFSET_Y,
					SHIP_COLLISION_WIDTH, SHIP_COLLISION_HEIGHT);
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

bool SpaceMapLogic::view() {
	writeString("View\n");

	// TODO: the original refuses the switch with no fuel or while docked,
	// and seeds the cockpit's heading from the current drift
	Common::SharedPtr<SpaceCockpitLogic> cockpitLogic(new SpaceCockpitLogic());
	cockpitLogic->setupSectorEnemies();
	_G(logic) = cockpitLogic;
	g_engine->replaceView("SpaceCockpit");
	return true;
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
