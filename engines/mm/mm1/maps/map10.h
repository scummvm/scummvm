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

#ifndef MM1_MAPS_MAP10_H
#define MM1_MAPS_MAP10_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map10 : public Map {
	typedef void (Map10:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special02();
	void special17();
	void special18();
	void special19();
	void special20();
	void special23();
	void special29();
	void special30();
	void special31();
	void special32();
	void special33();

	const SpecialFn SPECIAL_FN[34] = {
		&Map10::special00,
		&Map10::special01,
		&Map10::special02,
		&Map10::special02,
		&Map10::special02,
		&Map10::special02,
		&Map10::special02,
		&Map10::special02,
		&Map10::special02,
		&Map10::special02,
		&Map10::special02,
		&Map10::special02,
		&Map10::special02,
		&Map10::special02,
		&Map10::special02,
		&Map10::special02,
		&Map10::special02,
		&Map10::special17,
		&Map10::special18,
		&Map10::special19,
		&Map10::special20,
		&Map10::special20,
		&Map10::special20,
		&Map10::special23,
		&Map10::special23,
		&Map10::special23,
		&Map10::special23,
		&Map10::special23,
		&Map10::special23,
		&Map10::special29,
		&Map10::special30,
		&Map10::special31,
		&Map10::special32,
		&Map10::special33
	};
public:
	Map10() : Map(10, "cave6", 0x51b, 1) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
