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

#if !defined(AFX_DGPOINTTOCURVECONSTRAINT_H__B2A6E502_3E47_498C_8CCE_018BD6761412__INCLUDED_)
#define AFX_DGPOINTTOCURVECONSTRAINT_H__B2A6E502_3E47_498C_8CCE_018BD6761412__INCLUDED_


#include "dgConstraint.h"

/*
template<class T>	class dgPool;

typedef void (dgApi *OnPointToCurveCallback) (void *context, const dgVector& point, dgVector& pointInCurve, dgVector& tangentInCurve);

class dgPointToCurveConstraint: public dgConstraint
{
	public:

	private:

	virtual void SetDestructorCallback (OnConstraintDestroy destructor);
	virtual dgUnsigned32 JacobianDerivative (dgContraintDescritor& params); 
//	virtual dgUnsigned32 CalcRestrictedDOF();

	static dgPointToCurveConstraint* Create(dgWorld* world);
	virtual void Remove(dgWorld* world);

	void SetPivotPoint(const dgVector &pivot, OnPointToCurveCallback curve, void *context);

	dgPointToCurveConstraint();
	virtual ~dgPointToCurveConstraint();


	dgVector m_pivot;
	void *m_context;
	OnPointToCurveCallback m_curve;
	OnConstraintDestroy m_destructor;


	friend class dgWorld;
	friend class dgPointToCurveConstraintArray;
	friend class dgPool<dgPointToCurveConstraint>;
};

class dgPointToCurveConstraintArray: public dgPoolContainer<dgPointToCurveConstraint>
{
};

*/

#endif // !defined(AFX_DGPOINTTOCURVECONSTRAINT_H__B2A6E502_3E47_498C_8CCE_018BD6761412__INCLUDED_)

