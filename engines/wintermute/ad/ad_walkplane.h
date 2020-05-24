// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#ifndef WINTERMUTE_AD_WALKPLANE_H
#define WINTERMUTE_AD_WALKPLANE_H

#include "../base/base_scriptable.h"

namespace Wintermute {

class Mesh;

class AdWalkplane : public BaseScriptable
{
public:
	bool m_ReceiveShadows;
	bool Persist(BasePersistenceManager* PersistMgr);
	bool m_Active;
	Mesh* m_Mesh;
	AdWalkplane(BaseGame* inGame);
	virtual ~AdWalkplane();
};

}

#endif
