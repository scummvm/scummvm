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

#ifndef M4_RIDDLE_ROOMS_SECTION3_H
#define M4_RIDDLE_ROOMS_SECTION3_H

#include "m4/riddle/rooms/section.h"
#include "m4/riddle/rooms/section3/room301.h"
#include "m4/riddle/rooms/section3/room303.h"
#include "m4/riddle/rooms/section3/room304.h"
#include "m4/riddle/rooms/section3/room305.h"
#include "m4/riddle/rooms/section3/room309.h"
#include "m4/riddle/rooms/section3/room352.h"
#include "m4/riddle/rooms/section3/room354.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Section3 : public Section {
private:
	Room301 _room301;
	Room303 _room303;
	Room304 _room304;
	Room305 _room305;
	Room309 _room309;
	Room352 _room352;
	Room354 _room354;

public:
	Section3();
	virtual ~Section3() {}

	void daemon() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
