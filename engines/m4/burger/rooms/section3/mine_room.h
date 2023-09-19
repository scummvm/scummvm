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

#ifndef M4_BURGER_ROOMS_SECTION3_MINE_ROOM_H
#define M4_BURGER_ROOMS_SECTION3_MINE_ROOM_H

#include "m4/burger/rooms/section3/section3_room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum MineDirection {
	DIR_NORTH = 0, DIR_SOUTH = 1, DIR_WEST = 2, DIR_EAST = 3
};

struct MineEntry {
	int32 _unknown;
	int16 _indexes[7];
	int32 _offset;
	int16 _field16;
};

class MineRoom : public Section3Room {
private:
	static const MineEntry MINE_DATA[];
	static const char *SAID[][4];
	int _mineCtr = 0;

	int getPigDistance() const;
	void changeRoom(MineDirection dir);

public:
	MineRoom() : Section3Room() {}
	virtual ~MineRoom() {}

	void preload() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
