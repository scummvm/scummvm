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

#ifndef M4_BURGER_ROOMS_SECTION7_ROOM706_H
#define M4_BURGER_ROOMS_SECTION7_ROOM706_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room706 : public Room {
private:
	int _series1 = -1;
	int _series2 = -1;
	int _series3 = -1;
	int _series4 = -1;
	machine *_series5 = nullptr;
	machine *_series6 = nullptr;
	machine *_series7 = nullptr;
	machine *_series8 = nullptr;
	bool _flag5 = false;
	bool _flag6 = false;
	bool _flag7 = false;
	int _volume = 0;
	int _astralShould = 0;

	void conv83();
	void getFlumixShould();

public:
	Room706() : Room() {
	}
	~Room706() override {
	}

	void preload() override;
	void init() override;
	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
