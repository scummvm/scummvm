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

#include "hpl1/engine/scene/MeshEntity.h"

#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/Mesh.h"
#include "hpl1/engine/graphics/RenderList.h"
#include "hpl1/engine/graphics/SubMesh.h"
#include "hpl1/engine/graphics/VertexBuffer.h"
#include "hpl1/engine/resources/AnimationManager.h"
#include "hpl1/engine/resources/MaterialManager.h"
#include "hpl1/engine/resources/MeshManager.h"
#include "hpl1/engine/resources/Resources.h"

#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/MeshLoaderHandler.h"

#include "hpl1/engine/graphics/Animation.h"
#include "hpl1/engine/graphics/AnimationTrack.h"
#include "hpl1/engine/graphics/Bone.h"
#include "hpl1/engine/graphics/BoneState.h"
#include "hpl1/engine/graphics/Skeleton.h"

#include "hpl1/engine/scene/AnimationState.h"
#include "hpl1/engine/scene/NodeState.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/SoundEntity.h"
#include "hpl1/engine/scene/World3D.h"

#include "hpl1/engine/physics/PhysicsBody.h"
#include "hpl1/engine/physics/PhysicsWorld.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/game/Game.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMeshEntity::cMeshEntity(const tString asName, cMesh *apMesh, cMaterialManager *apMaterialManager,
						 cMeshManager *apMeshManager, cAnimationManager *apAnimationManager) : iRenderable(asName) {
	mpMaterialManager = apMaterialManager;
	mpMeshManager = apMeshManager;
	mpAnimationManager = apAnimationManager;

	mpWorld = NULL;

	mpCallback = NULL;

	mbCastShadows = false;

	mpMesh = apMesh;

	mpRootNode = NULL;
	mpRootCallback = NULL;

	mpBody = NULL;

	mbSkeletonPhysics = false;
	mfSkeletonPhysicsWeight = 1.0f;
	mbSkeletonPhysicsFading = false;
	mfSkeletonPhysicsFadeSpeed = 1.0f;

	mbSkeletonPhysicsSleeping = false;
	mbSkeletonPhysicsCanSleep = true;

	mbSkeletonColliders = false;

	mbUpdatedBones = false;

	mlStartSleepCount = 0;
	mlUpdateCount = 0;
	mfTimeStepAccum = 0;

	////////////////////////////////////////////////
	// Create sub entities
	for (int i = 0; i < mpMesh->GetSubMeshNum(); i++) {
		cSubMesh *pSubMesh = mpMesh->GetSubMesh(i);
		cSubMeshEntity *pSub = hplNew(cSubMeshEntity, (pSubMesh->GetName(), this, pSubMesh, mpMaterialManager));

		// Log("Creating sub entity %s\n",pSub->GetName().c_str());

		mvSubMeshes.push_back(pSub);
		m_mapSubMeshes.insert(tSubMeshEntityMap::value_type(mpMesh->GetSubMesh(i)->GetName(), pSub));

		iVertexBuffer *pVtxBuffer = mpMesh->GetSubMesh(i)->GetVertexBuffer();

		if (mpMesh->GetNodeNum() <= 0) {
			mBoundingVolume.AddArrayPoints(pVtxBuffer->GetArray(eVertexFlag_Position),
										   pVtxBuffer->GetVertexNum());
		} else {
			pSub->mBoundingVolume.AddArrayPoints(pVtxBuffer->GetArray(eVertexFlag_Position),
												 pVtxBuffer->GetVertexNum());
			pSub->mBoundingVolume.CreateFromPoints(kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)]);
		}
	}

	if (mpMesh->GetNodeNum() <= 0) {
		mBoundingVolume.CreateFromPoints(kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)]);

		// Log("CREATED BV Min: %s Max: %s\n",	mBoundingVolume.GetMin().ToString().c_str(),
		//									mBoundingVolume.GetMax().ToString().c_str());
	}

	////////////////////////////////////////////////
	// Create animation states
	mvAnimationStates.reserve(mpMesh->GetAnimationNum());
	for (int i = 0; i < mpMesh->GetAnimationNum(); i++) {
		cAnimation *pAnimation = mpMesh->GetAnimation(i);

		cAnimationState *pAnimState = hplNew(cAnimationState, (pAnimation, pAnimation->GetName(), NULL));

		mvAnimationStates.push_back(pAnimState);

		tAnimationStateIndexMap::value_type value(pAnimState->GetName(), (int)mvAnimationStates.size() - 1);
		m_mapAnimationStateIndices.insert(value);
	}

	////////////////////////////////////////////////
	// Create Nodes
	mbHasNodes = false;
	if (mpMesh->GetNodeNum() > 0 &&
		(mpMesh->GetPhysicsJointNum() <= 0 || mpMesh->GetSkeleton() == NULL) &&
		mpMesh->GetAnimationNum() > 0) {
		mbHasNodes = true;
		mvNodeStates.reserve(mpMesh->GetNodeNum());

		// Create the root node and attach all node without parents to this.
		mpRootNode = hplNew(cNode3D, ("NodeRoot", false));

		// Create the root callback
		mpRootCallback = hplNew(cMeshEntityRootNodeUpdate, ());
		this->AddCallback(mpRootCallback);

		// Fill the node array.
		for (int i = 0; i < mpMesh->GetNodeNum(); i++) {
			cNode3D *pMeshNode = mpMesh->GetNode(i);

			cBoneState *pNode = hplNew(cBoneState, (pMeshNode->GetName(), false));
			pNode->SetMatrix(pMeshNode->GetLocalMatrix());

			/*Log("Node: %s has local translate: %s world translate: %s\n",
					pNode->GetName(),
					pMeshNode->GetLocalMatrix().GetTranslation().ToString().c_str(),
					pMeshNode->GetWorldMatrix().GetTranslation().ToString().c_str());*/

			// Add node to array and add it's index to the map.
			mvNodeStates.push_back(pNode);
			m_mapNodeStateIndices.insert(tNodeStateIndexMap::value_type(pNode->GetName(), i));

			// Connect with sub mesh entity
			cSubMeshEntity *pSubEntity = GetSubMeshEntityName(pMeshNode->GetSource());
			if (pSubEntity) {
				pSubEntity->SetLocalNode(pNode);
			}
		}

		// Set parents and children of the nodes in the array
		for (int i = 0; i < (int)mvNodeStates.size(); i++) {
			cNode3D *pStateNode = mvNodeStates[i];
			cNode3D *pMeshNode = mpMesh->GetNode(i);

			// Set the parent if there is one
			if (pMeshNode->GetParent()) {
				cNode3D *pParentNode = GetNodeStateFromName(pMeshNode->GetParent()->GetName());
				if (pParentNode)
					pStateNode->SetParent(pParentNode);
				else
					pStateNode->SetParent(mpRootNode);
			}
			// If not set root node as parent.
			else {
				pStateNode->SetParent(mpRootNode);
			}

			// Add children if there are any.
			cNodeIterator it = pMeshNode->GetChildIterator();
			while (it.HasNext()) {
				cNode3D *pChildNode = static_cast<cNode3D *>(it.Next());
				pStateNode->AddChild(GetNodeStateFromName(pChildNode->GetName()));
			}
		}

		// make sure all nodes are updated.
		mpRootNode->SetMatrix(cMatrixf::Identity);

		UpdateBVFromSubs();
	}

	////////////////////////////////////////////////
	// Create Joint Nodes
	// These are created to support joints.
	else if (mpMesh->GetPhysicsJointNum() > 0 || mpMesh->HasSeveralBodies()) {
		mbHasNodes = true;

		// Fill the node array.
		for (int i = 0; i < mpMesh->GetNodeNum(); i++) {
			cNode3D *pMeshNode = mpMesh->GetNode(i);

			// Log("MeshNode %s\n local: (%s)\n world: (%s)\n",pMeshNode->GetName(),
			//									pMeshNode->GetLocalPosition().ToString().c_str(),
			//									pMeshNode->GetWorldPosition().ToString().c_str());

			// Set the sub entity with the mesh node world matrix
			cSubMeshEntity *pSubEntity = GetSubMeshEntityName(pMeshNode->GetSource());
			if (pSubEntity) {
				/*cSubMesh *pSubMesh = */ pSubEntity->GetSubMesh();
				cMatrixf mtxSub = pMeshNode->GetWorldMatrix();

				pSubEntity->SetMatrix(mtxSub);

				// Log(" SubEnity: %s\n",pSubEntity->GetWorldPosition().ToString().c_str());
			}
		}

		UpdateBVFromSubs();
	}

	////////////////////////////////////////////////
	// Create Skeleton

	// Create the node states from skeleton
	cSkeleton *pSkeleton = mpMesh->GetSkeleton();

	// Create bones states if there is a skeleton.
	if (pSkeleton) {
		mbApplyTransformToBV = false;
		mbHasNodes = false;

		if (mpRootNode == NULL) {
			// Create the root node and attach all node without parents to this.
			mpRootNode = hplNew(cNode3D, ("NodeRoot", false));

			// Create the root callback
			mpRootCallback = hplNew(cMeshEntityRootNodeUpdate, ());
			this->AddCallback(mpRootCallback);
		}

		mvBoneStates.reserve(pSkeleton->GetBoneNum());

		// Fill the state array with the bones so
		// that each state has the same index as the bones.
		for (int i = 0; i < pSkeleton->GetBoneNum(); i++) {
			cBone *pBone = pSkeleton->GetBoneByIndex(i);

			cBoneState *pState = hplNew(cBoneState, (pBone->GetName(), false));
			pState->SetMatrix(pBone->GetLocalTransform());

			// Log("Created bone state: '%s'\n",pState->GetName());

			// Add bone to array and add it's index to the map.
			mvBoneStates.push_back(pState);
			m_mapBoneStateIndices.insert(tBoneIdxNameMap::value_type(pState->GetName(), i));
		}

		// Set parents and children of the nodes in the array
		for (int i = 0; i < (int)mvBoneStates.size(); i++) {
			cNode3D *pState = mvBoneStates[i];
			cBone *pBone = pSkeleton->GetBoneByIndex(i);

			// Log("State: %s\n",pState->GetName());

			// Set the parent if there is one
			// TODO: Perhaps this should be removed.
			if (pBone->GetParent()) {
				cNode3D *pParentState = GetBoneStateFromName(pBone->GetParent()->GetName());
				if (pParentState)
					pState->SetParent(pParentState);
				else
					pState->SetParent(mpRootNode);
				// else
				//	Error("Couldn't find parent bone state '%s' in bone\n",pBone->GetParent()->GetName().c_str(),
				//														pBone->GetName().c_str());
				// if(pParentState)Log("  Parent: %s\n",pParentState->GetName());
			} else {
				pState->SetParent(mpRootNode);
			}

			// Add children if there are any.
			/*cBoneIterator it = pBone->GetChildIterator();
			while(it.HasNext())
			{
				cBone* pChildBone = it.Next();
				cNode3D *pChildState = GetBoneStateFromName(pChildBone->GetName());
				if(pChildState)
					pState->AddChild(pChildState);
				//else
				//	Error("Couldn't find child bone state '%s'",pChildBone->GetName());
				//if(pChildState)Log("  Child: %s\n",pChildState->GetName());
			}*/
		}

		// Create an array to fill with bone matrices
		mvBoneMatrices.resize(pSkeleton->GetBoneNum());

		// Reset all bones states
		for (size_t i = 0; i < mvBoneStates.size(); i++) {
			cNode3D *pState = mvBoneStates[i];
			cBone *pBone = mpMesh->GetSkeleton()->GetBoneByIndex((int)i);

			pState->SetMatrix(pBone->GetLocalTransform());
		}

		// Create temp bone nodes
		mvTempBoneStates.resize(mvBoneStates.size());
		for (size_t i = 0; i < mvTempBoneStates.size(); i++) {
			mvTempBoneStates[i] = hplNew(cBoneState, (mvBoneStates[i]->GetName(), false));
		}
	}

	////////////////////////////////////////////////////
	// If it has no nodes, attach the sub entities to the
	// main mesh
	if (mbHasNodes == false) {
		for (size_t i = 0; i < mvSubMeshes.size(); ++i) {
			AddChild(mvSubMeshes[i]);
		}
	}
}

