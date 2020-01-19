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

#ifndef ULTIMA_ULTIMA1_WIDGETS_MERCHANT_H
#define ULTIMA_ULTIMA1_WIDGETS_MERCHANT_H

#include "ultima/ultima1/widgets/person.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

class Merchant : public Person {
public:
	/**
	 * Constructor
	 */
	Merchant(Ultima1Game *game, Map::Ultima1Map::MapBase *map, int hitPoints) :
		Person(game, map, 50, hitPoints) {}
	Merchant(Ultima1Game *game, Map::Ultima1Map::MapBase *map, uint tileNum, int hitPoints) :
		Person(game, map, tileNum, hitPoints) {}

	/**
	 * Destructor
	 */
	virtual ~Merchant() {}
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
