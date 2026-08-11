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

#ifndef M4_RIDDLE_ROOMS_SECTION7_ROOM710_H
#define M4_RIDDLE_ROOMS_SECTION7_ROOM710_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room710 : public Room {
public:
	Room710() : Room() {}
	~Room710() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	bool _centerDoorPullAnimationFl = false;
	bool _ladderActiveFl = false;

	int32 _710Rpld2Series = 0;
	int32 _710Rpld3Series = 0;
	int32 _710Rpro1Series = 0;
	int32 _710Rpro2Series = 0;
	int32 _710Rpro3Series = 0;
	int32 _710Rpro4Series = 0;
	int32 _laderComesDownSeries = 0;
	int32 _mazeCentreDoorLiteSeries = 0;
	int32 _ripPullsRopeForLaderSeries = 0;
	int32 _ripTrekMedReachHandPos1Series = 0;

	machine *_710Rpld4Mach = nullptr;
	machine *_710Rpld5Mach = nullptr;
	machine *_710Rprs1Mach = nullptr;
	machine *_710Rprs2Mach = nullptr;
	machine *_710Rprs3Mach = nullptr;
	machine *_710Rprs4Mach = nullptr;
	machine *_ripContraptionMach = nullptr;
	machine *_ripContractionMach02 = nullptr;
	machine *_ripPullMach = nullptr;
	machine *_ripPullMach02 = nullptr;
	machine *_ripPullsKeyropeMach = nullptr;
	machine *_ripReacherMach = nullptr;
	machine *_safariShadow1Mach = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