//-----------------------------------------------------------------------

cMeshEntity::~cMeshEntity() {
	for (tEntity3DListIt it = mlstAttachedEntities.begin(); it != mlstAttachedEntities.end(); ++it) {
		// iEntity3D *pEntity = *it;
		//  TODO: if(mpWorld) mpWorld->DestroyUnkownEntity(pEntity);
	}

	for (int i = 0; i < (int)mvSubMeshes.size(); i++) {
		hplDelete(mvSubMeshes[i]);
	}

	if (mpRootNode)
		hplDelete(mpRootNode);
	if (mpRootCallback)
		hplDelete(mpRootCallback);

	mpMeshManager->Destroy(mpMesh);

	STLDeleteAll(mvNodeStates);
	STLDeleteAll(mvBoneStates);

	STLDeleteAll(mvTempBoneStates);

	STLDeleteAll(mvAnimationStates);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// ROOT NODE UDPATE CALLBACK
//////////////////////////////////////////////////////////////////////////

void cMeshEntityRootNodeUpdate::OnTransformUpdate(iEntity3D *apEntity) {
	cMeshEntity *pMeshEntity = static_cast<cMeshEntity *>(apEntity);

	pMeshEntity->mpRootNode->SetMatrix(apEntity->GetWorldMatrix());
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

// Ths functions sets the matrices of the bones according the matrices of
// the body that is attached to each bone.
// To get max speed all this is doen recursivly from the root and then down.
// To usre function iterate notes of root bone and call this function for each.
void cMeshEntity::SetBoneMatrixFromBodyRec(const cMatrixf &a_mtxParentWorld, cBoneState *apBoneState) {
	iPhysicsBody *pBody = apBoneState->GetBody();

	if (pBody) {
		cMatrixf mtxBoneWorld = cMath::MatrixMul(pBody->GetWorldMatrix(), apBoneState->GetInvBodyMatrix());
		cMatrixf mtxParentInv = cMath::MatrixInverse(a_mtxParentWorld);

		apBoneState->SetMatrix(cMath::MatrixMul(mtxParentInv, mtxBoneWorld), false);

		cNodeIterator BoneIt = apBoneState->GetChildIterator();
		while (BoneIt.HasNext()) {
			cBoneState *pBoneState = static_cast<cBoneState *>(BoneIt.Next());
			SetBoneMatrixFromBodyRec(mtxBoneWorld, pBoneState);
		}
	} else {
		apBoneState->UpdateWorldTransform();
		const cMatrixf &mtxBoneWorld = apBoneState->GetWorldMatrix();

		cNodeIterator BoneIt = apBoneState->GetChildIterator();
		while (BoneIt.HasNext()) {
			cBoneState *pBoneState = static_cast<cBoneState *>(BoneIt.Next());

			SetBoneMatrixFromBodyRec(mtxBoneWorld, pBoneState);
		}
	}
}

//-----------------------------------------------------------------------

// int glDebugTabs=0;
void cMeshEntity::UpdateNodeMatrixRec(const cMatrixf &a_mtxParentWorld, cNode3D *apNode) {
	if (apNode->IsActive()) {
		// tString sTemp="";for(int i=0; i<glDebugTabs;++i)sTemp+=" ";
		// LogUpdate("   %sUpdate %s\n",sTemp.c_str(),apNode->GetName());
		apNode->UpdateMatrix(false);
	}

	apNode->UpdateWorldTransform();
	const cMatrixf &mtxWorld = apNode->GetWorldMatrix();

	// glDebugTabs++;

	cNodeIterator NodeIt = apNode->GetChildIterator();
	while (NodeIt.HasNext()) {
		cNode3D *pChildNode = static_cast<cNode3D *>(NodeIt.Next());

		UpdateNodeMatrixRec(mtxWorld, pChildNode);
	}

	// glDebugTabs--;
}

//-----------------------------------------------------------------------

void cMeshEntity::UpdateLogic(float afTimeStep) {
	// LogUpdate("---- Start mesh update ----------------------\n");

	// if(!IsStatic()){START_TIMING_TAB(Skeleton);}

	/////////////////////////////////////////////////////
	// If not rendered the previous frame then we do not update as often.
	bool bRegularUpdate = GetGlobalRenderCount() == cRenderList::GetGlobalRenderCount();

	// Check if any skeleton  child has been rendered.
	if (mpMesh->GetSkeleton() && bRegularUpdate == false) {
		for (size_t i = 0; i < mvBoneStates.size(); ++i) {
			cBoneState *pState = mvBoneStates[i];

			cEntityIterator it = pState->GetEntityIterator();
			while (it.HasNext()) {
				iEntity3D *pEntity = static_cast<iEntity3D *>(it.Next());
				if (pEntity->GetGlobalRenderCount() == cRenderList::GetGlobalRenderCount()) {
					bRegularUpdate = true;
					break;
				}
			}
			if (bRegularUpdate)
				break;
		}
	}

	const int lMaxSleepCount = 30;
	if (bRegularUpdate == false) {
		if (mlStartSleepCount < lMaxSleepCount)
			++mlStartSleepCount;
	} else {
		mlStartSleepCount = 0;
	}

	if (mlStartSleepCount >= lMaxSleepCount) {
		++mlUpdateCount;
		mfTimeStepAccum += afTimeStep;

		if (mlUpdateCount < 20) {
			// if(cString::GetLastStringPos(msName, "infected")>=0) LogUpdate("   return\n");
			return;
		}

		mlUpdateCount = 0;
		afTimeStep = mfTimeStepAccum;
		mfTimeStepAccum = 0;
	} else {
		mlUpdateCount = 0;
		mfTimeStepAccum = 0;
	}
	// if(cString::GetLastStringPos(msName, "infected")>=0) LogUpdate("   do stuff\n");

	/////////////////////////////////////////////
	// Update the skeleton physics fade
	if (mbSkeletonPhysicsFading && mbSkeletonPhysics) {
		mfSkeletonPhysicsWeight -= afTimeStep * mfSkeletonPhysicsFadeSpeed;
		if (mfSkeletonPhysicsWeight <= 0) {
			mbSkeletonPhysicsFading = false;
			mfSkeletonPhysicsWeight = 1.0f;
			SetSkeletonPhysicsActive(false);
		}
	}

	// LogUpdate(" -skeleton physics\n");
	/////////////////////////////////////////////
	// Check if all bodies connected to the skeleton is at rest,
	// If so we can skip skinning the body and simply just use the mesh as is.
	//(has some problems so turned off at the moment)
	mbSkeletonPhysicsSleeping = false;
	if (mbSkeletonPhysics && mfSkeletonPhysicsWeight == 1.0f && mbSkeletonPhysicsCanSleep) {
		bool bEnabled = false;
		for (int bone = 0; bone < GetBoneStateNum(); ++bone) {
			cBoneState *pState = GetBoneState(bone);
			iPhysicsBody *pBody = pState->GetBody();

			if (pBody && pBody->GetEnabled()) {
				bEnabled = true;
				break;
			}
		}
		if (bEnabled == false) {
			// mbSkeletonPhysicsSleeping = true;
		}
	}

	// if(!IsStatic()){STOP_TIMING_TAB(Skeleton);}

	/////////////////////////////////////////////
	// Update animations

	//////////////////////////////////////
	// Check if it has nodes, then use special animation update,
	// else it as skepetn which needs another update.
	if (mbHasNodes) {
		// Reset all node states to prepare for animations.
		if (mvAnimationStates.size() > 0) {
			for (size_t i = 0; i < mvNodeStates.size(); i++) {
				cNode3D *pState = mvNodeStates[i];
				if (pState->IsActive())
					pState->SetMatrix(cMatrixf::Identity);
			}
		}

		/////////////////////////
		// Go through all animations states and set the node's
		// temporary states
		bool bAnimated = false;
		for (size_t i = 0; i < mvAnimationStates.size(); i++) {
			cAnimationState *pAnimState = mvAnimationStates[i];
			if (pAnimState->IsActive()) {
				bAnimated = true;

				cAnimation *pAnim = pAnimState->GetAnimation();

				for (int j = 0; j < pAnim->GetTrackNum(); j++) {
					cAnimationTrack *pTrack = pAnim->GetTrack(j);

					if (pTrack->GetNodeIndex() < 0) {
						pTrack->SetNodeIndex(GetNodeStateIndex(pTrack->GetName()));
					}
					cNode3D *pNodeState = GetNodeState(pTrack->GetNodeIndex());

					if (pNodeState->IsActive())
						pTrack->ApplyToNode(pNodeState, pAnimState->GetTimePosition(), pAnimState->GetWeight());
				}

				pAnimState->Update(afTimeStep);
			}
		}

		//////////////////////
		// Go through all states and update the matrices (and thereby adding the animations together).
		if (mvAnimationStates.size() > 0 && bAnimated) {
			cNodeIterator NodeIt = mpRootNode->GetChildIterator();
			while (NodeIt.HasNext()) {
				cNode3D *pBoneState = static_cast<cNode3D *>(NodeIt.Next());

				UpdateNodeMatrixRec(mpRootNode->GetWorldMatrix(), pBoneState);
			}
		}

		//////////////////////
		// Call callback to be run after animation
		if (mpCallback)
			mpCallback->AfterAnimationUpdate(this, afTimeStep);

		UpdateBVFromSubs();

		// Update the entity transform, this so that the portal info, attached entities, callbacks and stuff is updated.
		SetTransformUpdated(true);
	}
	//////////////////////////////////////
	// If the entity has a skeleton:
	else if (mpMesh->GetSkeleton()) {
		// if(!IsStatic()){START_TIMING_TAB(Animation);}

		////////////////////////
		// Check if it is animated
		bool bAnimationActive = false;
		for (size_t i = 0; i < mvAnimationStates.size(); i++) {
			if (mvAnimationStates[i]->IsActive()) {
				bAnimationActive = true;
				break;
			}
		}

		//////////
		// Reset all bones states
		// LogUpdate(" -reset bones\n");
		if (bAnimationActive || mbUpdatedBones == false || (mbSkeletonPhysics && !mbSkeletonPhysicsSleeping)) {
			for (size_t i = 0; i < mvBoneStates.size(); i++) {
				cNode3D *pState = mvBoneStates[i];
				cBone *pBone = mpMesh->GetSkeleton()->GetBoneByIndex((int)i);

				if (pState->IsActive()) {
					pState->SetMatrix(pBone->GetLocalTransform(), false);
				}

				// can optimize this by doing it in the order of the tree
				// and using recursive. (should be enough as is...)
				if (mbSkeletonPhysics && mfSkeletonPhysicsWeight != 1.0f) {
					mvTempBoneStates[i]->SetMatrix(pBone->GetLocalTransform(), false);
				}
			}
		}

		///////////////////////////
		// Update skeleton physics
		// LogUpdate(" -reset physics again\n");
		if (mbSkeletonPhysics && (!mbSkeletonPhysicsSleeping || mbUpdatedBones == false)) {
			mbUpdatedBones = true;
			cNodeIterator BoneIt = mpRootNode->GetChildIterator();
			while (BoneIt.HasNext()) {
				cBoneState *pBoneState = static_cast<cBoneState *>(BoneIt.Next());

				SetBoneMatrixFromBodyRec(mpRootNode->GetWorldMatrix(), pBoneState);
			}

			// Interpolate matrices
			if (mfSkeletonPhysicsWeight != 1.0f) {
				for (size_t i = 0; i < mvBoneStates.size(); i++) {
					cMatrixf mtxMixLocal = cMath::MatrixSlerp(mfSkeletonPhysicsWeight,
															  mvTempBoneStates[i]->GetLocalMatrix(),
															  mvBoneStates[i]->GetLocalMatrix(),
															  true);

					mvBoneStates[i]->SetMatrix(mtxMixLocal, false);
				}
			}
		}

		// if(!IsStatic()){STOP_TIMING_TAB(Animation);}

		// if(!IsStatic()){START_TIMING_TAB(Bones);}
		//////////////////////////////////
		// Go through all animations states and update the bones
		// LogUpdate(" -animation states\n");
		for (size_t i = 0; i < mvAnimationStates.size(); i++) {
			cAnimationState *pAnimState = mvAnimationStates[i];

			// Log("Testing state: '%s'\n",pAnimState->GetName());

			if (pAnimState->IsActive()) {
				cAnimation *pAnim = pAnimState->GetAnimation();

				for (int i2 = 0; i2 < pAnim->GetTrackNum(); i2++) {
					cAnimationTrack *pTrack = pAnim->GetTrack(i2);

					cNode3D *pState = GetBoneState(pTrack->GetNodeIndex());

					// Log("Animating bone %s\n",pState->GetName());

					if (pState->IsActive())
						pTrack->ApplyToNode(pState, pAnimState->GetTimePosition(), pAnimState->GetWeight());
				}

				// Log("Time: %f\n",pAnimState->GetTimePosition());

				pAnimState->Update(afTimeStep);
			}
		}
		// if(!IsStatic()){STOP_TIMING_TAB(Bones);}

		// if(!IsStatic()){START_TIMING_TAB(States);}
		//////////////////////////////////
		// Go through all states and update the matrices (and thereby adding the animations together).
		// LogUpdate(" -Bone update\n");
		if (bAnimationActive) {
			cNodeIterator NodeIt = mpRootNode->GetChildIterator();
			while (NodeIt.HasNext()) {
				cNode3D *pBoneState = static_cast<cNode3D *>(NodeIt.Next());

				UpdateNodeMatrixRec(mpRootNode->GetWorldMatrix(), pBoneState);
			}

			// Entities are updated after BV is calculated, as the entity has the rootnode attached to it.
		}
		// if(!IsStatic()){STOP_TIMING_TAB(States);}

		// if(!IsStatic()){START_TIMING_TAB(Colliders);}
		//////////////////////////////////
		// Update the colliders if they are active
		// Note this must be done after all bone states are updated.
		// LogUpdate(" -Colliders\n");
		if (mbSkeletonColliders && mbSkeletonPhysics == false) {
			for (size_t i = 0; i < mvBoneStates.size(); i++) {
				cBoneState *pState = mvBoneStates[i];
				iPhysicsBody *pColliderBody = pState->GetColliderBody();

				if (pColliderBody) {
					cMatrixf mtxBody = cMath::MatrixMul(pState->GetWorldMatrix(), pState->GetBodyMatrix());
					pColliderBody->SetMatrix(mtxBody);
				}
			}
		}
		// if(!IsStatic()){STOP_TIMING_TAB(Colliders);}

		// Call callback
		if (mpCallback)
			mpCallback->AfterAnimationUpdate(this, afTimeStep);

		///////////////////////////
		// Update bounding volume
		// LogUpdate(" -BV update\n");
		UpdateBVFromSubs();

		// Update the entity transform, this so that the portal info, attached entities, callbacks and stuff is updated.
		SetTransformUpdated(true);
	}

	// if(!IsStatic()){START_TIMING_TAB(SubEntities);}
	/////////////////////////////////////////
	/// Update sub entities
	// LogUpdate(" -Subs\n");
	for (size_t i = 0; i < mvSubMeshes.size(); ++i) {
		cSubMeshEntity *pSub = mvSubMeshes[i];

		pSub->UpdateLogic(afTimeStep);
	}
	// if(!IsStatic()){STOP_TIMING_TAB(SubEntities);}

	/////////////////////////////////////////
	/// Update animation events
	for (size_t i = 0; i < mvAnimationStates.size(); ++i) {
		cAnimationState *pState = mvAnimationStates[i];

		if (pState->IsActive() == false || pState->IsPaused())
			continue;

		for (int j = 0; j < pState->GetEventNum(); ++j) {
			cAnimationEvent *pEvent = pState->GetEvent(j);

			if (pEvent->mfTime >= pState->GetPreviousTimePosition() &&
				pEvent->mfTime < pState->GetTimePosition()) {
				HandleAnimationEvent(pEvent);
			}
		}
	}

	// LogUpdate("---- End mesh update ----------------------\n");
}

//-----------------------------------------------------------------------

cAnimationState *cMeshEntity::AddAnimation(cAnimation *apAnimation, const tString &asName, float afBaseSpeed) {
	cAnimationState *pAnimState = hplNew(cAnimationState, (apAnimation, asName, mpAnimationManager));

	pAnimState->SetBaseSpeed(afBaseSpeed);

	mvAnimationStates.push_back(pAnimState);

	tAnimationStateIndexMap::value_type value(pAnimState->GetName(), (int)mvAnimationStates.size() - 1);
	m_mapAnimationStateIndices.insert(value);

	return pAnimState;
}

void cMeshEntity::ClearAnimations() {
	STLDeleteAll(mvAnimationStates);
}

//-----------------------------------------------------------------------

cAnimationState *cMeshEntity::GetAnimationState(int alIndex) {
	return mvAnimationStates[alIndex];
}
int cMeshEntity::GetAnimationStateIndex(const tString &asName) {
	tAnimationStateIndexMapIt it = m_mapAnimationStateIndices.find(asName);
	if (it != m_mapAnimationStateIndices.end()) {
		return it->second;
	} else {
		return -1;
	}
}
cAnimationState *cMeshEntity::GetAnimationStateFromName(const tString &asName) {
	int lIdx = GetAnimationStateIndex(asName);
	if (lIdx >= 0) {
		return mvAnimationStates[lIdx];
	} else {
		return NULL;
	}
}
int cMeshEntity::GetAnimationStateNum() {
	return (int)mvAnimationStates.size();
}

//-----------------------------------------------------------------------

void cMeshEntity::Play(int alIndex, bool abLoop, bool bStopPrev) {
	if (bStopPrev)
		Stop();

	mvAnimationStates[alIndex]->SetActive(true);
	mvAnimationStates[alIndex]->SetTimePosition(0);
	mvAnimationStates[alIndex]->SetLoop(abLoop);
	mvAnimationStates[alIndex]->SetWeight(1);
}

void cMeshEntity::PlayName(const tString &asName, bool abLoop, bool bStopPrev) {
	int lIdx = GetAnimationStateIndex(asName);
	if (lIdx >= 0) {
		Play(lIdx, abLoop, bStopPrev);
	} else {
		Warning("Can not find animation '%s' in meshentity '%s'\n", asName.c_str(),
				msName.c_str());
	}
}

void cMeshEntity::Stop() {
	for (size_t i = 0; i < mvAnimationStates.size(); i++) {
		mvAnimationStates[i]->SetActive(false);
		mvAnimationStates[i]->SetTimePosition(0);
	}
}

//-----------------------------------------------------------------------

cBoneState *cMeshEntity::GetBoneState(int alIndex) {
	return mvBoneStates[alIndex];
}

int cMeshEntity::GetBoneStateIndex(const tString &asName) {
	tNodeStateIndexMapIt it = m_mapBoneStateIndices.find(asName);
	if (it != m_mapBoneStateIndices.end()) {
		return it->second;
	} else {
		return -1;
	}
}

cBoneState *cMeshEntity::GetBoneStateFromName(const tString &asName) {
	int lIdx = GetBoneStateIndex(asName);
	if (lIdx >= 0) {
		return mvBoneStates[lIdx];
	} else {
		return NULL;
	}
}
int cMeshEntity::GetBoneStateNum() {
	return (int)mvBoneStates.size();
}

//----------------------------------------------------------------------

void cMeshEntity::SetSkeletonPhysicsActive(bool abX) {
	mbSkeletonPhysics = abX;

	mbUpdatedBones = false;
	ResetGraphicsUpdated();

	mbSkeletonPhysicsFading = false;
	mfSkeletonPhysicsWeight = 1.0f;

	for (int bone = 0; bone < GetBoneStateNum(); ++bone) {
		cBoneState *pState = GetBoneState(bone);
		iPhysicsBody *pBody = pState->GetBody();
		iPhysicsBody *pColliderBody = pState->GetColliderBody();

		if (pBody) {
			pBody->SetActive(abX);
			pBody->SetEnabled(abX);

			if (abX == false) {
				pBody->SetLinearVelocity(0);
				pBody->SetAngularVelocity(0);
			}

			if (mbSkeletonColliders) {
				pColliderBody->SetActive(!abX);
			}
		}
	}
}

bool cMeshEntity::GetSkeletonPhysicsActive() {
	return mbSkeletonPhysics;
}

//----------------------------------------------------------------------

void cMeshEntity::FadeSkeletonPhysicsWeight(float afTime) {
	if (mbSkeletonPhysics) {
		mbSkeletonPhysicsFading = true;
		mfSkeletonPhysicsFadeSpeed = 1.0f / afTime;

		for (int bone = 0; bone < GetBoneStateNum(); ++bone) {
			cBoneState *pState = GetBoneState(bone);
			iPhysicsBody *pBody = pState->GetBody();
			/*iPhysicsBody *pColliderBody = */ pState->GetColliderBody();

			if (pBody)
				pBody->SetActive(false);
		}
	}
}

//----------------------------------------------------------------------

float cMeshEntity::GetSkeletonPhysicsWeight() {
	return mfSkeletonPhysicsWeight;
}
void cMeshEntity::SetSkeletonPhysicsWeight(float afX) {
	mfSkeletonPhysicsWeight = afX;
}

//----------------------------------------------------------------------

void cMeshEntity::SetSkeletonCollidersActive(bool abX) {
	mbSkeletonColliders = abX;

	// Set active to the correct state.
	for (int bone = 0; bone < GetBoneStateNum(); ++bone) {
		cBoneState *pState = GetBoneState(bone);
		iPhysicsBody *pColliderBody = pState->GetColliderBody();

		if (pColliderBody) {
			if (abX && !mbSkeletonPhysics)
				pColliderBody->SetActive(true);
			else if (!abX)
				pColliderBody->SetActive(false);
		}
	}
}
bool cMeshEntity::GetSkeletonCollidersActive() {
	return mbSkeletonColliders;
}

//----------------------------------------------------------------------

void cMeshEntity::AlignBodiesToSkeleton(bool abCalculateSpeed) {
	for (int bone = 0; bone < GetBoneStateNum(); ++bone) {
		cBoneState *pState = GetBoneState(bone);
		iPhysicsBody *pBody = pState->GetBody();

		if (pBody) {
			cMatrixf mtxBody = cMath::MatrixMul(pState->GetWorldMatrix(), pState->GetBodyMatrix());
			pBody->SetMatrix(mtxBody);

			if (abCalculateSpeed) {
				// TODO: calculate speed based on the previous frame of the animation.
			}
		}
	}
}

//----------------------------------------------------------------------

cMatrixf cMeshEntity::CalculateTransformFromSkeleton(cVector3f *apPostion, cVector3f *apAngles) {
	// Root bone
	cNodeIterator StateIt = GetRootNode()->GetChildIterator();
	cBoneState *pBoneState = static_cast<cBoneState *>(StateIt.Next());

	// Get the root bone (should only be one)
	cBoneIterator BoneIt = GetMesh()->GetSkeleton()->GetRootBone()->GetChildIterator();
	cBone *pBone = BoneIt.Next();

	// Rotation and postion
	cMatrixf mtxInvBind = pBone->GetInvWorldTransform();
	cMatrixf mtxInvBone = cMath::MatrixInverse(pBoneState->GetWorldMatrix());
	cVector3f vStateForward = mtxInvBone.GetForward();
	cVector3f vBindForward = mtxInvBind.GetForward();

	float fBindYAngle = -cMath::GetAngleFromPoints2D(0, cVector2f(-vBindForward.x, -vBindForward.z));
	float fStateYAngle = -cMath::GetAngleFromPoints2D(0, cVector2f(-vStateForward.x, -vStateForward.z));
	float fYAngle = fStateYAngle - fBindYAngle;

	cMatrixf mtxTransform = cMath::MatrixRotateY(fYAngle);

	cVector3f vRootBoneOffset = pBone->GetLocalTransform().GetTranslation();
	vRootBoneOffset.y = 0;
	vRootBoneOffset = cMath::MatrixMul(mtxTransform, vRootBoneOffset);

	mtxTransform.SetTranslation(pBoneState->GetWorldPosition()); // - vRootBoneOffset);

	if (apPostion)
		*apPostion = pBoneState->GetWorldPosition(); // - vRootBoneOffset;
	if (apAngles)
		*apAngles = cVector3f(0, fYAngle, 0);

	return mtxTransform;
}

//----------------------------------------------------------------------

bool cMeshEntity::CheckColliderShapeCollision(iPhysicsWorld *apWorld,
											  iCollideShape *apShape, const cMatrixf &a_mtxShape,
											  tVector3fList *apPosList, tIntList *apNumList) {
	bool bCollision = false;
	cCollideData collideData;
	collideData.SetMaxSize(1);

	for (size_t i = 0; i < mvBoneStates.size(); ++i) {
		cBoneState *pState = mvBoneStates[i];
		iPhysicsBody *pBody = pState->GetColliderBody();
		if (pBody == NULL)
			continue;

		cMatrixf mtxBody = cMath::MatrixMul(pState->GetWorldMatrix(), pState->GetBodyMatrix());
		pBody->SetMatrix(mtxBody);

		bool bRet = apWorld->CheckShapeCollision(pBody->GetShape(),
												 pBody->GetLocalMatrix(),
												 apShape, a_mtxShape,
												 collideData, 1);

		if (bRet) {
			bCollision = true;
			if (!apPosList && !apNumList)
				break;

			if (apPosList)
				apPosList->push_back(collideData.mvContactPoints[0].mvPoint);
			if (apNumList)
				apNumList->push_back((int)i);
		}
	}

	return bCollision;
}

//----------------------------------------------------------------------

void cMeshEntity::ResetGraphicsUpdated() {
	for (size_t i = 0; i < mvSubMeshes.size(); ++i) {
		mvSubMeshes[i]->mbGraphicsUpdated = false;
	}
	mbUpdatedBones = false;
}

//----------------------------------------------------------------------

cNode3D *cMeshEntity::GetNodeState(int alIndex) {
	return mvNodeStates[alIndex];
}

int cMeshEntity::GetNodeStateIndex(const tString &asName) {
	tNodeStateIndexMapIt it = m_mapNodeStateIndices.find(asName);
	if (it != m_mapNodeStateIndices.end()) {
		return it->second;
	} else {
		return -1;
	}
}

cNode3D *cMeshEntity::GetNodeStateFromName(const tString &asName) {
	int lIdx = GetNodeStateIndex(asName);
	if (lIdx >= 0) {
		return mvNodeStates[lIdx];
	} else {
		return NULL;
	}
}
int cMeshEntity::GetNodeStateNum() {
	return (int)mvNodeStates.size();
}

//-----------------------------------------------------------------------

void cMeshEntity::SetCastsShadows(bool abX) {
	if (abX == mbCastShadows)
		return;

	mbCastShadows = abX;
	for (int i = 0; i < (int)mvSubMeshes.size(); i++) {
		mvSubMeshes[i]->SetCastsShadows(abX);
	}
}

//-----------------------------------------------------------------------

cSubMeshEntity *cMeshEntity::GetSubMeshEntity(unsigned int alIdx) {
	if (alIdx >= mvSubMeshes.size())
		return NULL;

	return mvSubMeshes[alIdx];
}

cSubMeshEntity *cMeshEntity::GetSubMeshEntityName(const tString &asName) {
	tSubMeshEntityMapIt it = m_mapSubMeshes.find(asName);
	if (it == m_mapSubMeshes.end())
		return NULL;

	return it->second;
}

int cMeshEntity::GetSubMeshEntityNum() {
	return (int)mvSubMeshes.size();
}

//-----------------------------------------------------------------------

void cMeshEntity::UpdateGraphics(cCamera3D *apCamera, float afFrameTime, cRenderList *apRenderList) {
	// Update the bone matrices
	cSkeleton *pSkeleton = mpMesh->GetSkeleton();
	if (pSkeleton) {
		// Save the root node to a temp matrix
		/*cMatrixf mtxTemp;
		if(mpRootNode)
		{
			mtxTemp = mpRootNode->GetWorldMatrix();
			mpRootNode->SetMatrix(cMatrixf::Identity);
		}*/
		cMatrixf *pInvWorldMtx = GetInvModelMatrix();

		for (int i = 0; i < pSkeleton->GetBoneNum(); i++) {
			cBone *pBone = pSkeleton->GetBoneByIndex(i);
			cNode3D *pState = mvBoneStates[i];

			// Transform the movement of the bone into the
			// Bind pose's local space.
			cMatrixf mtxLocal = cMath::MatrixMul(*pInvWorldMtx, pState->GetWorldMatrix());

			mvBoneMatrices[i] = cMath::MatrixMul(mtxLocal, pBone->GetInvWorldTransform());
		}

		// Set back the matrix.
		/*if(mpRootNode){
			mpRootNode->SetMatrix(mtxTemp);
			SetTransformUpdated(true);
		}
		else {
			SetTransformUpdated(true);
		}*/
	}
}

//-----------------------------------------------------------------------

bool cMeshEntity::AttachEntityToParent(iEntity3D *apEntity, const tString &asParent) {
	mlstAttachedEntities.push_back(apEntity);

	// If no parent is specified add to first sub entity.
	if (asParent == "") {
		// Log("Added %s to mesh\n",apEntity->GetName().c_str());
		AddChild(apEntity);
		return true;
	}

	// Check submeshes
	cSubMeshEntity *pSubEntity = GetSubMeshEntityName(asParent);
	if (pSubEntity) {
		// Log("Added %s to subentity %s\n",apEntity->GetName().c_str(),pSubEntity->GetName().c_str());
		pSubEntity->AddChild(apEntity);
		return true;
	}

	// Check nodes
	cNode3D *pNode = GetNodeStateFromName(asParent);
	if (pNode) {
		// Log("Added %s to node %s\n",apEntity->GetName().c_str(),pNode->GetName());
		pNode->AddEntity(apEntity);
		return true;
	}

	// Check bones
	cNode3D *pBone = GetBoneStateFromName(asParent);
	if (pBone) {
		// Log("Added %s to bone\n",apEntity->GetName().c_str());
		pBone->AddEntity(apEntity);
		return true;
	}

	Warning("Parent '%s' couldn't be found! Failed to attach '%s' to '%s'.Attaching directly to mesh.\n", asParent.c_str(),
			apEntity->GetName().c_str(), GetName().c_str());
	AddChild(apEntity);

	return false;
}

//-----------------------------------------------------------------------

void cMeshEntity::SetRendered(bool abX) {
	if (abX == mbRendered)
		return;

	mbRendered = abX;
	for (int i = 0; i < (int)mvSubMeshes.size(); i++) {
		mvSubMeshes[i]->SetRendered(abX);
		mvSubMeshes[i]->SetGlobalRenderCount(cRenderList::GetGlobalRenderCount());
	}
	SetGlobalRenderCount(cRenderList::GetGlobalRenderCount());
	mlStartSleepCount = 0;
}

//-----------------------------------------------------------------------

iMaterial *cMeshEntity::GetMaterial() {
	return NULL;
}

//-----------------------------------------------------------------------

iVertexBuffer *cMeshEntity::GetVertexBuffer() {
	return NULL;
}

//-----------------------------------------------------------------------

bool cMeshEntity::IsShadowCaster() {
	return mbCastShadows;
}

//-----------------------------------------------------------------------

cBoundingVolume *cMeshEntity::GetBoundingVolume() {
	return &mBoundingVolume;
}

//-----------------------------------------------------------------------

cMatrixf *cMeshEntity::GetModelMatrix(cCamera3D *apCamera) {
	mtxTemp = GetWorldMatrix();
	return &mtxTemp;
}

//-----------------------------------------------------------------------

int cMeshEntity::GetMatrixUpdateCount() {
	return GetTransformUpdateCount();
}

//-----------------------------------------------------------------------

eRenderableType cMeshEntity::GetRenderType() {
	return eRenderableType_Mesh;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIAVTE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cMeshEntity::HandleAnimationEvent(cAnimationEvent *apEvent) {
	if (apEvent->msValue == "")
		return;

	switch (apEvent->mType) {
	case eAnimationEventType_PlaySound: {
		cSoundEntity *pSound = mpWorld->CreateSoundEntity("AnimEvent", apEvent->msValue, true);
		if (pSound) {
			cNodeIterator nodeIt = mpRootNode->GetChildIterator();
			if (nodeIt.HasNext()) {
				iNode *pNode = nodeIt.Next();
				pNode->AddEntity(pSound);
			} else {
				pSound->SetPosition(mBoundingVolume.GetWorldCenter());
			}
		}
		break;
	}
	default:
		break;
	}
}

//-----------------------------------------------------------------------

void cMeshEntity::UpdateBVFromSubs() {
	/////////////////////////////////////
	// Skeleton
	if (mpMesh->GetSkeleton()) {
		if (mvBoneStates.empty()) {
			////////////////////////////////
			// Using vertices

			// Go through all the sub meshes and build BV from vertices.
			for (int i = 0; i < GetSubMeshEntityNum(); i++) {
				cSubMeshEntity *pSub = GetSubMeshEntity(i);

				iVertexBuffer *pVtxBuffer = pSub->GetVertexBuffer();

				mBoundingVolume.AddArrayPoints(pVtxBuffer->GetArray(eVertexFlag_Position),
											   pVtxBuffer->GetVertexNum());
			}
			mBoundingVolume.CreateFromPoints(kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)]);
		} else {
			////////////////////////////////
			// Using bones
			cVector3f vMin = mvBoneStates[0]->GetWorldPosition();
			cVector3f vMax = vMin;

			for (size_t i = 1; i < mvBoneStates.size(); ++i) {
				cBoneState *pState = mvBoneStates[i];
				cVector3f vPos = pState->GetWorldPosition();

				if (vMax.x < vPos.x)
					vMax.x = vPos.x;
				if (vMax.y < vPos.y)
					vMax.y = vPos.y;
				if (vMax.z < vPos.z)
					vMax.z = vPos.z;

				if (vMin.x > vPos.x)
					vMin.x = vPos.x;
				if (vMin.y > vPos.y)
					vMin.y = vPos.y;
				if (vMin.z > vPos.z)
					vMin.z = vPos.z;
			}

			// Add 10 percent to the box borders.
			vMin -= (vMax - vMin) * 0.1f + 0.1f;
			vMax += (vMax - vMin) * 0.1f + 0.1f;

			mBoundingVolume.SetTransform(cMatrixf::Identity);
			mBoundingVolume.SetLocalMinMax(vMin, vMax);
		}

	}
	////////////////////////////////////
	// Nodes
	else {
		// Use this to make sure the the nodes are in the same postions as when
		// exported. This to give a working Bounding Volume.
		// This fells kind a slow... but might be only way..

		cMatrixf mtxTemp2;
		if (mpRootNode) {
			mtxTemp2 = mpRootNode->GetWorldMatrix();
			mpRootNode->SetMatrix(cMatrixf::Identity);
		}

		cVector3f vFinalMin = mvSubMeshes[0]->mBoundingVolume.GetMin();
		cVector3f vFinalMax = mvSubMeshes[0]->mBoundingVolume.GetMax();

		for (int i = 1; i < (int)mvSubMeshes.size(); i++) {
			cVector3f vMin = mvSubMeshes[i]->mBoundingVolume.GetMin();
			cVector3f vMax = mvSubMeshes[i]->mBoundingVolume.GetMax();

			if (vFinalMin.x > vMin.x)
				vFinalMin.x = vMin.x;
			if (vFinalMax.x < vMax.x)
				vFinalMax.x = vMax.x;

			if (vFinalMin.y > vMin.y)
				vFinalMin.y = vMin.y;
			if (vFinalMax.y < vMax.y)
				vFinalMax.y = vMax.y;

			if (vFinalMin.z > vMin.z)
				vFinalMin.z = vMin.z;
			if (vFinalMax.z < vMax.z)
				vFinalMax.z = vMax.z;
		}

		mBoundingVolume.SetLocalMinMax(vFinalMin, vFinalMax);

		if (mpRootNode) {
			mpRootNode->SetMatrix(mtxTemp2);
		} else {
		}
	}
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cSaveData_cMeshEntity, cSaveData_iRenderable)
	kSerializeVar(msMeshName, eSerializeType_String)

		kSerializeVar(mbCastShadows, eSerializeType_Bool)

			kSerializeVar(mlBodyId, eSerializeType_Int32)

				kSerializeClassContainer(mvSubEntities, cSaveData_cSubMeshEntity, eSerializeType_Class)
					kSerializeClassContainer(mvAnimStates, cSaveData_cAnimationState, eSerializeType_Class)
						kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject *cSaveData_cMeshEntity::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	cResources *pResources = apGame->GetResources();
	cWorld3D *pWorld = apGame->GetScene()->GetWorld3D();

	/*cMeshLoaderHandler *pMeshLoadHandler = */ pResources->GetMeshLoaderHandler();
	/*cFileSearcher *pFileSearcher = */ pResources->GetFileSearcher();

	///////////////////////////////////
	// Load mesh
	cMesh *pMesh = pResources->GetMeshManager()->CreateMesh(msMeshName);
	if (pMesh == NULL)
		return NULL;

	///////////////////////
	// Load Animations
	// TODO: bad, bad bad...redo animation storing.
	/*if(pMesh->GetAnimationNum()<=1 && mvAnimStates.Size()>1)
	{
		pMesh->ClearAnimations(true);

		for(size_t i=0; i< mvAnimStates.Size(); ++i)
		{
			cAnimation *pAnimation = pResources->GetAnimationManager()->CreateAnimation(mvAnimStates[i].msAnimationName);

			pAnimation->SetAnimationName(mvAnimStates[i].msName);
			pAnimation->SetDefaultSpeed(mvAnimStates[i].mfDefaultSpeed);

			pMesh->AddAnimation(pAnimation);
		}
	}*/

	///////////////////////
	/// Create entity
	cMeshEntity *pEntity = pWorld->CreateMeshEntity(msName, pMesh);

	///////////////////////
	/// Add sub meshes to the save object handler.
	for (int sub = 0; sub < pEntity->GetSubMeshEntityNum(); ++sub) {
		cSubMeshEntity *pSub = pEntity->GetSubMeshEntity(sub);

		pSub->LoadFromSaveData(&mvSubEntities[sub]);
		apSaveObjectHandler->Add(pSub);
	}

	return pEntity;
}

