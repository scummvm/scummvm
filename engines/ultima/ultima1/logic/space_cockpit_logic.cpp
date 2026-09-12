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
#include "ultima/ultima1/logic/space_cockpit_logic.h"
#include "ultima/ultima1/logic/space_map_logic.h"
#include "ultima/ultima1/data/space_map.h"
#include "ultima/ultima1/views/space_cockpit.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

void SpaceCockpitLogic::setSpeed(int speed) {
	writeString("Speed %d\n", speed);

	int reqFuel = ABS(speed - _G(cockpitSpeed)) * 4;
	if (reqFuel > shipFuel()) {
		writeString("Not enough fuel!\n");
	} else {
		_G(cockpitSpeed) = speed;
		subtractFuel(reqFuel);
	}

	prompt();
}

void SpaceCockpitLogic::keypress(Common::KeyCode keycode) {
	if (keycode >= Common::KEYCODE_1 && keycode <= Common::KEYCODE_8) {
		setSpeed(keycode - Common::KEYCODE_0);
		return;
	}

	SpaceLogic::keypress(keycode);
}

void SpaceCockpitLogic::tick() {
	// The original advances the starfield every cockpit frame; at 20fps
	// that's far too fast, so only every 3rd tick
	if (++_tickCounter < 3)
		return;
	_tickCounter = 0;

	_G(starfield).advance();
	redrawMap();
}

bool SpaceCockpitLogic::move(Data::Direction dir) {
	writeString("%s\n", Data::SPACE_COCKPIT_DIRECTION_NAMES[dir]);

	if (shipFuel() < 2) {
		writeString("Not enough fuel!\n");
		return true;
	}
	subtractFuel(2);

	// Steer by setting the starfield pan velocity - the view's per-frame
	// update slides the centre and clamps it at the viewport edges
	Data::SpaceStarfield &sf = _G(starfield);
	switch (dir) {
	case Data::DIR_LEFT:
		sf._panX = -4;
		sf._panY = 0;
		break;
	case Data::DIR_RIGHT:
		sf._panX = 4;
		sf._panY = 0;
		break;
	case Data::DIR_UP:
		sf._panY = -4;
		sf._panX = 0;
		break;
	case Data::DIR_DOWN:
		sf._panY = 4;
		sf._panX = 0;
		break;
	default:
		break;
	}

	return true;
}

bool SpaceCockpitLogic::fire() {
	// TODO: handleFireCommand - the actual enemy-ship combat
	writeString("Fire!\n");
	playFX(1);
	return true;
}

bool SpaceCockpitLogic::pass() {
	// Re-centres the starscape and stops any drift
	writeString("Center\n");

	Data::SpaceStarfield &sf = _G(starfield);
	sf._centerX = 0x9f;
	sf._centerY = 0x4f;
	sf._panX = sf._panY = 0;
	redrawMap();
	return true;
}

bool SpaceCockpitLogic::hyperjump() {
	// Left open (no newline) so the animation can append " Lightspeed!" to
	// the same line once it hits full scrolling speed
	writeString("HyperJump...");

	if (shipFuel() < 100) {
		writeString("Not enough fuel!\n");
		return true;
	}

	// Heads one sector over in whichever direction the ship is currently
	// facing on the overhead map - the original derives this from which
	// edge of the sector the ship exited, but facing is the equivalent
	// concept here, and doesn't require the ship to actually be moving
	Data::SpaceMapShip &ship = _G(savegame)._starmap
		._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY]
		._ships[_G(savegame)._shipIndex];
	int dx = Data::SPACE_FACING_DX[ship._facing];
	int dy = Data::SPACE_FACING_DY[ship._facing];

	Views::SpaceCockpit *view = dynamic_cast<Views::SpaceCockpit *>(g_engine->findView("SpaceCockpit"));
	assert(view);
	view->startHyperjump(dx, dy);

	// The animation owns the commands area until it finishes or is
	// interrupted - suppress the usual immediate end-of-turn/prompt
	return false;
}

void SpaceCockpitLogic::hyperjumpLightspeed() {
	writeString(" Lightspeed!");
}

void SpaceCockpitLogic::completeHyperjump(int dx, int dy) {
	subtractFuel(100);

	int newSectorX = (_G(savegame)._sectorX + dx + Data::SPACE_SECTOR_COUNT) % Data::SPACE_SECTOR_COUNT;
	int newSectorY = (_G(savegame)._sectorY + dy + Data::SPACE_SECTOR_COUNT) % Data::SPACE_SECTOR_COUNT;

	Data::SpaceMapCell &oldCell = _G(savegame)._starmap._sectors[_G(savegame)._sectorX][_G(savegame)._sectorY];
	Data::SpaceMapCell &newCell = _G(savegame)._starmap._sectors[newSectorX][newSectorY];

	// Find a free ship slot in the destination sector to arrive in
	int freeSlot = -1;
	for (int i = 0; i < Data::SPACE_SHIPS_PER_SECTOR; ++i) {
		if (newCell._ships[i]._shipType == Data::SHIP_NONE) {
			freeSlot = i;
			break;
		}
	}

	resetLine();
	if (freeSlot == -1) {
		writeString("No room to arrive in that sector!\n");
	} else {
		// Move the player's ship record into the new sector's slot, vacating
		// the old one, and reposition to the same fixed point space entry uses
		Data::SpaceMapShip ship = oldCell._ships[_G(savegame)._shipIndex];
		oldCell._ships[_G(savegame)._shipIndex] = Data::SpaceMapShip();

		ship._x = 260;
		ship._y = 90;
		ship._facing = Data::FACING_LEFT;
		newCell._ships[freeSlot] = ship;

		_G(savegame)._sectorX = (int16)newSectorX;
		_G(savegame)._sectorY = (int16)newSectorY;
		_G(savegame)._shipIndex = (int16)freeSlot;

		writeString("\x10HyperJump completed.\n");
	}

	redrawMap();
	redrawStats();
	prompt();
}

void SpaceCockpitLogic::abortHyperjump() {
	resetLine();
	writeString("Hyperjump aborted!\n");
	redrawMap();
	prompt();
}

bool SpaceCockpitLogic::view() {
	writeString("View\n");
	_G(logic) = Common::SharedPtr<Logic>(new SpaceMapLogic());
	g_engine->replaceView("SpaceMap");
	return true;
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
