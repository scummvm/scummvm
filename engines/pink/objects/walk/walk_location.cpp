/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"

#include "pink/archive.h"
#include "pink/pink.h"
#include "pink/objects/walk/walk_location.h"

namespace Pink {

void WalkLocation::deserialize(Pink::Archive &archive) {
	NamedObject::deserialize(archive);
	_neighbors.deserialize(archive);
}

void WalkLocation::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\tWalkLocation: _name =%s", _name.c_str());
	debugC(6, kPinkDebugLoadingObjects, "\tNeighbors:");
	for (uint i = 0; i < _neighbors.size(); ++i) {
		debugC(6, kPinkDebugLoadingObjects, "\t\t%s", _neighbors[i].c_str());
	}
}

} // End of namespace Pink
