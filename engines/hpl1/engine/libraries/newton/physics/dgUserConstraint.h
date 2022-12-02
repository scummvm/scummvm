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

#if !defined(AFX_DGUSERCONSTRAINT_H__97A0E0E3_C511_4761_9346_BAEC6E6322BA__INCLUDED_)
#define AFX_DGUSERCONSTRAINT_H__97A0E0E3_C511_4761_9346_BAEC6E6322BA__INCLUDED_


#include "dgBilateralConstraint.h"

//class dgWorld;

class dgUserConstraint: public dgBilateralConstraint {
public:

protected:
	virtual dgUnsigned32 JacobianDerivative(dgContraintDescritor &params) = 0;
	dgUserConstraint(dgWorld *const manager, dgBody *const dyn0, dgBody *const dyn1, dgInt32 constraintID);

	virtual void GetInfo(dgConstraintInfo *const info) const;

	virtual ~dgUserConstraint();
};

#endif // !defined(AFX_DGUSERCONSTRAINT_H__97A0E0E3_C511_4761_9346_BAEC6E6322BA__INCLUDED_)

