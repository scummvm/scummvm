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

#ifndef MM1_MAPS_MAP23_H
#define MM1_MAPS_MAP23_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map23 : public Map {
	typedef void (Map23:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special02();
	void special03();
	void special08();
	void special09();
	void special10();
	void special11();
	void special12();
	void special13();
	void fountain();

	const SpecialFn SPECIAL_FN[14] = {
		&Map23::special00,
		&Map23::special01,
		&Map23::special02,
		&Map23::special03,
		&Map23::special03,
		&Map23::special03,
		&Map23::special03,
		&Map23::special03,
		&Map23::special08,
		&Map23::special09,
		&Map23::special10,
		&Map23::special11,
		&Map23::special12,
		&Map23::special13
	};
public:
	Map23() : Map(23, "areac2", 0xa11, 1) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
