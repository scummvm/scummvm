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

#ifndef MM1_MAPS_MAP02_H
#define MM1_MAPS_MAP02_H

#include "mm/mm1/maps/map_town.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map02 : public MapTown {
	typedef void (Map02:: *SpecialFn)();
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
	void special18();
	void special20();
	void special21();

	const SpecialFn SPECIAL_FN[22] = {
		&Map02::special00,
		&Map02::special01,
		&Map02::special02,
		&Map02::special03,
		&Map02::special04,
		&Map02::special05,
		&Map02::special06,
		&Map02::special07,
		&Map02::special08,
		&Map02::special09,
		&Map02::special10,
		&Map02::special11,
		&Map02::special12,
		&Map02::special13,
		&Map02::special14,
		&Map02::special15,
		&Map02::special15,
		&Map02::special15,
		&Map02::special18,
		&Map02::special15,
		&Map02::special20,
		&Map02::special21
	};
public:
	Map02() : MapTown(2, "algary", 0x203, 1) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
