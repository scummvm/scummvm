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

#ifndef M4_RIDDLE_ROOMS_SECTION8_ROOM802_H
#define M4_RIDDLE_ROOMS_SECTION8_ROOM802_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room802 : public Room {
public:
	Room802() : Room() {}
	~Room802() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;


private:
	int32 _802Sack2 = 0;
	int32 _handInWall = 0;
	int32 _handInWallPartlyDug = 0;
	int32 _holeInWall = 0;
	int32 _lookWithMatch = 0;
	int32 _ripArmXPos3 = 0;
	int32 _ripDigsWall = 0;
	int32 _ripDownStairs = 0;
	int32 _ripDragsSack = 0;
	int32 _ripLooksAtHand = 0;
	int32 _ripTalkOffTd33 = 0;
	int32 _ripTugsBeforeDigging = 0;
	int32 _ripTugsOnArm = 0;
	int32 _ripUpStairs = 0;
	int32 _sackAgainstWall = 0;
	
	machine *_handInWallMach = nullptr;
	machine *_holeInWallMach = nullptr;
	machine *_ripWalksDownstairsMach = nullptr;
	machine *_sackAgainstWallMach = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
