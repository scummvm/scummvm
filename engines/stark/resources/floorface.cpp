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

#include "engines/stark/resources/floorface.h"

#include "engines/stark/debug.h"
#include "engines/stark/formats/xrc.h"
#include "engines/stark/resources/floor.h"

namespace Stark {
namespace Resources {

FloorFace::FloorFace(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		Resource(parent, subType, index, name),
		_distanceFromCamera(0),
		_unk2(0) {
	_type = TYPE;

	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		_indices[i] = 0;
	}
}

FloorFace::~FloorFace() {
}

bool FloorFace::isPointInside(const Math::Vector3d &point) const {
	// Compute the barycentric coordinates of the point in the triangle
	float area = 1.0 / 2.0
			* (-_vertices[1].y() * _vertices[2].x()
					+ _vertices[0].y() * (-_vertices[1].x() + _vertices[2].x())
					+ _vertices[0].x() * (_vertices[1].y() - _vertices[2].y())
					+ _vertices[1].x() * _vertices[2].y());

	int32 sign = area < 0 ? -1 : 1;

	float s = (_vertices[0].y() * _vertices[2].x() - _vertices[0].x() * _vertices[2].y()
			+ (_vertices[2].y() - _vertices[0].y()) * point.x()
			+ (_vertices[0].x() - _vertices[2].x()) * point.y())
					* sign;

	float t = (_vertices[0].x() * _vertices[1].y() - _vertices[0].y() * _vertices[1].x()
			+ (_vertices[0].y() - _vertices[1].y()) * point.x()
			+ (_vertices[1].x() - _vertices[0].x()) * point.y())
					* sign;

	// Check the coordinates are in the triangle
	return s > 0 && t > 0 && (s + t) < 2.0 * area * sign;
}

void FloorFace::computePointHeight(Math::Vector3d &point) const {
	// Compute the barycentric coordinates of the point in the triangle
	float area = 1.0 / 2.0
			* (-_vertices[1].y() * _vertices[2].x()
					+ _vertices[0].y() * (-_vertices[1].x() + _vertices[2].x())
					+ _vertices[0].x() * (_vertices[1].y() - _vertices[2].y())
					+ _vertices[1].x() * _vertices[2].y());

	int32 sign = area < 0 ? -1 : 1;

	float s = (_vertices[0].y() * _vertices[2].x() - _vertices[0].x() * _vertices[2].y()
			+ (_vertices[2].y() - _vertices[0].y()) * point.x()
			+ (_vertices[0].x() - _vertices[2].x()) * point.y())
					* sign / (2.0 * area);

	float t = (_vertices[0].x() * _vertices[1].y() - _vertices[0].y() * _vertices[1].x()
			+ (_vertices[0].y() - _vertices[1].y()) * point.x()
			+ (_vertices[1].x() - _vertices[0].x()) * point.y())
					* sign / (2.0 * area);

	// Compute the Z coordinate of the point
	float pointZ = (1.0 - s - t) * _vertices[0].z() + s * _vertices[1].z() + t * _vertices[2].z();

	point.setValue(2, pointZ);
}

float FloorFace::getDistanceFromCamera() const {
	return _distanceFromCamera;
}

void FloorFace::readData(XRCReadStream *stream) {
	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		_indices[i] = stream->readSint16LE();
	}

	_distanceFromCamera = stream->readFloat();

	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		stream->readSint16LE(); // Skipped in the original
	}

	_unk2 = stream->readFloat();
}

void FloorFace::onAllLoaded() {
	Resource::onAllLoaded();
	Floor *floor = Resource::cast<Floor>(_parent);

	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		_vertices[i] = floor->getVertex(_indices[i]);
	}
}

void FloorFace::printData() {
	debug("indices: %d %d %d, distanceFromCamera %f, unk2 %f", _indices[0], _indices[1], _indices[2], _distanceFromCamera, _unk2);
}

} // End of namespace Resources
} // End of namespace Stark
