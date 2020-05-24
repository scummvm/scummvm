// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#ifndef WINTERMUTE_AD_BLOCK_H
#define WINTERMUTE_AD_BLOCK_H

#include "../base/base_scriptable.h"

namespace Wintermute {

class Mesh;

class AdBlock : public BaseScriptable
{
public:
	bool m_ReceiveShadows;
	bool persist(BasePersistenceManager* PersistMgr);
	bool m_Active;
	AdBlock(BaseGame* inGame);
	virtual ~AdBlock();
	Mesh* m_Mesh;
};

}

#endif
