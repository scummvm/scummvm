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

#ifndef M4_RIDDLE_ROOMS_SECTION8_ROOM811_H
#define M4_RIDDLE_ROOMS_SECTION8_ROOM811_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room811 : public Room {
public:
	Room811() : Room() {}
	~Room811() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	int32 _811MusicFadingVol = 0;

	int32 _811BA01Series = 0;
	int32 _811BA02Series = 0;
	int32 _811BA03Series = 0;
	int32 _811BA04Series = 0;
	int32 _811BA05Series = 0;
	int32 _811MC01Series = 0;
	int32 _811RP01Series = 0;
	int32 _811EndSeries = 0;

	machine *_811BaronMach = nullptr;
	machine *_811McMach = nullptr;
	machine *_811Pu01Mach = nullptr;
	machine *_811RipleyMach = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
