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

#ifndef M4_RIDDLE_ROOMS_SECTION2_ROOM201_H
#define M4_RIDDLE_ROOMS_SECTION2_ROOM201_H

#include "m4/riddle/rooms/section2/section2_room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room201 : public Section2Room {
private:
	bool _flag1 = false;
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val7 = 0;
	int _val8 = 0;
	int _val9 = 0;
	int _nod = -1;
	int _guyWriting = -1;
	int _guyParcel = -1;
	int _series1 = -1;
	int _series2 = -2;
	int _series3 = -1;
	int _series4 = -1;
	int _series5 = -1;
	int _series6 = -2;
	int _series7 = -1;
	int _series8 = -1;
	machine *_machine1 = nullptr;
	machine *_machine2 = nullptr;
	machine *_machine3 = nullptr;
	machine *_machine4 = nullptr;
	int _mei1 = -1;
	int _mei2 = -1;
	int _mei3 = -1;

public:
	Room201() : Section2Room() {}
	~Room201() override {}

	void init() override;
	void daemon() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
