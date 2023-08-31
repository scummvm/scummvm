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

#ifndef M4_BURGER_ROOMS_SECTION7_H
#define M4_BURGER_ROOMS_SECTION7_H

#include "m4/burger/rooms/room.h"
#include "m4/burger/rooms/section.h"
#include "m4/burger/rooms/section7/room701.h"
#include "m4/burger/rooms/section7/room702.h"
#include "m4/burger/rooms/section7/room706.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Section7 : public Rooms::Section {
private:
	Room701 _room701;
	Room702 _room702;
	Room706 _room706;

public:
	Section7();
	virtual ~Section7() {}
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
