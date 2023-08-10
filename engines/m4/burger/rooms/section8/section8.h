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

#ifndef M4_BURGER_ROOMS_SECTION8_H
#define M4_BURGER_ROOMS_SECTION8_H

#include "m4/burger/rooms/room.h"
#include "m4/burger/rooms/section.h"
#include "m4/burger/rooms/section8/room801.h"
#include "m4/burger/rooms/section8/room802.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Section8 : public Rooms::Section {
private:
	Room801 _room801;
	Room802 _room802;

public:
	Section8();
	virtual ~Section8() {}
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
