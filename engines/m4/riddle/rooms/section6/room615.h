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

#ifndef M4_RIDDLE_ROOMS_SECTION6_ROOM615_H
#define M4_RIDDLE_ROOMS_SECTION6_ROOM615_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room615 : public Room {
private:
	int _ripTalking = 0;
	int _ripTalk = 0;
	int _loop0 = 0;
	int _loop1 = 0;
	int _loop2 = 0;
	int _loop3 = 0;
	int _loop4 = 0;
	int _loop5 = 0;
	int _loop6 = 0;
	int _loop7 = 0;
	int _ear2 = 0;
	machine *_untie = nullptr;
	machine *_pu = nullptr;
	machine *_tt = nullptr;
	machine *_ripTalker = nullptr;
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 90;

	bool _flag1 = false;
	int _ctr1 = 0;

public:
	Room615() : Room() {}
	~Room615() override {}

	void preload() override;
	void init() override;
	void daemon() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
