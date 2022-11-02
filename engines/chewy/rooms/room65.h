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

#ifndef CHEWY_ROOMS_ROOM65_H
#define CHEWY_ROOMS_ROOM65_H

namespace Chewy {
namespace Rooms {

class Room65 {
public:
	static int16 _scrollX;
	static int16 _scrollY[2];

public:
	static void entry();
	static void xit();
	static void atds_string_start(int16 dia_nr, int16 str_nr,
		int16 person_nr, int16 mode);
};

} // namespace Rooms
} // namespace Chewy

#endif
