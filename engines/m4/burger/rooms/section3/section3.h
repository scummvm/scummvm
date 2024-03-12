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

#ifndef M4_BURGER_ROOMS_SECTION3_H
#define M4_BURGER_ROOMS_SECTION3_H

#include "m4/burger/rooms/room.h"
#include "m4/burger/rooms/section.h"
#include "m4/burger/rooms/section3/room301.h"
#include "m4/burger/rooms/section3/room302.h"
#include "m4/burger/rooms/section3/room303.h"
#include "m4/burger/rooms/section3/room304.h"
#include "m4/burger/rooms/section3/room305.h"
#include "m4/burger/rooms/section3/room306.h"
#include "m4/burger/rooms/section3/room307.h"
#include "m4/burger/rooms/section3/room310.h"
#include "m4/burger/rooms/section3/mine.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Section3 : public Rooms::Section {
private:
	Room301 _room301;
	Room302 _room302;
	Room303 _room303;
	Room304 _room304;
	Room305 _room305;
	Room306 _room306;
	Room307 _room307;
	Room310 _room310;
	Mine _mine;
	int _random1 = 0;

	void mine_check_maze();

public:
	Section3();
	virtual ~Section3() {}

	void init() override {
		mine_check_maze();
	}
	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
