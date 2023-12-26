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

#ifndef M4_BURGER_ROOMS_SECTION6_H
#define M4_BURGER_ROOMS_SECTION6_H

#include "m4/burger/rooms/room.h"
#include "m4/burger/rooms/section.h"
#include "m4/burger/rooms/section6/room601.h"
#include "m4/burger/rooms/section6/room602.h"
#include "m4/burger/rooms/section6/room603.h"
#include "m4/burger/rooms/section6/room604.h"
#include "m4/burger/rooms/section6/room605.h"
#include "m4/burger/rooms/section6/room606.h"
#include "m4/burger/rooms/section6/room608.h"
#include "m4/burger/rooms/section6/room609.h"
#include "m4/burger/rooms/section6/room610.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Section6 : public Rooms::Section {
private:
	Room601 _room601;
	Room602 _room602;
	Room603 _room603;
	Room604 _room604;
	Room605 _room605;
	Room606 _room606;
	Room608 _room608;
	Room609 _room609;
	Room610 _room610;

	machine *_series1 = nullptr;
	machine *_series2 = nullptr;
	machine *_series3 = nullptr;
	int _val1 = 0;

	void freeDigi(int state);

public:
	static int _state1;
	static int _state2;
	static int _gerbilState;
	static int _state4;
	static int _savedX;
	static int _series603;

public:
	Section6();
	virtual ~Section6() {}

	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
