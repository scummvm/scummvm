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

#ifndef M4_RIDDLE_ROOMS_SECTION2_H
#define M4_RIDDLE_ROOMS_SECTION2_H

#include "m4/riddle/rooms/section.h"
#include "m4/riddle/rooms/section2/room201.h"
#include "m4/riddle/rooms/section2/room202.h"
#include "m4/riddle/rooms/section2/room203.h"
#include "m4/riddle/rooms/section2/room204.h"
#include "m4/riddle/rooms/section2/room205.h"
#include "m4/riddle/rooms/section2/room207.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

extern const int16 S2_MEI_NORMAL_DIRS[];
extern const char *S2_MEI_NORMAL_NAMES[];
extern const int16 S2_MEI_SHADOW_DIRS[];
extern const char *S2_MEI_SHADOW_NAMES[];

extern const int16 S2_PEASANT_NORMAL_DIRS[];
extern const char *S2_PEASANT_NORMAL_NAMES[];
extern const int16 S2_PEASANT_SHADOW_DIRS[];
extern const char *S2_PEASANT_SHADOW_NAMES[];

class Section2 : public Section {
private:
	Room201 _room201;
	Room202 _room202;
	Room203 _room203;
	Room204 _room204;
	Room205 _room205;
	Room207 _room207;
public:
	Section2();
	virtual ~Section2() {}

	void daemon() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
