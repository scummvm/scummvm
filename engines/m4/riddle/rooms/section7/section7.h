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

#ifndef M4_RIDDLE_ROOMS_SECTION7_H
#define M4_RIDDLE_ROOMS_SECTION7_H

#include "m4/riddle/rooms/section.h"
#include "m4/riddle/rooms/section7/room701.h"
#include "m4/riddle/rooms/section7/room702.h"
#include "m4/riddle/rooms/section7/room703.h"
#include "m4/riddle/rooms/section7/room704.h"
#include "m4/riddle/rooms/section7/room705.h"
#include "m4/riddle/rooms/section7/room706.h"
#include "m4/riddle/rooms/section7/room707.h"
#include "m4/riddle/rooms/section7/room709.h"
#include "m4/riddle/rooms/section7/room710.h"
#include "m4/riddle/rooms/section7/room711.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Section7 : public Section {
private:
	Room701 _room701;
	Room702 _room702;
	Room703 _room703;
	Room704 _room704;
	Room705 _room705;
	Room706 _room706;
	Room707 _room707;
	Room709 _room709;
	Room710 _room710;
	Room711 _room711;

public:
	Section7();
	virtual ~Section7() {}

	void daemon() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
