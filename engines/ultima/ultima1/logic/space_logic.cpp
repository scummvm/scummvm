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

#include "ultima/ultima1/logic/space_logic.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/data/space_map.h"
#include "ultima/ultima1/metaengine.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/shared/engine/events.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

SpaceLogic::SpaceLogic() {
	_G(map)._mapType = Data::MAPTYPE_SPACE;
}

void SpaceLogic::entering() {
	// The original re-launches SPACE.EXE each time, always starting fresh
	// at the station sector with a newly rolled galaxy, flying the shuttle
	// out of the docking bay
	_G(savegame)._sectorX = Data::SPACE_STATION_X;
	_G(savegame)._sectorY = Data::SPACE_STATION_Y;
	_G(savegame)._shipIndex = 2;
	_G(cockpitSpeed) = 0;
	_G(savegame)._starmap.setup();

	// Seed the player's own ship slot from the savegame. setupSpaceMap also
	// parks two fighters at the station and positions everything - still
	// TODO - but the fuel/shield are what the HUD and flight need
	Data::SpaceMapShip &ship =
		_G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY]._ships[_G(savegame)._shipIndex];
	ship._shipType = Data::SHIP_SHUTTLE;
	ship._fuel = _G(savegame)._shipFuel;
	ship._shield = _G(savegame)._shipShield;
	// setupSpaceMap docks the player's shuttle at a fixed point of its own,
	// not relative to the station anchor like the two parked fighters
	ship._x = 260;
	ship._y = 90;
	ship._facing = Data::FACING_LEFT;

	redrawStats();
}

void SpaceLogic::endOfTurn() {
	redrawMap();
	redrawStats();
	// TODO: death in space (hits/shields at zero) returns to Sosaria via
	// OUT.EXE, not the land resurrection sequence
}

int SpaceLogic::shipFuel() const {
	return _G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY]._ships[_G(savegame)._shipIndex]._fuel;
}

void SpaceLogic::subtractFuel(int amount) {
	int16 &fuel = _G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY]._ships[_G(savegame)._shipIndex]._fuel;
	fuel = (fuel > amount) ? (int16)(fuel - amount) : 0;
	redrawStats();
}

int SpaceLogic::shipShields() const {
	return _G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY]._ships[_G(savegame)._shipIndex]._shield;
}

void SpaceLogic::subtractShields(int amount) {
	int16 &shield = _G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY]._ships[_G(savegame)._shipIndex]._shield;
	shield = (shield > amount) ? (int16)(shield - amount) : 0;
	redrawStats();
}

void SpaceLogic::death() {
	g_engine->addView("SpaceDead");
}

void SpaceLogic::keypress(Common::KeyCode keycode) {
	writeString("Huh?\n");
	playFX(1);
	prompt();
}

bool SpaceLogic::inform() {
	writeString("Inform and Search");
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

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
