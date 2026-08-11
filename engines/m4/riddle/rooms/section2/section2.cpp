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

#include "m4/riddle/rooms/section2/section2.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

const int16 S2_MEI_NORMAL_DIRS[] = { 220, 221, 222, 223, 224, -1 };
const char *S2_MEI_NORMAL_NAMES[] = {
	"mei chen trek walk pos1",
	"mei chen trek walk pos2",
	"mei chen trek walk pos3",
	"mei chen trek walk pos4",
	"mei chen trek walk pos5"
};
const int16 S2_MEI_SHADOW_DIRS[] = { 230, 231, 232, 233, 234, -1 };
const char *S2_MEI_SHADOW_NAMES[] = {
	"mctsh1",
	"mctsh2",
	"mctsh3",
	"mctsh4",
	"mctsh5"
};

const int16 S2_PEASANT_NORMAL_DIRS[] = { 240, -1 };
const char *S2_PEASANT_NORMAL_NAMES[] = { "pesky walk pos3" };
const int16 S2_PEASANT_SHADOW_DIRS[] = { 250, -1 };
const char *S2_PEASANT_SHADOW_NAMES[] = { "shadow pesky walk pos3" };


Section2::Section2() : Rooms::Section() {
	add(201, &_room201);
	add(202, &_room202);
	add(203, &_room203);
	add(204, &_room204);
	add(205, &_room205);
	add(207, &_room207);
}

void Section2::daemon() {
	_G(kernel).continue_handling_trigger = true;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
