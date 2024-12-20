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

#ifndef M4_RIDDLE_ROOMS_SECTION7_ROOM709_H
#define M4_RIDDLE_ROOMS_SECTION7_ROOM709_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

struct Maze709Struc {
	int32 _field_0;
	int32 _field_4;
	int32 _field_8;
	int32 _field_C;
	int32 _field_10;
	int32 _field_14;
	int32 _field_18;
	int32 _field_1C;
};

class Room709 : public Room {
public:
	Room709() : Room() {}
	~Room709() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	int32 _field44 = 0;
	int32 _field48 = 0;
	int32 _field74 = 0;
	int32 _field78 = 0;
	int32 _field7C = 0;
	int32 _field70 = 0;
	int32 _field80_save = 0;
	int32 _field84 = 0;
	int32 _field88 = 0;

	int32 _709NearDoorLiteSeries = 0;
	int32 _709rpro1Series = 0;
	int32 _709rpro2Series = 0;
	int32 _709rpro3Series = 0;
	int32 _709rpro4Series = 0;
	int32 _btnFlag = 0;
	int32 _mazeCenterDoorLiteSeries = 0;
	int32 _mazeLeftDoorLiteSeries = 0;
	int32 _mazeRightDoorLiteSeries = 0;
	int32 _ripTrekHeadTurnPos1Series = 0;
	int32 _ripTrekLowReachPos2Series = 0;

	machine *_field24Mach = nullptr;
	machine *_field2CMach = nullptr;
	machine *_field30Mach = nullptr;

	machine *_709IncenseHolderMach = nullptr;
	machine *_709rpro5Mach = nullptr;
	machine *_709rpro6Mach = nullptr;
	machine *_709rpro7Mach = nullptr;
	machine *_709rpro8Mach = nullptr;
	machine *_709ChiselMach = nullptr;
	machine *_ripPullMach = nullptr;

	Maze709Struc _maze709Arr[99];

	static void clearPressed(void *, void *);
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
