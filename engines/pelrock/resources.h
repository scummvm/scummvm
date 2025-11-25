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
#ifndef PELROCK_RESOURCES_H
#define PELROCK_RESOURCES_H

#include "common/array.h"
#include "common/file.h"
#include "common/scummsys.h"

#include "pelrock/types.h"

namespace Pelrock {

class ResourceManager {
public:
	ResourceManager();
	~ResourceManager();
	void loadRoomMetadata(Common::File *roomFile, int roomOffset);
	void loadRoomTalkingAnimations(int roomNumber);

private:
	Common::Array<AnimSet> loadRoomAnimations(Common::File *roomFile, int roomOffset);
	Common::Array<HotSpot> loadHotspots(Common::File *roomFile, int roomOffset);
	Common::Array<Exit> loadExits(Common::File *roomFile, int roomOffset);
	Common::Array<WalkBox> loadWalkboxes(Common::File *roomFile, int roomOffset);
	Common::Array<Description> loadRoomDescriptions(Common::File *roomFile, int roomOffset, uint32_t &outPos);
};

} // End of namespace Pelrock

#endif
