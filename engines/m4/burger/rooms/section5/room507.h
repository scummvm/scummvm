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

#ifndef M4_BURGER_ROOMS_SECTION5_ROOM507_H
#define M4_BURGER_ROOMS_SECTION5_ROOM507_H

#include "m4/burger/rooms/section5/section5_room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room507 : public Section5Room {
private:
	static const char *SAID[][4];
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
	static int32 _state1;
	static int32 _state2;
	static int32 _state3;
	static int32 _state4;
	static int32 _state5;
	machine *_series1 = nullptr;
	machine *_series2 = nullptr;
	machine *_series3 = nullptr;
	machine *_series4 = nullptr;
	int _ctr = 0;
	int _triggers[5];
	bool _flag1 = false;
	bool _flag2 = false;
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _itemNum = 0;

	void loadSeries();

public:
	Room507();
	~Room507() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