//-----------------------------------------------------------------------

int cSaveData_cMeshEntity::GetSaveCreatePrio() {
	return 2;
}

//-----------------------------------------------------------------------

iSaveData *cMeshEntity::CreateSaveData() {
	return hplNew(cSaveData_cMeshEntity, ());
}

//-----------------------------------------------------------------------

void cMeshEntity::SaveToSaveData(iSaveData *apSaveData) {
	kSaveData_SaveToBegin(cMeshEntity);

	////////////////////////////
	// Set the name of the data
	pData->msMeshName = mpMesh->GetName();

	////////////////////////////
	// Sub Entities
	pData->mvSubEntities.Resize(GetSubMeshEntityNum());
	for (int i = 0; i < GetSubMeshEntityNum(); i++) {
		cSubMeshEntity *pSubEntity = GetSubMeshEntity(i);

		pSubEntity->SaveToSaveData(&pData->mvSubEntities[i]);
	}

	////////////////////////////
	// Animation states
	pData->mvAnimStates.Resize(GetAnimationStateNum());
	for (int i = 0; i < GetAnimationStateNum(); i++) {
		cAnimationState *pAnimState = GetAnimationState(i);

		pAnimState->SaveToSaveData(&pData->mvAnimStates[i]);
	}
	// Log("MeshEntity %s has %d animation states saved\n",GetName().c_str(),pData->mvAnimStates.Size());

	////////////////////////////
	// Variables
	kSaveData_SaveTo(mbCastShadows);

	////////////////////////////
	// Pointers
	kSaveData_SaveObject(mpBody, mlBodyId);
}

