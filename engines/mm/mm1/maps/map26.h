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

#ifndef MM1_MAPS_MAP26_H
#define MM1_MAPS_MAP26_H

#include "mm/mm1/maps/map_desert.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map26 : public MapDesert {
	typedef void (Map26:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special02();
	void special03();
	void addFlag();

	const SpecialFn SPECIAL_FN[4] = {
		&Map26::special00,
		&Map26::special01,
		&Map26::special02,
		&Map26::special03
	};
public:
	Map26() : MapDesert(26, "aread1", 0x505, 2, 247, MapDesert::RND_FULL) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
