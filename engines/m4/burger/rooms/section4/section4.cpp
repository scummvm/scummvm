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

#include "m4/burger/rooms/section4/section4.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Section4::PARSER[] = {
	"DEED",
	"TAKE",
	nullptr,
	"WILBUR",
	"400w001",
	nullptr,
	nullptr,
	"Th-th-th-that's all folks...",
	"JAWZ O' LIFE",
	"TAKE",
	nullptr,
	"BARRED WINDOW",
	"404w102",
	"BARRED WINDOW ",
	"404w102",
	"WILBUR",
	"400w001",
	nullptr,
	nullptr,
	"Th-th-th-that's all folks...",
	"FISH",
	"LOOK AT",
	"400w100",
	"TAKE",
	nullptr,
	"MUFFIN",
	"405w101",
	"WILBUR",
	"400w101",
	nullptr,
	"400w102",
	"Th-th-th-that's all folks...",
	"DOG COLLAR",
	"LOOK AT",
	"400w103",
	"TAKE",
	nullptr,
	"PEGLEG  ",
	"406w101",
	"WILBUR",
	"400w104",
	nullptr,
	"400W105",
	"Th-th-th-that's all folks...",
	"HOOK",
	"LOOK AT",
	"400w106",
	"TAKE",
	nullptr,
	"WILBUR",
	"400w001",
	nullptr,
	"400w107",
	"Th-th-th-that's all folks...",
	"RECORDS",
	"LOOK AT",
	"400w108",
	"TAKE",
	nullptr,
	"WILBUR",
	"400w001",
	nullptr,
	"400w109",
	"Th-th-th-that's all folks...",
	"QUARTER",
	"LOOK AT",
	"400w110",
	"TAKE",
	nullptr,
	"WILBUR",
	"400w001",
	nullptr,
	"400w111",
	"Th-th-th-that's all folks...",
	"DISC",
	"LOOK AT",
	"400w002",
	"TAKE",
	"400w003",
	nullptr,
	"400w004",
	"Th-th-th-that's all folks...",
	"DISC ",
	"LOOK AT",
	"400w002",
	"TAKE",
	"400w003",
	nullptr,
	"400w004",
	"Th-th-th-that's all folks...",
	nullptr
};
	

Section4::Section4() : Rooms::Section() {
	add(401, &_room401);
	add(402, &_room402);
	add(404, &_room404);
	add(405, &_room405);
	add(406, &_room406);
	add(407, &_room407);
}

void Section4::daemon() {
	// TODO
	_G(kernel).continue_handling_trigger = true;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
