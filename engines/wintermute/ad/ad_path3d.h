// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#ifndef WINTERMUTE_AD_PATH3D_H
#define WINTERMUTE_AD_PATH3D_H

#include "../base/base.h"
#include "../coll_templ.h"
#include "math/vector3d.h"
#include "../persistent.h"

namespace Wintermute {

class AdPath3D : public BaseClass
{
public:
	Math::Vector3d* GetCurrent();
	Math::Vector3d* GetNext();
	Math::Vector3d* GetFirst();
	bool SetReady(bool ready);
	void AddPoint(float x, float y, float z);
	void AddPoint(Math::Vector3d Point);
	void Reset();
	AdPath3D(BaseGame* inGame);
	virtual ~AdPath3D();
	bool m_Ready;
	BaseArray<Math::Vector3d*> _points;
	int m_CurrIndex;

	DECLARE_PERSISTENT(AdPath3D, BaseClass);
};

}

#endif
