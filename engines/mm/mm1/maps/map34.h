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

#ifndef MM1_MAPS_MAP34_H
#define MM1_MAPS_MAP34_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map34 : public Map {
	typedef void (Map34:: *SpecialFn)();
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
	void special12();
	void special13();
	void special16();
	void special17();
	void special18();
	void special19();
	void special20();
	void special21();

	const SpecialFn SPECIAL_FN[23] = {
		&Map34::special00,
		&Map34::special01,
		&Map34::special02,
		&Map34::special03,
		&Map34::special04,
		&Map34::special05,
		&Map34::special06,
		&Map34::special07,
		&Map34::special08,
		&Map34::special09,
		&Map34::special10,
		&Map34::special10,
		&Map34::special12,
		&Map34::special13,
		&Map34::special13,
		&Map34::special13,
		&Map34::special16,
		&Map34::special17,
		&Map34::special18,
		&Map34::special19,
		&Map34::special20,
		&Map34::special21,
		&Map34::special01
	};
public:
	Map34() : Map(34, "doom", 0x706, 3, "Castle Doom") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
