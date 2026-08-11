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

#ifndef M4_RIDDLE_ROOMS_SECTION9_ROOM907_H
#define M4_RIDDLE_ROOMS_SECTION9_ROOM907_H

#include "m4/core/rooms.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room907 : public Room {
public:
	Room907() : Room() {}
	~Room907() override {}

	void preload() override;
	void init() override;
	void daemon() override;

private:
	static void escapePressed(void *, void *);
	machine * _roomStates_tt = nullptr;
	bool _buttonDown = false;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
