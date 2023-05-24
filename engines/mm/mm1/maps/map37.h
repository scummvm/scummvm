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

#ifndef MM1_MAPS_MAP37_H
#define MM1_MAPS_MAP37_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map37 : public Map {
	typedef void (Map37:: *SpecialFn)();
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
	void special19();

	const SpecialFn SPECIAL_FN[20] = {
		&Map37::special00,
		&Map37::special01,
		&Map37::special02,
		&Map37::special03,
		&Map37::special04,
		&Map37::special05,
		&Map37::special06,
		&Map37::special07,
		&Map37::special08,
		&Map37::special09,
		&Map37::special10,
		&Map37::special11,
		&Map37::special12,
		&Map37::special13,
		&Map37::special14,
		&Map37::special15,
		&Map37::special16,
		&Map37::special16,
		&Map37::special16,
		&Map37::special19
	};
public:
	Map37() : Map(37, "qvl1", 0xf03, 3, "Wizard's Lair 1") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
