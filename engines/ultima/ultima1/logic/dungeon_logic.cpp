
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

#include "ultima/ultima1/logic/dungeon_logic.h"
#include "ultima/ultima1/metaengine.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

DungeonLogic::DungeonLogic() {
	_G(map)._mapType = Data::MAPTYPE_DUNGEON;
}

void DungeonLogic::entering() {
	_G(savegame)._locationPosition = Common::Point(1, 1);
	_G(savegame)._dungeonLevel = 1;
	_G(savegame)._dungeonDirection = 4;
	_G(savegame)._dungeonExitHitPoints = 0;
}

bool DungeonLogic::move(Data::Direction dir) {
	return true;
}

void DungeonLogic::updateCreatures() {
	// TODO
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
