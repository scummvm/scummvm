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

#ifndef M4_RIDDLE_ROOMS_SECTION5_ROOM506_H
#define M4_RIDDLE_ROOMS_SECTION5_ROOM506_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room506 : public Room {
private:
	RGB8 _palette[39];
	bool _flag1 = false;
	bool _flag2 = false;
	bool _flag3 = false;
	bool _flag4 = false;
	int _destX = 0, _destY = 0;
	int _destFacing = 0;
	machine *_ripley = nullptr;
	machine *_dome = nullptr;
	machine *_skullSparkle = nullptr;
	machine *_domeRotation = nullptr;
	int _ripLowReach = 0;

	void restorePalette();
	void setupPalette();
	void saveWalk();

public:
	Room506();
	~Room506() override {}

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
