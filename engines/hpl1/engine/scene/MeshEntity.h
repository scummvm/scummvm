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

#ifndef HPL_MESH_ENTITY_H
#define HPL_MESH_ENTITY_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/Renderable.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/math/MeshTypes.h"
#include "hpl1/engine/scene/Entity3D.h"
#include "hpl1/engine/system/SystemTypes.h"

#include "hpl1/engine/scene/SubMeshEntity.h"

#include "common/array.h"
#include "hpl1/engine/scene/AnimationState.h"
#include "common/stablemap.h"

namespace hpl {

class cMaterialManager;
class cMeshManager;
class cAnimationManager;
class cMesh;
class cSubMesh;
class cMeshEntity;
class cAnimation;
class cAnimationState;
class cNodeState;
class cBone;
class cNode3D;
class iCollideShape;
class iPhysicsBody;
class iPhysicsWorld;
class cWorld3D;

//-----------------------------------------------------------------------

class cMeshEntityRootNodeUpdate : public iEntityCallback {
public:
	void OnTransformUpdate(iEntity3D *apEntity);
};

//------------------------------------------

class cMeshEntityCallback {
public:
	virtual ~cMeshEntityCallback() = default;
	virtual void AfterAnimationUpdate(cMeshEntity *apMeshEntity, float afTimeStep) = 0;
};

//------------------------------------------

kSaveData_ChildClass(iRenderable, cMeshEntity) {
	kSaveData_ClassInit(cMeshEntity) public : tString msMeshName;

	bool mbCastShadows;

	int mlBodyId;

	cContainerVec<cSaveData_cSubMeshEntity> mvSubEntities;

	cContainerVec<cSaveData_cAnimationState> mvAnimStates;

	virtual iSaveObject *CreateSaveObject(cSaveObjectHandler * apSaveObjectHandler, cGame * apGame);
	virtual int GetSaveCreatePrio();
};

//------------------------------------------

class cMeshEntity : public iRenderable {
	typedef iRenderable super;
	friend class cSubMeshEntity;
	friend class cMeshEntityRootNodeUpdate;
	friend class cMesh;

public:
	cMeshEntity(const tString asName, cMesh *apMesh, cMaterialManager *apMaterialManager,
				cMeshManager *apMeshManager, cAnimationManager *apAnimationManager);
	~cMeshEntity();

	void SetWorld(cWorld3D *apWorld) { mpWorld = apWorld; }
	cWorld3D *GetWorld() { return mpWorld; }

	void SetCastsShadows(bool abX);

	cNode3D *GetRootNode() { return mpRootNode; }

	void SetCallback(cMeshEntityCallback *apCallback) { mpCallback = apCallback; }

	// Sub mesh entities
	cSubMeshEntity *GetSubMeshEntity(unsigned int alIdx);
	cSubMeshEntity *GetSubMeshEntityName(const tString &asName);
	int GetSubMeshEntityNum();

	// Animation states
	cAnimationState *AddAnimation(cAnimation *apAnimation, const tString &asName, float afBaseSpeed);
	void ClearAnimations();

	cAnimationState *GetAnimationState(int alIndex);
	int GetAnimationStateIndex(const tString &asName);
	cAnimationState *GetAnimationStateFromName(const tString &asName);
	int GetAnimationStateNum();

	// Animation controller
	void Play(int alIndex, bool abLoop, bool bStopPrev);
	void PlayName(const tString &asName, bool abLoop, bool bStopPrev);
	void Stop();

	// Bone states
	cBoneState *GetBoneState(int alIndex);
	int GetBoneStateIndex(const tString &asName);
	cBoneState *GetBoneStateFromName(const tString &asName);
	int GetBoneStateNum();

	void SetSkeletonPhysicsActive(bool abX);
	bool GetSkeletonPhysicsActive();

	void SetSkeletonPhysicsCanSleep(bool abX) { mbSkeletonPhysicsCanSleep = abX; }
	bool GetSkeletonPhysicsCanSleep() { return mbSkeletonPhysicsCanSleep; }

	float GetSkeletonPhysicsWeight();
	void SetSkeletonPhysicsWeight(float afX);

	void FadeSkeletonPhysicsWeight(float afTime);

	void SetSkeletonCollidersActive(bool abX);
	bool GetSkeletonCollidersActive();

	void AlignBodiesToSkeleton(bool abCalculateSpeed);

	cMesh *GetMesh() { return mpMesh; }

