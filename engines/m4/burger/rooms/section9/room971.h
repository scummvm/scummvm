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

#ifndef M4_BURGER_ROOMS_SECTION9_ROOM971_H
#define M4_BURGER_ROOMS_SECTION9_ROOM971_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room971 : public Rooms::Room {
private:
	static const seriesStreamBreak SERIES1[];
	static const seriesStreamBreak SERIES2[];
	static const seriesStreamBreak SERIES3[];
	static const seriesStreamBreak SERIES4[];
	static const seriesStreamBreak SERIES5[];
	static const seriesStreamBreak SERIES6[];
	static const seriesStreamBreak SERIES7[];
	static const seriesStreamBreak SERIES8[];
	static const seriesStreamBreak SERIES9[];
	static const seriesStreamBreak SERIES10[];
	static const seriesStreamBreak SERIES11[];
	static const seriesStreamBreak SERIES12[];
	static const seriesStreamBreak SERIES13[];
	static const seriesStreamBreak SERIES14[];
	static const seriesStreamBreak SERIES15[];
	static const seriesStreamBreak SERIES16[];
	int32 _comeSoonS = 0;
	machine *_comeSoon = nullptr;

public:
	Room971() : Rooms::Room() {}
	~Room971() override {}

	void preload() override;
	void init() override;
	void daemon() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
