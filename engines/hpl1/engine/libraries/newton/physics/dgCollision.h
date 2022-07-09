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

#ifndef AFX_DGCOLLISION_H__57E159CE_6B6F_42DE_891C_1F6C38EB9D29__INCLUDED_
#define AFX_DGCOLLISION_H__57E159CE_6B6F_42DE_891C_1F6C38EB9D29__INCLUDED_

#ifdef _WIN32
//	#define DG_INLINE_FUNTION inline
#define DG_INLINE_FUNTION DG_INLINE
//	#define DG_INLINE_FUNTION __forceinline
#else
#define DG_INLINE_FUNTION inline
#endif

#include "dgBody.h"
#include "hpl1/engine/libraries/newton/core/dg.h"

class dgBody;
class dgCollision;
class dgMeshEffect;
class dgContactPoint;
class dgPolygonSoupDesc;
class dgCollisionConvex;
class dgCollisionConvexHull;
class dgPolygonSoupRayHitDesc;

#define DG_MAX_COLLISION_PADDING dgFloat32(1.0f / 8.0f)
//#define DG_MAX_COLLISION_PADDING dgFloat32 (1.0f)

typedef dgInt32 (*dgCollisionCompoundBreakableCallback)(dgMeshEffect *const solid, void *userData, dgMatrix &planeMatrixOut);

#ifdef _DEBUG
//	#define DG_DEBUG_AABB
#endif

//#define SERIALIZE_END	'dne '
#define SERIALIZE_END 0x646e6520
#define PREFILTER_RAYCAST(filter, body, collision, userData) (filter && !filter(body, collision, userData))

enum dgCollisionID {
	m_boxCollision = 0,
	m_coneCollision,
	m_sphereCollision,
	m_capsuleCollision,
	m_cylinderCollision,
	m_compoundCollision,
	m_convexHullCollision,
	m_convexCollisionModifier,
	m_chamferCylinderCollision,
	m_boundingBoxHierachy,
	m_nullCollision,
	m_heightField,
	m_userMeshCollision,
	m_sceneCollision,
	m_compoundBreakable,

	m_polygonCollision,
	m_ellipseCollision,
	m_convexConvexIntance,

};

class dgCollisionInfo {
public:
	struct dgBoxData {
		dgFloat32 m_x;
		dgFloat32 m_y;
		dgFloat32 m_z;
	};

	struct dgSphereData {
		dgFloat32 m_r0;
		dgFloat32 m_r1;
		dgFloat32 m_r2;
	};

	struct dgCylinderData {
		dgFloat32 m_r0;
		dgFloat32 m_r1;
		dgFloat32 m_height;
	};

	struct dgCapsuleData {
		dgFloat32 m_r0;
		dgFloat32 m_r1;
		dgFloat32 m_height;
	};

	struct dgConeData {
		dgFloat32 m_r;
		dgFloat32 m_height;
	};

	struct dgChamferCylinderData {
		dgFloat32 m_r;
		dgFloat32 m_height;
	};

	struct dgConvexHullData {
		dgInt32 m_vertexCount;
		dgInt32 m_strideInBytes;
		dgInt32 m_faceCount;
		dgVector *m_vertex;
	};

	struct dgConvexModifierData {
		dgCollision *m_child;
	};

	struct dgCoumpountCollisionData {
		dgInt32 m_chidrenCount;
		dgCollision **m_chidren;
	};

	struct dgCollisionBVHData {
		dgInt32 m_vertexCount;
		dgInt32 m_indexCount;
	};

	struct dgHeightMapCollisionData {
		dgInt32 m_width;
		dgInt32 m_height;
		dgInt32 m_gridsDiagonals;
		dgFloat32 m_horizonalScale;
		dgFloat32 m_verticalScale;
		dgUnsigned16 *m_elevation;
		dgInt8 *m_atributes;
	};

	struct dgSceneData {
		dgInt32 m_childrenProxyCount;
	};

