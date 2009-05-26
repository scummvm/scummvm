/* Residual - Virtual machine to run LucasArts' 3D adventure games
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

#ifndef GRIM_WALKPLANE_H
#define GRIM_WALKPLANE_H

#include "common/str.h"

#include "graphics/vector3d.h"

namespace Grim {

class TextSplitter;

class Sector {
public:
	Sector() : _vertices(0) {}
	~Sector() { if (_vertices) delete[] _vertices; }

	void load(TextSplitter &ts);

	void setVisible(bool visible);

	const char *name() const { return _name.c_str(); }
	int id() const { return _id; }
	int type() const { return _type; } // FIXME: Implement type de-masking
	bool visible() const { return _visible; }
	bool isPointInSector(Graphics::Vector3d point) const;

	Graphics::Vector3d projectToPlane(Graphics::Vector3d point) const;
	Graphics::Vector3d projectToPuckVector(Graphics::Vector3d v) const;

	Graphics::Vector3d closestPoint(Graphics::Vector3d point) const;

	// Interface to trace a ray to its exit from the polygon
	struct ExitInfo {
		Graphics::Vector3d exitPoint;
		float angleWithEdge;
		Graphics::Vector3d edgeDir;
	};
	void getExitInfo(Graphics::Vector3d start, Graphics::Vector3d dir, struct ExitInfo *result);

	int getNumVertices() { return _numVertices; }
	Graphics::Vector3d *getVertices() { return _vertices; }
	Graphics::Vector3d getNormal() { return _normal; }

private:
	int _numVertices, _id;

	Common::String _name;
	int _type;
	bool _visible;
	Graphics::Vector3d *_vertices;
	float _height;

	Graphics::Vector3d _normal;
};

} // end of namespace Grim

#endif
