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

#include "ultima/ultima1/widgets/merchant.h"
#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

EMPTY_MESSAGE_MAP(Merchant, Person);

bool Merchant::checkCuaghtStealing() {
	Shared::Character &c = *_game->_party;
	int randVal = _game->getRandomNumber(1, 255);
	bool flag = areGuardsHostile() || randVal < 38;

	if (!flag && c._class == CLASS_THIEF)
		return false;
	if (!flag && randVal > 77)
		return false;

	addInfoMsg("");
	addInfoMsg(_game->_res->CAUGHT);
	static_cast<Maps::MapCityCastle *>(_map)->_guardsHostile = true;
	return true;
}

void Merchant::noKingsPermission() {
	addInfoMsg("");
	addInfoMsg(_game->_res->NO_KINGS_PERMISSION);
	_game->playFX(1);
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
