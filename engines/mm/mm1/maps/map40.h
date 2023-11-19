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

#ifndef MM1_MAPS_MAP40_H
#define MM1_MAPS_MAP40_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map40 : public Map {
	typedef void (Map40:: *SpecialFn)();
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
	void special18();
	void special19();
	void special20();

	const SpecialFn SPECIAL_FN[23] = {
		&Map40::special00,
		&Map40::special01,
		&Map40::special02,
		&Map40::special03,
		&Map40::special04,
		&Map40::special05,
		&Map40::special06,
		&Map40::special07,
		&Map40::special08,
		&Map40::special09,
		&Map40::special10,
		&Map40::special11,
		&Map40::special12,
		&Map40::special13,
		&Map40::special14,
		&Map40::special15,
		&Map40::special16,
		&Map40::special17,
		&Map40::special18,
		&Map40::special19,
		&Map40::special20,
		&Map40::special02,
		&Map40::special02
	};
public:
	Map40() : Map(40, "rwl2", 0x702, 3, "Warrior's Stronghold 2") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;

	/**
	 * Called when party chooses to resist Lord Archer
	 */
	void archerResist();

	/**
	 * Called when party chooses to submit to Lord Archer
	 */
	void archerSubmit();
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
