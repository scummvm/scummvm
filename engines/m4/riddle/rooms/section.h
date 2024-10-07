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

#ifndef M4_RIDDLE_ROOMS_SECTION_H
#define M4_RIDDLE_ROOMS_SECTION_H

#include "m4/core/rooms.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Section : public M4::Section {
public:
	Section() : M4::Section() {}
	~Section() override {}

	void global_room_init() override;
	void tick() override;
	void daemon() override;
};

/**
 * Dummy section 1, since Riddle doesn't have any rooms in it
 */
class Section1 : public Section {
public:
	Section1() : Section() {}
	~Section1() override {}
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
