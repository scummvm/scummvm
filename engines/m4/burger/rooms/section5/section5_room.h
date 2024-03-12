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

#ifndef M4_BURGER_ROOMS_SECTION5_ROOM_H
#define M4_BURGER_ROOMS_SECTION5_ROOM_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Section5Room : public Rooms::Room {
protected:
	struct BorkPoint {
		int16 _x;
		int16 _y;
	};
	const BorkPoint *_borkTable = nullptr;
	Series _borkStairs;
	HotSpotRec _borkHotspot;
	int _borkThreshold = 0;
	bool _initFlag = false;

public:
	Section5Room() : Rooms::Room() {
	}
	~Section5Room() override {}

	void preload() override;
	void init() override;

	/**
	 * Used to tell if x,y is over the walker hotspot
	 */
	HotSpotRec *custom_hotspot_which(int32 x, int32 y) override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
