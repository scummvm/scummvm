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

#include "engines/stark/formats/xrc.h"

#include "engines/stark/resources/floorface.h"
#include "engines/stark/resources/floorfield.h"

#include "engines/stark/services/stateprovider.h"

#include "common/math.h"

namespace Stark {
namespace Resources {

Floor::Floor(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_facesCount(0) {
	_type = TYPE;
}

Floor::~Floor() {
}

Math::Vector3d Floor::getVertex(uint32 index) const {
	return _vertices[index];
}

int32 Floor::findFaceContainingPoint(const Math::Vector3d &point) const {
	for (uint32 i = 0; i < _faces.size(); i++) {
		if (_faces[i]->hasVertices() && _faces[i]->isPointInside(point)) {
			return i;
		}
	}

	return -1;
}

void Floor::computePointHeightInFace(Math::Vector3d &point, uint32 faceIndex) const {
	_faces[faceIndex]->computePointHeight(point);
}

int32 Floor::findFaceHitByRay(const Math::Ray &ray, Math::Vector3d &intersection) const {
	for (uint32 i = 0; i < _faces.size(); i++) {
		// TODO: Check the ray's intersection with an AABB first if this ends up being slow
		if (_faces[i]->intersectRay(ray, intersection)) {
			if (_faces[i]->isEnabled()) {
				return i;
			} else {
				return -1; // Disabled faces block the ray
			}
		}
	}

	return -1;
}

int32 Floor::findFaceClosestToRay(const Math::Ray &ray, Math::Vector3d &center) const {
	float minDistance = FLT_MAX;
	int32 minFace = -1;

	// For some reason, face 0 is not being considered
	for (uint32 i = 1; i < _faces.size(); i++) {
		if (_faces[i]->isEnabled() && _faces[i]->hasVertices()) {
			float distance = _faces[i]->distanceToRay(ray);
			if (distance < minDistance) {
				minFace = i;
				minDistance = distance;
			}
		}
	}

	if (minFace >= 0) {
		center = _faces[minFace]->getCenter();
	}

	return minFace;
}

float Floor::getDistanceFromCamera(uint32 faceIndex) const {
	FloorFace *face = _faces[faceIndex];
	return face->getDistanceFromCamera();
}

FloorFace *Floor::getFace(uint32 index) const {
	return _faces[index];
}

bool Floor::isSegmentInside(const Math::Line3d &segment) const {
	// The segment is inside the floor if at least one of its extremities is,
	// and it does not cross any floor border / disabled floor faces

	int32 beginFace = findFaceContainingPoint(segment.begin());
	if (beginFace < 0) {
		// The segment begin point is not on the floor
		return false;
	}

	if (!_faces[beginFace]->isEnabled()) {
		// The segment begin point is not enabled
		return false;
	}

	for (uint i = 0; i < _edges.size(); i++) {
		const FloorEdge &edge = _edges[i];
		if ((edge.isFloorBorder() || !edge.isEnabled()) && edge.intersectsSegment(this, segment)) {
			return false;
		}
	}

	return true;
}

void Floor::readData(Formats::XRCReadStream *stream) {
	_facesCount = stream->readUint32LE();
	uint32 vertexCount = stream->readUint32LE();

	for (uint i = 0; i < vertexCount; i++) {
		Math::Vector3d v = stream->readVector3();
		_vertices.push_back(v);
	}
}

void Floor::onAllLoaded() {
	Object::onAllLoaded();

	_faces = listChildren<FloorFace>();

	buildEdgeList();
}

void Floor::saveLoad(ResourceSerializer *serializer) {
	for (uint i = 0; i < _edges.size(); i++) {
		_edges[i].saveLoad(serializer);
	}
}

void Floor::buildEdgeList() {
	_edges.clear();

	// Add the triangle edges from all our faces
	for (uint i = 0; i < _faces.size(); i++) {
		if (_faces[i]->hasVertices()) {
			addFaceEdgeToList(i, 2, 0);
			addFaceEdgeToList(i, 0, 1);
			addFaceEdgeToList(i, 1, 2);
		}
	}

	// Add the edges to their faces
	for (uint i = 0; i < _edges.size(); i++) {
		int32 faceIndex1 = _edges[i].getFaceIndex1();
		int32 faceIndex2 = _edges[i].getFaceIndex2();

		if (faceIndex1 >= 0) {
			_faces[faceIndex1]->addEdge(&_edges[i]);
		}

		if (faceIndex2 >= 0) {
			_faces[faceIndex2]->addEdge(&_edges[i]);
		}
	}

	// Build a list of neighbours for each edge
	for (uint i = 0; i < _edges.size(); i++) {
		_edges[i].buildNeighbours(this);
		_edges[i].computeMiddle(this);
	}
}

void Floor::addFaceEdgeToList(uint32 faceIndex, uint32 index1, uint32 index2) {
	uint32 vertexIndex1 = _faces[faceIndex]->getVertexIndex(index1);
	uint32 vertexIndex2 = _faces[faceIndex]->getVertexIndex(index2);
	uint32 startIndex = MIN(vertexIndex1, vertexIndex2);
	uint32 endIndex = MAX(vertexIndex1, vertexIndex2);

	// Check if we already have an edge with the same vertices
	for (uint i = 0; i < _edges.size(); i++) {
		if (_edges[i].hasVertices(startIndex, endIndex)) {
			_edges[i].setOtherFace(faceIndex);
			return;
		}
	}

	_edges.push_back(FloorEdge(startIndex, endIndex, faceIndex));
}

void Floor::enableFloorField(FloorField *floorfield, bool enable) {
	for (uint i = 0; i < _faces.size(); i++) {
		if (floorfield->hasFace(i)) {
			_faces[i]->enable(enable);
		}
	}
}

void Floor::printData() {
	debug("face count: %d", _facesCount);

	Common::StreamDebug debug = streamDbg();
	for (uint i = 0; i < _vertices.size(); i++) {
		debug << i << ": " << _vertices[i] << "\n";
	}
}

FloorEdge::FloorEdge(uint16 vertexIndex1, uint16 vertexIndex2, uint32 faceIndex1) :
        _vertexIndex1(vertexIndex1),
        _vertexIndex2(vertexIndex2),
        _faceIndex1(faceIndex1),
        _faceIndex2(-1),
        _enabled(true) {
}

bool FloorEdge::hasVertices(uint16 vertexIndex1, uint16 vertexIndex2) const {
	return _vertexIndex1 == vertexIndex1 && _vertexIndex2 == vertexIndex2;
}

void FloorEdge::setOtherFace(uint32 faceIndex) {
	_faceIndex2 = faceIndex;
}

Common::Array<FloorEdge *> FloorEdge::getNeighbours() const {
	return _neighbours;
}

float FloorEdge::costTo(const FloorEdge *other) const {
	return _middle.getDistanceTo(other->_middle);
}

Math::Vector3d FloorEdge::getPosition() const {
	return _middle;
}

void FloorEdge::buildNeighbours(const Floor *floor) {
	_neighbours.clear();

	if (_faceIndex1 >= 0) {
		addNeighboursFromFace(floor->getFace(_faceIndex1));
	}

	if (_faceIndex2 >= 0) {
		addNeighboursFromFace(floor->getFace(_faceIndex2));
	}
}

void FloorEdge::addNeighboursFromFace(const FloorFace *face) {
	Common::Array<FloorEdge *> faceEdges = face->getEdges();
	for (uint i = 0; i < faceEdges.size(); i++) {
		if (faceEdges[i] != this) {
			_neighbours.push_back(faceEdges[i]);
		}
	}
}

void FloorEdge::computeMiddle(const Floor *floor) {
	Math::Vector3d vertex1 = floor->getVertex(_vertexIndex1);
	Math::Vector3d vertex2 = floor->getVertex(_vertexIndex2);
	_middle = (vertex1 + vertex2) / 2.0;
}

int32 FloorEdge::getFaceIndex1() const {
	return _faceIndex1;
}

int32 FloorEdge::getFaceIndex2() const {
	return _faceIndex2;
}

bool FloorEdge::isFloorBorder() const {
	return _faceIndex2 == -1;
}

bool FloorEdge::intersectLine2d(const Math::Line3d &s1, const Math::Line3d &s2) {
	const Math::Vector3d &s1begin = s1.begin();
	const Math::Vector3d &s1end = s1.end();
	const Math::Vector3d &s2begin = s2.begin();
	const Math::Vector3d &s2end = s2.end();

	float denom = ((s2end.y() - s2begin.y()) * (s1end.x() - s1begin.x())) -
	              ((s2end.x() - s2begin.x()) * (s1end.y() - s1begin.y()));

	float nume_a = ((s2end.x() - s2begin.x()) * (s1begin.y() - s2begin.y())) -
	               ((s2end.y() - s2begin.y()) * (s1begin.x() - s2begin.x()));

	float nume_b = ((s1end.x() - s1begin.x()) * (s1begin.y() - s2begin.y())) -
	               ((s1end.y() - s1begin.y()) * (s1begin.x() - s2begin.x()));

	if (denom == 0.0f) {
		return false; // Segments are collinear
	}

	float ua = nume_a / denom;
	float ub = nume_b / denom;

	// Non inclusive bounds check, one of the vertices of one segment being inside
	// the other segment is not considered to be an intersection.
	// This is the only difference with Line3d::intersectLine2d.
	return ua > 0 && ua < 1 && ub > 0 && ub < 1;
}

bool FloorEdge::intersectsSegment(const Floor *floor, const Math::Line3d &segment) const {
	Math::Vector3d vertex1 = floor->getVertex(_vertexIndex1);
	Math::Vector3d vertex2 = floor->getVertex(_vertexIndex2);
	Math::Line3d edgeSegment = Math::Line3d(vertex1, vertex2);

	return intersectLine2d(edgeSegment, segment);
}

void FloorEdge::enable(bool e) {
	_enabled = e;
}

bool FloorEdge::isEnabled() const {
	return _enabled;
}

void FloorEdge::saveLoad(ResourceSerializer *serializer) {
	serializer->syncAsUint32LE(_enabled);
}

} // End of namespace Resources
} // End of namespace Stark
