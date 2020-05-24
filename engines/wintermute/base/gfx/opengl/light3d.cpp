// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#include "light3d.h"
#include "../../../math/math_util.h"
#include "../../../wintypes.h"
#include "math/glmath.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
Light3D::Light3D(BaseGame* inGame): BaseScriptable(inGame, false, false)
{
	m_DiffuseColor = BYTETORGBA(255, 255, 255, 255);
	m_Pos = Math::Vector3d(0, 0, 0);
	m_Target = Math::Vector3d(0, 0, 0);
	m_IsSpotlight = false;
	m_Falloff = 0;
	m_Active = true;

	m_Distance = 0.0f;
	m_IsAvailable = false;
}


//////////////////////////////////////////////////////////////////////////
Light3D::~Light3D()
{

}


//////////////////////////////////////////////////////////////////////////
bool Light3D::SetLight(int Index)
{
	//Implement this later

//	LPDIRECT3DDEVICE Device = ((CBRenderD3D*)Game->m_Renderer)->m_Device;

//	D3DLIGHT d3dLight;
//	ZeroMemory(&d3dLight, sizeof(D3DLIGHT));

//	d3dLight.Type = m_IsSpotlight ? D3DLIGHT_SPOT : D3DLIGHT_POINT;
//	d3dLight.Diffuse.r = (float)D3DCOLGetR(m_DiffuseColor) / 256.0f;
//	d3dLight.Diffuse.g = (float)D3DCOLGetG(m_DiffuseColor) / 256.0f;
//	d3dLight.Diffuse.b = (float)D3DCOLGetB(m_DiffuseColor) / 256.0f;

//	d3dLight.Position.x = m_Pos.x;
//	d3dLight.Position.y = m_Pos.y;
//	d3dLight.Position.z = m_Pos.z;
	
//	if(m_IsSpotlight)
//	{
//		Math::Vector3d Dir = m_Target - m_Pos;
//		d3dLight.Direction = Dir;
//		d3dLight.Range = D3DXVec3Length(&Dir);

//		d3dLight.Theta        = 0.5f;
//		d3dLight.Phi          = 1.0f;
//		d3dLight.Falloff      = 1.0f;
//		d3dLight.Attenuation0 = 1.0f;
//	}
//	else
//	{
//		d3dLight.Range = 100000.0f;
//		d3dLight.Attenuation0 = 1.0f;
//	}
	
//	bool ret = Device->SetLight(Index, &d3dLight);

//	if(m_Active)
//		Device->LightEnable(Index, TRUE);

//	return ret;
	return true;
}


//////////////////////////////////////////////////////////////////////////
bool Light3D::GetViewMatrix(Math::Matrix4 *ViewMatrix)
{
	Math::Vector3d up = Math::Vector3d(0.0f, 1.0f, 0.0f);
	*ViewMatrix = Math::makeLookAtMatrix(m_Pos, m_Target, up);
	return true;
}


//////////////////////////////////////////////////////////////////////////
bool Light3D::Persist(BasePersistenceManager* PersistMgr)
{
	PersistMgr->transferBool("m_Active", &m_Active);
	PersistMgr->transferUint32("m_DiffuseColor", &m_DiffuseColor);
	return true;
}

}
