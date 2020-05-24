// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme

#include "ad_path3d.h"
#include "../base/base_persistence_manager.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdPath3D, false);

//////////////////////////////////////////////////////////////////////////
AdPath3D::AdPath3D(BaseGame* inGame) : BaseClass (inGame)
{
	m_CurrIndex = -1;
	m_Ready = false;
}


//////////////////////////////////////////////////////////////////////////
AdPath3D::~AdPath3D()
{
	Reset();
}


//////////////////////////////////////////////////////////////////////////
void AdPath3D::Reset()
{
	for(int i=0; i<_points.size(); i++)
		delete _points[i];

	_points.clear();
	m_CurrIndex = -1;
	m_Ready = false;
}


//////////////////////////////////////////////////////////////////////////
void AdPath3D::AddPoint(Math::Vector3d Point)
{
	_points.add(new Math::Vector3d(Point));
}


//////////////////////////////////////////////////////////////////////////
void AdPath3D::AddPoint(float x, float y, float z)
{
	_points.add(new Math::Vector3d(x, y, z));
}


//////////////////////////////////////////////////////////////////////////
bool AdPath3D::SetReady(bool ready)
{
	bool orig = m_Ready;
	m_Ready = ready;

	return orig;
}


//////////////////////////////////////////////////////////////////////////
Math::Vector3d* AdPath3D::GetFirst()
{
	if(_points.size() > 0){
		m_CurrIndex = 0;
		return _points[m_CurrIndex];
	}
	else return NULL;
}


//////////////////////////////////////////////////////////////////////////
Math::Vector3d* AdPath3D::GetNext()
{
	m_CurrIndex++;
	if(m_CurrIndex < _points.size()) return _points[m_CurrIndex];
	else return NULL;
}


//////////////////////////////////////////////////////////////////////////
Math::Vector3d* AdPath3D::GetCurrent()
{
	if(m_CurrIndex >= 0 && m_CurrIndex < _points.size()) return _points[m_CurrIndex];
	else return NULL;
}


//////////////////////////////////////////////////////////////////////////
bool AdPath3D::persist(BasePersistenceManager* PersistMgr){

	PersistMgr->transferPtr(TMEMBER(_gameRef));

	PersistMgr->transferSint32(TMEMBER(m_CurrIndex));
	PersistMgr->transferBool(TMEMBER(m_Ready));

	if(PersistMgr->getIsSaving())
	{
		int j = _points.size();
		PersistMgr->transferSint32("ArraySize", &j);
		for(int i=0; i<j; i++)
		{
			PersistMgr->transferFloat("x", &_points[i]->x());
			PersistMgr->transferFloat("y", &_points[i]->y());
			PersistMgr->transferFloat("z", &_points[i]->z());
		}
	}
	else
	{
		int j = 0;
		PersistMgr->transferSint32("ArraySize", &j);
		for(int i=0; i<j; i++)
		{
			float x, y, z;
			PersistMgr->transferFloat("x", &x);
			PersistMgr->transferFloat("y", &y);
			PersistMgr->transferFloat("z", &z);
			AddPoint(x, y, z);
		}

	}

	return true;
}

}
