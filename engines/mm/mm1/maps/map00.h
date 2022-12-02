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

#ifndef MM1_MAPS_MAP00_H
#define MM1_MAPS_MAP00_H

#include "mm/mm1/maps/map_town.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map00 : public MapTown {
	typedef void (Map00:: *SpecialFn)();
private:
	void searchStatue();

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
	void special17();
	void special18();
	void special19();
	void special20();
	void special21();
	void special22();
	void special23();

	const SpecialFn SPECIAL_FN[24] = {
		&Map00::special00,
		&Map00::special01,
		&Map00::special02,
		&Map00::special03,
		&Map00::special04,
		&Map00::special05,
		&Map00::special06,
		&Map00::special07,
		&Map00::special08,
		&Map00::special09,
		&Map00::special10,
		&Map00::special11,
		&Map00::special12,
		&Map00::special13,
		&Map00::special14,
		&Map00::special15,
		&Map00::special16,
		&Map00::special17,
		&Map00::special18,
		&Map00::special19,
		&Map00::special20,
		&Map00::special21,
		&Map00::special22,
		&Map00::special23
	};
public:
	Map00() : MapTown(0, "sorpigal", 0x604, 1) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	virtual void special();
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
