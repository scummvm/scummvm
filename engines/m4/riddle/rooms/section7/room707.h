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

#ifndef M4_RIDDLE_ROOMS_SECTION7_ROOM707_H
#define M4_RIDDLE_ROOMS_SECTION7_ROOM707_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room707 : public Room {
public:
	Room707() : Room() {}
	~Room707() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	int32 _field44 = 0;

	int32 _peerIntoCupolaPos1Series = 0;
	int32 _peerIntoCupolaPos2Series = 0;
	int32 _ripDownStairsPos4Series = 0;
	int32 _ripDownStairsPos8Series = 0;
	int32 _ripLooksDownSeries = 0;
	int32 _ripUpStairsPos10Series = 0;
	int32 _ripUpStairsPos2Series = 0;
	int32 _ripWalksDownFarStairsSeries = 0;
	int32 _ripWalksDownLeftStairsSeries = 0;
	int32 _ripWalksUpFarStairsSeries = 0;
	int32 _ripWalksUpLeftStairsSeries = 0;

	machine *_popUpMach = nullptr;
	machine *_ripStairsMach = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
