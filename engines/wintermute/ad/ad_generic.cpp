// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#include "ad_generic.h"
#include "../base/base_persistence_manager.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
AdGeneric::AdGeneric(BaseGame* inGame): BaseScriptable(inGame, false, false)
{
	m_Mesh = NULL;
	m_Active = true;
	m_ReceiveShadows = false;
}


//////////////////////////////////////////////////////////////////////////
AdGeneric::~AdGeneric()
{
	if (m_Mesh)
	{
		delete m_Mesh;
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdGeneric::persist(BasePersistenceManager* PersistMgr)
{
	PersistMgr->transferBool(TMEMBER(m_Active));
	return true;
}

}
