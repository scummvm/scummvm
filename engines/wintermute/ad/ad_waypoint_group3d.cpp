/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/ad/ad_waypoint_group3d.h"

namespace Wintermute {

//IMPLEMENT_PERSISTENT(AdWaypointGroup3D, false);

//////////////////////////////////////////////////////////////////////////
AdWaypointGroup3D::AdWaypointGroup3D(BaseGame *inGame) : BaseClass(inGame), _active(true) {
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

	// implement this later

	//	if(Mesh->m_NumVertices > 0)	Min = Max = Mesh->m_Vertices[0].m_Pos;

	//	for(int i=0; i<Mesh->m_NumVertices; i++){
	//		Min.x = min(Min.x, Mesh->m_Vertices[i].m_Pos.x);
	//		Min.y = min(Min.y, Mesh->m_Vertices[i].m_Pos.y);
	//		Min.z = min(Min.z, Mesh->m_Vertices[i].m_Pos.z);

	//		Max.x = max(Max.x, Mesh->m_Vertices[i].m_Pos.x);
	//		Max.y = max(Max.y, Mesh->m_Vertices[i].m_Pos.y);
	//		Max.z = max(Max.z, Mesh->m_Vertices[i].m_Pos.z);
	//	}

	Math::Vector3d *vect = new Math::Vector3d;
	vect->x() = min.x() + (max.x() - min.x()) / 2;
	vect->y() = min.y() + (max.y() - min.y()) / 2;
	vect->z() = min.z() + (max.z() - min.z()) / 2;

	_points.add(vect);

	return true;
}

/*
//////////////////////////////////////////////////////////////////////////
HRESULT AdWaypointGroup3D::Persist(CBPersistMgr* PersistMgr){

	PersistMgr->Transfer(TMEMBER(Game));
	PersistMgr->Transfer(TMEMBER(m_Active));
	//_points.Persist(PersistMgr);

	return S_OK;
}
*/

} // namespace Wintermute
