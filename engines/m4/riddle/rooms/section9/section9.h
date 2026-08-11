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

#ifndef M4_RIDDLE_ROOMS_SECTION9_H
#define M4_RIDDLE_ROOMS_SECTION9_H

#include "m4/riddle/rooms/section.h"
#include "m4/riddle/rooms/section9/room901.h"
#include "m4/riddle/rooms/section9/room902.h"
#include "m4/riddle/rooms/section9/room903.h"
#include "m4/riddle/rooms/section9/room905.h"
#include "m4/riddle/rooms/section9/room906.h"
#include "m4/riddle/rooms/section9/room907.h"
#include "m4/riddle/rooms/section9/room908.h"
#include "m4/riddle/rooms/section9/room917.h"
#include "m4/riddle/rooms/section9/room918.h"
#include "m4/riddle/rooms/section9/room996.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Section9 : public Section {
private:
	Room901 _room901;
	Room902 _room902;
	Room903 _room903;
	Room905 _room905;
	Room906 _room906;
	Room907 _room907;
	Room908 _room908;
	Room917 _room917;
	Room918 _room918;
	Room996 _room996;
public:
	Section9();
	virtual ~Section9() {}

	void daemon() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
