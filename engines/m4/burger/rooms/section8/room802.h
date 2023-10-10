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

#ifndef M4_BURGER_ROOMS_SECTION8_ROOM802_H
#define M4_BURGER_ROOMS_SECTION8_ROOM802_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room802 : public Room {
private:
	machine *_series1 = nullptr;
	machine *_series2 = nullptr;
	Series _series3;
	machine *_series5 = nullptr;
	int _val1 = 11;
	int _val2 = 2;
	int _val4 = 0;
	int _val5 = 0;
	bool _flag1 = false;
	bool _flag2 = false;

	void loadSeries();
	int getWilburShould();
	int getFlumixShould();

public:
	Room802() : Room() {}
	~Room802() override {}

	void init() override;
	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
