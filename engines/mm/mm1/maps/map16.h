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

#ifndef MM1_MAPS_MAP16_H
#define MM1_MAPS_MAP16_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map16 : public Map {
	typedef void (Map16:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special02();
	void special03();

	const SpecialFn SPECIAL_FN[8] = {
		&Map16::special00,
		&Map16::special01,
		&Map16::special02,
		&Map16::special03,
		&Map16::special03,
		&Map16::special03,
		&Map16::special03,
		&Map16::special03
	};
public:
	Map16() : Map(16, "areaa3", 0xb02, 2) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;

	/**
	 * Spins the wheel and grants each party member a result
	 */
	void wheelSpin();
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
