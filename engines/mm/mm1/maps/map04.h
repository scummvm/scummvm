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

#ifndef MM1_MAPS_MAP04_H
#define MM1_MAPS_MAP04_H

#include "mm/mm1/maps/map_town.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define MAP04_PASSAGE_OVERRIDE 169
#define MAP04_STAIRS_OVERRIDE 196
#define MAP04_TREASURE_STOLEN 971

class Map04 : public MapTown {
	typedef void (Map04:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special02();
	void special03();
	void special04();
	void special05();
	void special06();
	void special07();
	void special09();
	void special10();
	void special11();
	void special12();
	void special13();
	void special18();
	void special20();
	void special21();

	const SpecialFn SPECIAL_FN[22] = {
		&Map04::special00,
		&Map04::special01,
		&Map04::special02,
		&Map04::special03,
		&Map04::special04,
		&Map04::special05,
		&Map04::special06,
		&Map04::special07,
		&Map04::special08,
		&Map04::special09,
		&Map04::special10,
		&Map04::special11,
		&Map04::special12,
		&Map04::special13,
		&Map04::special13,
		&Map04::special13,
		&Map04::special13,
		&Map04::special13,
		&Map04::special18,
		&Map04::special13,
		&Map04::special20,
		&Map04::special21
	};
public:
	Map04() : MapTown(4, "erliquin", 0xb1a, 3) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;

	void special08();
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
