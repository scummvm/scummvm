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

#if !defined(__DGUPVECTORCONSTRAINT_563GFT35684GT__INCLUDED_)
#define __DGUPVECTORCONSTRAINT_563GFT35684GT__INCLUDED_
#include "dgBilateralConstraint.h"

//template<class T> class dgPool;

class dgUpVectorConstraint;

typedef dgUnsigned32 (dgApi *dgUpVectorJointCallBack) (const dgUpVectorConstraint& upVector);

class dgUpVectorConstraint: public dgBilateralConstraint
{
	public:
	void SetJointParameterCallBack (dgUpVectorJointCallBack callback);
	void InitPinDir (const dgVector& pin);
	
	dgVector GetPinDir () const;
	void SetPinDir (const dgVector& pin);

	virtual bool IsBilateral () const;

	private:
	dgUpVectorConstraint();
	virtual ~dgUpVectorConstraint();
//	virtual void Remove(dgWorld* world);

	virtual dgUnsigned32 JacobianDerivative (dgContraintDescritor& params); 
//	static dgUpVectorConstraint* Create(dgWorld* world);

	dgUpVectorJointCallBack m_callBack;
//	dgUnsigned32 m_reserve[3];

	friend class dgWorld;
//	friend class dgPool<dgUpVectorConstraint>;
};

/*
class dgUpVectorConstraintArray: public dgPoolContainer<dgUpVectorConstraint>
{
};
*/

#endif // !defined(__DGUPVECTORCONSTRAINT_563GFT35684GT__INCLUDED_)

