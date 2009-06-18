/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#include "common/util.h"

#include "engines/grim/grim.h"
#include "engines/grim/walkplane.h"
#include "engines/grim/textsplit.h"

namespace Grim {

void Sector::load(TextSplitter &ts) {
	char buf[256];
	int id = 0, i = 0;
	Graphics::Vector3d tempVert;

	// Sector NAMES can be null, but ts isn't flexible enough
	if (strlen(ts.currentLine()) > strlen(" sector"))
		ts.scanString(" sector %256s", 1, buf);
	else {
		ts.nextLine();
		strcpy(buf, "");
	}

	ts.scanString(" id %d", 1, &id);

	_name = buf;
	_id = id;
	ts.scanString(" type %256s", 1, buf);

	if (strstr(buf, "walk"))
		_type = 0x1000;

	else if (strstr(buf, "funnel"))
		_type = 0x1100;
	else if (strstr(buf, "camera"))
		_type = 0x2000;
	else if (strstr(buf, "special"))
		_type = 0x4000;
	else if (strstr(buf, "chernobyl"))
		_type = 0x8000;
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
	_vertices = new Graphics::Vector3d[_numVertices + 1];

	ts.scanString(" vertices: %f %f %f", 3, &_vertices[0].x(), &_vertices[0].y(), &_vertices[0].z());
	for (i = 1; i < _numVertices; i++)
		ts.scanString(" %f %f %f", 3, &_vertices[i].x(), &_vertices[i].y(), &_vertices[i].z());

	// Repeat the last vertex for convenience
	_vertices[_numVertices] = _vertices[0];

	_normal = cross(_vertices[1] - _vertices[0], _vertices[_numVertices - 1] - _vertices[0]);
	float length = _normal.magnitude();
	if (length > 0)
		_normal /= length;
}

void Sector::setVisible(bool visible) {
	_visible = visible;
}

bool Sector::isPointInSector(Graphics::Vector3d point) const {
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
		Graphics::Vector3d edge = _vertices[i + 1] - _vertices[i];
		Graphics::Vector3d delta = point - _vertices[i];
		if (edge.x() * delta.y() < edge.y() * delta.x())
			return false;
	}
	return true;
}

Graphics::Vector3d Sector::projectToPlane(Graphics::Vector3d point) const {
	if (_normal.z() == 0)
		error("Trying to walk along vertical plane");

	// Formula: return p - (n . (p - v_0))/(n . k) k
	Graphics::Vector3d result = point;
	result.z() -= dot(_normal, point - _vertices[0]) / _normal.z();
	return result;
}

Graphics::Vector3d Sector::projectToPuckVector(Graphics::Vector3d v) const {
	if (_normal.z() == 0)
		error("Trying to walk along vertical plane");

	Graphics::Vector3d result = v;
	result.z() -= dot(_normal, v) / _normal.z();
	return result;
}

// Find the closest point on the walkplane to the given point
Graphics::Vector3d Sector::closestPoint(Graphics::Vector3d point) const {
	// First try to project to the plane
	Graphics::Vector3d p2 = point;
	p2 -= (dot(_normal, p2 - _vertices[0])) * _normal;
	if (isPointInSector(p2))
		return p2;

	// Now try to project to some edge
	for (int i = 0; i < _numVertices; i++) {
		Graphics::Vector3d edge = _vertices[i + 1] - _vertices[i];
		Graphics::Vector3d delta = point - _vertices[i];
		float scalar = dot(delta, edge) / dot(edge, edge);
		if (scalar >= 0 && scalar <= 1 && delta.x() * edge.y() > delta.y() * edge.x())
			// That last test is just whether the z-component
			// of delta cross edge is positive; we don't
			// want to return opposite edges.
			return _vertices[i] + scalar * edge;
	}

	// Otherwise, just find the closest vertex
	float minDist = (point - _vertices[0]).magnitude();
	int index = 0;
	for (int i = 1; i < _numVertices; i++) {
		float currDist = (point - _vertices[i]).magnitude();
		if (currDist < minDist) {
			minDist = currDist;
			index = i;
		}
	}
	return _vertices[index];
}

void Sector::getExitInfo(Graphics::Vector3d start, Graphics::Vector3d dir, struct ExitInfo *result) {
	start = projectToPlane(start);
	dir = projectToPuckVector(dir);

	// First find the edge the ray exits through: this is where
	// the z-component of (v_i - start) x dir changes sign from
	// positive to negative.

	// First find a vertex such that the cross product has
	// positive z-component.
	int i;
	for (i = 0; i < _numVertices; i++) {
		Graphics::Vector3d delta = _vertices[i] - start;
		if (delta.x() * dir.y() > delta.y() * dir.x())
			break;
	}

	// Now continue until the cross product has negative
	// z-component.
	while (i < _numVertices) {
		i++;
		Graphics::Vector3d delta = _vertices[i] - start;
		if (delta.x() * dir.y() <= delta.y() * dir.x())
			break;
	}

	result->edgeDir = _vertices[i] - _vertices[i - 1];
	result->angleWithEdge = angle(dir, result->edgeDir);

	Graphics::Vector3d edgeNormal(result->edgeDir.y(), -result->edgeDir.x(), 0);
	result->exitPoint = start + (dot(_vertices[i] - start, edgeNormal) / dot(dir, edgeNormal)) * dir;
}

} // end of namespace Grim
