// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#ifndef WINTERMUTE_LIGHT3D_H
#define WINTERMUTE_LIGHT3D_H

#include "../../base_persistence_manager.h"
#include "../../base_scriptable.h"
#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Wintermute {

class Light3D : public BaseScriptable
{
public:
	bool Persist(BasePersistenceManager* PersistMgr);
	bool GetViewMatrix(Math::Matrix4 *ViewMatrix);
	Light3D(BaseGame* inGame);
	virtual ~Light3D();
	uint32 m_DiffuseColor;
	Math::Vector3d m_Pos;
	Math::Vector3d m_Target;
	bool m_IsSpotlight;
	bool m_Active;
	float m_Falloff;
	
	float m_Distance;
	bool m_IsAvailable;

	bool SetLight(int Index=0);
};

}

#endif
