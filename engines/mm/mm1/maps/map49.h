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

#ifndef MM1_MAPS_MAP49_H
#define MM1_MAPS_MAP49_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map49 : public Map {
	typedef void (Map49:: *SpecialFn)();
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
	void special16();
	void special20();
	void special22();
	void messageEncounter(const Common::String &line);

	const SpecialFn SPECIAL_FN[23] = {
		&Map49::special00,
		&Map49::special01,
		&Map49::special02,
		&Map49::special03,
		&Map49::special04,
		&Map49::special05,
		&Map49::special06,
		&Map49::special07,
		&Map49::special08,
		&Map49::special09,
		&Map49::special08,
		&Map49::special09,
		&Map49::special08,
		&Map49::special09,
		&Map49::special08,
		&Map49::special09,
		&Map49::special16,
		&Map49::special16,
		&Map49::special08,
		&Map49::special09,
		&Map49::special20,
		&Map49::special20,
		&Map49::special22
	};
public:
	Map49() : Map(49, "alamar", 0xb07, 3, "Castle Alamar") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
