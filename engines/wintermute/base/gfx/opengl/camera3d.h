// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#ifndef WINTERMUTE_CAMERA3D_H
#define WINTERMUTE_CAMERA3D_H

#include "../../base_named_object.h"
#include "math/vector3d.h"
#include "math/matrix4.h"

namespace Wintermute {

class Camera3D : public BaseNamedObject
{
public:
	void Move(float Speed);
	void RotateView(float X, float Y, float Z);
	void SetupPos(Math::Vector3d Pos, Math::Vector3d Target, float Bank=0);
	bool GetViewMatrix(Math::Matrix4* ViewMatrix);
	Camera3D(BaseGame* inGame);
	virtual ~Camera3D();

	Math::Vector3d m_Position;
	Math::Vector3d m_Target;
	float m_Bank;
	float m_FOV;
	float m_OrigFOV;
	float m_NearClipPlane;
	float m_FarClipPlane;

	bool loadFrom3DS(byte** buffer);
};

}

#endif
