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

#ifndef M4_BURGER_ROOMS_SECTION1_ROOM138_H
#define M4_BURGER_ROOMS_SECTION1_ROOM138_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room138 : public Room {
private:
	machine *_series1 = nullptr;
	machine *_series2 = nullptr;
	int _series3 = 0;
	int _frame = 0;
	int _deputyMode = 0;
	int _deputyShould = 0;
	int _val4 = 0;
	int _sherrifMode = 0;
	int _sherrifShould = 0;
	int _wilburMode = 0;
	int _wilburShould = 0;
	bool _assetsFreed = false;
	bool _flag1 = false;

	void freeAssets();
	void randomDigi();
	void loadSeries();
	void freeSeries();

public:
	Room138() : Room() {}
	~Room138() override {}

	void preload() override;
	void init() override;
	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
