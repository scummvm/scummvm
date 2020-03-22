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

#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/game/screen.h"
#include "ultima/ultima4/game/game.h"
#include "common/algorithm.h"

namespace Ultima {
namespace Ultima4 {

using namespace Std;

bool Object::setDirection(Direction d) {
	return _tile.setDirection(d);
}

void Object::setMap(class Map *m) {
	if (Common::find(_maps.begin(), _maps.end(), m) == _maps.end())
		_maps.push_back(m);
}

Map *Object::getMap() {
	if (_maps.empty())
		return NULL;
	return _maps.back();
}

void Object::remove() {
	unsigned int size = _maps.size();
	for (unsigned int i = 0; i < size; i++) {
		if (i == size - 1)
			_maps[i]->removeObject(this);
		else _maps[i]->removeObject(this, false);
	}
}

void Object::animateMovement() {
	//TODO abstract movement - also make screen.h and game.h not required
	screenTileUpdate(&g_game->_mapArea, _prevCoords, false);
	if (screenTileUpdate(&g_game->_mapArea, _coords, false))
		screenWait(1);
}

} // End of namespace Ultima4
} // End of namespace Ultima
