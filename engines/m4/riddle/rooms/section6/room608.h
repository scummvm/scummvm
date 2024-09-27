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

#ifndef M4_RIDDLE_ROOMS_SECTION6_ROOM608_H
#define M4_RIDDLE_ROOMS_SECTION6_ROOM608_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room608 : public Room {
private:
	int _rp01 = 0;
	int _rp09 = 0;
	int _ripHandChin = 0;
	int _ripLowReach = 0;
	int _ripLowReach2 = 0;
	int _ripTalker = 0;
	int _tt01 = 0;
	int _tt02 = 0;
	int _tt03 = 0;
	int _tt05 = 0;
	int _old01 = 0;
	int _old02 = 0;
	int _old05 = 0;
	int _old08 = 0;
	int _old05a = 0;
	int _old05b = 0;
	int _old05c = 0;
	int _old05d = 0;
	int _old05f = 0;
	int _all5a = 0;
	int _loop0 = 0;
	int _loop1 = 0;
	int _horn = 0;
	machine *_ripley = nullptr;
	machine *_shadow = nullptr;
	machine *_shadow5 = nullptr;
	machine *_tt = nullptr;
	machine *_ol = nullptr;
	machine *_ol2 = nullptr;
	machine *_stump = nullptr;
	machine *_lighter = nullptr;
	machine *_pole = nullptr;
	machine *_puffin = nullptr;
	machine *_end1 = nullptr;
	machine *_end2 = nullptr;
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;

	void conv608a();
	bool takeStump1();
	bool takePuffin();
	bool stumpHole();
	bool takeStump2();
	bool takeLighter();
	bool hornCordWater();

public:
	Room608() : Room() {}
	~Room608() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
