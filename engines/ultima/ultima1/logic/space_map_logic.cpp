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

// Thrust (Up) is a unit nudge along whichever way the ship is currently
// facing, not a fixed screen axis - so facing Left and thrusting builds up
// leftward drift, and reversing facing then thrusting decelerates/reverses
// whatever drift is already there rather than adding a second, unrelated one
constexpr int FACING_DX[4] = { -1, 1, 0, 0 };
constexpr int FACING_DY[4] = { 0, 0, -1, 1 };

static int wrapCoord(int value, int minVal, int maxVal, int wrapSize) {
	if (value < minVal)
		return value + wrapSize;
	if (value > maxVal)
		return value - wrapSize;
	return value;
}

// True if a ship of the usual sprite size at (x,y) would overlap the given
// rect - used to detect running into the station or another ship (the only
// thing that actually triggers "Crunch!"; the sector edges just wrap)
static bool overlapsRect(int x, int y, int rx, int ry, int rw, int rh) {
	return x < rx + rw && x + Data::SPACE_SHIP_TILE_WIDTH > rx &&
		y < ry + rh && y + Data::SPACE_SHIP_TILE_HEIGHT > ry;
}

bool SpaceMapLogic::move(Data::Direction dir) {
	writeString("%s\n", Data::DIRECTION_NAMES[dir]);

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
		_G(sectorDriftX) = CLIP(_G(sectorDriftX) + FACING_DX[ship._facing], -MAX_DRIFT, MAX_DRIFT);
		_G(sectorDriftY) = CLIP(_G(sectorDriftY) + FACING_DY[ship._facing], -MAX_DRIFT, MAX_DRIFT);
		_G(shipExhaustCountdown) += 10;
		break;
	case Data::DIR_DOWN:
		// Brake - stop drifting and clear the exhaust trail. Rotating
		// (Left/Right) leaves the drift untouched - only Down cancels it
		_G(sectorDriftX) = 0;
		_G(sectorDriftY) = 0;
		_G(shipExhaustCountdown) = 0;
		break;
	default:
		break;
	}

	redrawMap();
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

		// The only real obstacles are the station (in its own sector) and
		// any other ship sitting in the sector - the sector edges themselves
		// just wrap around
		bool crunch = false;
		if (_G(savegame)._sectorX == Data::SPACE_STATION_X && _G(savegame)._sectorY == Data::SPACE_STATION_Y) {
			crunch = overlapsRect(newX, newY, Data::SPACE_STATION_SCREEN_X, Data::SPACE_STATION_SCREEN_Y,
				Data::SPACE_STATION_WIDTH, Data::SPACE_STATION_HEIGHT);
		}
		for (int i = 0; !crunch && i < Data::SPACE_SHIPS_PER_SECTOR; ++i) {
			if (i == _G(savegame)._shipIndex)
				continue;
			const Data::SpaceMapShip &other = cell._ships[i];
			if (other._shipType != Data::SHIP_NONE)
				crunch = overlapsRect(newX, newY, other._x, other._y, Data::SPACE_SHIP_TILE_WIDTH, Data::SPACE_SHIP_TILE_HEIGHT);
		}

		if (crunch) {
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
		} else {
			ship._x = (int16)newX;
			ship._y = (int16)newY;
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

	// TODO: the original refuses the switch while enemies remain in the
	// sector ("Thou must eliminate all enemy craft first!"), with no fuel,
	// or while docked; and seeds the cockpit's heading from the current
	// drift
	_G(logic) = Common::SharedPtr<Logic>(new SpaceCockpitLogic());
	g_engine->replaceView("SpaceCockpit");
	return true;
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
