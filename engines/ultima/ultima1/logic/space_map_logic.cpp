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

#include "ultima/ultima1/logic/space_map_logic.h"
#include "ultima/ultima1/logic/space_cockpit_logic.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

bool SpaceMapLogic::move(Data::Direction dir) {
	writeString("%s\n", Data::DIRECTION_NAMES[dir]);

	// TODO: handleOverheadArrows - nudges the drift heading (sectorChangeX/Y)
	// by 1, spends 5 fuel (2 for Left/Right), "No fuel!" when empty, and
	// "Crunch!" + shield drain on a collision. The drift itself and the
	// sector crossing run in the overhead view's per-frame update
	return true;
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
