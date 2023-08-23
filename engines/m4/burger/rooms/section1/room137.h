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

#ifndef M4_BURGER_ROOMS_SECTION1_ROOM137_H
#define M4_BURGER_ROOMS_SECTION1_ROOM137_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room137 : public Room {
private:
	bool _flag1 = false;
	bool _flag2 = false;
	bool _flag3 = false;
	int _volume = 255;
	int _mode1 = 0;
	int _mode2 = 0;
	int _mode3 = 0;
	int _mode4 = 0;
	const char *_digi1 = nullptr;
	machine *_series1 = nullptr;
	machine *_series2 = nullptr;
	machine *_series3 = nullptr;

	void conv15();
	void unloadAssets();
	void jawz();
	void sub1();
	void sub2();

public:
	Room137() : Room() {}
	~Room137() override {}

	void init() override;
	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
