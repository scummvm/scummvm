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

#ifndef MM1_MAPS_MAP25_H
#define MM1_MAPS_MAP25_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map25 : public Map {
	typedef void (Map25:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special02();
	void nativesAttack();

	const SpecialFn SPECIAL_FN[3] = {
		&Map25::special00,
		&Map25::special01,
		&Map25::special02
	};
public:
	Map25() : Map(25, "areac4", 0xf04, 2) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
