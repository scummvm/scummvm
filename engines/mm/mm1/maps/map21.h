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

#ifndef MM1_MAPS_MAP21_H
#define MM1_MAPS_MAP21_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map21 : public Map {
	typedef void (Map21:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special02();
	void special03();
	void special04();

	const SpecialFn SPECIAL_FN[8] = {
		&Map21::special00,
		&Map21::special01,
		&Map21::special02,
		&Map21::special03,
		&Map21::special04,
		&Map21::special04,
		&Map21::special04,
		&Map21::special04
	};

	void askTrivia(int questionNum);
public:
	Map21() : Map(21, "areab4", 0xd03, 1) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
