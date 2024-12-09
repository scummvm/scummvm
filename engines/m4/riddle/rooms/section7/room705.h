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

#ifndef M4_RIDDLE_ROOMS_SECTION7_ROOM705_H
#define M4_RIDDLE_ROOMS_SECTION7_ROOM705_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room705 : public Room {
public:
	Room705() : Room() {}
	~Room705() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	void conv705a();
	void conv705b();
	void playCheckBrochureAnim(int32 trigger, const char *digiName);
	void useWheelOnLeftNiche(int32 trigger, int val1);
	void useWheelOnRightNiche(int32 trigger, int val1);

	int32 _field60_mode = 0;
	int32 _field64_should = 0;

	int32 _705Eye1aSeries = 0;
	int32 _705Eye4aSeries = 0;
	int32 _705Monk1Series = 0;
	int32 _705Monk2Series = 0;
	int32 _705Monk3Series = 0;
	int32 _705Monk4Series = 0;
	int32 _705RipGoesUpStairsSeries = 0;
	int32 _ripChecksBrochureSeries = 0;
	int32 _ripGoesDownStairsSeries = 0;
	int32 _ripLooksDownSeries = 0;
	int32 _ripTrekMedReachPos1Series = 0;

	machine *_leftWheelMach = nullptr;
	machine *_monkMach = nullptr;
	machine *_rightWheelMach = nullptr;
	machine *_ripStairsMach = nullptr;

};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
