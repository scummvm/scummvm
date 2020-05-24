// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#ifndef WINTERMUTE_AD_PATH_POINT3D_H
#define WINTERMUTE_AD_PATH_POINT3D_H

#include "math/vector3d.h"
#include "../persistent.h"
#include "../base/base.h"

namespace Wintermute {

class AdPathPoint3D: public BaseClass
{
public:
	DECLARE_PERSISTENT(AdPathPoint3D, BaseClass);
	AdPathPoint3D(Math::Vector3d Pos, float Dist);
	AdPathPoint3D();
	virtual ~AdPathPoint3D();
	AdPathPoint3D* m_Origin;
	bool m_Marked;
	float m_Distance;
	Math::Vector3d m_Pos;
};

}

#endif
