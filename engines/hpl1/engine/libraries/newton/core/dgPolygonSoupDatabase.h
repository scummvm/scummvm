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

/****************************************************************************
*
*  Visual C++ 6.0 created by: Julio Jerez
*
****************************************************************************/
#ifndef __dgPolygonSoupDatabase0x23413452233__
#define __dgPolygonSoupDatabase0x23413452233__


#include "dgStdafx.h"
#include "dgRef.h"
#include "dgArray.h"
#include "dgIntersections.h"

class dgMatrix;



class dgPolygonSoupDatabase { //: public dgRef
public:
	dgFloat32 GetRadius() const;
	dgInt32 GetVertexCount() const;
	dgInt32 GetStrideInBytes() const;
	dgFloat32 *GetLocalVertexPool() const;

	dgUnsigned32 GetTagId(const dgInt32 *face) const;
	void SetTagId(const dgInt32 *face, dgUnsigned32 newID) const;

	virtual void Serialize(dgSerialize callback, void *const userData) const = 0;
	virtual void Deserialize(dgDeserialize callback, void *const userData) = 0;


	virtual void GetAABB(dgVector &p0, dgVector &p1) const;
//	bool RayTest (const dgVector& p0, const dgVector& p1) const;
//	dgFloat32 RayHit (const dgVector& p0, const dgVector& p1, dgVector& unNormalizeNormal) const;


protected:
	virtual void ForAllSectors(const dgVector &min, const dgVector &max, dgAABBIntersectCallback callback, void *const context) const;
	virtual void ForAllSectorsSimd(const dgVector &min, const dgVector &max, dgAABBIntersectCallback callback, void *const context) const;
	virtual void ForAllSectorsRayHit(const dgFastRayTest &ray, dgRayIntersectCallback callback, void *const context) const;
	virtual void ForAllSectorsRayHitSimd(const dgFastRayTest &ray, dgRayIntersectCallback callback, void *const context) const;

	dgPolygonSoupDatabase(const char *name = NULL);
	virtual ~dgPolygonSoupDatabase();

//	dgAddRtti(dgRef);

	dgInt32 m_vertexCount;
	dgInt32 m_strideInBytes;
	dgFloat32 *m_localVertex;
};


inline dgPolygonSoupDatabase::dgPolygonSoupDatabase(const char *name)
//	:dgRef(name)
{
	m_vertexCount = 0;
	m_strideInBytes = 0;
	m_localVertex = NULL;
}

inline dgPolygonSoupDatabase::~dgPolygonSoupDatabase() {
	if (m_localVertex) {
		dgFreeStack(m_localVertex);
	}
}


inline dgUnsigned32 dgPolygonSoupDatabase::GetTagId(const dgInt32 *face) const {
	return dgUnsigned32(face[-1]);
}

inline void dgPolygonSoupDatabase::SetTagId(const dgInt32 *facePtr, dgUnsigned32 newID) const {
	dgUnsigned32 *face;
	face = const_cast<dgUnsigned32 *>((const dgUnsigned32 *)facePtr);
	face[-1] = newID;
}

inline dgInt32 dgPolygonSoupDatabase::GetVertexCount()  const {
	return m_vertexCount;
}

inline dgFloat32 *dgPolygonSoupDatabase::GetLocalVertexPool() const {
	return m_localVertex;
}

inline dgInt32 dgPolygonSoupDatabase::GetStrideInBytes() const {
	return m_strideInBytes;
}

inline dgFloat32 dgPolygonSoupDatabase::GetRadius() const {
	return 0.0f;
}

inline void dgPolygonSoupDatabase::ForAllSectorsSimd(const dgVector &min, const dgVector &max, dgAABBIntersectCallback callback, void *const context) const {
	NEWTON_ASSERT(0);
}



inline void dgPolygonSoupDatabase::ForAllSectors(const dgVector &min, const dgVector &max, dgAABBIntersectCallback callback, void *const context) const {
	NEWTON_ASSERT(0);
}


inline void dgPolygonSoupDatabase::GetAABB(dgVector &p0, dgVector &p1) const {
}


inline void dgPolygonSoupDatabase::ForAllSectorsRayHit(const dgFastRayTest &ray, dgRayIntersectCallback callback, void *const context) const {
	NEWTON_ASSERT(0);
}

inline void dgPolygonSoupDatabase::ForAllSectorsRayHitSimd(const dgFastRayTest &ray, dgRayIntersectCallback callback, void *const context) const {
	NEWTON_ASSERT(0);
}

#endif
