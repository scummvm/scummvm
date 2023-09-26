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

#ifndef M4_BURGER_ROOMS_SECTION4_H
#define M4_BURGER_ROOMS_SECTION4_H

#include "m4/burger/rooms/room.h"
#include "m4/burger/rooms/section.h"
#include "m4/burger/rooms/section4/room401.h"
#include "m4/burger/rooms/section4/room402.h"
#include "m4/burger/rooms/section4/room404.h"
#include "m4/burger/rooms/section4/room405.h"
#include "m4/burger/rooms/section4/room406.h"
#include "m4/burger/rooms/section4/room407.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Section4 : public Rooms::Section {
	struct TeleportEntry {
		int _room;
		int _newRoom1;
		int _newRoom2;
	};
private:
	static const char *PARSER[];
	static const TeleportEntry TELEPORTS[];

private:
	Room401 _room401;
	Room402 _room402;
	Room404 _room404;
	Room405 _room405;
	Room406 _room406;
	Room407 _room407;

	bool teleport();

public:
	static void poof(int trigger);
	static bool checkOrderWindow();

public:
	Section4();
	virtual ~Section4() {}

	void init() override;
	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
