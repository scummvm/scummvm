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

#include "ultima/ultima1/widgets/person.h"
#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/ultima1/widgets/guard.h"
#include "ultima/ultima1/widgets/princess.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

EMPTY_MESSAGE_MAP(Person, UrbanWidget);

bool Person::areGuardsHostile() const {
	Maps::MapCityCastle *cityCastle = static_cast<Maps::MapCityCastle *>(_map);
	return cityCastle->_guardsHostile;
}

int Person::getRandomDelta() const {
	return _game->getRandomNumber(2) - 1;
}

void Person::synchronize(Common::Serializer &s) {
	UrbanWidget::synchronize(s);
	Creature::synchronize(s);
}

bool Person::subtractHitPoints(uint amount) {
	bool result = Shared::Maps::Creature::subtractHitPoints(amount);
	if (result) {
		// Common code for gaining experience for killing different kinds of people
		Shared::Character &c = *_game->_party;
		if (dynamic_cast<Widgets::Princess *>(this) == nullptr)
			c._experience++;
		if (dynamic_cast<Widgets::Guard *>(this))
			c._experience += 14;
	}

	return result;
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
