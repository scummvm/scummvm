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

#ifndef MM1_MAPS_MAP43_H
#define MM1_MAPS_MAP43_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map43 : public Map {
	typedef void (Map43:: *SpecialFn)();
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
	void updateFlags();

	const SpecialFn SPECIAL_FN[9] = {
		&Map43::special00,
		&Map43::special01,
		&Map43::special02,
		&Map43::special03,
		&Map43::special04,
		&Map43::special05,
		&Map43::special06,
		&Map43::special07,
		&Map43::special08
	};
public:
	Map43() : Map(43, "whitew", 0xa11, 3, "Castle White Wolf") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;

	/**
	 * Accepts a quest from Inspectron
	 */
	void acceptQuest();

	/**
	 * Does a check for whether Inspectron's current quest
	 * is complete or not
	 */
	Common::String checkQuestComplete();
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
