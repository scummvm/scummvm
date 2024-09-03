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

#ifndef M4_RIDDLE_ROOMS_SECTION4_ROOM409_H
#define M4_RIDDLE_ROOMS_SECTION4_ROOM409_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room409 : public Room {
private:
	int _volume = 0;
	bool _flag = false;
	machine *_ripley = nullptr;
	machine *_shadow = nullptr;
	machine *_turtleRope = nullptr;
	machine *_wolfWalker = nullptr;
	machine *_doorOpen = nullptr;
	int _seriesShadow = 0;
	int _seriesDoorOpen = 0;
	int _seriesRope = 0;
	int _seriesWolfOpensDoor = 0;
	int _seriesReadTelegram = 0;
	int _seriesRp02 = 0;
	int _seriesHeadTurn = 0;
	int _seriesTalker = 0;
	int _seriesReachHand = 0;

public:
	Room409() : Room() {}
	~Room409() override {}

	void init() override;
	void daemon() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
