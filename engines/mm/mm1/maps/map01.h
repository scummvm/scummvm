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

#ifndef MM1_MAPS_MAP01_H
#define MM1_MAPS_MAP01_H

#include "mm/mm1/maps/map_town.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map01 : public MapTown {
	typedef void (Map01:: *SpecialFn)();
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
	void special15();
	void special16();

	const SpecialFn SPECIAL_FN[17] = {
		&Map01::special00,
		&Map01::special01,
		&Map01::special02,
		&Map01::special03,
		&Map01::special04,
		&Map01::special05,
		&Map01::special06,
		&Map01::special07,
		&Map01::special08,
		&Map01::special09,
		&Map01::special10,
		&Map01::special11,
		&Map01::special12,
		&Map01::special13,
		&Map01::special14,
		&Map01::special15,
		&Map01::special16
	};
public:
	Map01() : MapTown(1, "portsmit", 0xc03, 1, "Portsmith") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
