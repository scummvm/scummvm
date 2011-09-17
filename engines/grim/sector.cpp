/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/util.h"
#include "common/memstream.h"

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/sector.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/savegame.h"
#include "engines/grim/colormap.h"
#include "engines/grim/scene.h"

namespace Grim {

Sector::Sector(const Sector &other) {
	*this = other;
}

Sector::~Sector() {
	if (_vertices)
		delete[] _vertices;
	if (_origVertices)
		delete[] _origVertices;
}

void Sector::saveState(SaveGame *savedState) const {
	savedState->writeLESint32(_numVertices);
	savedState->writeLESint32(_id);
	savedState->writeLESint32(_type);
	savedState->writeLESint32(_visible);
	savedState->writeFloat(_height);

	savedState->writeString(_name);

	for (int i = 0; i < _numVertices + 1; ++i) {
		savedState->writeVector3d(_vertices[i]);
	}

	savedState->writeVector3d(_normal);

	savedState->writeFloat(_shrinkRadius);
	savedState->writeLESint32(_invalid);
	if (_shrinkRadius != 0.f && !_invalid) {
		for (int i = 0; i < _numVertices + 1; ++i) {
			savedState->writeVector3d(_origVertices[i]);
		}
	}
}

bool Sector::restoreState(SaveGame *savedState) {
	_numVertices = savedState->readLESint32();
	_id          = savedState->readLESint32();
	_type        = (SectorType)savedState->readLESint32();
	_visible     = savedState->readLESint32();
	_height      = savedState->readFloat();

	_name 		 = savedState->readString();

	_vertices = new Math::Vector3d[_numVertices + 1];
	for (int i = 0; i < _numVertices + 1; ++i) {
		_vertices[i] = savedState->readVector3d();
	}

	_normal = savedState->readVector3d();

	_shrinkRadius = savedState->readFloat();
	_invalid = savedState->readLESint32();
	if (_shrinkRadius != 0.f && !_invalid) {
		_origVertices = new Math::Vector3d[_numVertices + 1];
		for (int i = 0; i < _numVertices + 1; ++i) {
			_origVertices[i] = savedState->readVector3d();
		}
	} else {
		_origVertices = NULL;
	}

	return true;
}

void Sector::load(TextSplitter &ts) {
	char buf[256];
	int ident = 0, i = 0;
	Math::Vector3d tempVert;

	// Sector NAMES can be null, but ts isn't flexible enough
	if (strlen(ts.getCurrentLine()) > strlen(" sector"))
		ts.scanString(" sector %256s", 1, buf);
	else {
		ts.nextLine();
		strcpy(buf, "");
	}

	ts.scanString(" id %d", 1, &ident);

	_name = buf;
	_id = ident;
	ts.scanString(" type %256s", 1, buf);

	if (strstr(buf, "walk"))
		_type = WalkType;

	else if (strstr(buf, "funnel"))
		_type = FunnelType;
	else if (strstr(buf, "camera"))
		_type = CameraType;
	else if (strstr(buf, "special"))
		_type = SpecialType;
	else if (strstr(buf, "chernobyl"))
		_type = HotType;
	else if (gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
		error("Unknown sector type '%s' in room setup", buf);

	ts.scanString(" default visibility %256s", 1, buf);
	if (strcmp(buf, "visible") == 0)
		_visible = true;
	else if (strcmp(buf, "invisible") == 0)
		_visible = false;
	else
		error("Invalid visibility spec: %s", buf);
	ts.scanString(" height %f", 1, &_height);
	ts.scanString(" numvertices %d", 1, &_numVertices);
	_vertices = new Math::Vector3d[_numVertices + 1];

	ts.scanString(" vertices: %f %f %f", 3, &_vertices[0].x(), &_vertices[0].y(), &_vertices[0].z());
	for (i = 1; i < _numVertices; i++)
		ts.scanString(" %f %f %f", 3, &_vertices[i].x(), &_vertices[i].y(), &_vertices[i].z());

	// Repeat the last vertex for convenience
	_vertices[_numVertices] = _vertices[0];

	_normal = cross(_vertices[1] - _vertices[0], _vertices[_numVertices - 1] - _vertices[0]);
	float length = _normal.getMagnitude();
	if (length > 0)
		_normal /= length;
}

void Sector::loadBinary(Common::MemoryReadStream *ms) {
	_numVertices = ms->readUint32LE();
	_vertices = new Math::Vector3d[_numVertices];
	for(int i = 0; i < _numVertices; i++) {
		ms->read(_vertices[i].getData(), 12);
	}

	char name[128];
	int nameLength = ms->readUint32LE();

	_name = ms->read(name, nameLength);

	_id = ms->readUint32LE();

	_visible = ms->readByte();

	_type = (SectorType)ms->readUint32LE();

	// this probably does something more than skip bytes, but ATM I don't know what
	int skip = ms->readUint32LE();
	ms->seek(skip * 4, SEEK_CUR);

	ms->read(&_height, 4);
}

void Sector::setVisible(bool vis) {
	_visible = vis;
}

void Sector::shrink(float radius) {
	if ((getType() & WalkType) == 0 || _shrinkRadius == radius)
		return;

	_shrinkRadius = radius;
	if (!_origVertices) {
		_origVertices = _vertices;
		_vertices = new Math::Vector3d[_numVertices + 1];
	}

	// Move each vertex inwards by the given amount.
	for (int j = 0; j < _numVertices; j++) {
		Math::Vector3d shrinkDir;

		for (int k = 0; k < g_grim->getCurrScene()->getSectorCount(); k++) {
			Sector *other = g_grim->getCurrScene()->getSectorBase(k);
			if ((other->getType() & WalkType) == 0)
				continue;

			for (int l = 0; l < other->_numVertices; l++) {
				Math::Vector3d* otherVerts = other->_vertices;
				if (other->_origVertices)
					otherVerts = other->_origVertices;
				if ((otherVerts[l] - _origVertices[j]).getMagnitude() < 0.01f) {
					Math::Vector3d e1 = otherVerts[l + 1] - otherVerts[l];
					Math::Vector3d e2;
					if (l - 1 >= 0)
						e2 = otherVerts[l] - otherVerts[l - 1];
					else
						e2 = otherVerts[l] - otherVerts[other->_numVertices - 1];
					e1.normalize();
					e2.normalize();
					Math::Vector3d bisector = (e1 - e2);
					bisector.normalize();
					shrinkDir += bisector;
				}
			}
		}

		if (shrinkDir.getMagnitude() > 0.1f) {
			shrinkDir.normalize();
			_vertices[j] = _origVertices[j] + shrinkDir * radius;
		} else {
			_vertices[j] = _origVertices[j];
		}
	}

	_vertices[_numVertices] = _vertices[0];

	// Make sure the sector is still convex.
	for (int j = 0; j < _numVertices; j++) {
		Math::Vector3d e1 = _vertices[j + 1] - _vertices[j];
		Math::Vector3d e2;
		if (j - 1 >= 0)
			e2 = _vertices[j] - _vertices[j - 1];
		else
			e2 = _vertices[j] - _vertices[_numVertices - 1];

		if (e1.x() * e2.y() > e1.y() * e2.x()) {
			// Not convex, so mark the sector invalid.
			_invalid = true;
			delete[] _vertices;
			_vertices = _origVertices;
			_origVertices = NULL;
			break;
		}
	}
}

void Sector::unshrink() {
	if (_shrinkRadius != 0.f) {
		_shrinkRadius = 0.f;
		_invalid = false;
		if (_origVertices) {
			delete[] _vertices;
			_vertices = _origVertices;
			_origVertices = NULL;
		}
	}
}

bool Sector::isPointInSector(const Math::Vector3d &point) const {
	// The algorithm: for each edge A->B, check whether the z-component
	// of (B-A) x (P-A) is >= 0.  Then the point is at least in the
	// cylinder above&below the polygon.  (This works because the polygons'
	// vertices are always given in counterclockwise order, and the
	// polygons are always convex.)
	//
	// Checking the box height on the first point fixes problems with Manny
	// changing sectors outside Velasco's storeroom.  We make an exceptions
	// for heights of 0 and 9999 since these appear to have special meaning.
	// In order to have the entrance to the Blue Casket work we need to
	// handle the vertices having different z-coordinates.
	// TODO: Improve height checking for when vertices have different
	// z-coordinates so the railing in Cafe Calavera works properly.
	if (_height != 0.0f && _height != 9999.0f) {
		bool heightOK = false;
		// Handle height above Z
		if ((point.z() >= _vertices[0].z()) && (point.z() <= _vertices[0].z() + _height))
			heightOK = true;
		// Handle height below Z
		if ((point.z() <= _vertices[0].z()) && (point.z() >= _vertices[0].z() - _height))
			heightOK = true;

		for (int i = 0; i < _numVertices; i++) {
			if (_vertices[i + 1].z() != _vertices[i].z())
				heightOK = true;
		}
		if (!heightOK) {
/* Use this for debugging problems at height interfaces
			if (gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL) {
				printf("Rejected trigger due to height: %s (%f)\n", _name.c_str(), _height);
				printf("Actor Z: %f\n", point.z());
				for (int i = 0; i < _numVertices; i++)
					printf("(%d) Z: %f\n", i, _vertices[i].z());
			}
*/
			return false;
		}
	}

	for (int i = 0; i < _numVertices; i++) {
		Math::Vector3d edge = _vertices[i + 1] - _vertices[i];
		Math::Vector3d delta = point - _vertices[i];
		if (edge.x() * delta.y() < edge.y() * delta.x())
			return false;
	}
	return true;
}

Common::List<Math::Line3d> Sector::getBridgesTo(Sector *sector) const {
	// This returns a list of "bridges", which are edges that can be travelled
	// through to get to another sector. 0 bridges mean the sectors aren't
	// connected.

	// The algorithm starts by considering all the edges of sector A
	// bridges. It then narrows them down by cutting the bridges against
	// sector B, so we end up with a list of lines which are at the border
	// of sector A and inside sector B.

	Common::List<Math::Line3d> bridges;
	Common::List<Math::Line3d>::iterator it;

	for (int i = 0; i < _numVertices; i++){
		bridges.push_back(Math::Line3d(_vertices[i], _vertices[i+1]));
	}

	Math::Vector3d* sectorVertices = sector->getVertices();
	for (int i = 0; i < sector->getNumVertices(); i++) {
		Math::Vector3d pos, edge, delta_b1, delta_b2;
		Math::Line3d line(sectorVertices[i], sectorVertices[i+1]);
		it = bridges.begin();
		while (it != bridges.end()) {
			Math::Line3d& bridge = (*it);
			edge = line.end() - line.begin();
			delta_b1 = bridge.begin() - line.begin();
			delta_b2 = bridge.end() - line.begin();
			bool b1_out = edge.x() * delta_b1.y() < edge.y() * delta_b1.x();
			bool b2_out = edge.x() * delta_b2.y() < edge.y() * delta_b2.x();

			if (b1_out && b2_out) {
				// Both points are outside.
				it = bridges.erase(it);
				continue;
			} else if (b1_out) {
				if (bridge.intersectLine2d(line, &pos)) {
					bridge = Math::Line3d(pos, bridge.end());
				}
			} else if (b2_out) {
				if (bridge.intersectLine2d(line, &pos)) {
					bridge = Math::Line3d(bridge.begin(), pos);
				}
			}

			if ((bridge.end() - bridge.begin()).getMagnitude() < 0.01f) {
				it = bridges.erase(it);
				continue;
			}
			++it;
		}
	}

	// All the bridges should be at the same height on both sectors.
	while (it != bridges.end()) {
		if (fabs(getProjectionToPlane((*it).begin()).z() - sector->getProjectionToPlane((*it).begin()).z()) > 0.01f ||
			fabs(getProjectionToPlane((*it).end()).z() - sector->getProjectionToPlane((*it).end()).z()) > 0.01f) {
			it = bridges.erase(it);
			continue;
		}
		++it;
	}
	return bridges;
}

Math::Vector3d Sector::getProjectionToPlane(const Math::Vector3d &point) const {
	if (_normal.z() == 0)
		error("Trying to walk along vertical plane");

	// Formula: return p - (n . (p - v_0))/(n . k) k
	Math::Vector3d result = point;
	result.z() -= dot(_normal, point - _vertices[0]) / _normal.z();
	return result;
}

Math::Vector3d Sector::getProjectionToPuckVector(const Math::Vector3d &v) const {
	if (_normal.z() == 0)
		error("Trying to walk along vertical plane");

	Math::Vector3d result = v;
	result.z() -= dot(_normal, v) / _normal.z();
	return result;
}

// Find the closest point on the walkplane to the given point
Math::Vector3d Sector::getClosestPoint(const Math::Vector3d &point) const {
	// First try to project to the plane
	Math::Vector3d p2 = point;
	p2 -= (dot(_normal, p2 - _vertices[0])) * _normal;
	if (isPointInSector(p2))
		return p2;

	// Now try to project to some edge
	for (int i = 0; i < _numVertices; i++) {
		Math::Vector3d edge = _vertices[i + 1] - _vertices[i];
		Math::Vector3d delta = point - _vertices[i];
		float scalar = dot(delta, edge) / dot(edge, edge);
		if (scalar >= 0 && scalar <= 1 && delta.x() * edge.y() > delta.y() * edge.x())
			// That last test is just whether the z-component
			// of delta cross edge is positive; we don't
			// want to return opposite edges.
			return _vertices[i] + scalar * edge;
	}

	// Otherwise, just find the closest vertex
	float minDist = (point - _vertices[0]).getMagnitude();
	int index = 0;
	for (int i = 1; i < _numVertices; i++) {
		float currDist = (point - _vertices[i]).getMagnitude();
		if (currDist < minDist) {
			minDist = currDist;
			index = i;
		}
	}
	return _vertices[index];
}

void Sector::getExitInfo(const Math::Vector3d &s, const Math::Vector3d &dirVec, struct ExitInfo *result) const {
	Math::Vector3d start = getProjectionToPlane(s);
	Math::Vector3d dir = getProjectionToPuckVector(dirVec);

	// First find the edge the ray exits through: this is where
	// the z-component of (v_i - start) x dir changes sign from
	// positive to negative.

	// First find a vertex such that the cross product has
	// positive z-component.
	int i;
	for (i = 0; i < _numVertices; i++) {
		Math::Vector3d delta = _vertices[i] - start;
		if (delta.x() * dir.y() > delta.y() * dir.x())
			break;
	}

	// Now continue until the cross product has negative
	// z-component.
	while (i < _numVertices) {
		i++;
		Math::Vector3d delta = _vertices[i] - start;
		if (delta.x() * dir.y() <= delta.y() * dir.x())
			break;
	}

	result->edgeDir = _vertices[i] - _vertices[i - 1];
	result->angleWithEdge = angle(dir, result->edgeDir);
	result->edgeVertex = i - 1;

	Math::Vector3d edgeNormal(result->edgeDir.y(), -result->edgeDir.x(), 0);
	float d = dot(dir, edgeNormal);
	// This is 0 for the albinizod monster in the at set
	if (!d)
		d = 1.f;
	result->exitPoint = start + (dot(_vertices[i] - start, edgeNormal) / d ) * dir;
}

Sector &Sector::operator=(const Sector &other) {
	_numVertices = other._numVertices;
	_id = other._id;
	_name = other._name;
	_type = other._type;
	_visible = other._visible;
	_vertices = new Math::Vector3d[_numVertices + 1];
	for (int i = 0; i < _numVertices + 1; ++i) {
		_vertices[i] = other._vertices[i];
	}
	if (other._origVertices) {
		_origVertices = new Math::Vector3d[_numVertices + 1];
		for (int i = 0; i < _numVertices + 1; ++i) {
			_origVertices[i] = other._origVertices[i];
		}
	} else {
		_origVertices = NULL;
	}
	_height = other._height;
	_normal = other._normal;
	_shrinkRadius = other._shrinkRadius;
	_invalid = other._invalid;

	return *this;
}

bool Sector::operator==(const Sector &other) const {
	bool ok = _numVertices == other._numVertices &&
	_id == other._id &&
	_name == other._name &&
	_type == other._type &&
	_visible == other._visible;
	for (int i = 0; i < _numVertices + 1; ++i) {
		ok = ok && _vertices[i] == other._vertices[i];
	}
	ok = ok && _height == other._height &&
	_normal == other._normal;

	return ok;
}

} // end of namespace Grim
