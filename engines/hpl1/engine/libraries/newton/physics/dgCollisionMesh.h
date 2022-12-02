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

#if !defined(__DGCOLLISIONPOLYGONALSOUP_H__)
#define __DGCOLLISIONPOLYGONALSOUP_H__


#include "dgCollision.h"
#include "dgCollisionConvex.h"

#define DG_MAX_COLLIDING_FACES   (1024 * 2)
#define DG_MAX_COLLIDING_VERTEX  (DG_MAX_COLLIDING_FACES * 4)


class dgCollisionMesh;
typedef void (*dgCollisionMeshCollisionCallback)(const dgBody *bodyWithTreeCollision, const dgBody *const body, dgInt32 faceID,
        dgInt32 vertexCount, const dgFloat32 *vertex, dgInt32 vertexStrideInBytes);


DG_MSC_VECTOR_ALIGMENT
class dgPolygonMeshDesc {
public:
	// colliding box in polygonSoup local space
	dgVector m_boxP0;
	dgVector m_boxP1;
	dgInt32 m_threadNumber;
	dgInt32 m_faceCount;
	dgInt32 m_vertexStrideInBytes;
	void *m_userData;
	dgFloat32 *m_vertex;
	dgInt32 *m_userAttribute;
	dgInt32 *m_faceIndexCount;
	dgInt32 *m_faceVertexIndex;
	dgBody *m_objBody;
	dgBody *m_polySoupBody;
	dgFloat32 *m_faceMaxSize;

	// private data;
	const dgCollisionMesh *m_me;
	dgInt32 *m_faceNormalIndex;
	dgInt32 *m_faceAdjencentEdgeNormal;

	dgInt32 m_globalIndexCount;
	dgInt32 m_globalUserAttribute[DG_MAX_COLLIDING_FACES];
	dgInt32 m_globalFaceIndexCount[DG_MAX_COLLIDING_FACES];
	dgInt32 m_globalFaceNormalIndex[DG_MAX_COLLIDING_FACES];
	dgFloat32 m_globalFaceMaxSize[DG_MAX_COLLIDING_FACES];
	dgInt32 m_globalFaceVertexIndex[DG_MAX_COLLIDING_VERTEX];
	dgInt32 m_globalAdjencentEdgeNormal[DG_MAX_COLLIDING_VERTEX];


} DG_GCC_VECTOR_ALIGMENT;

DG_MSC_VECTOR_ALIGMENT
class dgCollisionMeshRayHitDesc {
public:
	dgCollisionMeshRayHitDesc()
		: m_matrix(dgGetIdentityMatrix()) {
	}

	dgVector m_localP0;
	dgVector m_localP1;
	dgVector m_normal;
	dgUnsigned32 m_userId;
	void  *m_userData;
	void  *m_altenateUserData;
	dgMatrix m_matrix;
} DG_GCC_VECTOR_ALIGMENT;



class dgCollisionMesh: public dgCollision {
public:
	class DG_CLIPPED_FACE_EDGE {
	public:
		DG_CLIPPED_FACE_EDGE *m_next;
		DG_CLIPPED_FACE_EDGE *m_twin;
		dgInt32 m_incidentNormal;
		dgInt32 m_incidentVertex;
	};

	DG_MSC_VECTOR_ALIGMENT
	class dgGetVertexListIndexList {
	public:
		dgInt32 *m_indexList;
		dgInt32 *m_userDataList;
		dgFloat32 *m_veterxArray;
		dgInt32 m_triangleCount;
		dgInt32 m_maxIndexCount;
		dgInt32 m_vertexCount;
		dgInt32 m_vertexStrideInBytes;
	} DG_GCC_VECTOR_ALIGMENT;

	DG_MSC_VECTOR_ALIGMENT
	class dgCollisionConvexPolygon: public dgCollisionConvex {
	public:
		dgCollisionConvexPolygon(dgMemoryAllocator *const allocator);
		~dgCollisionConvexPolygon();

