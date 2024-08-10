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

#ifndef M4_RIDDLE_ROOMS_SECTION4_ROOM402_H
#define M4_RIDDLE_ROOMS_SECTION4_ROOM402_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room402 : public Room {
private:
	int _val1 = 0;
	int _val2 = 0;
	char *_soundPtr1 = nullptr;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	char *_soundPtr2 = nullptr;
	int _val8 = 0;
	int _currentNode = 0;
	int _val10 = 0;
	int _val11 = 0;
	int _val12 = 0;
	int _val13 = 0;
	machine *_wolfie = nullptr;
	machine *_wolfieMach = nullptr;
	int _shadow3 = 0;
	int _ripPaysWolfie = 0;
	int _ripTalkWolf = 0;
	int _ripTalker = 0;
	int _wolfClipping = 0;
	int _wolfClippersDown = 0;
	int _wolfShakesHead = 0;
	int _wolfWantsMoney = 0;
	int _ripHeadTurn = 0;
	int _ripDownStairs = 0;
	machine *_ripLeaving = nullptr;
	machine *_branch = nullptr;
	int _wolfTurnsClipping = 0;
	int _turtlePopup = 0;
	int _ripLeanWall = 0;
	machine *_wolfWalker = nullptr;

	void conv402a();

public:
	Room402() : Room() {}
	~Room402() override {}

	void preload() override;
	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
