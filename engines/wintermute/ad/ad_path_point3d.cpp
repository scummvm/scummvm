// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#include "ad_path_point3d.h"
#include "../base/base_persistence_manager.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdPathPoint3D, false);

//////////////////////////////////////////////////////////////////////////
AdPathPoint3D::AdPathPoint3D(): BaseClass()
{
	m_Pos = Math::Vector3d(0, 0, 0);
	m_Distance = 0;

	m_Marked = false;
	m_Origin = NULL;
}


//////////////////////////////////////////////////////////////////////////
AdPathPoint3D::AdPathPoint3D(Math::Vector3d Pos, float Dist)
{
	m_Pos = Pos;
	m_Distance = Dist;

	m_Marked = false;
	m_Origin = NULL;
}


//////////////////////////////////////////////////////////////////////////
AdPathPoint3D::~AdPathPoint3D()
{
	m_Origin = NULL;
}


//////////////////////////////////////////////////////////////////////////
bool AdPathPoint3D::persist(BasePersistenceManager* PersistMgr){
	
	PersistMgr->transferFloat(TMEMBER(m_Distance));
	PersistMgr->transferBool(TMEMBER(m_Marked));
	PersistMgr->transferPtr(TMEMBER(m_Origin));

	PersistMgr->transferFloat("x", &m_Pos.x());
	PersistMgr->transferFloat("y", &m_Pos.y());
	PersistMgr->transferFloat("z", &m_Pos.z());

	return true;
}

}
