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

#include "ultima/ultima1/widgets/urban_player.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/game.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

void UrbanPlayer::moveTo(const Point &destPos, Shared::Maps::Direction dir) {
	Person::moveTo(destPos, dir);
	Shared::Maps::Map *map = _game->getMap();

	if (destPos.x < 0 || destPos.y < 0 || destPos.x >= (int)map->width() || destPos.y >= (int)map->height()) {
		// Handling for leaving locations by walking off the edge of the map
		if (isPrincessSaved())
			princessSaved();

		// Load the overworld map
		map->load(Maps::MAP_OVERWORLD);
	}
}

bool UrbanPlayer::isPrincessSaved() const {
	return false;
}

void UrbanPlayer::princessSaved() {
	// TODO
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
