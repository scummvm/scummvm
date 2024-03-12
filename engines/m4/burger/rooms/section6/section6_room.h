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

#ifndef M4_BURGER_ROOMS_SECTION6_ROOM_H
#define M4_BURGER_ROOMS_SECTION6_ROOM_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Section6Room : public Rooms::Room {
private:
	char _gerbilsName[16];
	char _gerbilsVerb[16];
protected:
	struct GerbilPoint {
		int16 _x;
		int16 _y;
	};
	const GerbilPoint *_gerbilTable = nullptr;
	HotSpotRec _gerbilHotspot;
	machine *_sectionMachine1 = nullptr;
	machine *_sectionMachine2 = nullptr;
	int32 _sectionSeries1 = 0;
	int32 _sectionSeries2 = 0;
	int32 _sectionSeries3 = 0;

public:
	Section6Room();
	~Section6Room() override {}

	HotSpotRec *custom_hotspot_which(int32 x, int32 y) override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
