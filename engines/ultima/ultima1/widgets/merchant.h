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

/**
 * Base class for merchants in the cities and castles
 */
class Merchant : public Person {
	DECLARE_MESSAGE_MAP;
protected:
	/**
	 * Checks whether the player is caught stealing, and if so, makes the guards hostile
	 */
	bool checkCuaghtStealing();

	/**
	 * Adds a response that you don't have the king's permission
	 */
	void noKingsPermission();
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	Merchant(Ultima1Game *game, Maps::MapBase *map, int hitPoints) :
		Person(game, map, 50, hitPoints) {}

	/**
	 * Constructor
	 */
	Merchant(Ultima1Game *game, Maps::MapBase *map, uint tileNum, int hitPoints) :
		Person(game, map, tileNum, hitPoints) {}

	/**
	 * Constructor
	 */
	Merchant(Ultima1Game *game, Maps::MapBase *map) : Person(game, map, 50) {}

	/**
	 * Does the get action
	 */
	virtual void get() {}

	/**
	 * Does the steal action
	 */
	virtual void steal() {}
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
