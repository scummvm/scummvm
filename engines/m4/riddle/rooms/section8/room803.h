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

#ifndef M4_RIDDLE_ROOMS_SECTION8_ROOM803_H
#define M4_RIDDLE_ROOMS_SECTION8_ROOM803_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room803 : public Room {
public:
	Room803() : Room() {}
	~Room803() override {}

	void preload() override;
	void init() override;
	void parser() override;
	void daemon() override;

private:
	int32 _meiStepOffPile = 0;
	int32 _meiTalksPos3 = 0;
	int32 _ripTalkerPos5 = 0;
	int32 _var1 = 0;

	machine *_mcWalkerMach = nullptr;
	machine *_meiLooksAboutMach = nullptr;
	machine *_meiStepOffPileMach = nullptr;
	machine *_ripStanderMach = nullptr;

	void initWalker();
	void loadWalkSeries();
	void daemonSub1();
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
