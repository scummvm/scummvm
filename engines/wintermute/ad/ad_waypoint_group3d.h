// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#ifndef WINTERMUTE_AD_WAYPOINT_GROUP_3D_H
#define WINTERMUTE_AD_WAYPOINT_GROUP_3D_H

#include "../base/base.h"
#include "math/vector3d.h"
#include "../coll_templ.h"

namespace Wintermute {

class CAdSceneGeometry;
class Mesh;

class AdWaypointGroup3D : public BaseClass
{
public:
	//DECLARE_PERSISTENT(AdWaypointGroup3D, BaseClass);
	bool m_Active;
	bool AddFromMesh(Mesh* Mesh);
	AdWaypointGroup3D(BaseGame* inGame);
	virtual ~AdWaypointGroup3D();
	BaseArray<Math::Vector3d*> _points;
};

}

#endif
