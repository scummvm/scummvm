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

#if !defined(AFX_DGSLIDINGCONSTRAINT_H__3FF7D7AA_90CC_4BA5_B2A4_D3BA51AD2FFD__INCLUDED_)
#define AFX_DGSLIDINGCONSTRAINT_H__3FF7D7AA_90CC_4BA5_B2A4_D3BA51AD2FFD__INCLUDED_
#include "dgBilateralConstraint.h"

//template<class T> class dgPool;

class dgSlidingConstraint;
typedef dgUnsigned32(dgApi *dgSlidingJointAcceleration)(const NewtonJoint *const slider, NewtonHingeSliderUpdateDesc *const desc);

class dgSlidingConstraint: public dgBilateralConstraint {
public:
	dgFloat32 GetJointPosit() const;
	dgFloat32 GetJointVeloc() const;
	dgVector GetJointForce() const;
	dgFloat32 CalculateStopAccel(dgFloat32 distance, const dgJointCallBackParam *param) const;
	void SetJointParameterCallBack(dgSlidingJointAcceleration callback);

private:
	dgSlidingConstraint();
	virtual ~dgSlidingConstraint();
//	virtual void Remove(dgWorld* world);

	virtual dgUnsigned32 JacobianDerivative(dgContraintDescritor &params);
//	static dgSlidingConstraint* Create(dgWorld* world);


	dgFloat32 m_posit;
	dgSlidingJointAcceleration m_jointAccelFnt;

//#ifdef __USE_DOUBLE_PRECISION__
//	dgUnsigned32 m_reserve[1];
//#else
//	dgUnsigned32 m_reserve[2];
//#endif

	friend class dgWorld;
//	friend class dgPool<dgSlidingConstraint>;
};

/*
class dgSlidingConstraintArray: public dgPoolContainer<dgSlidingConstraint>
{
};
*/

#endif // !defined(AFX_DGSLIDINGCONSTRAINT_H__3FF7D7AA_90CC_4BA5_B2A4_D3BA51AD2FFD__INCLUDED_)

