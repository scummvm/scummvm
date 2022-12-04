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

#ifndef MM1_MAPS_MAP08_H
#define MM1_MAPS_MAP08_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map08 : public Map {
	typedef void (Map08:: *SpecialFn)();
private:
	void special00();
	void special01();
	void special02();
	void special04();
	void special05();
	void special06();
	void special08();
	void special20();
	void addTreasure();

	const SpecialFn SPECIAL_FN[24] = {
		&Map08::special00,
		&Map08::special01,
		&Map08::special02,
		&Map08::special02,
		&Map08::special04,
		&Map08::special05,
		&Map08::special06,
		&Map08::special06,
		&Map08::special08,
		&Map08::special08,
		&Map08::special08,
		&Map08::special08,
		&Map08::special06,
		&Map08::special06,
		&Map08::special06,
		&Map08::special06,
		&Map08::special06,
		&Map08::special06,
		&Map08::special06,
		&Map08::special06,
		&Map08::special20,
		&Map08::special20,
		&Map08::special20,
		&Map08::special20
	};

	/**
	 * Correct code entered
	 */
	void correctCode();

	/**
	 * Incorrect code entered
	 */
	void incorrectCode();

public:
	Map08() : Map(8, "cave4", 0x202, 1) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;

	/**
	 * Access code entered
	 */
	void codeEntered(const Common::String &code);
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
