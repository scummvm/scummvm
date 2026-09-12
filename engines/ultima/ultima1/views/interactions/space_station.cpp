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

#include "ultima/ultima1/views/interactions/space_station.h"
#include "ultima/ultima1/data/space_map.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

constexpr int DOCKING_FEE = 500;

bool SpaceStation::msgFocus(const FocusMessage &msg) {
	setBounds(Common::Rect(0, 0, 0, 0));

	// The ship is now docked - stop drifting and drop the exhaust trail
	_G(sectorDriftX) = 0;
	_G(sectorDriftY) = 0;
	_G(shipExhaustCountdown) = 0;

	writeString("Docked!  Welcome to base!\n");

	if (_G(savegame)._equippedArmor < Data::ARMOR_VACUUM_SUIT) {
		writeString("The spacedoors open, exposing\n");
		writeString("thee to a complete vacuum!\n");
		g_engine->addView("SpaceDead");
		return true;
	}

	Data::SpaceMapCell &cell = _G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY];

	if (_G(savegame)._coins < DOCKING_FEE) {
		writeString("Sorry, but thou canst not\n");
		writeString("refuel thy ship without\n");
		writeString("the 500p docking fee.\n");
		finish();
		return true;
	}

	// The ship that arrived gets refueled and its shields restored,
	// regardless of which ship (if any) the player ends up flying out in
	_G(savegame)._coins -= DOCKING_FEE;
	Data::SpaceMapShip &dockedShip = cell._ships[_G(savegame)._shipIndex];
	dockedShip._fuel = Data::shipFuelCapacity(dockedShip._shipType);
	dockedShip._shield = Data::shipShieldCapacity(dockedShip._shipType);

	int realShipCount = 0;
	for (int i = 0; i < Data::SPACE_SHIPS_PER_SECTOR; ++i) {
		if (cell._ships[i]._shipType != Data::SHIP_NONE)
			++realShipCount;
	}

	if (realShipCount > 1) {
		_state = STATE_CHOOSE_SHIP;
		resetLine();
		writeString("Choose thy ship: ");
		showCursor();
	} else {
		finish();
	}

	return true;
}

bool SpaceStation::msgAction(const ActionMessage &msg) {
	// Only a docking-port digit (handled in msgKeypress) does anything here
	return true;
}

bool SpaceStation::msgKeypress(const KeypressMessage &msg) {
	if (_state != STATE_CHOOSE_SHIP)
		return true;

	if (msg.keycode < Common::KEYCODE_1 || msg.keycode > Common::KEYCODE_4)
		return true;

	int edge = msg.keycode - Common::KEYCODE_1;
	writeString("%d\n", edge + 1);

	Data::SpaceMapCell &cell = _G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY];
	int slot = cell.shipAtEdge(edge);

	if (slot == -1) {
		writeString("That port is not available.\n");
		resetLine();
		writeString("Choose thy ship: ");
		showCursor();
		return true;
	}

	_G(savegame)._shipIndex = slot;
	writeString("Thou art in the %s ship.\n", Data::SPACE_DOCK_EDGE_NAMES[edge]);
	finish();
	return true;
}

void SpaceStation::finish() {
	g_engine->baseView()->findView("Stats")->draw();

	resetLine();
	prompt();
	close();
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
