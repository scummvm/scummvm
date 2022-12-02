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

#ifndef MM1_MAPS_MAP13_H
#define MM1_MAPS_MAP13_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map13 : public Map {
	typedef void (Map13:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special02();
	void special06();
	void special10();
	void special18();
	void special22();

	const SpecialFn SPECIAL_FN[23] = {
		&Map13::special00,
		&Map13::special01,
		&Map13::special02,
		&Map13::special02,
		&Map13::special02,
		&Map13::special02,
		&Map13::special06,
		&Map13::special06,
		&Map13::special06,
		&Map13::special06,
		&Map13::special10,
		&Map13::special10,
		&Map13::special10,
		&Map13::special10,
		&Map13::special10,
		&Map13::special10,
		&Map13::special10,
		&Map13::special10,
		&Map13::special18,
		&Map13::special18,
		&Map13::special18,
		&Map13::special18,
		&Map13::special22
	};
public:
	Map13() : Map(13, "cave9", 0xa00, 2) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;

	/**
	 * Start an encounter
	 */
	void encounter(size_t count, byte id1, byte id2);
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
