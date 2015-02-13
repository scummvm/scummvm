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

#include "engines/stark/resources/floor.h"

#include "engines/stark/debug.h"
#include "engines/stark/formats/xrc.h"
#include "engines/stark/resources/floorface.h"

namespace Stark {
namespace Resources {

Floor::Floor(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_facesCount(0) {
	_type = TYPE;
}

Floor::~Floor() {
}

Math::Vector3d Floor::getVertex(uint32 indice) const {
	return _vertices[indice];
}

int32 Floor::findFaceContainingPoint(const Math::Vector3d &point) const {
	for (uint32 i = 0; i < _faces.size(); i++) {
		if (_faces[i]->isPointInside(point)) {
			return i;
		}
	}

	return -1;
}

void Floor::computePointHeightInFace(Math::Vector3d &point, uint32 faceIndex) const {
	_faces[faceIndex]->computePointHeight(point);
}

float Floor::getDistanceFromCamera(uint32 faceIndex) const {
	FloorFace *face = _faces[faceIndex];
	return face->getDistanceFromCamera();
}

void Floor::readData(XRCReadStream *stream) {
	_facesCount = stream->readUint32LE();
	uint32 positionsCount = stream->readUint32LE();

	for (uint i = 0; i < positionsCount; i++) {
		Math::Vector3d v = stream->readVector3();
		_vertices.push_back(v);
	}
}

void Floor::onAllLoaded() {
	Object::onAllLoaded();

	_faces = listChildren<FloorFace>();
}

void Floor::printData() {
	debug("face count: %d", _facesCount);

	Common::Debug debug = streamDbg();
	for (uint i = 0; i < _vertices.size(); i++) {
		debug << i << ": " << _vertices[i] << "\n";
	}
}

} // End of namespace Resources
} // End of namespace Stark
