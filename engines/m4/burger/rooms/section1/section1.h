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

#ifndef M4_BURGER_ROOMS_SECTION1_H
#define M4_BURGER_ROOMS_SECTION1_H

#include "m4/burger/rooms/room.h"
#include "m4/burger/rooms/section.h"
#include "m4/burger/rooms/section1/room101.h"
#include "m4/burger/rooms/section1/room102.h"
#include "m4/burger/rooms/section1/room103.h"
#include "m4/burger/rooms/section1/room104.h"
#include "m4/burger/rooms/section1/room105.h"
#include "m4/burger/rooms/section1/room106.h"
#include "m4/burger/rooms/section1/room120.h"
#include "m4/burger/rooms/section1/room133_136.h"
#include "m4/burger/rooms/section1/room134_135.h"
#include "m4/burger/rooms/section1/room137.h"
#include "m4/burger/rooms/section1/room138.h"
#include "m4/burger/rooms/section1/room139_144.h"
#include "m4/burger/rooms/section1/room140_141.h"
#include "m4/burger/rooms/section1/room142.h"
#include "m4/burger/rooms/section1/room143.h"
#include "m4/burger/rooms/section1/room145.h"
#include "m4/burger/rooms/section1/room170.h"
#include "m4/burger/rooms/section1/room171.h"
#include "m4/burger/rooms/section1/room172.h"
#include "m4/burger/rooms/section1/room173.h"
#include "m4/burger/rooms/section1/room174.h"
#include "m4/burger/rooms/section1/room175.h"
#include "m4/burger/rooms/section1/room176.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum Goto1 {
	GOTO_TOWN_HALL = 1001,
	GOTO_OLD_BRIDGE = 1008
};

class Section1 : public Rooms::Section {
private:
	Room101 _room101;
	Room102 _room102;
	Room103 _room103;
	Room104 _room104;
	Room105 _room105;
	Room106 _room106;
	Room120 _room120;
	Room133_136 _room133_136;
	Room134_135 _room134_135;
	Room137 _room137;
	Room138 _room138;
	Room139_144 _room139_144;
	Room140_141 _room140_141;
	Room142 _room142;
	Room143 _room143;
	Room145 _room145;
	Room170 _room170;
	Room171 _room171;
	Room172 _room172;
	Room173 _room173;
	Room174 _room174;
	Room175 _room175;
	Room176 _room176;

	int _trigger = 0;
	int _series1 = -1;
	int _series2 = -1;
	machine *_play = nullptr;
	int _state1 = 0;

	void updateWalker_(int x, int y, int dir, int trigger, bool mode);
public:
	Section1();
	virtual ~Section1() {}

	void daemon() override;

	static void updateWalker(int x, int y, int dir, int trigger, bool mode = false);
	static void updateDisablePlayer();
	static void walk(int facing = -1, int trigger = -1);
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
