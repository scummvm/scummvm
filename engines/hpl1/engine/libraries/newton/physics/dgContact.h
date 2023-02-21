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

#if !defined(AFX_DGCONTACT_H__BDE5B2AC_5834_46FD_A994_65D855788F69__INCLUDED_)
#define AFX_DGCONTACT_H__BDE5B2AC_5834_46FD_A994_65D855788F69__INCLUDED_


#include "dgConstraint.h"


class dgBody;
class dgWorld;
class dgContact;
class dgCollision;
class dgContactPoint;
class dgContactMaterial;
class dgPolygonMeshDesc;


#define DG_MAX_CONTATCS         128
#define DG_CACHE_PAIR_BUFFER    256

typedef bool (dgApi *OnAABBOverlap)(const dgContactMaterial &material, const dgBody &body0, const dgBody &body1, dgInt32 threadIndex);
typedef void (dgApi *OnContactCallback)(dgContact &contactJoint, dgFloat32 timestep, dgInt32 threadIndex);


class dgActiveContacts: public dgList<dgContact *> {
public:
	dgActiveContacts(dgMemoryAllocator *const allocator)
		: dgList<dgContact*>(allocator) {
	}
};

class dgCollidingPairCollector {
public:
	struct dgPair {
		dgBody *m_body0;
		dgBody *m_body1;
		dgContact *m_contact;
		const dgContactMaterial *m_material;
		dgContactPoint *m_contactBuffer;
		dgInt16 m_contactCount;
		dgInt16 m_isTrigger;
	};

	struct dgThreadPairCache {
		dgInt32 m_count;
		dgPair m_chacheBuffer[DG_CACHE_PAIR_BUFFER];
	};
	dgThreadPairCache *m_chacheBuffers[DG_MAXIMUN_THREADS];


	dgCollidingPairCollector();
	~dgCollidingPairCollector();

	void Init();
	void SetCaches(dgThreadPairCache *const chaches);
	void FlushChache(dgThreadPairCache *const chache);
	void AddPair(dgBody *const body0, dgBody *const body1, dgInt32 threadIndex);


	dgPair *m_pairs;
	dgInt32 m_count;
	dgInt32 m_maxSize;
	dgBody *m_sentinel;
};



DG_MSC_VECTOR_ALIGMENT
class dgContactPoint {
public:
	dgVector m_point;
	dgVector m_normal;
	dgBody *m_body0;
	dgBody *m_body1;
	dgCollision *m_collision0;
	dgCollision *m_collision1;
	dgInt64 m_userId;
	dgFloat32 m_penetration;
	dgInt32 m_isEdgeContact;
} DG_GCC_VECTOR_ALIGMENT;

DG_MSC_VECTOR_ALIGMENT
class dgCollisionParamProxy {
public:
	dgBody *m_floatingBody;
	dgBody *m_referenceBody;
	dgCollision *m_floatingCollision;
	dgCollision *m_referenceCollision;
	dgMatrix m_floatingMatrix;
	dgMatrix m_referenceMatrix;
	dgFloat32 m_timestep;
	dgFloat32 m_penetrationPadding;
	dgInt32 m_continueCollision;
	dgInt32 m_unconditionalCast;
	dgInt32 m_threadIndex;
	dgInt32 m_maxContacts;
	dgContactPoint *m_contacts;

	// used but Mink solver
	const dgMatrix *m_localMatrixInv;
	const dgPolygonMeshDesc *m_polyMeshData;

	dgUnsigned32 m_isTriggerVolume : 1;
	dgUnsigned32 m_inTriggerVolume : 1;


	dgCollisionParamProxy(dgInt32 threadIndex) {
		m_threadIndex = threadIndex;
		m_polyMeshData = NULL;
		m_localMatrixInv = NULL;
//		m_projectContinueCollisionContacts = 1;
	}

} DG_GCC_VECTOR_ALIGMENT;



DG_MSC_VECTOR_ALIGMENT
class dgContactMaterial: public dgContactPoint {
public:

	enum {
		m_collisionEnable__ = 1 << 0,
		m_friction0Enable__ = 1 << 1,
		m_friction1Enable__ = 1 << 2,
		m_override0Accel__ = 1 << 3,
		m_override1Accel__ = 1 << 4,
		m_overrideNormalAccel__ = 1 << 5,
		m_collisionContinueCollisionEnable__ = 1 << 6,
	};

