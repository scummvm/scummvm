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

#ifndef MM1_MAPS_MAP41_H
#define MM1_MAPS_MAP41_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map41 : public Map {
	typedef void (Map41:: *SpecialFn)();
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
	void special16();
	void special17();
	void special18();
	void special19();
	void special20();
	void special21();
	void special22();
	void special23();
	void showSign(const Common::String &line);

	const SpecialFn SPECIAL_FN[25] = {
		&Map41::special00,
		&Map41::special01,
		&Map41::special02,
		&Map41::special03,
		&Map41::special04,
		&Map41::special05,
		&Map41::special06,
		&Map41::special07,
		&Map41::special08,
		&Map41::special09,
		&Map41::special10,
		&Map41::special11,
		&Map41::special12,
		&Map41::special13,
		&Map41::special14,
		&Map41::special14,
		&Map41::special16,
		&Map41::special17,
		&Map41::special18,
		&Map41::special19,
		&Map41::special20,
		&Map41::special21,
		&Map41::special22,
		&Map41::special23,
		&Map41::special04
	};
public:
	Map41() : Map(41, "enf1", 0xf04, 3, "Minotaur Stronghold 1") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
