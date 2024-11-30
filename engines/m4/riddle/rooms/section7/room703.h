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

#ifndef M4_RIDDLE_ROOMS_SECTION7_ROOM703_H
#define M4_RIDDLE_ROOMS_SECTION7_ROOM703_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room703 : public Room {
public:
	Room703() : Room() {}
	~Room703() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	static void callback(frac16 myMessage, machine *sender);
	void conv703a();
	void playCheckBrochureAnim(const char *digiName, int32 trigger);
	void useWheelOnNiche(int32 trigger, int val1);
	
	machine *_monkMach = nullptr;
	machine *_prayerWheelMach = nullptr;
	machine *_ripStairsMach = nullptr;
	
	int32 _field5C_mode = 0;
	int32 _field60_should = 0;

	int32 _703Eye4aSeries = 0;
	int32 _703RipGoesDownStairsSeries = 0;
	int32 _monk1Series = 0;
	int32 _monk2Series = 0;
	int32 _monk3Series = 0;
	int32 _monk4Series = 0;
	int32 _ripChecksBrochureSeries = 0;
	int32 _ripGoesUpStairsSeries = 0;
	int32 _ripLooksDownSeries = 0;
	int32 _ripTrekMedReachHandPos1Series = 0;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
