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
	int32 _leftActive;
	int32 _rightActive;
	int32 _straightActive;
	int32 _backActive;
	int32 _leftIndex;
	int32 _rightIndex;
	int32 _straightIndex;
	int32 _backIndex;
};

class Room709 : public Room {
public:
	Room709();
	~Room709() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;
	void syncGame(Common::Serializer &s) override;

private:
	bool _chiselActiveFl = false;
	bool _incenseBurnerActiveFl = false;
	bool _pullCenterFl = false;
	bool _pullLeftFl = false;
	bool _pullNearFl = false;
	bool _pullRightFl = false;

	int32 _mazeCurrentIndex = 0;

	int32 _709NearDoorLiteSeries = 0;
	int32 _709rpro1Series = 0;
	int32 _709rpro2Series = 0;
	int32 _709rpro3Series = 0;
	int32 _709rpro4Series = 0;
	int32 _mazeCenterDoorLiteSeries = 0;
	int32 _mazeLeftDoorLiteSeries = 0;
	int32 _mazeRightDoorLiteSeries = 0;
	int32 _ripTrekHeadTurnPos1Series = 0;
	int32 _ripTrekLowReachPos2Series = 0;

	machine *_709ChiselMach = nullptr;
	machine *_709IncenseHolderMach = nullptr;
	machine *_709rpro5Mach = nullptr;
	machine *_709rpro6Mach = nullptr;
	machine *_709rpro7Mach = nullptr;
	machine *_709rpro8Mach = nullptr;
	machine *_ripPullMach = nullptr;
	machine *_ripPullMach02 = nullptr;
	machine *_ripPullMach03 = nullptr;
	machine *_ripPullMach04 = nullptr;
	machine *_ripPullMach05 = nullptr;
	machine *_safariShadow1Mach = nullptr;

	Maze709Struc _maze709Arr[99];

	void resetMaze();
	static void clearPressed(void *, void *);
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
