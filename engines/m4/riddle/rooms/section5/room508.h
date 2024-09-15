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

#ifndef M4_RIDDLE_ROOMS_SECTION5_ROOM508_H
#define M4_RIDDLE_ROOMS_SECTION5_ROOM508_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room508 : public Room {
private:
	int _val1 = 0;
	int _val2 = 0;
	int _ripReturnsToStander = 0;
	int _chainBreaking = 0;
	int _lightAppearing = 0;
	int _ripPutsShovel = 0;
	int _ripTryTurnDome = 0;
	int _ripSketching = 0;
	int _flick = 0;
	machine *_ripley = nullptr;
	machine *_ripStartTurnDome = nullptr;
	machine *_chainAfterBreak = nullptr;
	machine *_domeAfterTurn = nullptr;
	machine *_skull = nullptr;
	machine *_shovel = nullptr;
	machine *_statue = nullptr;
	machine *_light = nullptr;
	machine *_chain = nullptr;
	machine *_x = nullptr;
	machine *_spect = nullptr;
public:
	Room508() : Room() {}
	~Room508() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
