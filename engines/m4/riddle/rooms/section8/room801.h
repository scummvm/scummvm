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

#ifndef M4_RIDDLE_ROOMS_SECTION8_ROOM801_H
#define M4_RIDDLE_ROOMS_SECTION8_ROOM801_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room801 : public Room {
public:
	Room801() : Room() {}
	~Room801() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	void room801_conv801a();

	int32 _roomStates_ear2 = 0;
	int32 _roomStates_field4 = 0;
	int32 _roomStates_field6 = 0;
	int32 _roomStates_field18 = 0;
	int32 _roomStates_field24 = 0;
	int32 _roomStates_field2A = 0;
	int32 _roomStates_field4C2 = 0;
	int32 _roomStates_field502 = 0;
	int32 _roomStates_field60 = 0;
	int32 _roomStates_field642 = 0;
	int32 _roomStates_field682 = 0;
	int32 _roomStates_field702 = 0;
	int32 _roomStates_field742 = 0;
	int32 _roomStates_field6C2 = 0;
	int32 _roomStates_field782 = 0;
	int32 _roomStates_field7C = 0;
	int32 _roomStates_field7E = 0;
	int32 _roomStates_field80 = 0;
	int32 _roomStates_loop0 = 0;
	int32 _roomStates_loop1 = 0;
	int32 _roomStates_loop2 = 0;
	int32 _roomStates_loop3 = 0;
	int32 _roomStates_loop4 = 0;
	int32 _roomStates_loop62 = 0;
	int32 _roomStates_loop72 = 0;
	int32 _roomStates_pu = 0;
	int32 _roomStates_pu2 = 0;
	int32 _roomStates_ripTalk = 0;
	int32 _roomStates_ripTalker = 0;
	int32 _roomStates_ripTalker2 = 0;
	int32 _roomStates_ripTalking = 0;
	int32 _roomStates_tt = 0;
	int32 _roomStates_tt2 = 0;
	int32 _roomStates_untie2 = 0;
	int32 _roomStates_val1 = 0;
	int32 _roomStates_val2 = 0;
	int32 _roomStates_val3 = 0;
	int32 _var1 = 0;

	machine *_roomStates_field86h = nullptr;
	machine *_roomStates_field8Ah = nullptr;
	machine *_roomStates_field8Eh = nullptr;
	machine *_roomStates_field92h = nullptr;
	machine *_roomStates_field96h = nullptr;
	machine *_roomStates_field9Ah = nullptr;
	machine *_roomStates_field9Eh = nullptr;
	machine *_roomStates_fieldA2h = nullptr;
	machine *_roomStates_fieldA6h = nullptr;
	machine *_roomStates_fieldAAh = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