//-----------------------------------------------------------------------

void cMeshEntity::LoadFromSaveData(iSaveData *apSaveData) {
	kSaveData_LoadFromBegin(cMeshEntity);

	///////////////////////
	// Sub entities
	// This is done on creation instead.
	/*for(size_t i=0; i<pData->mvSubEntities.Size(); i++)
	{
		cSubMeshEntity *pSubEntity = GetSubMeshEntity((int)i);

		pSubEntity->LoadFromSaveData(&pData->mvSubEntities[i]);
	}*/

	///////////////////////
	// Set Animation states data
	for (size_t i = 0; i < pData->mvAnimStates.Size(); ++i) {
		cAnimationState *pAnimationState = GetAnimationState((int)i);

		pAnimationState->LoadFromSaveData(&pData->mvAnimStates[i]);
	}
	// Log("LOADED: MeshEntity %s has %d animation states, in entity: %d\n",GetName().c_str(),
	//			pData->mvAnimStates.Size(),GetAnimationStateNum());

	//////////////////////
	// Variables
	kSaveData_LoadFrom(mbCastShadows);
}

//-----------------------------------------------------------------------

void cMeshEntity::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	kSaveData_SetupBegin(cMeshEntity);

	///////////////////////
	// Sub entities
	// Should not be needed since they are added to the save object handler.
	for (int i = 0; i < GetSubMeshEntityNum(); i++) {
		cSubMeshEntity *pSubEntity = GetSubMeshEntity(i);
		pSubEntity->SaveDataSetup(apSaveObjectHandler, apGame);
	}

	///////////////////////
	// Body
	kSaveData_LoadObject(mpBody, mlBodyId, iPhysicsBody *);
	if (mpBody) {
		mpBody->CreateNode()->AddEntity(this);
	}
}

//-----------------------------------------------------------------------
} // namespace hpl
