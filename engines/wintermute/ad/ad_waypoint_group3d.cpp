// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme

#include "ad_waypoint_group3d.h"

namespace Wintermute {

//IMPLEMENT_PERSISTENT(AdWaypointGroup3D, false);

//////////////////////////////////////////////////////////////////////////
AdWaypointGroup3D::AdWaypointGroup3D(BaseGame* inGame):BaseClass(inGame)
{
	m_Active = true;
}


//////////////////////////////////////////////////////////////////////////
AdWaypointGroup3D::~AdWaypointGroup3D()
{
	for(int i=0; i<_points.size(); i++) delete _points[i];
	_points.clear();
}


//////////////////////////////////////////////////////////////////////////
bool AdWaypointGroup3D::AddFromMesh(Mesh *Mesh)
{
	Math::Vector3d Min = Math::Vector3d(0,0,0);
	Math::Vector3d Max = Math::Vector3d(0,0,0);

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

	Math::Vector3d* vect = new Math::Vector3d;
	vect->x() = Min.x() + (Max.x() - Min.x()) / 2;
	vect->y() = Min.y() + (Max.y() - Min.y()) / 2;
	vect->z() = Min.z() + (Max.z() - Min.z()) / 2;

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

}
