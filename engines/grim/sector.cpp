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

#include "common/util.h"

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/sector.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/savegame.h"
#include "engines/grim/set.h"

namespace Grim {

Sector::Sector() :
		_vertices(nullptr), _origVertices(nullptr), _sortplanes(nullptr),_invalid(false),
		_shrinkRadius(0.f), _numVertices(0), _id(0), _numSortplanes(0),
		_type(NoneType), _visible(false), _height(0.f) {
 }

Sector::Sector(const Sector &other) :
		_vertices(nullptr), _origVertices(nullptr), _sortplanes(nullptr),
		_numSortplanes(0) {
	*this = other;
}

Sector::~Sector() {
	delete[] _vertices;
	delete[] _origVertices;
	delete[] _sortplanes;
}

void Sector::saveState(SaveGame *savedState) const {
	savedState->writeLESint32(_numVertices);
	savedState->writeLESint32(_id);
	savedState->writeLESint32(_type);
	savedState->writeBool(_visible);
	savedState->writeFloat(_height);

	savedState->writeString(_name);

	for (int i = 0; i < _numVertices + 1; ++i) {
		savedState->writeVector3d(_vertices[i]);
	}

	savedState->writeVector3d(_normal);

	savedState->writeFloat(_shrinkRadius);
	savedState->writeBool(_invalid);
	if (_shrinkRadius != 0.f && !_invalid) {
		for (int i = 0; i < _numVertices + 1; ++i) {
			savedState->writeVector3d(_origVertices[i]);
		}
	}

	if (savedState->saveMinorVersion() > 8 && g_grim->getGameType() == GType_MONKEY4) {
		savedState->writeLEUint32(_numSortplanes);
		for (int i = 0; i < _numSortplanes; ++i) {
			savedState->writeLEUint32(_sortplanes[i]);
		}
	}
}

bool Sector::restoreState(SaveGame *savedState) {
	_numVertices = savedState->readLESint32();
	_id          = savedState->readLESint32();
	_type        = (SectorType)savedState->readLESint32();
	_visible     = savedState->readBool();
	_height      = savedState->readFloat();

	_name        = savedState->readString();

	_vertices = new Math::Vector3d[_numVertices + 1];
	for (int i = 0; i < _numVertices + 1; ++i) {
		_vertices[i] = savedState->readVector3d();
	}

	_normal = savedState->readVector3d();

	_shrinkRadius = savedState->readFloat();
	_invalid = savedState->readBool();
	if (_shrinkRadius != 0.f && !_invalid) {
		_origVertices = new Math::Vector3d[_numVertices + 1];
		for (int i = 0; i < _numVertices + 1; ++i) {
			_origVertices[i] = savedState->readVector3d();
		}
	} else {
		_origVertices = nullptr;
	}
	if (savedState->saveMinorVersion() > 8 && g_grim->getGameType() == GType_MONKEY4) {
		_numSortplanes = savedState->readLEUint32();
		_sortplanes = new int[_numSortplanes];
		for (int i = 0; i < _numSortplanes; ++i) {
			_sortplanes[i] = savedState->readLEUint32();
		}
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
	else
		Debug::error(Debug::Sets, "Unknown sector type '%s' in room setup", buf);

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

	_normal = Math::Vector3d::crossProduct(_vertices[1] - _vertices[0],
										   _vertices[_numVertices - 1] - _vertices[0]);
	float length = _normal.getMagnitude();
	if (length > 0)
		_normal /= length;

	// Remastered
	if (!ts.checkString("numtris")) {
		return;
	}
	int _numTris;
	ts.scanString(" numtris %d", 1, &_numTris);
	//_vertices = new Math::Vector3d[_numVertices + 1];
	int a,b,c;
	if (_numTris > 0) {
		ts.scanString(" triangles: %f %f %f", 3, &a, &b, &c);
		for (i = 1; i < _numTris; i++)
			ts.scanString(" %f %f %f", 3, &a, &b, &c);
	}
}

void Sector::loadBinary(Common::SeekableReadStream *data) {
	_numVertices = data->readUint32LE();
	_vertices = new Math::Vector3d[_numVertices + 1];
	for (int i = 0; i < _numVertices; i++) {
		_vertices[i].readFromStream(data);
	}

	// Repeat the last vertex for convenience
	_vertices[_numVertices] = _vertices[0];

	_normal = Math::Vector3d::crossProduct(_vertices[1] - _vertices[0],
										   _vertices[_numVertices - 1] - _vertices[0]);
	float length = _normal.getMagnitude();
	if (length > 0)
		_normal /= length;

	char name[128];
	int nameLength = data->readUint32LE();

	data->read(name, nameLength);
	_name = name;

	_id = data->readUint32LE();

	_visible = data->readByte();

	_type = (SectorType)data->readUint32LE();

	_numSortplanes = data->readUint32LE();
	_sortplanes = new int[_numSortplanes];
	for (int i = 0; i < _numSortplanes; ++i) {
		_sortplanes[i] = data->readUint32LE();
	}

	_height = data->readFloatLE();
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

		for (int k = 0; k < g_grim->getCurrSet()->getSectorCount(); k++) {
			Sector *other = g_grim->getCurrSet()->getSectorBase(k);
			if ((other->getType() & WalkType) == 0)
				continue;

			for (int l = 0; l < other->_numVertices; l++) {
				Math::Vector3d *otherVerts = other->_vertices;
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
			_origVertices = nullptr;
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
			_origVertices = nullptr;
		}
	}
}

float Sector::distanceToPoint(const Math::Vector3d &point) const {
	// The plane has equation ax + by + cz + d = 0
	float a = _normal.x();
	float b = _normal.y();
	float c = _normal.z();
	float d = -_vertices[0].x() * a - _vertices[0].y() * b - _vertices[0].z() * c;

	// dist is positive if it is above the plain, negative if it is
	// below and 0 if it is on the plane.
	float dist = (a * point.x() + b * point.y() + c * point.z() + d);
	dist /= sqrt(a * a + b * b + c * c);
	return dist;
}

bool Sector::isPointInSector(const Math::Vector3d &point) const {
	// Calculate the distance of the point from the plane of the sector.
	// Return false if it isn't within a margin.
	if (_height < 9000.f) { // No need to check when height is 9999.

		float dist = distanceToPoint(point);

		if (fabsf(dist) > _height + 0.01) // Add an error margin
			return false;
	}

	// On the plane, so check if it is inside the polygon.
	for (int i = 0; i < _numVertices; i++) {
		Math::Vector3d edge = _vertices[i + 1] - _vertices[i];
		Math::Vector3d delta = point - _vertices[i];
		Math::Vector3d cross = Math::Vector3d::crossProduct(edge, delta);
		if (cross.dotProduct(_normal) < -0.000001f) // not "< 0.f" here, since the value could be something like -7.45058e-09 and it
			return false;                        // shuoldn't return. that was causing issue #610 (infinite loop in de.forklift_actor.dismount)
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

	for (int i = 0; i < _numVertices; i++) {
		bridges.push_back(Math::Line3d(_vertices[i], _vertices[i + 1]));
	}

	Math::Vector3d *sectorVertices = sector->getVertices();
	for (int i = 0; i < sector->getNumVertices(); i++) {
		Math::Vector3d pos, edge, delta_b1, delta_b2;
		Math::Line3d line(sectorVertices[i], sectorVertices[i + 1]);
		it = bridges.begin();
		while (it != bridges.end()) {
			Math::Line3d &bridge = (*it);
			edge = line.end() - line.begin();
			delta_b1 = bridge.begin() - line.begin();
			delta_b2 = bridge.end() - line.begin();
			Math::Vector3d cross_b1 = Math::Vector3d::crossProduct(edge, delta_b1);
			Math::Vector3d cross_b2 = Math::Vector3d::crossProduct(edge, delta_b2);
			bool b1_out = cross_b1.dotProduct(_normal) < 0;
			bool b2_out = cross_b2.dotProduct(_normal) < 0;

			bool useXZ = (g_grim->getGameType() == GType_MONKEY4);

			if (b1_out && b2_out) {
				// Both points are outside.
				it = bridges.erase(it);
				continue;
			} else if (b1_out) {
				if (bridge.intersectLine2d(line, &pos, useXZ)) {
					bridge = Math::Line3d(pos, bridge.end());
				}
			} else if (b2_out) {
				if (bridge.intersectLine2d(line, &pos, useXZ)) {
					bridge = Math::Line3d(bridge.begin(), pos);
				}
			}

			++it;
		}
	}

	// All the bridges should be at the same height on both sectors.
	it = bridges.begin();
	while (it != bridges.end()) {
		if (g_grim->getGameType() == GType_MONKEY4) {
			// Set pac contains sectors which are not parallel to any
			// other sector or share any edge. Since one sector isn't
			// a plane, finding the intersections in 3D would be complicated.
			//
			// Checking for bridges using a projection in 2D and having a height
			// threshold to avoid that characters jump from lower to higher floors
			// seems to be a good compromise.
			//
			// The value of at least 0.1 was chosen to fix a path finding issue
			// in set pac when guybrush tried to reach the pile of rocks.
			if (fabs(getProjectionToPlane((*it).begin()).y() - sector->getProjectionToPlane((*it).begin()).y()) > 0.1f ||
					fabs(getProjectionToPlane((*it).end()).y() - sector->getProjectionToPlane((*it).end()).y()) > 0.1f) {
				it = bridges.erase(it);
				continue;
			}
		} else {
			if (fabs(getProjectionToPlane((*it).begin()).z() - sector->getProjectionToPlane((*it).begin()).z()) > 0.01f ||
					fabs(getProjectionToPlane((*it).end()).z() - sector->getProjectionToPlane((*it).end()).z()) > 0.01f) {
				it = bridges.erase(it);
				continue;
			}
		}
		++it;
	}
	return bridges;
}

Math::Vector3d Sector::getProjectionToPlane(const Math::Vector3d &point) const {
	if (_normal.getMagnitude() == 0)
		error("Sector normal is (0,0,0)");

	// Formula: return p - n * (n . (p - v_0))
	Math::Vector3d result = point;
	result -= _normal * _normal.dotProduct(point - _vertices[0]);
	return result;
}

Math::Vector3d Sector::getProjectionToPuckVector(const Math::Vector3d &v) const {
	if (_normal.getMagnitude() == 0)
		error("Sector normal is (0,0,0)");

	Math::Vector3d result = v;
	result -= _normal * _normal.dotProduct(v);
	return result;
}

// Find the closest point on the walkplane to the given point
Math::Vector3d Sector::getClosestPoint(const Math::Vector3d &point) const {
	// First try to project to the plane
	Math::Vector3d p2 = getProjectionToPlane(point);
	if (isPointInSector(p2))
		return p2;

	// Now try to project to some edge
	for (int i = 0; i < _numVertices; i++) {
		Math::Vector3d edge = _vertices[i + 1] - _vertices[i];
		Math::Vector3d delta = point - _vertices[i];
		float scalar = Math::Vector3d::dotProduct(delta, edge) / Math::Vector3d::dotProduct(edge, edge);
		Math::Vector3d cross = Math::Vector3d::crossProduct(delta, edge);
		if (scalar >= 0 && scalar <= 1 && cross.dotProduct(_normal) > 0)
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
		Math::Vector3d cross = Math::Vector3d::crossProduct(delta, dir);
		if (cross.dotProduct(_normal) > 0)
			break;
	}

	// Now continue until the cross product has negative
	// z-component.
	while (i < _numVertices) {
		i++;
		Math::Vector3d delta = _vertices[i] - start;
		Math::Vector3d cross = Math::Vector3d::crossProduct(delta, dir);
		if (cross.dotProduct(_normal) <= 0)
			break;
	}

	result->edgeDir = _vertices[i] - _vertices[i - 1];
	result->angleWithEdge = Math::Vector3d::angle(dir, result->edgeDir);
	result->edgeVertex = i - 1;

	Math::Vector3d edgeNormal = Math::Vector3d::crossProduct(result->edgeDir, _normal);
	float d = Math::Vector3d::dotProduct(dir, edgeNormal);
	// This is 0 for the albinizod monster in the at set
	if (!d)
		d = 1.f;
	result->exitPoint = start + (Math::Vector3d::dotProduct(_vertices[i] - start, edgeNormal) / d) * dir;
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
		_origVertices = nullptr;
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
