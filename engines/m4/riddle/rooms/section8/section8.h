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

#ifndef M4_RIDDLE_ROOMS_SECTION8_H
#define M4_RIDDLE_ROOMS_SECTION8_H

#include "m4/riddle/rooms/section.h"
#include "m4/riddle/rooms/section8/room801.h"
#include "m4/riddle/rooms/section8/room802.h"
#include "m4/riddle/rooms/section8/room803.h"
#include "m4/riddle/rooms/section8/room804.h"
#include "m4/riddle/rooms/section8/room805.h"
#include "m4/riddle/rooms/section8/room806.h"
#include "m4/riddle/rooms/section8/room807.h"
#include "m4/riddle/rooms/section8/room808.h"
#include "m4/riddle/rooms/section8/room809.h"
#include "m4/riddle/rooms/section8/room810.h"
#include "m4/riddle/rooms/section8/room811.h"
#include "m4/riddle/rooms/section8/room814.h"
#include "m4/riddle/rooms/section8/room818.h"
#include "m4/riddle/rooms/section8/room824.h"
#include "m4/riddle/rooms/section8/room834.h"
#include "m4/riddle/rooms/section8/room844.h"
#include "m4/riddle/rooms/section8/room850.h"
#include "m4/riddle/rooms/section8/room860.h"
#include "m4/riddle/rooms/section8/room861.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

extern const int16 S8_SHADOW_DIRS1[];
extern const char *S8_SHADOW_NAMES1[];

extern const int16 S8_SHADOW_DIRS2[];
extern const char *S8_SHADOW_NAMES2[];

class Section8 : public Section {
private:
	Room801 _room801;
	Room802 _room802;
	Room803 _room803;
	Room804 _room804;
	Room805 _room805;
	Room806 _room806;
	Room807 _room807;
	Room808 _room808;
	Room809 _room809;
	Room810 _room810;
	Room811 _room811;
	Room814 _room814;
	Room818 _room818;
	Room824 _room824;
	Room834 _room834;
	Room844 _room844;
	Room850 _room850;
	Room860 _room860;
	Room861 _room861;

public:
	Section8();
	virtual ~Section8() {}

	void daemon() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