	dgMatrix m_offsetMatrix;
	dgInt32 m_collisionType;
	dgInt32 m_refCount;
	dgInt32 m_userDadaID;
	union {
		dgBoxData m_box;
		dgConeData m_cone;
		dgSphereData m_sphere;
		dgCapsuleData m_capsule;
		dgCylinderData m_cylinder;
		dgChamferCylinderData m_chamferCylinder;
		dgConvexHullData m_convexHull;
		dgConvexModifierData m_convexModifierData;
		dgCoumpountCollisionData m_compoundCollision;
		dgCollisionBVHData m_bvhCollision;
		dgHeightMapCollisionData m_heightFieldCollision;
		dgSceneData m_sceneCollision;
		dgFloat32 m_paramArray[32];
	};
};

typedef dgInt32(dgApi *OnCompoundCollisionPrefilter)(const dgBody *bodyA, const dgCollision *collisionA, const dgBody *bodyB, const dgCollision *collisionB);
typedef void(dgApi *OnDebugCollisionMeshCallback)(void *userData, int vertexCount, const dgFloat32 *FaceArray, int faceId);

class dgCollisionBoundPlaneCache {
public:
	dgCollisionBoundPlaneCache() {
		memset(m_planes, 0, sizeof(m_planes));
	}
	dgPlane m_planes[2];
};

DG_MSC_VECTOR_ALIGMENT
class dgCollision //: public dgRef
{
public:
	enum RTTI {
		dgCollisionNull_RTTI = 1 << 0,
		dgCollisionBox_RTTI = 1 << 1,
		dgCollisionCone_RTTI = 1 << 2,
		dgCollisionSphere_RTTI = 1 << 3,
		dgCollisionEllipse_RTTI = 1 << 4,
		dgCollisionCapsule_RTTI = 1 << 5,
		dgCollisionCylinder_RTTI = 1 << 6,
		dgCollisionConvexHull_RTTI = 1 << 7,
		dgCollisionChamferCylinder_RTTI = 1 << 8,
		dgCollisionConvexModifier_RTTI = 1 << 9,
		dgCollisionConvexPolygon_RTTI = 1 << 10,
		dgConvexCollision_RTTI = 1 << 11,
		dgCollisionCompound_RTTI = 1 << 12,

		dgCollisionBVH_RTTI = 1 << 13,
		dgCollisionMesh_RTTI = 1 << 14,
		dgCollisionUserMesh_RTTI = 1 << 15,
		dgCollisionHeightField_RTTI = 1 << 16,
		dgCollisionScene_RTTI = 1 << 17,
		dgCollisionCompoundBreakable_RTTI = 1 << 18,
	};

	DG_CLASS_ALLOCATOR(allocator)

	const dgMatrix &GetOffsetMatrix() const;
	void SetOffsetMatrix(const dgMatrix &matrix);

	dgCollisionID GetCollisionPrimityType() const;
	static dgUnsigned32 Quantize(dgFloat32 value);
	static dgUnsigned32 MakeCRC(void *buffer, int size);

	dgUnsigned32 SetUserDataID() const;
	void SetUserDataID(dgUnsigned32 userData);

	dgInt32 IsType(RTTI type) const {
		return type & m_rtti;
	}

	virtual void *GetUserData() const;
	virtual void SetUserData(void *const userData);

