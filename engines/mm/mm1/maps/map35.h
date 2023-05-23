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

#ifndef MM1_MAPS_MAP35_H
#define MM1_MAPS_MAP35_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map35 : public Map {
	typedef void (Map35:: *SpecialFn)();
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
	void updateFlags();

	const SpecialFn SPECIAL_FN[11] = {
		&Map35::special00,
		&Map35::special01,
		&Map35::special02,
		&Map35::special03,
		&Map35::special04,
		&Map35::special05,
		&Map35::special06,
		&Map35::special07,
		&Map35::special07,
		&Map35::special09,
		&Map35::special09
	};
public:
	Map35() : Map(35, "blackrn", 0xf08, 3, "Castle Blackridge North") {}

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
