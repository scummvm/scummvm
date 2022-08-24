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

// Based on Phantasma code by Thomas Harte (2013)

#include "freescape/objects/entrance.h"

namespace Freescape {

RoomStructure::RoomStructure(const Common::Array<byte> _structure) {
	objectID = 255;
	structure = _structure;
}

Entrance::Entrance(
	uint16 _objectID,
	const Math::Vector3d &_origin,
	const Math::Vector3d &_rotation) {
	objectID = _objectID;
	origin = _origin;
	rotation = _rotation;
	flags = 0;
}

Entrance::~Entrance() {}

bool Entrance::isDrawable() { return false; }
bool Entrance::isPlanar() { return true; }

} // End of namespace Freescape