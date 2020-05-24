// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#include "ad_walkplane.h"
#include "../base/base_persistence_manager.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
AdWalkplane::AdWalkplane(BaseGame* inGame): BaseScriptable(inGame, false, false)
{
	m_Mesh = NULL;
	m_Active = true;
	m_ReceiveShadows = false;
}


//////////////////////////////////////////////////////////////////////////
AdWalkplane::~AdWalkplane()
{
	if (m_Mesh)
	{
		delete m_Mesh;
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdWalkplane::Persist(BasePersistenceManager* PersistMgr)
{
	PersistMgr->transferBool(TMEMBER(m_Active));
	return true;
}

}
