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

#ifndef M4_BURGER_ROOMS_SECTION5_ROOM505_H
#define M4_BURGER_ROOMS_SECTION5_ROOM505_H

#include "m4/burger/rooms/section5/section5_room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room505 : public Section5Room {
private:
	static const char *SAID[][4];
	static const BorkPoint BORK[];
	static const seriesPlayBreak PLAY1[];
	static const seriesPlayBreak PLAY2[];
	char _bork[8], _lookAt[8];
	int _borkState = 0;
	int _destTrigger = 0;
	int _val3 = 0;
	int _val4 = 0;
	machine *_fan = nullptr;

public:
	Room505();
	~Room505() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
