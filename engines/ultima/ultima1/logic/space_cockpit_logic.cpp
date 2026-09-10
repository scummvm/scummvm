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
}

void SpaceCockpitLogic::keypress(Common::KeyCode keycode) {
	if (keycode >= Common::KEYCODE_1 && keycode <= Common::KEYCODE_8) {
		setSpeed(keycode - Common::KEYCODE_0);
		return;
	}

	SpaceLogic::keypress(keycode);
}

bool SpaceCockpitLogic::move(Data::Direction dir) {
	writeString("%s\n", Data::SPACE_COCKPIT_DIRECTION_NAMES[dir]);

	if (shipFuel() < 2) {
		writeString("Not enough fuel!\n");
		return true;
	}
	subtractFuel(2);

	// TODO: set the starfield pan velocity (viewPanDeltaX/Y) - Left/Right
	// pan X by -/+4, Climb/Dive pan Y by -/+4 - which the cockpit view's
	// per-frame starfield update then applies
	return true;
}

bool SpaceCockpitLogic::fire() {
	// TODO: handleFireCommand - the actual enemy-ship combat
	writeString("Fire!\n");
	playFX(1);
	return true;
}

bool SpaceCockpitLogic::pass() {
	// TODO: re-centres the starscape (clears the pan velocity, recentres
	// viewCenterX/Y, redraws the viewport border)
	writeString("Center\n");
	return true;
}

bool SpaceCockpitLogic::view() {
	writeString("View\n");
	_G(logic) = Common::SharedPtr<Logic>(new SpaceMapLogic());
	g_engine->replaceView("SpaceMap");
	return false;
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
