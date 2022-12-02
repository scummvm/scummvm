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

#ifndef MM1_MAPS_MAP07_H
#define MM1_MAPS_MAP07_H

#include "mm/mm1/maps/map.h"
#include "mm/mm1/data/character.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map07 : public Map {
	typedef void (Map07:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special02();
	void special03();
	void special04();
	void special05();
	void special07();
	void special09();
	void special11();
	void special13();
	void special14();
	void special15();
	void special16();
	void special17();
	void setMonsters(int id1 = 10, int id2 = 1);
	void poolYN(YNCallback callback);
	void applyCondition(Condition cond);

	const SpecialFn SPECIAL_FN[18] = {
		&Map07::special00,
		&Map07::special01,
		&Map07::special02,
		&Map07::special03,
		&Map07::special04,
		&Map07::special05,
		&Map07::special05,
		&Map07::special07,
		&Map07::special07,
		&Map07::special09,
		&Map07::special09,
		&Map07::special11,
		&Map07::special11,
		&Map07::special13,
		&Map07::special14,
		&Map07::special15,
		&Map07::special16,
		&Map07::special17
	};
public:
	Map07() : Map(7, "cave3", 0xC01, 1) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
