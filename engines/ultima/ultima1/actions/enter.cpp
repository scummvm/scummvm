/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima1/actions/enter.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/map.h"
#include "ultima/ultima1/core/resources.h"

namespace Ultima {
namespace Ultima1 {
namespace Actions {

BEGIN_MESSAGE_MAP(Enter, Action)
	ON_MESSAGE(EnterMsg)
END_MESSAGE_MAP()

bool Enter::EnterMsg(CEnterMsg &msg) {
	Ultima1Game *game = getRoot();
	Ultima1Map *map = getMap();
	U1MapTile mapTile;

	map->getTileAt(map->getPosition(), &mapTile);
	
	if (mapTile._locationNum == -1) {
		addStatusMsg(game->_res->ENTER_QUESTION);
		playFX(1);
	} else {
		// Load the location
		map->loadMap(mapTile._locationNum, getGameState()->_videoMode);

		// Add message for location having been entered
		addStatusMsg(game->_res->ENTERING);
		addStatusMsg(map->_name);
	}

	return true;
}

} // End of namespace Actions
} // End of namespace Ultima1
} // End of namespace Ultima
