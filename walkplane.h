/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
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

#ifndef WALKPLANE_H
#define WALKPLANE_H

#include "debug.h"
#include "vector3d.h"

#include <string>

class TextSplitter;

class Sector { 
public:
	Sector() : _vertices(0) {}
	~Sector() { if(_vertices) delete[] _vertices; }

	void load(TextSplitter &ts);

	void setVisible(bool visible);

	const char *name() const { return _name.c_str(); }
	const int id() const { return _id; }
	const int type() const { return _type; } // FIXME: Implement type de-masking
	bool visible() const { return _visible; }
	bool isPointInSector(Vector3d point) const;

	Vector3d projectToPlane(Vector3d point) const;
	Vector3d projectToPuckVector(Vector3d v) const;

	Vector3d closestPoint(Vector3d point) const;

	// Interface to trace a ray to its exit from the polygon
	struct ExitInfo {
		Vector3d exitPoint;
		float angleWithEdge;
		Vector3d edgeDir;
	};
	void getExitInfo(Vector3d start, Vector3d dir,
			 struct ExitInfo *result);

private:
	int _numVertices, _id;

	std::string _name;
	int _type;
	bool _visible;
	Vector3d *_vertices;
	float _height;

	Vector3d _normal;
};
#endif
