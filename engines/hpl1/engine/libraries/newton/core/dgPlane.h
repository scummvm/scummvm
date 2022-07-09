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

#ifndef __dgPlane__
#define __dgPlane__

#include "dgStdafx.h"
#include "dgVector.h"

DG_MSC_VECTOR_ALIGMENT
class dgPlane: public dgVector
{
	public:
	dgPlane ();
	dgPlane (dgFloat32 x, dgFloat32 y, dgFloat32 z, dgFloat32 w);
	dgPlane (const dgVector &normal, dgFloat32 distance); 
	dgPlane (const dgVector &P0, const dgVector &P1, const dgVector &P2);
	dgPlane Scale (dgFloat32 s) const;
	dgFloat32 Evalue (const dgFloat32 *point) const;
	dgFloat32 Evalue (const dgVector &point) const;
}DG_GCC_VECTOR_ALIGMENT;

DG_MSC_VECTOR_ALIGMENT
class dgBigPlane: public dgBigVector
{
	public:
	dgBigPlane ();
	dgBigPlane (dgFloat64 x, dgFloat64 y, dgFloat64 z, dgFloat64 w);
	dgBigPlane (const dgBigVector &normal, dgFloat64 distance); 
	dgBigPlane (const dgBigVector &P0, const dgBigVector &P1, const dgBigVector &P2);
	dgBigPlane Scale (dgFloat64 s) const;
	dgFloat64 Evalue (const dgFloat32 *point) const;
#ifndef __USE_DOUBLE_PRECISION__
	dgFloat64 Evalue (const dgFloat64 *point) const;
#endif
	dgFloat64 Evalue (const dgVector &point) const;
	dgFloat64 Evalue (const dgBigVector &point) const;
}DG_GCC_VECTOR_ALIGMENT;




DG_INLINE dgPlane::dgPlane () 
	:dgVector () 
{
}

DG_INLINE dgPlane::dgPlane (dgFloat32 x, dgFloat32 y, dgFloat32 z, dgFloat32 w)
	:dgVector (x, y, z, w) 
{
}

DG_INLINE dgPlane::dgPlane (const dgVector &normal, dgFloat32 distance) 
	:dgVector (normal)
{
	m_w = distance;
}

DG_INLINE dgPlane::dgPlane (const dgVector &P0, const dgVector &P1, const dgVector &P2)
	:dgVector ((P1 - P0) * (P2 - P0)) 
{
	m_w = - (*this % P0);
}

DG_INLINE dgPlane dgPlane::Scale (dgFloat32 s)	const
{
	return dgPlane (m_x * s, m_y * s, m_z * s, m_w * s);
}


DG_INLINE dgFloat32 dgPlane::Evalue (const dgFloat32 *point) const
{
	return m_x * point[0] + m_y * point[1] + m_z * point[2] + m_w;
}

DG_INLINE dgFloat32 dgPlane::Evalue (const dgVector &point) const
{
	return m_x * point.m_x + m_y * point.m_y + m_z * point.m_z + m_w;
}



DG_INLINE dgBigPlane::dgBigPlane () 
	:dgBigVector () 
{
}

DG_INLINE dgBigPlane::dgBigPlane (dgFloat64 x, dgFloat64 y, dgFloat64 z, dgFloat64 w)
	:dgBigVector (x, y, z, w) 
{
}


DG_INLINE dgBigPlane::dgBigPlane (const dgBigVector &normal, dgFloat64 distance) 
	:dgBigVector (normal)
{
	m_w = distance;
}

DG_INLINE dgBigPlane::dgBigPlane (const dgBigVector &P0, const dgBigVector &P1, const dgBigVector &P2)
	:dgBigVector ((P1 - P0) * (P2 - P0)) 
{
	m_w = - (*this % P0);
}

DG_INLINE dgBigPlane dgBigPlane::Scale (dgFloat64 s) const
{
	return dgBigPlane (m_x * s, m_y * s, m_z * s, m_w * s);
}

DG_INLINE dgFloat64 dgBigPlane::Evalue (const dgFloat32 *point) const
{
	return m_x * point[0] + m_y * point[1] + m_z * point[2] + m_w;
}

#ifndef __USE_DOUBLE_PRECISION__
DG_INLINE dgFloat64 dgBigPlane::Evalue (const dgFloat64 *point) const
{
	return m_x * point[0] + m_y * point[1] + m_z * point[2] + m_w;
}
#endif

DG_INLINE dgFloat64 dgBigPlane::Evalue (const dgVector &point) const
{
	return m_x * point.m_x + m_y * point.m_y + m_z * point.m_z + m_w;
}

DG_INLINE dgFloat64 dgBigPlane::Evalue (const dgBigVector &point) const
{
	return m_x * point.m_x + m_y * point.m_y + m_z * point.m_z + m_w;
}


#endif


