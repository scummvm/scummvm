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

#ifndef M4_RIDDLE_ROOMS_SECTION4_H
#define M4_RIDDLE_ROOMS_SECTION4_H

#include "m4/riddle/rooms/section.h"
#include "m4/riddle/rooms/section4/room401.h"
#include "m4/riddle/rooms/section4/room402.h"
#include "m4/riddle/rooms/section4/room403.h"
#include "m4/riddle/rooms/section4/room404.h"
#include "m4/riddle/rooms/section4/room405.h"
#include "m4/riddle/rooms/section4/room406.h"
#include "m4/riddle/rooms/section4/room407.h"
#include "m4/riddle/rooms/section4/room408.h"
#include "m4/riddle/rooms/section4/room409.h"
#include "m4/riddle/rooms/section4/room410.h"
#include "m4/riddle/rooms/section4/room413.h"
#include "m4/riddle/rooms/section4/room456.h"
#include "m4/riddle/rooms/section4/room493.h"
#include "m4/riddle/rooms/section4/room494.h"
#include "m4/riddle/rooms/section4/room495.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Section4 : public Section {
private:
	Room401 _room401;
	Room402 _room402;
	Room403 _room403;
	Room404 _room404;
	Room405 _room405;
	Room406 _room406;
	Room407 _room407;
	Room408 _room408;
	Room409 _room409;
	Room410 _room410;
	Room413 _room413;
	Room456 _room456;
	Room493 _room493;
	Room494 _room494;
	Room495 _room495;

public:
	Section4();
	virtual ~Section4() {}

	void daemon() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
