// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#include "ad_block.h"
#include "../base/base_persistence_manager.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
AdBlock::AdBlock(BaseGame* inGame): BaseScriptable(inGame, false, false)
{
	m_Mesh = NULL;
	m_Active = true;
	m_ReceiveShadows = false;
}


//////////////////////////////////////////////////////////////////////////
AdBlock::~AdBlock()
{
	if (m_Mesh)
	{
		delete m_Mesh;
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdBlock::persist(BasePersistenceManager* PersistMgr)
{
	PersistMgr->transferBool(TMEMBER(m_Active));
	return true;
}

}
