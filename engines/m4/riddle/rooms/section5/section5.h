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

#ifndef M4_RIDDLE_ROOMS_SECTION5_H
#define M4_RIDDLE_ROOMS_SECTION5_H

#include "m4/riddle/rooms/section.h"
#include "m4/riddle/rooms/section5/room501.h"
#include "m4/riddle/rooms/section5/room504.h"
#include "m4/riddle/rooms/section5/room506.h"
#include "m4/riddle/rooms/section5/room507.h"
#include "m4/riddle/rooms/section5/room508.h"
#include "m4/riddle/rooms/section5/room509.h"
#include "m4/riddle/rooms/section5/room510.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Section5 : public Section {
private:
	Room501 _room501;
	Room504 _room504;
	Room506 _room506;
	Room507 _room507;
	Room508 _room508;
	Room509 _room509;
	Room510 _room510;

public:
	Section5();
	virtual ~Section5() {}

	void daemon() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
