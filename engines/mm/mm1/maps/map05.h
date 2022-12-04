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

#ifndef MM1_MAPS_MAP05_H
#define MM1_MAPS_MAP05_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map05 : public Map {
	typedef void (Map05:: *SpecialFn)();
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
	void special14();
	void encounter(int monsterId);
	void showMessage(const Common::String &msg);
	static bool addScroll();
	static bool hasFlag();
	static void addFlag();

	const SpecialFn SPECIAL_FN[21] = {
		&Map05::special00,
		&Map05::special01,
		&Map05::special02,
		&Map05::special03,
		&Map05::special04,
		&Map05::special05,
		&Map05::special06,
		&Map05::special07,
		&Map05::special08,
		&Map05::special09,
		&Map05::special10,
		&Map05::special11,
		&Map05::special11,
		&Map05::special11,
		&Map05::special14,
		&Map05::special14,
		&Map05::special14,
		&Map05::special14,
		&Map05::special14,
		&Map05::special14,
		&Map05::special08
	};
public:
	Map05() : Map(5, "cave1", 0xa11, 1) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
