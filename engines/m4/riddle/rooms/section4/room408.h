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

#ifndef M4_RIDDLE_ROOMS_SECTION4_ROOM408_H
#define M4_RIDDLE_ROOMS_SECTION4_ROOM408_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room408 : public Room {
private:
	int _val1 = 0;
	int _val3 = 0;
	int _ripleyTrigger = -1;
	int _wolfTrigger = -1;
	int _currentNode = 0;
	int _ripleyMode = 0;
	int _ripleyShould = 0;
	int _wolfMode = 0;
	int _wolfShould = 0;
	machine *_exit = nullptr;
	int _ripExits = 0;
	machine *_edger = nullptr;
	machine *_plank = nullptr;
	int _wolf = 0;
	machine *_wolfie = nullptr;
	machine *_wolfWalker = nullptr;
	int _ripLowReacher = 0;
	int _ripHandsBehindBack = 0;
	machine *_ripley = nullptr;
	machine *_ripleyShadow = nullptr;
	int _ripShadowSeries = 0;
	int _ripTrekTwoHandTalk = 0;

	void conv408a();
	bool takePlank();
	bool takeEdger();

public:
	Room408() : Room() {}
	~Room408() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
	void syncGame(Common::Serializer &s) override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
