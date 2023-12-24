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

#ifndef M4_BURGER_ROOMS_SECTION1_ROOM172_H
#define M4_BURGER_ROOMS_SECTION1_ROOM172_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room172 : public Room {
private:
	machine *_series1 = nullptr;
	machine *_series1s = nullptr;
	Series _jug;
	//machine *_series4 = nullptr;
	Series _series5;
	machine *_series7 = nullptr;
	noWalkRect *_walk1 = nullptr;
	const char *_convName = nullptr;
	int _pollyShould = 0;

	void loadSeries7();
	void freeSeries5() {
		_series5.terminate();
	}
	void freeSeries7();
	void playDigi3();
	void conv41();
	void parserAction(const char *name);

public:
	Room172() : Room() {}
	~Room172() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
