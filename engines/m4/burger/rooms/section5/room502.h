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

#ifndef M4_BURGER_ROOMS_SECTION5_ROOM502_H
#define M4_BURGER_ROOMS_SECTION5_ROOM502_H

#include "m4/burger/rooms/section5/section5_room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room502 : public Section5Room {
private:
	static const BorkPoint BORK_TABLE[];
	noWalkRect *_walk1 = nullptr;
	Series _series1;
	machine *_series2 = nullptr;
	bool _flag1 = false;
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;

	void loadSeries1();
	void loadSeries2();
	void loadSeries3();
	void setup1();
	void setup2();
	void setup3();

public:
	Room502() : Section5Room() {}
	~Room502() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
