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

#ifndef HARVESTER_SAVELOAD_H
#define HARVESTER_SAVELOAD_H

#include "common/str.h"

namespace Harvester {

struct SaveRoomState {
	Common::String entranceName;
	Common::String roomName;
	Common::String musicPath;
	int playerX = 0;
	int playerY = 0;
	int playerZ = 0;
	int playerFacing = -1;
	int discNumber = 0;
	bool valid = false;

	void clear() {
		entranceName.clear();
		roomName.clear();
		musicPath.clear();
		playerX = 0;
		playerY = 0;
		playerZ = 0;
		playerFacing = -1;
		discNumber = 0;
		valid = false;
	}
};

} // End of namespace Harvester

#endif // HARVESTER_SAVELOAD_H
