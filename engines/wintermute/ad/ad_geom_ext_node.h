// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#ifndef WINTERMUTE_AD_GEOM_EXT_NODE_H
#define WINTERMUTE_AD_GEOM_EXT_NODE_H

#include "../base/base.h"
#include "ad_types.h"

namespace Wintermute {

class AdGeomExtNode : public BaseClass
{
public:
	AdGeomExtNode(BaseGame* inGame);
	virtual ~AdGeomExtNode(void);
	bool LoadBuffer(byte* Buffer, bool Complete);

	bool SetupNode(char* NamePattern, TGeomNodeType Type=GEOM_GENERIC, bool ReceiveShadows=false);
	bool MatchesName(char* Name);

	bool m_ReceiveShadows;
	TGeomNodeType m_Type;	

private:
	char* m_NamePattern;
};

}

#endif
