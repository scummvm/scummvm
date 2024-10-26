/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "common/util.h"

#include "engines/wintermute/ad/ad_waypoint_group3d.h"
#include "engines/wintermute/base/gfx/3dmesh.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
AdWaypointGroup3D::AdWaypointGroup3D(BaseGame *inGame) : BaseClass(inGame) {
	_active = true;
}

//////////////////////////////////////////////////////////////////////////
AdWaypointGroup3D::~AdWaypointGroup3D() {
	for (uint i = 0; i < _points.size(); i++) {
		delete _points[i];
	}
	_points.clear();
}

//////////////////////////////////////////////////////////////////////////
bool AdWaypointGroup3D::addFromMesh(Mesh3DS *mesh) {
	DXVector3 min = DXVector3(0, 0, 0);
	DXVector3 max = DXVector3(0, 0, 0);

	if (mesh->_numVertices > 0) {
		min = max = mesh->_vertices[0]._pos;
	}

	for (int i = 0; i < mesh->_numVertices; i++) {
		min._x = MIN(min._x, mesh->_vertices[i]._pos._x);
		min._y = MIN(min._y, mesh->_vertices[i]._pos._y);
		min._z = MIN(min._z, mesh->_vertices[i]._pos._z);

		max._x = MAX(max._x, mesh->_vertices[i]._pos._x);
		max._y = MAX(max._y, mesh->_vertices[i]._pos._y);
		max._z = MAX(max._z, mesh->_vertices[i]._pos._z);
	}

	DXVector3 *vect = new DXVector3;
	vect->_x = min._x + (max._x - min._x) / 2;
	vect->_y = min._y + (max._y - min._y) / 2;
	vect->_z = min._z + (max._z - min._z) / 2;

	_points.add(vect);

	return true;
}

} // namespace Wintermute
