/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/resources/floorfield.h"

#include "engines/stark/formats/xrc.h"

namespace Stark {
namespace Resources {

FloorField::~FloorField() {
}

FloorField::FloorField(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name) {
	_type = TYPE;
}

void FloorField::readData(Formats::XRCReadStream *stream) {
	uint32 count = stream->readUint32LE();
	for (uint i = 0; i < count; i++) {
		_facesInFloorField.push_back(stream->readByte());
	}
}

bool FloorField::hasFace(int32 floorFaceIndex) const {
	if (floorFaceIndex < 0 || floorFaceIndex >= (int32) _facesInFloorField.size()) {
		return false;
	}

	return _facesInFloorField[floorFaceIndex] != 0;
}

void FloorField::printData() {
	for (uint i = 0; i < _facesInFloorField.size(); i++) {
		debug("faceInFloorField[%d]: %d", i, _facesInFloorField[i]);
	}
}

} // End of namespace Resources
} // End of namespace Stark
