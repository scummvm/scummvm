/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#if !defined(AFX_DGCORKSCREWCONSTRAINT_H__GRT672DF293FR__INCLUDED_)
#define AFX_DGCORKSCREWCONSTRAINT_H__GRT672DF293FR__INCLUDED_
#include "dgBilateralConstraint.h"

//template<class T> class dgPool;

class dgCorkscrewConstraint;


typedef dgUnsigned32 (dgApi *dgCorkscrewJointAcceleration) (const dgCorkscrewConstraint& hinge, dgJointCallBackParam* param);

class dgCorkscrewConstraint: public dgBilateralConstraint
{
	public:
	dgFloat32 GetJointAngle () const;
	dgFloat32 GetJointOmega () const;
	dgFloat32 GetJointPosit () const;
	dgFloat32 GetJointVeloc () const;

	dgVector GetJointForce () const;
	dgFloat32 CalculateStopAlpha (dgFloat32 angle, const dgJointCallBackParam* param) const;
	dgFloat32 CalculateStopAccel (dgFloat32 distance, const dgJointCallBackParam* param) const;
	void SetJointParameterCallBack (dgCorkscrewJointAcceleration callback);

	
	private:
	virtual dgUnsigned32 JacobianDerivative (dgContraintDescritor& params); 
//	virtual void Remove(dgWorld* world);

	dgCorkscrewConstraint();
	virtual ~dgCorkscrewConstraint();
//	static dgCorkscrewConstraint* Create(dgWorld* world);

	dgFloat32 m_angle;
	dgFloat32 m_posit;
	dgCorkscrewJointAcceleration m_jointAccelFnt;
//	dgUnsigned32 m_reserve[1];

//#ifdef __USE_DOUBLE_PRECISION__
//	dgUnsigned32 m_reserve[3];
//#else
//	dgUnsigned32 m_reserve[1];
//#endif

	friend class dgWorld;
//	friend class dgPool<dgCorkscrewConstraint>;
};

//class dgCorkscrewConstraintArray: public dgPoolContainer<dgCorkscrewConstraint>
//{
//};

#endif // !defined(AFX_DGCORKSCREWCONSTRAINT_H__GRT672DF293FR__INCLUDED_)

