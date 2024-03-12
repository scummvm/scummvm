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

#ifndef MM1_MAPS_MAP53_H
#define MM1_MAPS_MAP53_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map53 : public Map {
	typedef void (Map53:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special03();
	void special04();
	void special05();
	void special06();
	void special07();
	void special08();
	void special09();
	void special10();
	void special11();
	void special12();
	void special13();
	void special14();

	const SpecialFn SPECIAL_FN[15] = {
		&Map53::special00,
		&Map53::special01,
		&Map53::special01,
		&Map53::special03,
		&Map53::special04,
		&Map53::special05,
		&Map53::special06,
		&Map53::special07,
		&Map53::special08,
		&Map53::special09,
		&Map53::special10,
		&Map53::special11,
		&Map53::special12,
		&Map53::special13,
		&Map53::special14
	};
public:
	Map53() : Map(53, "pp4", 0x201, 3, "Building of Gold 4") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
