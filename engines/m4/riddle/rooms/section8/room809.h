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

#ifndef M4_RIDDLE_ROOMS_SECTION8_ROOM809_H
#define M4_RIDDLE_ROOMS_SECTION8_ROOM809_H

#include "m4/riddle/rooms/section8/section8_room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room809 : public Section8Room {
public:
	Room809() : Section8Room() {}
	~Room809() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	int32 room809_sub1(int32 val1, int32 val2);

	int32 _field20 = 0;
	int32 _field24 = 0;
	int32 _field28 = 0;
	int32 _field2C = 0;
	
	const char *_enableHotspotName = nullptr;

	byte _byte1A1990[12]; // random size for the moment, enough for facings
	int32 _dword1A1998 = 0;
	int32 _dword1A1980_x = 0;


	int32 _809hallSeries = 0;
	int32 _809MusicFadingVol = 0;
	int32 _mcHandsBehindBackSeries = 0;
	int32 _ripTalkerPos5Series = 0;
	int32 _ripTrekHeadTurnPos5Series = 0;

	machine *_809rp01Mach = nullptr;
	machine *_809hallMach = nullptr;
	machine *_mcTrekMach = nullptr;

};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
