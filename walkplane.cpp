// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "walkplane.h"
#include "textsplit.h"

void Sector::load(TextSplitter &ts) {
	char buf[256];
	int id = 0;

	// Sector NAMES can be null, but ts isn't flexible enough
	if (strlen(ts.currentLine()) > strlen(" sector"))
		ts.scanString(" sector %256s", 1, buf);
	else {
		ts.nextLine();
		strcpy(buf, "");
	}

	ts.scanString(" id %d", 1, &id);
	load0(ts, buf, id);
}

void Sector::load0(TextSplitter &ts, char *name, int id) {
	char buf[256];
	int i = 0;
	float height = 12345.f; // Yaz: this is in the original code...
	Vector3d tempVert;

	name_ = name;
	id_ = id;
	ts.scanString(" type %256s", 1, buf);

	// Flags used in function at 4A66C0 (buildWalkPlane)

	if (strstr(buf, "walk"))
		type_ = 0x1000;

	else if (strstr(buf, "funnel"))
		type_ = 0x1100;
	else if (strstr(buf, "camera"))
		type_ = 0x2000;
	else if (strstr(buf, "special"))
		type_ = 0x4000;
	else if (strstr(buf, "chernobyl"))
		type_ = 0x8000;
	else
		error("Unknown sector type '%s' in room setup", buf);

	ts.scanString(" default visibility %256s", 1, buf);
	if (strcmp(buf, "visible") == 0)
		visible_ = true;
	else if (strcmp(buf, "invisible") == 0)
		visible_ = false;
	else
		error("Invalid visibility spec: %s\n", buf);
	ts.scanString(" height %f", 1, &height_);
	ts.scanString(" numvertices %d", 1, &numVertices_);
	vertices_ = new Vector3d[numVertices_ + 1];

	ts.scanString(" vertices: %f %f %f", 3, &vertices_[0].x(), &vertices_[0].y(),
		&vertices_[0].z());
	for (i=1;i<numVertices_;i++)
		ts.scanString(" %f %f %f", 3, &vertices_[i].x(), &vertices_[i].y(), &vertices_[i].z());

	// Repeat the last vertex for convenience
	vertices_[numVertices_] = vertices_[0];
}

void Sector::setVisible(bool visible) {
	visible_ = visible;
}

bool Sector::isPointInSector(Vector3d point) const {
	// The algorithm: for each edge A->B, check whether the z-component
	// of (B-A) x (P-A) is >= 0.  Then the point is at least in the
	// cylinder above&below the polygon.  (This works because the polygons'
	// vertices are always given in counterclockwise order, and the
	// polygons are always convex.)
	//
	// (I don't know whether the box height actually has to be considered;
	// if not then this will be fine as is.)

	for (int i = 0; i < numVertices_; i++) {
		Vector3d edge = vertices_[i+1] - vertices_[i];
		Vector3d delta = point - vertices_[i];
		if (edge.x() * delta.y() < edge.y() * delta.x())
			return false;
	}
	return true;
}
