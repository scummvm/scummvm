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

#ifndef MM1_MAPS_MAP51_H
#define MM1_MAPS_MAP51_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map51 : public Map {
	typedef void (Map51:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special02();
	void special03();
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
	void special17();
	void special18();
	void special19();

	const SpecialFn SPECIAL_FN[20] = {
		&Map51::special00,
		&Map51::special01,
		&Map51::special02,
		&Map51::special03,
		&Map51::special01,
		&Map51::special05,
		&Map51::special06,
		&Map51::special07,
		&Map51::special08,
		&Map51::special09,
		&Map51::special10,
		&Map51::special11,
		&Map51::special12,
		&Map51::special13,
		&Map51::special14,
		&Map51::special01,
		&Map51::special01,
		&Map51::special17,
		&Map51::special18,
		&Map51::special19
	};
public:
	Map51() : Map(51, "pp2", 0x701, 3, "Building of Gold 2") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
