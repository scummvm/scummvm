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

#include "m4/riddle/rooms/section8/section8.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

const int16 S8_SHADOW_DIRS1[] = { 230, 231, 232, 233, 234, -1 };
const char *S8_SHADOW_NAMES1[] = {
	"mctsh1",
	"mctsh2",
	"mctsh3",
	"mctsh4",
	"mctsh5"
};

const int16 S8_SHADOW_DIRS2[] = {220, 221, 222, 223, 224, -1};
const char *S8_SHADOW_NAMES2[] = {
	"mei chen trek walk pos1",
	"mei chen trek walk pos2",
	"mei chen trek walk pos3",
	"mei chen trek walk pos4",
	"mei chen trek walk pos5"
};


Section8::Section8() : Rooms::Section() {
	add(801, &_room801);
	add(802, &_room802);
	add(803, &_room803);
	add(804, &_room804);
	add(805, &_room805);
	add(806, &_room806);
	add(807, &_room807);
	add(808, &_room808);
	add(809, &_room809);
	add(810, &_room810);
	add(811, &_room811);
	add(814, &_room814);
	add(818, &_room818);
	add(824, &_room824);
	add(834, &_room834);
	add(844, &_room844);
	add(850, &_room850);
	add(860, &_room860);
	add(861, &_room861);
}

void Section8::daemon() {
	_G(kernel).continue_handling_trigger = true;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
