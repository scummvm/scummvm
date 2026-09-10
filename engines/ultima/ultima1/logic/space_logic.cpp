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
#include "ultima/ultima1/logic/space_logic.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/data/space_map.h"
#include "ultima/ultima1/metaengine.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

SpaceLogic::SpaceLogic() {
}

void SpaceLogic::entering() {
	// The original re-launches SPACE.EXE each time, always starting fresh
	// at the station sector with a newly rolled galaxy, flying the shuttle
	// out of the docking bay
	_G(sectorX) = Data::SPACE_STATION_X;
	_G(sectorY) = Data::SPACE_STATION_Y;
	_G(shipIndex) = 2;
	_G(cockpitView) = false;
	_G(cockpitSpeed) = 0;
	_G(spaceMap).setup();
}

int SpaceLogic::shipFuel() const {
	return _G(spaceMap)._sectors[_G(sectorX)][_G(sectorY)]._ships[_G(shipIndex)]._fuel;
}

void SpaceLogic::subtractFuel(int amount) {
	int16 &fuel = _G(spaceMap)._sectors[_G(sectorX)][_G(sectorY)]._ships[_G(shipIndex)]._fuel;
	fuel = (fuel > amount) ? (int16)(fuel - amount) : 0;
	// TODO: displayFuelNumber() - refresh the cockpit's fuel readout
}

void SpaceLogic::setSpeed(int speed) {
	writeString("Speed %d\n", speed);

	int reqFuel = ABS(speed - _G(cockpitSpeed)) * 4;
	if (reqFuel > shipFuel()) {
		writeString("Not enough fuel!\n");
	} else {
		_G(cockpitSpeed) = speed;
		subtractFuel(reqFuel);
	}
}

void SpaceLogic::keypress(Common::KeyCode keycode) {
	if (_G(cockpitView) && keycode >= Common::KEYCODE_1 && keycode <= Common::KEYCODE_8) {
		setSpeed(keycode - Common::KEYCODE_0);
		return;
	}

	writeString("Huh?\n");
	playFX(1);
}

bool SpaceLogic::inform() {
	writeString("Inform and Search\n");
	g_engine->addView("SectorScan");
	return false;
}

bool SpaceLogic::attack(Data::Direction dir) {
	writeString("Attack?\n");
	playFX(1);
	return true;
}

bool SpaceLogic::cast() {
	writeString("Cast?\n");
	playFX(1);
	return true;
}

bool SpaceLogic::climb() {
	writeString("Klimb?\n");
	playFX(1);
	return true;
}

bool SpaceLogic::fire() {
	// Overhead view - nothing to shoot at. In the cockpit view the original
	// runs handleFireCommand (space combat), which isn't ported yet
	writeString("Fire?\n");
	playFX(1);
	return true;
}

bool SpaceLogic::quit() {
	writeString("Quit?\n");
	playFX(1);
	return true;
}

bool SpaceLogic::ready() {
	writeString("Ready?\n");
	playFX(1);
	return true;
}

bool SpaceLogic::view() {
	// The original's View toggles between the cockpit and overhead sector
	// map (with fuel/collision checks and a "whilst in space dock" guard) -
	// not ported until the flight system is
	writeString("View?\n");
	playFX(1);
	return true;
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
