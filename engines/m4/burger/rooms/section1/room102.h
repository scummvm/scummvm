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

#ifndef M4_BURGER_ROOMS_SECTION1_ROOM102_H
#define M4_BURGER_ROOMS_SECTION1_ROOM102_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room102 : public Room {
private:
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = -1;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val7 = 0;
	int _val8 = 0;
	int _val9 = 0;
	int _val10 = 99999;
	int _val11 = 0;
	int _val12 = 0;
	int _val13 = 0;
	int _val14 = 0;
	int _val15 = -1;
	int _val16 = 0;
	machine *_series1 = nullptr;
	machine *_series2 = nullptr;
	machine *_laz1 = nullptr;
	machine *_laz2 = nullptr;
	int _index1 = -1, _index2 = -1;

	void setup(int val1, int val2);
	void setupWax();
	void setupLaz();
	const char *getDigi1(int num) const;
	const char *getDigi2(int num) const;

public:
	Room102() : Room() {}
	~Room102() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
