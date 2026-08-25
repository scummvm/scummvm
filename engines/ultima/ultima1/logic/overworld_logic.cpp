
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

#include "ultima/ultima1/logic/overworld_logic.h"
#include "ultima/ultima1/core/strings.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

const int8 DELTA_X[5] = { 0, -1, 1, 0, 0 };
const int8 DELTA_Y[5] = { 0, 0, 0, -1, 1 };

void OverworldLogic::action(int action) {
	switch (action) {
	case KEYBIND_UP:
		move(Data::DIR_UP);
		break;
	case KEYBIND_DOWN:
		move(Data::DIR_DOWN);
		break;
	case KEYBIND_LEFT:
		move(Data::DIR_LEFT);
		break;
	case KEYBIND_RIGHT:
		move(Data::DIR_RIGHT);
		break;
	default:
		break;
	}
}

void OverworldLogic::keypress(Common::KeyCode keycode) {

}

void OverworldLogic::move(Data::Direction dir) {
	int xp = getViewportX(Data::MAP_VISIBLE_CENTER_X + DELTA_X[dir]);
	int yp = getViewportY(Data::MAP_VISIBLE_CENTER_Y + DELTA_Y[dir]);
	int tile = getTileAt(xp, yp, 1);
	int transport = g_engine->_player._transportType;

	if (tile >= Data::TILE_FIRST_MONSTER && tile <= Data::TILE_LAST_MONSTER) {
		// TODO
	} else if (transport == Data::TRANSPORT_RAFT) {

	}
}

bool OverworldLogic::moveCheck(Data::Direction dir) {
	return false;
}

int OverworldLogic::getViewportX(int xp) const {
	int x = g_engine->_player._position.x - Data::MAP_VISIBLE_CENTER_X + xp;
	if (x < 0)
		x += Data::OVERWORLD_WIDTH;
	else if (x >= Data::OVERWORLD_WIDTH)
		x -= Data::OVERWORLD_WIDTH;

	return x;
}

int OverworldLogic::getViewportY(int yp) const {
	int y = g_engine->_player._position.y - Data::MAP_VISIBLE_CENTER_Y + yp;
	if (y < 0)
		y += Data::OVERWORLD_HEIGHT;
	else if (y >= Data::OVERWORLD_HEIGHT)
		y -= Data::OVERWORLD_HEIGHT;

	return y;
}

int OverworldLogic::getTileAt(int x, int y, int creatureIndex) const {
	int entityIndex = g_engine->_player.getEntityAt(x, y, creatureIndex);

	if (entityIndex >= 0)
		return g_engine->_player._overworldEntities[entityIndex]._type;

	return g_engine->_map.getMapTile(x, y);
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
