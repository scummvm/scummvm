/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_MESH_H
#define HPL_MESH_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/resources/ResourceBase.h"
#include "hpl1/engine/system/SystemTypes.h"

#include "hpl1/engine/physics/CollideShape.h"
#include "hpl1/engine/physics/PhysicsJoint.h"

#include "common/array.h"
#include "hpl1/engine/scene/Light3D.h"
#include "common/stablemap.h"
#include "common/multimap.h"

namespace hpl {

class cMaterialManager;
class cAnimationManager;
class cSubMesh;
class cMeshEntity;
class cSkeleton;
class cAnimation;
class cNode3D;
class iCollideShape;
class iPhysicsWorld;
class iPhysicsBody;
class iPhysicsJoint;
class cBillboard;
class cBeam;
class cParticleSystem3D;
class cSoundEntity;
class cWorld3D;

//--------------------------------------------------

typedef Common::Array<cAnimation *> tAnimationVec;
typedef tAnimationVec::iterator tAnimationVecIt;

typedef Common::StableMap<tString, int> tAnimationIndexMap;
typedef tAnimationIndexMap::iterator tAnimationIndexMapIt;

typedef Common::Array<cSubMesh *> tSubMeshVec;
typedef Common::Array<cSubMesh *>::iterator tSubMeshVecIt;

typedef Common::MultiMap<tString, cSubMesh *> tSubMeshMap;
typedef tSubMeshMap::iterator tSubMeshMapIt;

typedef Common::Array<cNode3D *> tNode3DVec;
typedef tNode3DVec::iterator tNode3DVecIt;

//--------------------------------------------------

class cMeshCollider {
public:
	tString msGroup;

	eCollideShapeType mType;
	cVector3f mvSize;
	cMatrixf m_mtxOffset;
};

typedef Common::Array<cMeshCollider *> tMeshColliderVec;
typedef tMeshColliderVec::iterator tMeshColliderVecIt;

//--------------------------------------------------

class cMeshJoint {
public:
	ePhysicsJointType mType;

	tString msName;

	bool mbCollide;

	cVector3f mvPivot;
	cVector3f mvPinDir;

	float mfMax;
	float mfMin;

	tString msChildBody;
	tString msParentBody;
};

typedef Common::Array<cMeshJoint *> tMeshJointVec;
typedef tMeshJointVec::iterator tMeshJointVecIt;

//--------------------------------------------------

class cMeshLight {
public:
	tString msParent;
	tString msFile;
	tString msName;

	eLight3DType mType;
	cMatrixf m_mtxTransform;
	cColor mColor;
	float mfRadius;
	float mfFOV;
	bool mbCastShadows;
};

typedef Common::Array<cMeshLight *> tMeshLightVec;
typedef tMeshLightVec::iterator tMeshLightVecIt;

//--------------------------------------------------

class cMeshBeam {
public:
	tString msFile;
	tString msName;

	tString msStartParent;
	cVector3f mvStartPosition;

	tString msEndParent;
	cVector3f mvEndPosition;
};

typedef Common::Array<cMeshBeam *> tMeshBeamVec;
typedef tMeshBeamVec::iterator tMeshBeamVecIt;

//--------------------------------------------------

class cMeshBillboard {
public:
	tString msParent;
	tString msFile;
	tString msName;

	cVector2f mvSize;
	cVector3f mvPosition;
	cVector3f mvAxis;
	float mfOffset;
};

typedef Common::Array<cMeshBillboard *> tMeshBillboardVec;
typedef tMeshBillboardVec::iterator tMeshBillboardVecIt;

//--------------------------------------------------

class cMeshParticleSystem {
public:
	tString msParent;
	tString msType;
	tString msName;

	cMatrixf m_mtxTransform;
	cVector3f mvSize;
};

typedef Common::Array<cMeshParticleSystem *> tMeshParticleSystemVec;
typedef tMeshParticleSystemVec::iterator tMeshParticleSystemVecIt;

//--------------------------------------------------

class cMeshSoundEntity {
public:
	tString msParent;
	tString msType;
	tString msName;

	cVector3f mvPosition;
};

typedef Common::Array<cMeshSoundEntity *> tMeshSoundEntityVec;
typedef tMeshSoundEntityVec::iterator tMeshSoundEntityVecIt;

//--------------------------------------------------

class cMeshReference {
public:
	tString msParent;
	tString msName;
	tString msFile;

	cMatrixf m_mtxTransform;
};

typedef Common::Array<cMeshReference *> tMeshReferenceVec;
typedef tMeshReferenceVec::iterator tMeshReferenceVecIt;

//--------------------------------------------------

class cMesh : public iResourceBase {
	friend class cSubMesh;
	friend class cMeshEntity;

public:
	cMesh(const tString asName, cMaterialManager *apMaterialManager,
		  cAnimationManager *apAnimationManager);
	~cMesh();

	bool CreateFromFile(const tString asFile);

	cSubMesh *CreateSubMesh(const tString &asName);

	cSubMesh *GetSubMesh(unsigned int alIdx);
	cSubMesh *GetSubMeshName(const tString &asName);
	int GetSubMeshNum();

	void SetSkeleton(cSkeleton *apSkeleton);
	cSkeleton *GetSkeleton();

