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

#ifndef MM1_MAPS_MAP03_H
#define MM1_MAPS_MAP03_H

#include "mm/mm1/maps/map_town.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map03 : public MapTown {
	typedef void (Map03:: *SpecialFn)();
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
	void special18();
	void special20();
	void special21();
	void special27();
	void special28();

	const SpecialFn SPECIAL_FN[29] = {
		&Map03::special00,
		&Map03::special01,
		&Map03::special02,
		&Map03::special03,
		&Map03::special04,
		&Map03::special05,
		&Map03::special06,
		&Map03::special07,
		&Map03::special08,
		&Map03::special09,
		&Map03::special10,
		&Map03::special11,
		&Map03::special12,
		&Map03::special13,
		&Map03::special14,
		&Map03::special15,
		&Map03::special15,
		&Map03::special15,
		&Map03::special18,
		&Map03::special15,
		&Map03::special20,
		&Map03::special21,
		&Map03::special15,
		&Map03::special15,
		&Map03::special15,
		&Map03::special15,
		&Map03::special15,
		&Map03::special27,
		&Map03::special28
	};
public:
	Map03() : MapTown(3, "dusk", 0x802, 1) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
