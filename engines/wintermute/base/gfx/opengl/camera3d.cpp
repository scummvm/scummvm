// This file is part of Wintermute Engine
// For conditions of distribution and use, see copyright notice in license.txt
// http://dead-code.org/redir.php?target=wme


#include "camera3d.h"
#include "math/angle.h"
#include "math/quat.h"
#include "../../../math/math_util.h"
#include "math/glmath.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
Camera3D::Camera3D(BaseGame* inGame): BaseNamedObject(inGame)
{
	m_Position = Math::Vector3d(0,0,0);
	m_Target = Math::Vector3d(0,0,0);
	m_Bank = 0.0f;
	m_FOV = m_OrigFOV = Math::Angle(45.0f).getRadians();
	m_NearClipPlane = m_FarClipPlane = -1.0f;
}


//////////////////////////////////////////////////////////////////////////
Camera3D::~Camera3D()
{

}


//////////////////////////////////////////////////////////////////////////
bool Camera3D::GetViewMatrix(Math::Matrix4 *ViewMatrix)
{
	Math::Vector3d up = Math::Vector3d(0.0f, 1.0f, 0.0f);

	if(m_Bank!=0)
	{
		Math::Matrix4 rotZ;
		rotZ.buildAroundZ(Math::Angle(m_Bank).getRadians());
		rotZ.transform(&up, false);
	}

	*ViewMatrix = Math::makeLookAtMatrix(m_Position, m_Target, up);
	return true;
}


//////////////////////////////////////////////////////////////////////////
void Camera3D::SetupPos(Math::Vector3d Pos, Math::Vector3d Target, float Bank)
{
	m_Position = Pos;
	m_Target = Target;
	m_Bank = Bank;
}


//////////////////////////////////////////////////////////////////////////
void Camera3D::RotateView(float X, float Y, float Z)
{
	Math::Vector3d vVector;						// Vector for the position/view.

	// Get our view vector (The direciton we are facing)
	vVector = m_Target - m_Position;		// This gets the direction of the view
	
	// Rotate the view along the desired axis
	if(X) {
		// Rotate the view vector up or down, then add it to our position
		m_Target.z() = (float)(m_Position.z() + sin(X)*vVector.y() + cos(X)*vVector.z());
		m_Target.y() = (float)(m_Position.y() + cos(X)*vVector.y() - sin(X)*vVector.z());
	}
	if(Y) {
		// Rotate the view vector right or left, then add it to our position
		m_Target.z() = (float)(m_Position.z() + sin(Y)*vVector.x() + cos(Y)*vVector.z());
		m_Target.x() = (float)(m_Position.x() + cos(Y)*vVector.x() - sin(Y)*vVector.z());
	}
	if(Z) {
		// Rotate the view vector diagnally right or diagnally down, then add it to our position
		m_Target.x() = (float)(m_Position.x() + sin(Z)*vVector.y() + cos(Z)*vVector.x());
		m_Target.y() = (float)(m_Position.y() + cos(Z)*vVector.y() - sin(Z)*vVector.x());
	}

}


//////////////////////////////////////////////////////////////////////////
void Camera3D::Move(float Speed)
{
	Math::Vector3d vVector;						// Init a vector for our view

	// Get our view vector (The direciton we are facing)
	vVector = m_Target - m_Position;		// This gets the direction of the view
	
	m_Position.x() += vVector.x() * Speed;		// Add our acceleration to our position's X
	m_Position.z() += vVector.z() * Speed;		// Add our acceleration to our position's Z
	m_Target.x() += vVector.x() * Speed;			// Add our acceleration to our view's X
	m_Target.z() += vVector.z() * Speed;			// Add our acceleration to our view's Z
}

}
