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

#ifndef M4_RIDDLE_ROOMS_SECTION3_ROOM304_H
#define M4_RIDDLE_ROOMS_SECTION3_ROOM304_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room304 : public Room {
private:
	bool _flag1 = false;
	int _val1 = 0;
	bool _useSword = false;
	bool _useHandlingStick = false;
	int _val4 = 0;
	machine *_sword = nullptr;
	machine *_trunk = nullptr;
	machine *_stick = nullptr;
	int _smoke = -1;
	machine *_mei = nullptr;
	machine *_machine2 = nullptr;
	machine *_machine3 = nullptr;
	machine *_pu = nullptr;
	machine *_cobraKills = nullptr;
	machine *_field60 = nullptr;
	int _cutSnake = -1;
	int _mei1 = -1;
	int _mei2 = -1;
	int _mei3 = -1;
	int _cobra = -1;
	int _headUp1 = -1;
	machine *_headUp2 = nullptr;
	int _suit1 = 0;
	int _suit2 = 0;
	int _suit3 = 0;
	int _safe1 = 0;
	int _safe2 = 0;
	machine *_safe3 = nullptr;

	int _ctr1 = 0;

	static void intrMsg(frac16 myMessage, struct machine *sender);
	void handlingStickAndSword();

public:
	Room304() : Room() {}
	~Room304() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
