// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#ifndef WINTERMUTE_AD_GENERIC_H
#define WINTERMUTE_AD_GENERIC_H

#include "../base/base_scriptable.h"

namespace Wintermute {

class Mesh;

class AdGeneric : public BaseScriptable
{
public:
	bool m_ReceiveShadows;
	bool persist(BasePersistenceManager* PersistMgr);
	bool m_Active;
	AdGeneric(BaseGame* inGame);
	virtual ~AdGeneric();
	Mesh* m_Mesh;
};

}

#endif
