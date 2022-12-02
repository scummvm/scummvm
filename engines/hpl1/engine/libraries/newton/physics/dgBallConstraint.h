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

#ifndef AFX_DGBALLCONSTRAINT_H__7C9E1F9A_5EC6_48BE_8C9F_FB90132C6619__INCLUDED_
#define AFX_DGBALLCONSTRAINT_H__7C9E1F9A_5EC6_48BE_8C9F_FB90132C6619__INCLUDED_
#include "dgBilateralConstraint.h"

#ifdef _MSC_VER
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#endif

// template<class T>    class dgPool;

class dgBallConstraint;
typedef dgUnsigned32(dgApi *dgBallJointFriction)(const dgBallConstraint &ball, dgFloat32 timestep);

class dgBallConstraint : public dgBilateralConstraint {
public:
	dgVector GetJointAngle() const;
	dgVector GetJointOmega() const;
	dgVector GetJointForce() const;
	void SetJointParameterCallBack(dgBallJointFriction callback);

	bool GetTwistLimitState() const;
	void SetTwistLimitState(bool state);

	bool GetConeLimitState() const;
	void SetConeLimitState(bool state);

	bool GetLatealLimitState() const;
	void SetLatealLimitState(bool state);

	void SetLimits(const dgVector &coneDir, dgFloat32 minConeAngle, dgFloat32 maxConeAngle, dgFloat32 maxTwistAngle,
	               const dgVector &bilateralDir, dgFloat32 negativeBilateralConeAngle, dgFloat32 positiveBilateralConeAngle);

private:
	dgBallConstraint();
	virtual ~dgBallConstraint();
	//  virtual void Remove (dgWorld* world);

	void SetPivotPoint(const dgVector &pivot);
	virtual dgUnsigned32 JacobianDerivative(dgContraintDescritor &params);
	//  static dgBallConstraint* Create(dgWorld* world);

	dgVector m_angles;
	union limits {
		unsigned m_ballLimits;
		struct s {
			dgUnsigned32 m_coneLimit : 1;
			dgUnsigned32 m_twistLimit : 1;
			dgUnsigned32 m_lateralLimit : 1;
		} s;
	} limits;
	dgFloat32 m_coneAngle;
	dgFloat32 m_twistAngle;
	dgFloat32 m_coneAngleCos;
	dgBallJointFriction m_jointUserCallback;

	//  dgUnsigned32 m_reserve[3];

	friend class dgWorld;
	//  friend class dgPool<dgBallConstraint>;
};

// class dgBallConstraintArray: public dgPoolContainer<dgBallConstraint>
//{
// };

#endif