		virtual dgInt32 CalculateSignature() const;
		virtual void Serialize(dgSerialize callback, void *const userData) const;
		virtual void SetCollisionBBox(const dgVector &p0, const dgVector &p1);
		virtual dgFloat32 RayCast(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const;
		virtual bool OOBBTest(const dgMatrix &matrix, const dgCollisionConvex *const shape, void *const cacheOrder) const;

		virtual dgVector SupportVertex(const dgVector &dir) const;
		virtual dgVector SupportVertexSimd(const dgVector &dir) const;

		virtual dgInt32 CalculatePlaneIntersection(const dgVector &normal, const dgVector &point, dgVector *const contactsOut) const;
		virtual dgInt32 CalculatePlaneIntersectionSimd(const dgVector &normal, const dgVector &point, dgVector *const contactsOut) const;

		virtual dgFloat32 GetVolume() const;
		virtual dgFloat32 GetBoxMinRadius() const;
		virtual dgFloat32 GetBoxMaxRadius() const;
		virtual void CalculateInertia(dgVector &inertia, dgVector &origin) const;
		virtual bool IsEdgeIntersection() const;

		void BeamClipping(const dgCollisionConvex *const hull, const dgMatrix &matrix, dgFloat32 size);
		void BeamClippingSimd(const dgCollisionConvex *const hull, const dgMatrix &matrix, dgFloat32 size);
		dgInt32 QuickTest(const dgCollisionConvex *const hull, const dgMatrix &matrix);
		dgInt32 QuickTestSimd(const dgCollisionConvex *const hull, const dgMatrix &matrix);
		dgInt32 QuickTestContinue(const dgCollisionConvex *const hull, const dgMatrix &matrix);
		dgInt32 QuickTestContinueSimd(const dgCollisionConvex *const hull, const dgMatrix &matrix);
		dgInt32 ClipContacts(dgInt32 count, dgContactPoint *const contactOut, const dgMatrix &globalMatrix) const;

		dgVector ClosestDistanceToTriangle(const dgVector &point, const dgVector &p0, const dgVector &p1, const dgVector &p2, bool &isEdge) const;
		bool PointToPolygonDistance(const dgVector &point, dgFloat32 radius, dgVector &out, bool &isEdge);
		bool DistanceToOrigen(const dgMatrix &matrix, const dgVector &scale, dgFloat32 radius, dgVector &out, bool &isEdge);
		dgFloat32 MovingPointToPolygonContact(const dgVector &p, const dgVector &veloc, dgFloat32 radius, dgContactPoint &contact);
		dgFloat32 MovingSphereToEdgeContact(const dgVector &center, const dgVector &veloc, dgFloat32 radius, const dgVector &v0, const dgVector &v1, dgVector &contactOutOnLine) const;

		void CalculateNormal();
		void CalculateNormalSimd();

		dgVector m_normal;
		dgVector m_localPoly[64];
		dgVector m_localPolySimd[64 * 3 / 4];
		dgInt32 m_clippEdgeNormal[64];
		dgInt32 m_count;
		dgInt32 m_paddedCount;
		dgInt32 m_normalIndex;
		dgInt32 m_stride;
		dgInt32 *m_index;
		dgInt32 *m_adjacentNormalIndex;
		dgFloat32 *m_vertex;
		mutable dgInt32 m_isEdgeIntersection;

	} DG_GCC_VECTOR_ALIGMENT;

	dgCollisionMesh(dgMemoryAllocator *const allocator, dgCollisionID type);
	dgCollisionMesh(dgWorld *const world, dgDeserialize deserialization, void *const userData);
	virtual ~dgCollisionMesh();

	void SetCollisionCallback(dgCollisionMeshCollisionCallback debugCallback);



	virtual void *GetUserData() const;
	virtual dgFloat32 GetVolume() const;
	virtual dgFloat32 GetBoxMinRadius() const;
	virtual dgFloat32 GetBoxMaxRadius() const;
	virtual void CalculateInertia(dgVector &inertia, dgVector &origin) const;
	virtual void GetVertexListIndexList(const dgVector &p0, const dgVector &p1, dgGetVertexListIndexList &data) const = 0;

	virtual void GetCollidingFaces(dgPolygonMeshDesc *const data) const = 0;
	virtual void GetCollidingFacesSimd(dgPolygonMeshDesc *const data) const = 0;

	dgCollisionMeshCollisionCallback GetDebugCollisionCallback() const {
		return m_debugCallback;
	}
//	dgCollisionMeshUserRayCastCallback GetDebugRayCastCallback() const { return m_userRayCastCallback;}

protected:
	virtual void SetCollisionBBox(const dgVector &p0, const dgVector &p1);

private:
	virtual dgInt32 CalculateSignature() const;
	dgVector SupportVertex(const dgVector &dir) const;
	virtual void CalcAABB(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const;
	virtual void CalcAABBSimd(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const;
	virtual bool OOBBTest(const dgMatrix &matrix, const dgCollisionConvex *const shape, void *const cacheOrder) const;


	virtual void DebugCollision(const dgMatrix &matrix, OnDebugCollisionMeshCallback callback, void *const userData) const;

	virtual dgVector CalculateVolumeIntegral(const dgMatrix &globalMatrix, GetBuoyancyPlane bouyancyPlane, void *const context) const;
	virtual dgFloat32 RayCast(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const = 0;
	virtual dgFloat32 RayCastSimd(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const = 0;

	dgInt32 CalculatePlaneIntersection(const dgFloat32 *vertex, const dgInt32 *index, dgInt32 indexCount, dgInt32 strideInFloat,
	                                   const dgPlane &localPlane, dgVector *const contactsOut) const;


	virtual void GetCollisionInfo(dgCollisionInfo *info) const;
	virtual void Serialize(dgSerialize callback, void *const userData) const;

#ifdef DG_DEBUG_AABB
	dgVector BoxSupportMapping(const dgVector &dir) const;
#endif

protected:
	dgVector m_boxSize;
	dgVector m_boxOrigin;

	dgMemoryAllocator *m_allocator;
	dgCollisionMeshCollisionCallback m_debugCallback;
//	dgCollisionMeshUserRayCastCallback m_userRayCastCallback;
	dgCollisionConvexPolygon *m_polygon[DG_MAXIMUN_THREADS];

	friend class dgWorld;
};



#endif

