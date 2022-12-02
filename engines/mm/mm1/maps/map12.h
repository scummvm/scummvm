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

#ifndef MM1_MAPS_MAP12_H
#define MM1_MAPS_MAP12_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Map12 : public Map {
	typedef void (Map12:: *SpecialFn)();
private:
	int _polyIndex = 0;

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
	void polyhedron(unsigned char side1, unsigned char side2);
	void setPolyhedron(int polyIndex);

	const SpecialFn SPECIAL_FN[18] = {
		&Map12::special00,
		&Map12::special01,
		&Map12::special02,
		&Map12::special03,
		&Map12::special04,
		&Map12::special05,
		&Map12::special06,
		&Map12::special07,
		&Map12::special08,
		&Map12::special09,
		&Map12::special10,
		&Map12::special11,
		&Map12::special12,
		&Map12::special13,
		&Map12::special14,
		&Map12::special15,
		&Map12::special16,
		&Map12::special17
	};
public:
	Map12() : Map(12, "cave8", 0x601, 1) {}

	/**
	 * Handles all special stuff that happens on the map
	 */
	void special() override;

	/**
	 * Spins a polyhedron
	 */
	void spinPolyhedron(byte newSide);
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
