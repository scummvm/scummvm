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

#ifndef M4_RIDDLE_ROOMS_SECTION4_ROOM405_H
#define M4_RIDDLE_ROOMS_SECTION4_ROOM405_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room405 : public Room {
private:
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val7 = 0;
	int _val8 = 0;
	int _val9 = 0;
	int _candlesBurning = 0;
	int _safariShadow = 0;
	int _lowReacher = 0;
	machine *_door = nullptr;
	machine *_candles = nullptr;
	machine *_bankNote = nullptr;
	machine *_baron = nullptr;
	machine *_baronWalker = nullptr;
	machine *_ripTalksBaron = nullptr;
	machine *_response = nullptr;
	int _currentNode = 0;
	Common::String _sound1;
	Common::String _sound2;
	int _baronShakeSit = 0;
	int _baronTalkLoop = 0;
	int _baronLeanForward = 0;
	int _baronTurnWalk = 0;
	int _ripHandLetter = 0;
	int _ripTalkGesture = 0;
	int _ripLeanForward = 0;
	int _ripHeadTurn = 0;

	void conv405a();
	void conv405a1();
	bool lookDoor();
	bool useDoor();
	bool takeDoor();
	bool takeBanknote();
	void doAction(const char *name);

public:
	Room405() : Room() {}
	~Room405() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
