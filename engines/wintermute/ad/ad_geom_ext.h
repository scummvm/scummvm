// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#ifndef WINTERMUTE_AD_GEOM_EXT_H
#define WINTERMUTE_AD_GEOM_EXT_H

#include "../base/base.h"
#include "../coll_templ.h"

namespace Wintermute {

class AdGeomExtNode;

class AdGeomExt : public BaseClass
{
public:
	AdGeomExt(BaseGame* inGame);
	virtual ~AdGeomExt(void);
	bool LoadFile(char* Filename);
	bool LoadBuffer(byte* Buffer);
	bool AddStandardNodes();
	AdGeomExtNode* MatchName(char* Name);

private:
	BaseArray<AdGeomExtNode*> m_Nodes;
};

}

#endif