	/**
	 * Calculates the transform (and angles and postion if wanted) of a mesh based on the postion of the root bone.
	 * This is useful when going from rag doll to mesh.
	 * \param *apPostion Can be NULL, the postion
	 * \param *apAngles Can be NULL, the angles.
	 */
	cMatrixf CalculateTransformFromSkeleton(cVector3f *apPostion, cVector3f *apAngles);

	/**
	 * Checks collision with the skeletons collider boides
	 * \param *apWorld Physics world
	 * \param *apShape The shape
	 * \param &a_mtxShape The shapes matrix
	 * \param *apPosList A list of positions that all contact points are stored in. can be NULL.
	 * \param *apNumList A list of ints of number of the bone state body hit. can be NULL.
	 */
	bool CheckColliderShapeCollision(iPhysicsWorld *apWorld,
									 iCollideShape *apShape, const cMatrixf &a_mtxShape,
									 tVector3fList *apPosList, tIntList *apNumList);

	void ResetGraphicsUpdated();

	// Node states
	cNode3D *GetNodeState(int alIndex);
	int GetNodeStateIndex(const tString &asName);
	cNode3D *GetNodeStateFromName(const tString &asName);
	int GetNodeStateNum();
	bool HasNodes() { return mbHasNodes; }

	bool AttachEntityToParent(iEntity3D *apEntity, const tString &asParent);

	// Entity implementation
	tString GetEntityType() { return "Mesh"; }
	bool IsVisible() { return IsRendered(); }
	void SetVisible(bool abVisible) { SetRendered(abVisible); }

	void UpdateLogic(float afTimeStep);

	// Renderable implementation.
	void UpdateGraphics(cCamera3D *apCamera, float afFrameTime, cRenderList *apRenderList);
	void SetRendered(bool abX);
	iMaterial *GetMaterial();
	iVertexBuffer *GetVertexBuffer();
	bool IsShadowCaster();
	cBoundingVolume *GetBoundingVolume();
	cMatrixf *GetModelMatrix(cCamera3D *apCamera);
	int GetMatrixUpdateCount();
	eRenderableType GetRenderType();

	void SetBody(iPhysicsBody *apBody) { mpBody = apBody; }
	iPhysicsBody *GetBody() { return mpBody; }

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

private:
	void UpdateNodeMatrixRec(const cMatrixf &a_mtxParentWorld, cNode3D *apNode);

	void HandleAnimationEvent(cAnimationEvent *apEvent);

	void SetBoneMatrixFromBodyRec(const cMatrixf &a_mtxParentWorld, cBoneState *apBoneState);

	void UpdateBVFromSubs();

	void BuildBoneStatesRec(cBone *apBone, cNode3D *apParent);

	cMaterialManager *mpMaterialManager;
	cMeshManager *mpMeshManager;
	cAnimationManager *mpAnimationManager;

	cWorld3D *mpWorld;

	tSubMeshEntityVec mvSubMeshes;
	tSubMeshEntityMap m_mapSubMeshes;

	tAnimationStateVec mvAnimationStates;
	tAnimationStateIndexMap m_mapAnimationStateIndices;

	tNodeStateVec mvBoneStates;
	tNodeStateIndexMap m_mapBoneStateIndices;
	tNodeStateVec mvTempBoneStates;

	Common::Array<cMatrixf> mvBoneMatrices;

	bool mbSkeletonPhysics;
	bool mbSkeletonPhysicsFading;
	float mfSkeletonPhysicsFadeSpeed;
	float mfSkeletonPhysicsWeight;

	bool mbSkeletonPhysicsSleeping;
	bool mbSkeletonPhysicsCanSleep;

	bool mbSkeletonColliders;

	bool mbUpdatedBones;

	bool mbHasNodes;
	tNodeStateVec mvNodeStates;
	tNodeStateIndexMap m_mapNodeStateIndices;

	int mlStartSleepCount;
	int mlUpdateCount;
	float mfTimeStepAccum;

	cMesh *mpMesh;

	cNode3D *mpRootNode;
	cMeshEntityRootNodeUpdate *mpRootCallback;

	cMeshEntityCallback *mpCallback;

	tEntity3DList mlstAttachedEntities;

	cMatrixf mtxTemp;

	iPhysicsBody *mpBody;

	// Properies:
	bool mbCastShadows;
};

//-----------------------------------------------------------------------

} // namespace hpl

#endif // HPL_MESH_ENTITY_H
