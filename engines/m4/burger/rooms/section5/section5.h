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

#ifndef M4_BURGER_ROOMS_SECTION5_H
#define M4_BURGER_ROOMS_SECTION5_H

#include "m4/burger/rooms/room.h"
#include "m4/burger/rooms/section.h"
#include "m4/burger/rooms/section5/room501.h"
#include "m4/burger/rooms/section5/room502.h"
#include "m4/burger/rooms/section5/room503.h"
#include "m4/burger/rooms/section5/room504.h"
#include "m4/burger/rooms/section5/room505.h"
#include "m4/burger/rooms/section5/room505.h"
#include "m4/burger/rooms/section5/room506.h"
#include "m4/burger/rooms/section5/room507.h"
#include "m4/burger/rooms/section5/room508.h"
#include "m4/burger/rooms/section5/room509.h"
#include "m4/burger/rooms/section5/room510.h"
#include "m4/burger/rooms/section5/room511.h"
#include "m4/burger/rooms/section5/room512.h"
#include "m4/burger/rooms/section5/room513.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Section5 : public Rooms::Section {
private:
	Room501 _room501;
	Room502 _room502;
	Room503 _room503;
	Room504 _room504;
	Room505 _room505;
	Room506 _room506;
	Room507 _room507;
	Room508 _room508;
	Room509 _room509;
	Room510 _room510;
	Room511 _room511;
	Room512 _room512;
	Room513 _room513;

	void takeGizmo(RGB8 *pal);

public:
	static const char *PARSER[];
	static const seriesPlayBreak PLAY1[];
	static machine *_bork;
	static const char *_digiName;
	static const char *_bgDigiName;

	static bool checkFlags();
	static void flagsTrigger();

public:
	Section5();
	virtual ~Section5() {}

	void init() override;
	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
