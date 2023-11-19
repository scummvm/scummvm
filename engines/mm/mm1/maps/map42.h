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

#ifndef MM1_MAPS_MAP42_H
#define MM1_MAPS_MAP42_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map42 : public Map {
	typedef void (Map42:: *SpecialFn)();
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
	void special17();

	const SpecialFn SPECIAL_FN[25] = {
		&Map42::special00,
		&Map42::special01,
		&Map42::special02,
		&Map42::special03,
		&Map42::special04,
		&Map42::special05,
		&Map42::special06,
		&Map42::special07,
		&Map42::special08,
		&Map42::special09,
		&Map42::special10,
		&Map42::special11,
		&Map42::special12,
		&Map42::special13,
		&Map42::special14,
		&Map42::special15,
		&Map42::special15,
		&Map42::special17,
		&Map42::special17,
		&Map42::special03,
		&Map42::special03,
		&Map42::special03,
		&Map42::special04,
		&Map42::special04,
		&Map42::special04
	};
public:
	Map42() : Map(42, "enf2", 0x704, 3, "Minotaur Stronghold 1") {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;

	/**
	 * Succeeded in the necessary quests
	 */
	void dogSuccess();

	/**
	 * Desecrating the dog statue
	 */
	void dogDesecrate();
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
