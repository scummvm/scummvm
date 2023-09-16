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

#ifndef M4_BURGER_ROOMS_SECTION4_ROOM406_H
#define M4_BURGER_ROOMS_SECTION4_ROOM406_H

#include "m4/burger/rooms/room.h"
#include "m4/burger/walker.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room406 : public Room {
private:
	static const char *SAID[][4];
	static const WilburMatch MATCH[];
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
	static const seriesPlayBreak PLAY23[];
	static const seriesPlayBreak PLAY24[];
	static const seriesPlayBreak PLAY25[];
	static const seriesPlayBreak PLAY26[];
	static const seriesPlayBreak PLAY27[];
	static const seriesPlayBreak PLAY28[];
	static const seriesPlayBreak PLAY29[];
	static const seriesPlayBreak PLAY30[];
	static const seriesPlayBreak PLAY31[];
	static const seriesPlayBreak PLAY32[];
	static const seriesPlayBreak PLAY33[];
	static const seriesPlayBreak PLAY34[];
	static const seriesPlayBreak PLAY35[];
	static const seriesPlayBreak PLAY36[];
	static long _state1;
	static long _state2;
	static long _state3;
	static long _state4;

public:
	Room406();
	~Room406() override {}

	void init() override;
	void daemon() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
