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

FloorFace::FloorFace(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
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
	float area = 1.0f / 2.0f
			* (-_vertices[1].y() * _vertices[2].x()
					+ _vertices[0].y() * (-_vertices[1].x() + _vertices[2].x())
					+ _vertices[0].x() * (_vertices[1].y() - _vertices[2].y())
					+ _vertices[1].x() * _vertices[2].y());

	float s = (_vertices[0].y() * _vertices[2].x() - _vertices[0].x() * _vertices[2].y()
			+ (_vertices[2].y() - _vertices[0].y()) * point.x()
			+ (_vertices[0].x() - _vertices[2].x()) * point.y())
					/ (2.0f * area);

	float t = (_vertices[0].x() * _vertices[1].y() - _vertices[0].y() * _vertices[1].x()
			+ (_vertices[0].y() - _vertices[1].y()) * point.x()
			+ (_vertices[1].x() - _vertices[0].x()) * point.y())
					/ (2.0f * area);

	// Check the coordinates are in the triangle
	return s > 0.0f && t > 0.0f && (1.0f - s - t) > 0.0f;
}

void FloorFace::computePointHeight(Math::Vector3d &point) const {
	// Compute the barycentric coordinates of the point in the triangle
	float area = 1.0f / 2.0f
			* (-_vertices[1].y() * _vertices[2].x()
					+ _vertices[0].y() * (-_vertices[1].x() + _vertices[2].x())
					+ _vertices[0].x() * (_vertices[1].y() - _vertices[2].y())
					+ _vertices[1].x() * _vertices[2].y());

	float s = (_vertices[0].y() * _vertices[2].x() - _vertices[0].x() * _vertices[2].y()
			+ (_vertices[2].y() - _vertices[0].y()) * point.x()
			+ (_vertices[0].x() - _vertices[2].x()) * point.y())
					/ (2.0f * area);

	float t = (_vertices[0].x() * _vertices[1].y() - _vertices[0].y() * _vertices[1].x()
			+ (_vertices[0].y() - _vertices[1].y()) * point.x()
			+ (_vertices[1].x() - _vertices[0].x()) * point.y())
					/ (2.0f * area);

	// Compute the Z coordinate of the point
	float pointZ = (1.0f - s - t) * _vertices[0].z() + s * _vertices[1].z() + t * _vertices[2].z();

	point.setValue(2, pointZ);
}

bool FloorFace::intersectRay(const Math::Ray &ray, Math::Vector3d &intersection) const {
	// Compute the triangle plane normal
	Math::Vector3d n = Math::Vector3d::crossProduct(_vertices[1] - _vertices[0],  _vertices[2] - _vertices[0]);
	if (n == Math::Vector3d()) {
		return false; // We don't handle degenerate triangles
	}

	// Point on triangle plane: dot(P - _vertices[0], n) = 0
	// Point on ray: P = origin + r * direction
	// Point on both => r = - dot(n, origin - _vertices[0]) / dot(n, direction)

	float num = -Math::Vector3d::dotProduct(n, ray.getOrigin() - _vertices[0]);
	float denom = Math::Vector3d::dotProduct(n, ray.getDirection());

	if (fabs(denom) < 0.00001) {
		// The ray is parallel to the plane
		return false;
	}

	float r = num / denom;
	if (r < 0.0) {
		// The ray goes away from the triangle
		return false;
	}

	// Compute the intersection point between the triangle plane and the ray
	intersection = ray.getOrigin() + r * ray.getDirection();

	// Check the intersection point is inside the triangle
	return isPointInside(intersection);
}

float FloorFace::distanceToRay(const Math::Ray &ray) const {
	Math::Vector3d center = getCenter();
	return Math::Vector3d::crossProduct(ray.getDirection(), center - ray.getOrigin()).getMagnitude();
}

float FloorFace::getDistanceFromCamera() const {
	return _distanceFromCamera;
}

int16 FloorFace::getVertexIndex(int32 index) const {
	assert(index < 3);
	return _indices[index];
}

void FloorFace::addEdge(FloorEdge *edge) {
	_edges.push_back(edge);
}

Common::Array<FloorEdge *> FloorFace::getEdges() const {
	return _edges;
}

FloorEdge *FloorFace::findNearestEdge(const Math::Vector3d &point) const {
	float minDistance = -1;
	FloorEdge *edge = nullptr;

	for (uint i = 0; i < _edges.size(); i++) {
		if (!_edges[i]->isEnabled()) {
			continue;
		}

		float distance = (point - _edges[i]->getPosition()).getSquareMagnitude();


		if (!edge || distance < minDistance) {
			minDistance = distance;
			edge = _edges[i];
		}
	}

	return edge;
}

Math::Vector3d FloorFace::getCenter() const {
	return (_vertices[0] + _vertices[1] + _vertices[2]) / 3.0;
}

bool FloorFace::hasVertices() const {
	return _indices[0] != 0 || _indices[1] != 0 || _indices[2] != 0;
}

void FloorFace::enable(bool e) {
	for (uint i = 0; i < _edges.size(); i++) {
		_edges[i]->enable(e);
	}
}

bool FloorFace::isEnabled() const {
	for (uint i = 0; i < _edges.size(); i++) {
		if (_edges[i]->isEnabled()) {
			return true;
		}
	}

	return false;
}

void FloorFace::readData(Formats::XRCReadStream *stream) {
	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		_indices[i] = stream->readSint16LE();
	}

	_distanceFromCamera = stream->readFloatLE();

	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		stream->readSint16LE(); // Skipped in the original
	}

	_unk2 = stream->readFloatLE();
}

void FloorFace::onAllLoaded() {
	Object::onAllLoaded();
	Floor *floor = Object::cast<Floor>(_parent);

	for (uint i = 0; i < ARRAYSIZE(_indices); i++) {
		_vertices[i] = floor->getVertex(_indices[i]);
	}
}

void FloorFace::printData() {
	debug("indices: %d %d %d, distanceFromCamera %f, unk2 %f", _indices[0], _indices[1], _indices[2], _distanceFromCamera, _unk2);
}

} // End of namespace Resources
} // End of namespace Stark
