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

#ifndef M4_BURGER_ROOMS_SECTION6_ROOM602_H
#define M4_BURGER_ROOMS_SECTION6_ROOM602_H

#include "m4/burger/rooms/section6/section6_room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room602 : public Section6Room {
	static const GerbilPoint GERBIL[];
	static const seriesPlayBreak PLAY1[];
	static const seriesPlayBreak PLAY2[];
	static const seriesPlayBreak PLAY3[];
	static const seriesPlayBreak PLAY4[];
	static const seriesPlayBreak PLAY5[];
	static const seriesPlayBreak PLAY6[];
	static const seriesPlayBreak PLAY7[];
	static const seriesPlayBreak PLAY8[];
	static const seriesPlayBreak PLAY9[];
	static const seriesPlayBreak PLAY10[];
	static const seriesPlayBreak PLAY11[];
	static const seriesPlayBreak PLAY12[];
	static const seriesPlayBreak PLAY13[];
	static const seriesPlayBreak PLAY14[];
	static const seriesPlayBreak PLAY15[];
	static const seriesPlayBreak PLAY16[];
	static const seriesPlayBreak PLAY17[];
	static const seriesPlayBreak PLAY18[];
	static const seriesPlayBreak PLAY19[];
	static const seriesPlayBreak PLAY20[];
	static const seriesPlayBreak PLAY21[];
	static const seriesPlayBreak PLAY22[];

private:
	noWalkRect *_walk1 = nullptr;
	int _series1 = 0;
	machine *_series2 = nullptr;
	machine *_series3 = nullptr;
	int _series4 = 0;
	int _series5 = 0;
	int _series6 = 0;
	int _series7 = 0;
	machine *_series8 = nullptr;
	machine *_series9 = nullptr;
	machine *_series10 = nullptr;
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	static long _test1;
	static long _test2;

public:
	Room602();
	~Room602() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
