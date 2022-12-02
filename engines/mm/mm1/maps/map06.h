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

#ifndef MM1_MAPS_MAP06_H
#define MM1_MAPS_MAP06_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map06 : public Map {
	typedef void (Map06:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special02();
	void special03();
	void special04();
	void special06();
	void special07();
	void special09();
	void special10();
	void special13();
	void special26();
	void slide();

	const SpecialFn SPECIAL_FN[27] = {
		&Map06::special00,
		&Map06::special01,
		&Map06::special02,
		&Map06::special03,
		&Map06::special04,
		&Map06::special04,
		&Map06::special06,
		&Map06::special07,
		&Map06::special07,
		&Map06::special09,
		&Map06::special10,
		&Map06::special10,
		&Map06::special10,
		&Map06::special13,
		&Map06::special13,
		&Map06::special13,
		&Map06::special13,
		&Map06::special13,
		&Map06::special13,
		&Map06::special13,
		&Map06::special13,
		&Map06::special13,
		&Map06::special13,
		&Map06::special13,
		&Map06::special13,
		&Map06::special13,
		&Map06::special26
	};
public:
	Map06() : Map(6, "cave2", 0x1, 1) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
