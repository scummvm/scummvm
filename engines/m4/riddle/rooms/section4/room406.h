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

#ifndef M4_RIDDLE_ROOMS_SECTION4_ROOM406_H
#define M4_RIDDLE_ROOMS_SECTION4_ROOM406_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room406 : public Room {
private:
	machine *_billiardTable = nullptr;
	machine *_painting = nullptr;
	machine *_mirror = nullptr;
	machine *_cardDoor = nullptr;
	machine *_gamesCabinet = nullptr;
	machine *_desk = nullptr;
	machine *_poolBall = nullptr;
	machine *_emptyDrawer = nullptr;
	machine *_envelope = nullptr;
	machine *_keys = nullptr;
	machine *_cards = nullptr;
	int _rptmhr = 0;
	int _rptmr15 = 0;
	int _val1 = 0;
	int _val2 = 0;
	int _ripReachHand = 0;
	int _ripHiHand = 0;
	int _pickupBall = 0;
	machine *_ripAction = nullptr;
	int _lookMirror = 0;
	int _tableRaises = 0;
	int _paintingOpening = 0;
	int _ripThrowsBall = 0;
	int _deskOpening = 0;
	int _cabinetOpens = 0;

	void setHotspots();
	bool takeKeys();
	bool takeBilliardBall();
	bool billiardBallOnTable();
	void useSwitchPaintingOpen();
	void useSwitchPaintingClosed();
	void useAceOfSpades1();
	void useAceOfSpades2();
	bool lookBilliardBall();

public:
	Room406() : Room() {}
	~Room406() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
