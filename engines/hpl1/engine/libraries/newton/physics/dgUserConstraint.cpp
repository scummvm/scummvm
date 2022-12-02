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

#include "dgUserConstraint.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dgUserConstraint::dgUserConstraint(dgWorld *const world, dgBody *const body0,
                                   dgBody *const body1, dgInt32 constraintID) : dgBilateralConstraint() {
	//  Init();
	//  m_lru = 0;
	m_maxDOF = 6;
	//  m_jointIsDead___ = false;
	m_enableCollision = false;
	m_constId = dgUnsigned32(dgUnknownConstraintId + constraintID);
	m_body0 = body0;
	m_body1 = body1;
	m_userData = NULL;
	m_destructor = NULL;

	world->AttachConstraint(this, body0, body1);
}

dgUserConstraint::~dgUserConstraint() {
}

void dgUserConstraint::GetInfo(dgConstraintInfo *const info) const {
	InitInfo(info);
}
