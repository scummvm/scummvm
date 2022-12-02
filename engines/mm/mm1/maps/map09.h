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

#ifndef MM1_MAPS_MAP09_H
#define MM1_MAPS_MAP09_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map09 : public Map {
	typedef void (Map09:: *SpecialFn)();
private:
	int _portalIndex = 0;

	void special00();
	void special01();
	void special02();
	void special03();
	void special04();
	void special05();
	void special06();
	void special07();
	void special08();
	void special09();
	void special14();
	void special18();
	void special25();
	void special26();
	void special27();
	void portal(int index);

	const SpecialFn SPECIAL_FN[28] = {
		&Map09::special00,
		&Map09::special01,
		&Map09::special02,
		&Map09::special03,
		&Map09::special04,
		&Map09::special05,
		&Map09::special06,
		&Map09::special07,
		&Map09::special08,
		&Map09::special09,
		&Map09::special09,
		&Map09::special09,
		&Map09::special09,
		&Map09::special09,
		&Map09::special14,
		&Map09::special14,
		&Map09::special14,
		&Map09::special14,
		&Map09::special18,
		&Map09::special18,
		&Map09::special18,
		&Map09::special18,
		&Map09::special18,
		&Map09::special18,
		&Map09::special18,
		&Map09::special25,
		&Map09::special26,
		&Map09::special27
	};
public:
	Map09() : Map(9, "cave5", 0x5, 1) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