	dgContactMaterial();
	void *GetUserData() const;
	void SetUserData(void *const userData);
	void SetCollisionCallback(OnAABBOverlap abbOvelap, OnContactCallback callback);
	void SetCompoundCollisionCallback(OnAABBOverlap abbOvelap);

	dgVector m_dir0;
	dgVector m_dir1;
	dgFloat32 m_normal_Force;
	dgFloat32 m_dir0_Force;
	dgFloat32 m_dir1_Force;
	dgFloat32 m_softness;
	dgFloat32 m_restitution;
	dgFloat32 m_staticFriction0;
	dgFloat32 m_staticFriction1;
	dgFloat32 m_dynamicFriction0;
	dgFloat32 m_dynamicFriction1;
	dgFloat32 m_penetrationPadding;
	dgInt32 m_flags;
//	union {
//		dgInt32 m_flags;
//		struct {
//			dgUnsigned32 m_collisionEnable                 : 1;
//			dgUnsigned32 m_friction0Enable                 : 1;
//			dgUnsigned32 m_friction1Enable                 : 1;
//			dgUnsigned32 m_override0Accel                  : 1;
//			dgUnsigned32 m_override1Accel                  : 1;
//			dgUnsigned32 m_overrideNormalAccel             : 1;
//			dgUnsigned32 m_collisionContinueCollisionEnable : 1;
//		};
//	};

private:
	void *m_userData;
	OnAABBOverlap m_aabbOverlap;
	OnContactCallback m_contactPoint;
	OnAABBOverlap m_compoundAABBOverlap;

	friend class dgWorld;
	friend class dgCollisionCompound;
	friend class dgBroadPhaseCollision;
	friend class dgSolverWorlkerThreads;
	friend class dgCollidingPairCollector;
	friend class dgBroadPhaseMaterialCallbackWorkerThread;

} DG_GCC_VECTOR_ALIGMENT;



DG_MSC_VECTOR_ALIGMENT
class dgContact:
	public dgConstraint,
	public dgList<dgContactMaterial> {

	dgContact(dgWorld *world);
	virtual ~dgContact();

	DG_CLASS_ALLOCATOR(allocator)

	virtual void GetInfo(dgConstraintInfo *const info) const;
	virtual dgUnsigned32 JacobianDerivative(dgContraintDescritor &params);
	virtual void JointAccelerations(const dgJointAccelerationDecriptor &params);
	virtual void JointAccelerationsSimd(const dgJointAccelerationDecriptor &params);
	virtual void JointVelocityCorrection(const dgJointAccelerationDecriptor &params);

	virtual void SetDestructorCallback(OnConstraintDestroy destructor);

	void JacobianContactDerivative(dgContraintDescritor &params, dgContactMaterial &contact, dgInt32 normalIndex, dgInt32 &frictionIndex);
	void CalculatePointDerivative(dgInt32 index, dgContraintDescritor &desc, const dgVector &dir, const dgPointParam &param) const;


	dgVector m_prevPosit0;
	dgVector m_prevPosit1;
	dgQuaternion m_prevRotation0;
	dgQuaternion m_prevRotation1;

	dgWorld *m_world;
	dgActiveContacts::dgListNode *m_contactNode;
	const dgContactMaterial *m_myCacheMaterial;
	dgInt32 m_broadphaseLru;

	friend class dgWorld;
	friend class dgActiveContacts;
	friend class dgTireCollision;
	friend class dgBroadPhaseCollision;
	friend class dgSolverWorlkerThreads;
	friend class dgCollidingPairCollector;
} DG_GCC_VECTOR_ALIGMENT;

inline void dgContactMaterial::SetCollisionCallback(OnAABBOverlap aabbOverlap, OnContactCallback contact) {
	m_aabbOverlap = aabbOverlap;
	m_contactPoint = contact;
}

inline void dgContactMaterial::SetCompoundCollisionCallback(OnAABBOverlap aabbOverlap) {
	m_compoundAABBOverlap = aabbOverlap;
}

inline void *dgContactMaterial::GetUserData() const {
	return m_userData;
}

inline void dgContactMaterial::SetUserData(void *const userData) {
	m_userData = userData;
}



inline void dgContact::SetDestructorCallback(OnConstraintDestroy destructor) {
}

#endif
