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

#ifndef M4_BURGER_ROOMS_SECTION1_ROOM142_H
#define M4_BURGER_ROOMS_SECTION1_ROOM142_H

#include "m4/burger/rooms/room.h"
#include "m4/burger/walker.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room142 : public Room {
	static const WilburMatch MATCH[];
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
private:
	machine *_series1 = nullptr;
	machine *_series2 = nullptr;
	machine *_series3 = nullptr;
	machine *_series4 = nullptr;
	machine *_series5 = nullptr;
	machine *_series6 = nullptr;
	machine *_series7 = nullptr;
	machine *_series8 = nullptr;

	noWalkRect *_noWalk = nullptr;
	int _actionType = 0;
	int _volume = 0;
	static int32 _val1;
	static int32 _val2;
	static int32 _val3;
	//int _val4 = 0;

	void checkAction();
	void faceTruck();
	void preloadAssets();
	void preloadAssets2();
	void triggerParser();
	void play015();

	/**
	 * Gets the X position for the edge of the road, given a Y pos.
	 * Used for the cutscene where Burl arrives, to determine whether
	 * he honks the horn at Wilbur to get off the road
	 */
	int getRoadEdgeX(int y) const {
		return (int)((double)(-y + 126) * -1.428571428571429);
	}

public:
	Room142();
	~Room142() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
