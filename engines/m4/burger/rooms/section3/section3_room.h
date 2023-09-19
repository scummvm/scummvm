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

#ifndef M4_BURGER_ROOMS_SECTION3_ROOM_H
#define M4_BURGER_ROOMS_SECTION3_ROOM_H

#include "m4/burger/rooms/room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Section3Room : public Rooms::Room {
private:
	const char *_digiName = nullptr;

protected:
	int _digiVolume = 0;

	virtual const char *getDigi() {
		return nullptr;
	}

	void set_palette_brightness(int start, int finish, int percent);
	void set_palette_brightness(int percent);

public:
	Section3Room() : Rooms::Room() {}
	virtual ~Section3Room() {}

	void preload() override;
	void setupDigi();
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
