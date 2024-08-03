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

#include "m4/riddle/rooms/section3/section3.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

const int16 S3_NORMAL_DIRS[] = { 200, 201, 202, -1 };
const char *S3_NORMAL_NAMES[] = {
	"Feng Li walk pos3",
	"Feng Li walk pos4",
	"Feng Li walk pos5"
};
const int16 S3_SHADOW_DIRS[] = { 210, 211, 212, -1 };
const char *S3_SHADOW_NAMES[] = {
	"candleman shadow3",
	"candleman shadow4",
	"candleman shadow5"
};

const int16 S3_NORMAL_DIRS2[] = { 220, 221, 222, -1 };
const char *S3_NORMAL_NAMES2[] = {
	"mei chen ny walker pos3",
	"mei chen ny walker pos4",
	"mei chen ny walker pos5"
};
const int16 S3_SHADOW_DIRS2[] = { 230, 231, 232, -1 };
const char *S3_SHADOW_NAMES2[] = {
	"candleman shadow3",
	"candleman shadow4",
	"candleman shadow5"
};


Section3::Section3() : Rooms::Section() {
	add(301, &_room301);
	add(303, &_room303);
	add(304, &_room304);
	add(305, &_room305);
	add(309, &_room309);
	add(352, &_room352);
	add(354, &_room354);
}

void Section3::daemon() {
	_G(kernel).continue_handling_trigger = true;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
