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
	Math::Vector3d min = Math::Vector3d(0, 0, 0);
	Math::Vector3d max = Math::Vector3d(0, 0, 0);

	if (mesh->vertexCount() > 0) {
		min = max = mesh->getVertexPosition(0);
	}

	for (int i = 0; i < mesh->vertexCount(); i++) {
		min.x() = MIN(min.x(), mesh->getVertexPosition(i)[0]);
		min.y() = MIN(min.y(), mesh->getVertexPosition(i)[1]);
		min.z() = MIN(min.z(), mesh->getVertexPosition(i)[2]);

		max.x() = MAX(max.x(), mesh->getVertexPosition(i)[0]);
		max.y() = MAX(max.y(), mesh->getVertexPosition(i)[1]);
		max.z() = MAX(max.z(), mesh->getVertexPosition(i)[2]);
	}

	Math::Vector3d *vect = new Math::Vector3d;
	vect->x() = min.x() + (max.x() - min.x()) / 2;
	vect->y() = min.y() + (max.y() - min.y()) / 2;
	vect->z() = min.z() + (max.z() - min.z()) / 2;

	_points.add(vect);

	return true;
}

} // namespace Wintermute
