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

#ifndef M4_BURGER_ROOMS_SECTION1_ROOM103_H
#define M4_BURGER_ROOMS_SECTION1_ROOM103_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room103 : public Room {
	static const seriesStreamBreak SERIES1[];
	static const seriesStreamBreak SERIES2[];
	static const seriesStreamBreak SERIES3[];
	static const seriesStreamBreak SERIES4[];
private:
	static int32 _val0;
	bool _flag1 = false;
	int _val2 = 0;
	int _val3 = 0;
	int _crow1Should = 0;
	int _crow3Should = 0;
	int _crow4Should = 0;
	int _val7 = 0;
	int _harryShould = 0;
	int _crow2Should = 0;
	int _frame = 0;
	int _digi1 = 0;
	machine *_series1 = nullptr;
	machine *_series2 = nullptr;

	void preloadDigi1();

public:
	Room103() : Room() {}
	~Room103() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
