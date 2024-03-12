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

#ifndef MM1_MAPS_MAP45_H
#define MM1_MAPS_MAP45_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map45 : public Map {
	typedef void (Map45:: *SpecialFn)();
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
	void special18();
	void special19();
	void special20();
	void special21();
	void showSign(const Common::String &line);

	const SpecialFn SPECIAL_FN[22] = {
		&Map45::special00,
		&Map45::special01,
		&Map45::special02,
		&Map45::special03,
		&Map45::special04,
		&Map45::special05,
		&Map45::special06,
		&Map45::special07,
		&Map45::special08,
		&Map45::special09,
		&Map45::special10,
		&Map45::special11,
		&Map45::special12,
		&Map45::special13,
		&Map45::special14,
		&Map45::special15,
		&Map45::special16,
		&Map45::special16,
		&Map45::special18,
		&Map45::special19,
		&Map45::special20,
		&Map45::special21
	};
public:
	Map45() : Map(45, "udrag1", 0xf05, 3, "Dragadune Ruins 2") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
