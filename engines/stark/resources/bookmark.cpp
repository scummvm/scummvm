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

#include "engines/stark/resources/bookmark.h"

#include "engines/stark/formats/xrc.h"

#include "engines/stark/resources/floor.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"

namespace Stark {
namespace Resources {

Bookmark::~Bookmark() {
}

Bookmark::Bookmark(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name) {
	_type = TYPE;
}

Math::Vector3d Bookmark::getPosition() const {
	Floor *floor = StarkGlobal->getCurrent()->getFloor();

	Math::Vector3d position = _position;

	int32 floorFaceIndex = floor->findFaceContainingPoint(position);
	floor->computePointHeightInFace(position, floorFaceIndex);

	return position;
}

void Bookmark::readData(Formats::XRCReadStream *stream) {
	_position.x() = stream->readFloatLE();
	_position.y() = stream->readFloatLE();
	_position.z() = 0;
}

void Bookmark::printData() {
	Common::StreamDebug debug = streamDbg();
	debug << "position: " << _position << "\n";
}

} // End of namespace Resources
} // End of namespace Stark
