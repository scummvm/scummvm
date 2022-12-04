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

#ifndef MM1_MAPS_MAP11_H
#define MM1_MAPS_MAP11_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map11 : public Map {
	typedef void (Map11:: *SpecialFn)();
private:
	int _dialIndex = 0;

	void special00();
	void special01();
	void special02();
	void special03();
	void special04();
	void special05();
	void special06();
	void special07();
	void special08();
	void pit();
	void selectDial(int dialIndex);

	const SpecialFn SPECIAL_FN[14] = {
		&Map11::special00,
		&Map11::special01,
		&Map11::special02,
		&Map11::special03,
		&Map11::special04,
		&Map11::special05,
		&Map11::special06,
		&Map11::special07,
		&Map11::special08,
		&Map11::special08,
		&Map11::special08,
		&Map11::special02,
		&Map11::special02,
		&Map11::special02
	};
public:
	Map11() : Map(11, "cave7", 0x212, 1) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;

	/**
	 * Creates a challenge encounter
	 */
	void challenge();

	/**
	 * Set the alphabetic character for the current dial
	 */
	void setDialChar(char c);

	/**
	 * Volcano god's clue
	 */
	void clue();

	/**
	 * Volcano god's riddle answer
	 */
	void riddleAnswer(const Common::String &answer);
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
