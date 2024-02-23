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

#ifndef M4_BURGER_ROOMS_SECTION8_ROOM801_H
#define M4_BURGER_ROOMS_SECTION8_ROOM801_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room801 : public Room {
private:
	int _series1 = 0;
	int _series2 = 0;
	int _series3 = 0;
	int _series4 = 0;
	machine *_series5 = nullptr;
	machine *_series6 = nullptr;
	machine *_series7 = nullptr;
	machine *_series8 = nullptr;
	machine *_series9 = nullptr;
	machine *_series10 = nullptr;
	machine *_series11 = nullptr;
	machine *_series12 = nullptr;
	int _wilburMode = 10;
	int _zlargShould = 0;

	int getWilburShould() const;
	void loadSeries1();
	void loadSeries2();
	void freeSeries1();

public:
	Room801() : Room() {}
	~Room801() override {}

	void preload() override;
	void init() override;
	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
