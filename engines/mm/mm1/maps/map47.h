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

#ifndef MM1_MAPS_MAP47_H
#define MM1_MAPS_MAP47_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map47 : public Map {
	typedef void (Map47:: *SpecialFn)();
private:
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
	void special10();
	void special11();
	void special12();
	void special13();
	void special14();
	void special15();
	void special16();
	void special17();
	void special23();

	const SpecialFn SPECIAL_FN[26] = {
		&Map47::special00,
		&Map47::special01,
		&Map47::special02,
		&Map47::special03,
		&Map47::special04,
		&Map47::special05,
		&Map47::special06,
		&Map47::special07,
		&Map47::special08,
		&Map47::special09,
		&Map47::special10,
		&Map47::special11,
		&Map47::special12,
		&Map47::special13,
		&Map47::special14,
		&Map47::special15,
		&Map47::special16,
		&Map47::special17,
		&Map47::special01,
		&Map47::special01,
		&Map47::special01,
		&Map47::special01,
		&Map47::special01,
		&Map47::special23,
		&Map47::special23,
		&Map47::special23
	};
public:
	Map47() : Map(47, "udrag3", 0x705, 3, "Dragadune Ruins 4") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;

	void poof();
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
