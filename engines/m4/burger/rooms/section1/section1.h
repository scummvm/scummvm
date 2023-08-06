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

#ifndef M4_BURGER_ROOMS_SECTION1_H
#define M4_BURGER_ROOMS_SECTION1_H

#include "m4/burger/rooms/room.h"
#include "m4/burger/rooms/section.h"
#include "m4/burger/rooms/section1/room101.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum Goto1 {
	GOTO_TOWN_HALL = 1001,
	GOTO_OLD_BRIDGE = 1008
};

class Section1 : public Rooms::Section {
private:
	Room101 _room101;

	int _trigger = 0;
	int _series1 = -1;
	int _series2 = -1;
	machine *_play = nullptr;
	int _state1 = 0;

	void updateWalker_(int x, int y, int dir, int trigger, bool mode);
public:
	Section1();
	virtual ~Section1() {}

	void daemon() override;

	static void updateWalker(int x, int y, int dir, int trigger, bool mode = false);
	static void updateDisablePlayer();
	static void walk(int facing, int trigger = -1);
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
