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

#ifndef M4_RIDDLE_ROOMS_SECTION9_ROOM996_H
#define M4_RIDDLE_ROOMS_SECTION9_ROOM996_H

#include "m4/riddle/rooms/room.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room996 : public Room {
public:
	Room996() : Room() {}
	~Room996() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	int32 _flagArray[12] = {};
	int32 _journalIndex = 1;
	int32 _moveValue = 0;
	bool _check201Fl = false;
	bool _check207Fl = false;
	bool _check208Fl = false;
	bool _check209Fl = false;

	machine *_roomStates_ear2 = nullptr;
	machine *_roomStates_field4 = nullptr;
	machine *_roomStates_field8 = nullptr;
	machine *_roomStates_field18 = nullptr;
	machine *_roomStates_field24 = nullptr;
	machine *_roomStates_field4C = nullptr;
	machine *_roomStates_loop0 = nullptr;
	machine *_roomStates_loop1 = nullptr;
	machine *_roomStates_loop2 = nullptr;
	machine *_roomStates_loop3 = nullptr;
	machine *_roomStates_loop4 = nullptr;
	machine *_roomStates_loop5 = nullptr;
	machine *_roomStates_loop6 = nullptr;
	machine *_roomStates_loop7 = nullptr;
	machine *_roomStates_pu = nullptr;
	machine *_roomStates_ripTalk = nullptr;
	machine *_roomStates_ripTalker = nullptr;
	machine *_roomStates_ripTalking = nullptr;
	machine *_roomStates_tt = nullptr;
	machine *_roomStates_untie = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
