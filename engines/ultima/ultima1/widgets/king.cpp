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

#include "ultima/ultima1/widgets/king.h"
#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/u1dialogs/king.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

EMPTY_MESSAGE_MAP(King, Person);

void King::movement() {
}

bool King::subtractHitPoints(uint amount) {
	Maps::MapCastle *map = static_cast<Maps::MapCastle *>(_map);

	// Lord British is invincible, the nasty git
	return map->isLordBritishCastle() ? false : Person::subtractHitPoints(amount);
}

void King::talk() {
	if (areGuardsHostile()) {
		addInfoMsg(_game->_res->HE_REJECTS_OFFER);
		_game->endOfTurn();
	
	} else {
		U1Dialogs::King *dialog = new U1Dialogs::King(_game, _map->getMapIndex());
		dialog->show();
	}
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
