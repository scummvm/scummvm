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

#ifndef M4_RIDDLE_ROOMS_SECTION4_ROOM495_H
#define M4_RIDDLE_ROOMS_SECTION4_ROOM495_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room495 : public Room {
private:
	int _val1 = 0;
	const char *_seriesName1 = nullptr;
	const char *_seriesName2 = nullptr;
	int _lastFrame = 0;
	
	void updateFlags1();
	void updateFlags2();
	void play(const char *seriesName1, const char *seriesName2,
		int lastFrame, int frameRate);

public:
	Room495() : Room() {}
	~Room495() override {}

	void preload() override;
	void init() override;
	void daemon() override;
	void pre_parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
