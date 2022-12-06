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

#ifndef MM1_MAPS_MAP31_H
#define MM1_MAPS_MAP31_H

#include "mm/mm1/maps/map_desert.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map31 : public MapDesert {
	typedef void (Map31:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special02();
	void special06();

	const SpecialFn SPECIAL_FN[7] = {
		&Map31::special00,
		&Map31::special01,
		&Map31::special02,
		&Map31::special02,
		&Map31::special02,
		&Map31::special02,
		&Map31::special06
	};
public:
	Map31() : MapDesert(31, "areae2", 0x706, 2, 80, MapDesert::RND_BASIC) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;

	/**
	 * Starts an encounter
	 */
	void encounter();

	/**
	 * Called if you attack the alien
	 */
	void hostile();

	/**
	 * Called if you specify neutral for alien
	 */
	void neutral();

	/**
	 * Called if you select to act friendly to the alien
	 */
	void friendly();
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
