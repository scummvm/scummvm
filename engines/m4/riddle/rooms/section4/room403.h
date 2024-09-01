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

#ifndef M4_RIDDLE_ROOMS_SECTION4_ROOM403_H
#define M4_RIDDLE_ROOMS_SECTION4_ROOM403_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room403 : public Room {
private:
	machine *_bell = nullptr;
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val7 = 0;
	int _val8 = 0;
	int _val9 = 0;
	int _val10 = 0;
	Common::String _sound1;
	int _val12 = 0;
	int _val13 = 0;
	int _plank = 0;
	machine *_ventClosed = nullptr;
	machine *_edger = nullptr;
	machine *_ladder = nullptr;
	machine *_board = nullptr;
	machine *_wolfie = nullptr;
	machine *_ripOnLadder = nullptr;
	int _ripClimbsLadder = 0;
	int _safariShadow = 0;
	int _wolfTurnTalk = 0;
	int _wolfTurnHand = 0;
	int _wolfTalkLeave = 0;
	int _ripTalkPay = 0;
	int _wolfEdger = 0;
	int _ripLegUp = 0;
	int _ripTurtle = 0;
	int _noTreat = 0;
	bool _flag1 = false;
	bool _flag2 = false;
	bool _flag3 = 0;
	int _ripRingsBell = 0;
	machine *_wolfWalker = nullptr;
	int _wolfAdmonish = 0;
	int _ripHeadTurn = 0;
	int _ripMedReach = 0;
	int _series1 = 0;
	int _ripPutBoard = 0;
	machine *_ripTalksWolf = nullptr;
	int _ripPlankEdger = 0;
	machine *_wolfJustSo = nullptr;
	int _wolfIndicatesTomb = 0;
	int _turtlePopup = 0;

	void conv403a();
	void conv403a1();
	void edgerBell();
	bool edgerUrn();
	bool stepLadderTomb();
	bool takePlank();
	bool takeEdger();
	bool takeStepLadder();
	void takeStepLadder_();
	void useJournal();
	void playNum1(int num);
	void playNum2(int num);

public:
	Room403() : Room() {}
	~Room403() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
