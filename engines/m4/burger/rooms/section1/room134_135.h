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

#ifndef M4_BURGER_ROOMS_SECTION1_ROOM134_135_H
#define M4_BURGER_ROOMS_SECTION1_ROOM134_135_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room134_135 : public Room {
private:
	int _volume = 0;
	int _val2 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val7 = 0;
	int _val8 = 0;
	int _val9 = 0;
	int _val10 = 0;
	int _val11 = 0;
	bool _flag1 = false;
	bool _odieLoaded = false;
	machine *_series1 = nullptr;
	machine *_series2 = nullptr;
	machine *_series3 = nullptr;
	machine *_series4 = nullptr;
	machine *_series5 = nullptr;
	machine *_series6 = nullptr;
	machine *_play1 = nullptr;
	machine *_play2 = nullptr;
	machine *_play3 = nullptr;
	machine *_play4 = nullptr;
	int _xp = 0, _yp = 0;
	int _frame = 0;
	const char *_name1 = nullptr;

	void conv01();
	void conv02();
	void conv03();
	void loadOdie();
	void loadDigi();

public:
	Room134_135() : Room() {}
	~Room134_135() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
