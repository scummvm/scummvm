
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

#include "ultima/ultima1/logic/locations_logic.h"
#include "ultima/ultima1/metaengine.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

void LocationsLogic::action(int action) {
	// TODO
}

void LocationsLogic::keypress(Common::KeyCode keycode) {
	action(KEYBIND_PASS);
}

/*-------------------------------------------------------------------*/

void CityLogic::enter() {
	_G(savegame)._locationPosition = Common::Point(19, 17);
}

/*-------------------------------------------------------------------*/

void CastleLogic::enter() {
	_G(savegame)._locationPosition = Common::Point(0, 9);
}

/*-------------------------------------------------------------------*/

void PillarLogic::enter() {
	//_G(savegame)._locationPosition = Common::Point(0, 9);
}

/*-------------------------------------------------------------------*/

void DungeonLogic::enter() {

}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
