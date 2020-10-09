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

#include "engines/stark/resources/path.h"

#include "engines/stark/formats/xrc.h"

#include "engines/stark/resources/floor.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"

namespace Stark {
namespace Resources {

Object *Path::construct(Object *parent, byte subType, uint16 index, const Common::String &name) {
	switch (subType) {
	case kPath2D:
		return new Path2D(parent, subType, index, name);
	case kPath3D:
		return new Path3D(parent, subType, index, name);
	default:
		error("Unknown path subtype %d", subType);
	}
}

Path::~Path() {
}

Path::Path(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_field_30(0) {
	_type = TYPE;
}

void Path::readData(Formats::XRCReadStream *stream) {
	_field_30 = stream->readUint32LE();
}

void Path::printData() {
	debug("field_30: %d", _field_30);
}

float Path::getEdgeLength(uint edgeIndex) const {
	Math::Vector3d edgeStart = getVertexPosition(edgeIndex);
	Math::Vector3d edgeEnd = getVertexPosition(edgeIndex + 1);

	return edgeStart.getDistanceTo(edgeEnd);
}

float Path::getWeightedEdgeLength(uint edgeIndex) const {
	float length = getEdgeLength(edgeIndex);
	float startWeight = getVertexWeight(edgeIndex);
	float endWeight = getVertexWeight(edgeIndex + 1);

	return 2000.0 * length / (startWeight + endWeight);
}

Math::Vector3d Path::getWeightedPositionInEdge(uint edgeIndex, float positionInEdge) {
	float edgeLength = getEdgeLength(edgeIndex);
	float weightedEdgeLength = getWeightedEdgeLength(edgeIndex);

	float startWeight = getVertexWeight(edgeIndex);
	float endWeight = getVertexWeight(edgeIndex + 1);

	float weightedEdgePosition = ((endWeight - startWeight) / (2 * weightedEdgeLength) * positionInEdge + startWeight) * 0.001
	      * positionInEdge / edgeLength;

	Math::Vector3d edgeStart = getVertexPosition(edgeIndex);
	Math::Vector3d edgeEnd = getVertexPosition(edgeIndex + 1);

	return edgeEnd * weightedEdgePosition + edgeStart * (1.0 - weightedEdgePosition);
}

float Path::getSortKey() const {
	return 0;
}

Math::Vector3d Path::getEdgeDirection(uint edgeIndex) const {
	return Math::Vector3d();
}


Path2D::Path2D(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Path(parent, subType, index, name) {
}

void Path2D::readData(Formats::XRCReadStream *stream) {
	Path::readData(stream);

	uint32 vertexCount = stream->readUint32LE();
	for (uint i = 0; i < vertexCount; i++) {
		Vertex vertex;
		vertex.weight = stream->readFloatLE();
		vertex.position = stream->readPoint();

		_vertices.push_back(vertex);
	}

	stream->readUint32LE(); // Unused in the original
}

void Path2D::printData() {
	Path::printData();

	for (uint i = 0; i < _vertices.size(); i++) {
		debug("vertex[%d]: (x %d, y %d), weight: %f", i,
		      _vertices[i].position.x, _vertices[i].position.y, _vertices[i].weight);
	}
}

Path2D::~Path2D() {
}

uint Path2D::getEdgeCount() const {
	return _vertices.size() - 1;
}

Math::Vector3d Path2D::getVertexPosition(uint vertexIndex) const {
	Common::Point point = _vertices[vertexIndex].position;
	return Math::Vector3d(point.x, point.y, 0.0);
}

float Path2D::getVertexWeight(uint vertexIndex) const {
	return _vertices[vertexIndex].weight;
}

Path3D::Path3D(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Path(parent, subType, index, name),
		_sortKey(0) {
}

void Path3D::readData(Formats::XRCReadStream *stream) {
	Path::readData(stream);

	uint32 vertexCount = stream->readUint32LE();
	for (uint i = 0; i < vertexCount; i++) {
		Vertex vertex;
		vertex.weight = stream->readFloatLE();
		vertex.position = stream->readVector3();

		_vertices.push_back(vertex);
	}

	_sortKey = stream->readFloatLE();
}

void Path3D::printData() {
	Path::printData();

	for (uint i = 0; i < _vertices.size(); i++) {
		debug("vertex[%d]: (x %f, y %f, z %f), weight: %f", i,
		      _vertices[i].position.x(), _vertices[i].position.y(), _vertices[i].position.z(), _vertices[i].weight);
	}

	debug("sortKey: %f", _sortKey);
}

Path3D::~Path3D() {
}

uint Path3D::getEdgeCount() const {
	return _vertices.size() - 1;
}

Math::Vector3d Path3D::getVertexPosition(uint vertexIndex) const {
	return _vertices[vertexIndex].position;
}

float Path3D::getVertexWeight(uint vertexIndex) const {
	return _vertices[vertexIndex].weight;
}

float Path3D::getSortKey() const {
	return _sortKey;
}

Math::Vector3d Path3D::getEdgeDirection(uint edgeIndex) const {
	Math::Vector3d direction = getVertexPosition(edgeIndex) - getVertexPosition(edgeIndex + 1);
	direction.normalize();
	return direction;
}

Math::Vector3d Path3D::getVertexPosition3D(uint vertexIndex, int32 *faceIndex) {
	Math::Vector3d vertex = getVertexPosition(vertexIndex);

	Floor *floor = StarkGlobal->getCurrent()->getFloor();
	if (floor) {
		int32 face = floor->findFaceContainingPoint(vertex);
		if (face >= 0) {
			floor->computePointHeightInFace(vertex, face);
		}

		if (faceIndex) {
			*faceIndex = face;
		}
	}

	return vertex;
}

} // End of namespace Resources
} // End of namespace Stark
