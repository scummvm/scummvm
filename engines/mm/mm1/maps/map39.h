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

#ifndef MM1_MAPS_MAP39_H
#define MM1_MAPS_MAP39_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map39 : public Map {
	typedef void (Map39:: *SpecialFn)();
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
	void special18();

	const SpecialFn SPECIAL_FN[19] = {
		&Map39::special00,
		&Map39::special01,
		&Map39::special02,
		&Map39::special03,
		&Map39::special04,
		&Map39::special05,
		&Map39::special06,
		&Map39::special07,
		&Map39::special08,
		&Map39::special09,
		&Map39::special10,
		&Map39::special11,
		&Map39::special12,
		&Map39::special13,
		&Map39::special14,
		&Map39::special05,
		&Map39::special05,
		&Map39::special05,
		&Map39::special18
	};
public:
	Map39() : Map(39, "rwl1", 0xf02, 3, "Warrior's Stronghold 1") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;

	void riddleAnswered(const Common::String &answer);
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
