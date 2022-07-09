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

#ifndef __dgQuaternion__
#define __dgQuaternion__

#include "dgStdafx.h"

class dgVector;
class dgMatrix;

DG_MSC_VECTOR_ALIGMENT
class dgQuaternion
{
	public:
	dgQuaternion (); 
	dgQuaternion (const dgMatrix &matrix);
	dgQuaternion (dgFloat32 q0, dgFloat32 q1, dgFloat32 q2, dgFloat32 q3); 
	dgQuaternion (const dgVector &unit_Axis, dgFloat32 angle = dgFloat32 (0.0f));
	
	void Scale (dgFloat32 scale); 
	void Normalize (); 
	inline dgFloat32 DotProduct (const dgQuaternion &QB) const;
	dgQuaternion Inverse () const; 

	dgQuaternion Slerp (const dgQuaternion &q1, dgFloat32 t) const;
	dgVector CalcAverageOmega (const dgQuaternion &q1, dgFloat32 dt) const;

	dgQuaternion operator* (const dgQuaternion &B) const;
	dgQuaternion operator+ (const dgQuaternion &B) const; 
	dgQuaternion operator- (const dgQuaternion &B) const; 

	dgFloat32 m_q0;
	dgFloat32 m_q1;
	dgFloat32 m_q2;
	dgFloat32 m_q3;
}DG_GCC_VECTOR_ALIGMENT;




inline dgQuaternion::dgQuaternion () 
{
	m_q0 = dgFloat32 (1.0f);
	m_q1 = dgFloat32 (0.0f);
	m_q2 = dgFloat32 (0.0f);
	m_q3 = dgFloat32 (0.0f);
}

inline dgQuaternion::dgQuaternion (dgFloat32 Q0, dgFloat32 Q1, dgFloat32 Q2, dgFloat32 Q3) 
{
	m_q0 = Q0;
	m_q1 = Q1;
	m_q2 = Q2;
	m_q3 = Q3;
//	_ASSERTE (dgAbsf (DotProduct (*this) -dgFloat32 (1.0f)) < dgFloat32(1.0e-4f));
}



inline void dgQuaternion::Scale (dgFloat32 scale) 
{
	m_q0 *= scale;
	m_q1 *= scale;
	m_q2 *= scale;
	m_q3 *= scale;
}

inline void dgQuaternion::Normalize () 
{
	Scale (dgRsqrt (DotProduct (*this)));
}

inline dgFloat32 dgQuaternion::DotProduct (const dgQuaternion &QB) const
{
	return m_q0 * QB.m_q0 + m_q1 * QB.m_q1 + m_q2 * QB.m_q2 + m_q3 * QB.m_q3;
}

inline dgQuaternion dgQuaternion::Inverse () const 
{
	return dgQuaternion (m_q0, -m_q1, -m_q2, -m_q3);
}

inline dgQuaternion dgQuaternion::operator+ (const dgQuaternion &B) const
{
	return dgQuaternion (m_q0 + B.m_q0, m_q1 + B.m_q1, m_q2 + B.m_q2, m_q3 + B.m_q3);
}

inline dgQuaternion dgQuaternion::operator- (const dgQuaternion &B) const
{
	return dgQuaternion (m_q0 - B.m_q0, m_q1 - B.m_q1, m_q2 - B.m_q2, m_q3 - B.m_q3);
}


inline dgQuaternion dgQuaternion::operator* (const dgQuaternion &B) const
{
	return dgQuaternion (B.m_q0 * m_q0 - B.m_q1 * m_q1 - B.m_q2 * m_q2 - B.m_q3 * m_q3, 
				 		 B.m_q1 * m_q0 + B.m_q0 * m_q1 - B.m_q3 * m_q2 + B.m_q2 * m_q3, 
						 B.m_q2 * m_q0 + B.m_q3 * m_q1 + B.m_q0 * m_q2 - B.m_q1 * m_q3, 
						 B.m_q3 * m_q0 - B.m_q2 * m_q1 + B.m_q1 * m_q2 + B.m_q0 * m_q3); 
}



#endif