	virtual dgVector SupportVertex(const dgVector &dir) const = 0;
	virtual void SetCollisionBBox(const dgVector &p0, const dgVector &p1) = 0;
	virtual void CalcAABB(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const = 0;
	virtual void CalcAABBSimd(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const = 0;
	virtual bool OOBBTest(const dgMatrix &matrix, const dgCollisionConvex *const shape, void *const cacheOrder) const = 0;
	virtual bool IsEdgeIntersection() const;

	virtual void DebugCollision(const dgMatrix &matrix, OnDebugCollisionMeshCallback callback, void *const userData) const = 0;
	virtual dgFloat32 RayCast(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const = 0;
	virtual dgFloat32 RayCastSimd(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const {
		_ASSERTE(0);
		return 0;
	};
	virtual dgFloat32 GetVolume() const = 0;

	virtual dgFloat32 GetBoxMinRadius() const = 0;
	virtual dgFloat32 GetBoxMaxRadius() const = 0;
	virtual void CalculateInertia(dgVector &inertia, dgVector &origin) const = 0;
	virtual dgVector CalculateVolumeIntegral(const dgMatrix &globalMatrix, GetBuoyancyPlane bouyancyPlane, void *const context) const = 0;
	virtual void Serialize(dgSerialize callback, void *const userData) const = 0;

	virtual void GetCollisionInfo(dgCollisionInfo *info) const;
	virtual void SerializeLow(dgSerialize callback, void *const userData) const;

	dgUnsigned32 GetSignature() const;

	// interface for the convex hull modifier
	virtual dgMatrix ModifierGetMatrix() const;
	virtual void ModifierSetMatrix(const dgMatrix &matrix);

	virtual bool IsTriggerVolume() const;
	virtual void SetAsTriggerVolume(bool mode);

	virtual void SetBreakImpulse(dgFloat32 force);
	virtual dgFloat32 GetBreakImpulse() const;

	dgCollision *AddRef();
	virtual dgInt32 Release();
	dgInt32 GetRefCount() const;

	dgMemoryAllocator *GetAllocator() const;

protected:
	dgCollision(dgMemoryAllocator *const allocator, dgUnsigned32 signature, const dgMatrix &matrix, dgCollisionID id);
	dgCollision(dgWorld *const world, dgDeserialize deserialization, void *const userData);
	virtual ~dgCollision();

	void SetSignature(dgInt32 signature);
	virtual dgInt32 CalculateSignature() const = 0;

	dgMatrix m_offset;
	dgMemoryAllocator *m_allocator;
	dgInt32 m_rtti;
	dgInt32 m_refCount;
	dgUnsigned32 m_userDataID;
	dgUnsigned32 m_signature;
	dgCollisionID m_collsionId;

private:
	//	dgAddRtti(dgRef);

	friend class dgBody;
	friend class dgWorld;
	friend class dgMinkowskiConv;
	friend class dgCollisionCompound;
} DG_GCC_VECTOR_ALIGMENT;

inline dgCollisionID dgCollision::GetCollisionPrimityType() const {
	return m_collsionId;
}

inline dgUnsigned32 dgCollision::GetSignature() const {
	return m_signature;
}

inline void dgCollision::SetSignature(dgInt32 signature) {
	m_signature = dgUnsigned32(signature);
}

inline const dgMatrix &dgCollision::GetOffsetMatrix() const {
	return m_offset;
}

inline dgMatrix dgCollision::ModifierGetMatrix() const {
	return dgGetIdentityMatrix();
}

inline void dgCollision::ModifierSetMatrix(const dgMatrix &matrix) {
}

inline void dgCollision::SetAsTriggerVolume(bool mode) {
}

inline bool dgCollision::IsTriggerVolume() const {
	return false;
}

inline dgUnsigned32 dgCollision::SetUserDataID() const {
	return m_userDataID;
}

inline void dgCollision::SetUserDataID(dgUnsigned32 userData) {
	m_userDataID = userData;
}

inline dgCollision *dgCollision::AddRef() {
	m_refCount++;
	return this;
}

inline dgInt32 dgCollision::Release() {
	m_refCount--;
	if (m_refCount) {
		return m_refCount;
	}
	delete this;
	return 0;
}

inline dgInt32 dgCollision::GetRefCount() const {
	return m_refCount;
}

inline void dgCollision::SetBreakImpulse(dgFloat32 force) {
}

inline dgFloat32 dgCollision::GetBreakImpulse() const {
	return dgFloat32(1.0e10f);
}

inline dgMemoryAllocator *dgCollision::GetAllocator() const {
	return m_allocator;
}

inline bool dgCollision::IsEdgeIntersection() const {
	return false;
}

#endif