	void AddAnimation(cAnimation *apAnimation);

	cAnimation *GetAnimation(int alIndex);
	cAnimation *GetAnimationFromName(const tString &asName);
	int GetAnimationIndex(const tString &asName);

	void ClearAnimations(bool abDeleteAll);

	int GetAnimationNum();

	void SetupBones();

	// Joints
	cMeshJoint *CreatePhysicsJoint(ePhysicsJointType);
	cMeshJoint *GetPhysicsJoint(int alIdx);
	int GetPhysicsJointNum();
	iPhysicsJoint *CreateJointInWorld(const tString &sNamePrefix, cMeshJoint *apMeshJoint,
									  iPhysicsBody *apParentBody, iPhysicsBody *apChildBody,
									  const cMatrixf &a_mtxOffset, iPhysicsWorld *apWorld);

	void CreateNodeBodies(iPhysicsBody **apRootBodyPtr, Common::Array<iPhysicsBody *> *apSubBodyVec,
						  cMeshEntity *apEntity, iPhysicsWorld *apWorld,
						  const cMatrixf &a_mtxTransform);

	void CreateJointsAndBodies(Common::Array<iPhysicsBody *> *apBodyVec, cMeshEntity *apEntity,
							   Common::Array<iPhysicsJoint *> *apJointVec,
							   const cMatrixf &a_mtxOffset, iPhysicsWorld *apWorld);

	bool HasSeveralBodies();

	// Colliders
	cMeshCollider *CreateCollider(eCollideShapeType aType);
	cMeshCollider *GetCollider(int alIdx);
	int GetColliderNum();
	iCollideShape *CreateCollideShape(iPhysicsWorld *apWorld);
	iCollideShape *CreateCollideShapeFromCollider(cMeshCollider *pCollider, iPhysicsWorld *apWorld);

	// Lights
	cMeshLight *CreateLight(eLight3DType aType);
	cMeshLight *GetLight(int alIdx);
	int GetLightNum();
	iLight3D *CreateLightInWorld(const tString &sNamePrefix, cMeshLight *apMeshLight,
								 cMeshEntity *apMeshEntity, cWorld3D *apWorld);

	// Billboards
	cMeshBillboard *CreateBillboard();
	cMeshBillboard *GetBillboard(int alIdx);
	int GetBillboardNum();
	cBillboard *CreateBillboardInWorld(const tString &sNamePrefix, cMeshBillboard *apMeshBillboard,
									   cMeshEntity *apMeshEntity, cWorld3D *apWorld);

	// Beams
	cMeshBeam *CreateBeam();
	cMeshBeam *GetBeam(int alIdx);
	int GetBeamNum();
	cBeam *CreateBeamInWorld(const tString &sNamePrefix, cMeshBeam *apMeshBeam,
							 cMeshEntity *apMeshEntity, cWorld3D *apWorld);

	// Particle systems
	cMeshParticleSystem *CreateParticleSystem();
	cMeshParticleSystem *GetParticleSystem(int alIdx);
	int GetParticleSystemNum();
	cParticleSystem3D *CreateParticleSystemInWorld(const tString &sNamePrefix, cMeshParticleSystem *apMeshPS,
												   cMeshEntity *apMeshEntity, cWorld3D *apWorld);

	// References
	cMeshReference *CreateReference();
	cMeshReference *GetReference(int alIdx);
	int GetReferenceNum();
	iEntity3D *CreateReferenceInWorld(const tString &sNamePrefix, cMeshReference *apMeshRef,
									  cMeshEntity *apMeshEntity, cWorld3D *apWorld,
									  const cMatrixf &a_mtxOffset);

	// Sound entities
	cMeshSoundEntity *CreateSoundEntity();
	cMeshSoundEntity *GetSoundEntity(int alIdx);
	int GetSoundEntityNum();
	cSoundEntity *CreateSoundEntityInWorld(const tString &sNamePrefix, cMeshSoundEntity *apMeshSound,
										   cMeshEntity *apMeshEntity, cWorld3D *apWorld);

	// Node
	cNode3D *GetRootNode();
	void AddNode(cNode3D *apNode);
	int GetNodeNum();
	cNode3D *GetNode(int alIdx);

	// Resources implementation
	bool reload() { return false; }
	void unload() {}
	void destroy() {}

private:
	cMaterialManager *mpMaterialManager;
	cAnimationManager *mpAnimationManager;

	tSubMeshVec mvSubMeshes;
	tSubMeshMap m_mapSubMeshes;

	cSkeleton *mpSkeleton;

	tAnimationVec mvAnimations;
	tAnimationIndexMap m_mapAnimIndices;

	cNode3D *mpRootNode;
	tNode3DVec mvNodes;

	tMeshJointVec mvPhysicJoints;
	tMeshColliderVec mvColliders;

	tMeshLightVec mvLights;
	tMeshBillboardVec mvBillboards;
	tMeshBeamVec mvBeams;
	tMeshParticleSystemVec mvParticleSystems;
	tMeshSoundEntityVec mvSoundEntities;
	tMeshReferenceVec mvReferences;
};

} // namespace hpl

#endif // HPL_MESH_H
