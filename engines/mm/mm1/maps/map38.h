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

#ifndef MM1_MAPS_MAP38_H
#define MM1_MAPS_MAP38_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map38 : public Map {
	typedef void (Map38:: *SpecialFn)();
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

	const SpecialFn SPECIAL_FN[19] = {
		&Map38::special00,
		&Map38::special01,
		&Map38::special02,
		&Map38::special03,
		&Map38::special04,
		&Map38::special05,
		&Map38::special06,
		&Map38::special07,
		&Map38::special08,
		&Map38::special09,
		&Map38::special10,
		&Map38::special11,
		&Map38::special12,
		&Map38::special13,
		&Map38::special14,
		&Map38::special15,
		&Map38::special16,
		&Map38::special17,
		&Map38::special18
	};
public:
	Map38() : Map(38, "qvl2", 0x703, 3, "Wizard's Lair 2") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
