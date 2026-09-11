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

// The overhead sector view's playable area (clampShipX/Y) - drifting past
// this is what triggers the viewport-border "Crunch!" bounce
constexpr int SECTOR_MIN_X = 20, SECTOR_MAX_X = 275;
constexpr int SECTOR_MIN_Y = 10, SECTOR_MAX_Y = 130;
constexpr int MAX_DRIFT = 9;

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

	// A ship sitting right on top of another counts as docked - thrusting
	// into it (anything but Down, which just brakes) is a crunch rather
	// than a move
	Data::SpaceMapCell &cell = _G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY];
	const Data::SpaceMapShip &ship = cell._ships[_G(savegame)._shipIndex];
	bool docked = false;
	for (int i = 0; i < Data::SPACE_SHIPS_PER_SECTOR && !docked; ++i) {
		if (i == _G(savegame)._shipIndex)
			continue;
		const Data::SpaceMapShip &other = cell._ships[i];
		docked = other._shipType != Data::SHIP_NONE && other._x == ship._x && other._y == ship._y;
	}

	if (docked && dir != Data::DIR_DOWN) {
		writeString("Crunch!\n");
		playFX(0);
		subtractShields(shipShields() / 4 + 5);
		// TODO: death in space - see SpaceLogic::endOfTurn
		if (shipShields() == 0)
			writeString("Thy shield is drained!\n");
		return true;
	}

	subtractFuel(fuelCost);

	if (dir == Data::DIR_DOWN) {
		// Brake - stop drifting and clear the exhaust trail
		_G(sectorDriftX) = 0;
		_G(sectorDriftY) = 0;
		_G(shipExhaustCountdown) = 0;
		redrawMap();
		return true;
	}

	if (dir == Data::DIR_LEFT || dir == Data::DIR_RIGHT) {
		// A lateral nudge doesn't sustain the exhaust trail the way
		// thrusting forward (Up) does
		_G(shipExhaustCountdown) = 0;
	}

	switch (dir) {
	case Data::DIR_LEFT:
		_G(sectorDriftX) = CLIP(_G(sectorDriftX) - 1, -MAX_DRIFT, MAX_DRIFT);
		break;
	case Data::DIR_RIGHT:
		_G(sectorDriftX) = CLIP(_G(sectorDriftX) + 1, -MAX_DRIFT, MAX_DRIFT);
		break;
	case Data::DIR_UP:
		_G(sectorDriftY) = CLIP(_G(sectorDriftY) - 1, -MAX_DRIFT, MAX_DRIFT);
		_G(shipExhaustCountdown) += 10;
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
		Data::SpaceMapShip &ship =
			_G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY]._ships[_G(savegame)._shipIndex];

		int newX = ship._x + _G(sectorDriftX);
		int newY = ship._y + _G(sectorDriftY);

		if (newX < SECTOR_MIN_X || newX > SECTOR_MAX_X || newY < SECTOR_MIN_Y || newY > SECTOR_MAX_Y) {
			writeString("Crunch!\n");
			playFX(0);
			subtractShields(shipShields() / 4 + 5);
			// TODO: death in space - see SpaceLogic::endOfTurn
			if (shipShields() == 0)
				writeString("Thy shield is drained!\n");

			// Bounce back off the border
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
