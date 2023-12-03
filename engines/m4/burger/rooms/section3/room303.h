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

#ifndef M4_BURGER_ROOMS_SECTION3_ROOM303_H
#define M4_BURGER_ROOMS_SECTION3_ROOM303_H

#include "m4/burger/rooms/section3/section3_room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room303 : public Section3Room {
private:
	static const char *SAID[][4];
	static const seriesStreamBreak SERIES1[];
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
	machine *_series1 = nullptr;
	int _series2 = -1;
	machine *_series3 = nullptr;
	int _series4 = -1;
	machine *_series5 = nullptr;
	int _series6 = -1;
	machine *_series7 = nullptr;
	machine *_series8 = nullptr;
	machine *_series9 = nullptr;
	int _triggers[5];
	int _ctr = 0;
	int _timer = 0;
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val7 = 0;
	int _val8 = 0;

	void frontYard();
	void doDaemon(int trigger);
	void freeSeries1();
	void freeSeries2();

protected:
	const char *getDigi() override;

public:
	Room303();
	~Room303() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
