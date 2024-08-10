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

#include "hpl1/engine/graphics/Mesh.h"

#include "hpl1/engine/graphics/Animation.h"
#include "hpl1/engine/graphics/Beam.h"
#include "hpl1/engine/graphics/BillBoard.h"
#include "hpl1/engine/graphics/Bone.h"
#include "hpl1/engine/graphics/BoneState.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/ParticleSystem3D.h"
#include "hpl1/engine/graphics/Skeleton.h"
#include "hpl1/engine/graphics/SubMesh.h"
#include "hpl1/engine/graphics/VertexBuffer.h"
#include "hpl1/engine/resources/AnimationManager.h"
#include "hpl1/engine/resources/MaterialManager.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/physics/PhysicsBody.h"
#include "hpl1/engine/physics/PhysicsJointBall.h"
#include "hpl1/engine/physics/PhysicsJointHinge.h"
#include "hpl1/engine/physics/PhysicsJointScrew.h"
#include "hpl1/engine/physics/PhysicsJointSlider.h"
#include "hpl1/engine/physics/PhysicsWorld.h"

#include "hpl1/engine/scene/Light3DPoint.h"
#include "hpl1/engine/scene/Light3DSpot.h"
#include "hpl1/engine/scene/MeshEntity.h"
#include "hpl1/engine/scene/Node3D.h"
#include "hpl1/engine/scene/SoundEntity.h"
#include "hpl1/engine/scene/World3D.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMesh::cMesh(const tString asName, cMaterialManager *apMaterialManager,
			 cAnimationManager *apAnimationManager) : iResourceBase(asName, 0) {
	mpMaterialManager = apMaterialManager;
	mpAnimationManager = apAnimationManager;
	mpSkeleton = NULL;

	mpRootNode = hplNew(cNode3D, ());
}

//-----------------------------------------------------------------------

cMesh::~cMesh() {
	for (int i = 0; i < (int)mvSubMeshes.size(); i++) {
		hplDelete(mvSubMeshes[i]);
	}
	if (mpSkeleton)
		hplDelete(mpSkeleton);

	for (int i = 0; i < (int)mvAnimations.size(); i++) {
		// mpAnimationManager->Destroy(mvAnimations[i]);
		hplDelete(mvAnimations[i]);
	}

	if (mpRootNode)
		hplDelete(mpRootNode);

	STLDeleteAll(mvColliders);
	STLDeleteAll(mvPhysicJoints);
	STLDeleteAll(mvLights);
	STLDeleteAll(mvBillboards);
	STLDeleteAll(mvBeams);
	STLDeleteAll(mvParticleSystems);
	STLDeleteAll(mvReferences);
	STLDeleteAll(mvSoundEntities);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cMesh::CreateFromFile(const tString asFile) {
	return false;
}

//-----------------------------------------------------------------------

cSubMesh *cMesh::CreateSubMesh(const tString &asName) {
	cSubMesh *pSubMesh = hplNew(cSubMesh, (asName, mpMaterialManager));

	pSubMesh->mpParent = this;

	mvSubMeshes.push_back(pSubMesh);
	m_mapSubMeshes.insert(tSubMeshMap::value_type(asName, pSubMesh));

	return pSubMesh;
}

//-----------------------------------------------------------------------

cSubMesh *cMesh::GetSubMesh(unsigned int alIdx) {
	if (alIdx >= mvSubMeshes.size())
		return NULL;

	return mvSubMeshes[alIdx];
}

cSubMesh *cMesh::GetSubMeshName(const tString &asName) {
	tSubMeshMapIt it = m_mapSubMeshes.find(asName);
	if (it == m_mapSubMeshes.end())
		return NULL;

	return it->second;
}

int cMesh::GetSubMeshNum() {
	return (int)mvSubMeshes.size();
}

//-----------------------------------------------------------------------

void cMesh::SetSkeleton(cSkeleton *apSkeleton) {
	mpSkeleton = apSkeleton;
}

cSkeleton *cMesh::GetSkeleton() {
	return mpSkeleton;
}
//-----------------------------------------------------------------------

void cMesh::AddAnimation(cAnimation *apAnimation) {
	mvAnimations.push_back(apAnimation);

	tAnimationIndexMap::value_type value(apAnimation->GetName(), (int)mvAnimations.size() - 1);
	m_mapAnimIndices.insert(value);
}

cAnimation *cMesh::GetAnimation(int alIndex) {
	return mvAnimations[alIndex];
}

cAnimation *cMesh::GetAnimationFromName(const tString &asName) {
	int lIdx = GetAnimationIndex(asName);
	if (lIdx >= 0) {
		return mvAnimations[lIdx];
	} else {
		return NULL;
	}
}

int cMesh::GetAnimationIndex(const tString &asName) {
	tAnimationIndexMapIt it = m_mapAnimIndices.find(asName);
	if (it != m_mapAnimIndices.end()) {
		return it->second;
	} else {
		return -1;
	}
}

void cMesh::ClearAnimations(bool abDeleteAll) {
	if (abDeleteAll) {
		for (int i = 0; i < (int)mvAnimations.size(); i++) {
			// mpAnimationManager->Destroy(mvAnimations[i]);
			hplDelete(mvAnimations[i]);
		}
	}

	mvAnimations.clear();
	m_mapAnimIndices.clear();
}

int cMesh::GetAnimationNum() {
	return (int)mvAnimations.size();
}

//-----------------------------------------------------------------------

void cMesh::SetupBones() {
	if (mpSkeleton == NULL)
		return;

	// Here is the place to add more hardware friendly stuffs.
}

//-----------------------------------------------------------------------

cMeshJoint *cMesh::CreatePhysicsJoint(ePhysicsJointType aType) {
	cMeshJoint *pJoint = hplNew(cMeshJoint, ());
	pJoint->mType = aType;

	mvPhysicJoints.push_back(pJoint);

	return pJoint;
}
cMeshJoint *cMesh::GetPhysicsJoint(int alIdx) {
	return mvPhysicJoints[alIdx];
}
int cMesh::GetPhysicsJointNum() {
	return (int)mvPhysicJoints.size();
}

//-----------------------------------------------------------------------

iPhysicsJoint *cMesh::CreateJointInWorld(const tString &sNamePrefix, cMeshJoint *apMeshJoint,
										 iPhysicsBody *apParentBody, iPhysicsBody *apChildBody,
										 const cMatrixf &a_mtxOffset, iPhysicsWorld *apWorld) {
	cVector3f vPivot = cMath::MatrixMul(a_mtxOffset, apMeshJoint->mvPivot);
	cVector3f vPinDir = cMath::MatrixMul(a_mtxOffset.GetRotation(), apMeshJoint->mvPinDir);

	///////////////////////////
	// Hinge
	if (apMeshJoint->mType == ePhysicsJointType_Hinge) {
		iPhysicsJointHinge *pJoint = apWorld->CreateJointHinge(sNamePrefix + apMeshJoint->msName,
															   vPivot, vPinDir, apParentBody,
															   apChildBody);

		pJoint->SetCollideBodies(apMeshJoint->mbCollide);

		pJoint->SetMinAngle(cMath::ToRad(-apMeshJoint->mfMin));
		pJoint->SetMaxAngle(cMath::ToRad(apMeshJoint->mfMax));

		return pJoint;
	}
	///////////////////////////
	// Ball
	else if (apMeshJoint->mType == ePhysicsJointType_Ball) {
		iPhysicsJointBall *pJoint = apWorld->CreateJointBall(sNamePrefix + apMeshJoint->msName,
															 vPivot, apParentBody, apChildBody);

		pJoint->SetCollideBodies(apMeshJoint->mbCollide);

		pJoint->SetConeLimits(vPinDir, cMath::ToRad(apMeshJoint->mfMin), cMath::ToRad(apMeshJoint->mfMax));

		return pJoint;
	}
	///////////////////////////
	// Slider
	else if (apMeshJoint->mType == ePhysicsJointType_Slider) {
		iPhysicsJointSlider *pJoint = apWorld->CreateJointSlider(sNamePrefix + apMeshJoint->msName, vPivot, vPinDir, apParentBody, apChildBody);

		pJoint->SetCollideBodies(apMeshJoint->mbCollide);

		pJoint->SetMinDistance(apMeshJoint->mfMin);
		pJoint->SetMaxDistance(apMeshJoint->mfMax);

		return pJoint;
	}
	///////////////////////////
	// Screw
	else if (apMeshJoint->mType == ePhysicsJointType_Screw) {
		iPhysicsJointScrew *pJoint = apWorld->CreateJointScrew(sNamePrefix + apMeshJoint->msName,
															   vPivot, vPinDir, apParentBody, apChildBody);

		pJoint->SetCollideBodies(apMeshJoint->mbCollide);

		pJoint->SetMinDistance(apMeshJoint->mfMin);
		pJoint->SetMaxDistance(apMeshJoint->mfMax);

		return pJoint;
	}

	return NULL;
}

//-----------------------------------------------------------------------

void cMesh::CreateNodeBodies(iPhysicsBody **apRootBodyPtr, Common::Array<iPhysicsBody *> *apBodyVec,
							 cMeshEntity *apEntity, iPhysicsWorld *apPhysicsWorld,
							 const cMatrixf &a_mtxTransform) {
	cMatrixf mtxOldOffset;

	// Log("Creating node bodies!\n");

	///////////////////////////////////
	// Create bodies for the sub meshes (if available
	for (int sub = 0; sub < GetSubMeshNum(); sub++) {
		cSubMesh *pSubMesh = GetSubMesh(sub);
		cSubMeshEntity *pSubEntity = apEntity->GetSubMeshEntity(sub);

		tCollideShapeVec vShapes;
		for (int shape = 0; shape < GetColliderNum(); shape++) {
			cMeshCollider *pColl = GetCollider(shape);
			if (pColl->msGroup == pSubMesh->GetGroup() && pColl->msGroup != "") {
				mtxOldOffset = pColl->m_mtxOffset;

				// Remove the scale
				cMatrixf mtxSub = pSubEntity->GetWorldMatrix();
				cMatrixf mtxScale = cMath::MatrixScale(pSubMesh->GetModelScale());
				mtxSub = cMath::MatrixMul(mtxSub, cMath::MatrixInverse(mtxScale));

				// Get the local offset of the collider, relative to the sub mesh
				pColl->m_mtxOffset = cMath::MatrixMul(cMath::MatrixInverse(mtxSub),
													  pColl->m_mtxOffset);

				// Create shape
				iCollideShape *pShape = CreateCollideShapeFromCollider(pColl, apPhysicsWorld);
				vShapes.push_back(pShape);

				// Log("Created shape size: %s offset: %s\n",pShape->GetSize().ToString().c_str(),pShape->);

				pColl->m_mtxOffset = mtxOldOffset;
			}
		}

		// Create the compound shape if needed.
		iCollideShape *pShape;
		if (vShapes.size() > 1) {
			pShape = apPhysicsWorld->CreateCompundShape(vShapes);
		} else if (vShapes.size() == 1) {
			pShape = vShapes[0];
		} else {
			Warning("No shapes for sub mesh '%s' with group: '%s'\n", pSubMesh->GetName().c_str(),
					pSubMesh->GetGroup().c_str());
			// return;
			continue;
		}

		// Create body and set mass to 0 since these bodies are animated and
		// can therefore be considered static.
		iPhysicsBody *pBody = apPhysicsWorld->CreateBody(apEntity->GetName() + "_" + pSubMesh->GetName(),
														 pShape);
		pBody->SetMass(0);
		pBody->SetGravity(false);

		pSubEntity->SetBody(pBody);
		pSubEntity->SetUpdateBody(true);

		apBodyVec->push_back(pBody);
	}

	///////////////////////////////////
	// Create bodies for root
	tCollideShapeVec vShapes;
	for (int shape = 0; shape < GetColliderNum(); shape++) {
		cMeshCollider *pColl = GetCollider(shape);
		if (pColl->msGroup == "") {
			// Create shape
			iCollideShape *pShape = CreateCollideShapeFromCollider(pColl, apPhysicsWorld);
			vShapes.push_back(pShape);

			/*cMatrixf mtxOffset = */ pShape->GetOffset();
			// Log("Created shape size: %s at %s. Mtx: %s\n",pShape->GetSize().ToString().c_str(),
			//					pShape->GetOffset().GetTranslation().ToString().c_str(),
			//					mtxOffset.ToString().c_str());
		}
	}

	bool bHasRoot = false;
	iCollideShape *pShape;
	if (vShapes.size() > 1) {
		pShape = apPhysicsWorld->CreateCompundShape(vShapes);
		bHasRoot = true;
	} else if (vShapes.size() == 1) {
		pShape = vShapes[0];
		bHasRoot = true;
	} else {
		return;
	}

	if (bHasRoot) {
		// Log("Creating root body!\n");

		iPhysicsBody *pBody = apPhysicsWorld->CreateBody(apEntity->GetName(),
														 pShape);
		pBody->CreateNode()->AddEntity(apEntity);
		pBody->SetMass(0);

		apEntity->SetBody(pBody);

		pBody->SetMatrix(a_mtxTransform);

		apBodyVec->push_back(pBody);
	}
}

//-----------------------------------------------------------------------

void cMesh::CreateJointsAndBodies(Common::Array<iPhysicsBody *> *apBodyVec, cMeshEntity *apEntity,
								  Common::Array<iPhysicsJoint *> *apJointVec,
								  const cMatrixf &a_mtxOffset, iPhysicsWorld *apPhysicsWorld) {
	Common::Array<iPhysicsBody *> vBodies;

	//////////////////////////////////
	// If the mesh has a skeleton, attach the bodies to the bones
	// in the skeleton.
	if (mpSkeleton) {
		// TODO: Set root node to identity matrix

		for (int bone = 0; bone < apEntity->GetBoneStateNum(); ++bone) {
			cBoneState *pBoneState = apEntity->GetBoneState(bone);

			/////////////////////////////////////////////////////
			// Iterate the colliders and get the sum of offsets.
			cVector3f vShapeWorldCenter(0, 0, 0);
			float fColliderNum = 0;
			for (int shape = 0; shape < GetColliderNum(); shape++) {
				cMeshCollider *pColl = GetCollider(shape);
				if (pColl->msGroup == pBoneState->GetName()) {
					vShapeWorldCenter += pColl->m_mtxOffset.GetTranslation();
					fColliderNum += 1;
				}
			}

			vShapeWorldCenter = vShapeWorldCenter / fColliderNum;

			cMatrixf mtxBodyWorld = cMath::MatrixTranslate(vShapeWorldCenter);
			cMatrixf mtxInvBodyWorld = cMath::MatrixInverse(mtxBodyWorld);

			cMatrixf mtxBone = pBoneState->GetWorldMatrix();
			cMatrixf mtxInvBone = cMath::MatrixInverse(mtxBone);

			cMatrixf mtxBoneToBody = cMath::MatrixMul(mtxInvBone, mtxBodyWorld);

			///////////////////////////////////////////////////////////
			// Iterate the colliders and search for the colliders for each object.
			cMatrixf mtxOldOffset;
			tCollideShapeVec vShapes;
			for (int shape = 0; shape < GetColliderNum(); shape++) {
				cMeshCollider *pColl = GetCollider(shape);
				if (pColl->msGroup == pBoneState->GetName()) {
					mtxOldOffset = pColl->m_mtxOffset;

					cMatrixf mtxBodyToCollider = cMath::MatrixMul(mtxInvBodyWorld, pColl->m_mtxOffset);
					pColl->m_mtxOffset = mtxBodyToCollider;

					iCollideShape *pShape = CreateCollideShapeFromCollider(pColl, apPhysicsWorld);
					vShapes.push_back(pShape);

					// Setting old offset
					pColl->m_mtxOffset = mtxOldOffset;
				}
			}

			////////////////////////////////
			// Create body
			if (vShapes.size() > 0) {
				// Create the compound shape if needed.
				iCollideShape *pShape = nullptr;
				if (vShapes.size() > 1) {
					pShape = apPhysicsWorld->CreateCompundShape(vShapes);
				} else if (vShapes.size() == 1) {
					pShape = vShapes[0];
				}

				//////////////////////////////////////////
				// Create normal body and set mass to 1 for now.
				iPhysicsBody *pBody = apPhysicsWorld->CreateBody(apEntity->GetName() + "_" + pBoneState->GetName(),
																 pShape);
				pBody->SetMass(1);
				pBody->SetActive(false);

				pBody->SetIsRagDoll(true);
				pBody->SetCollideRagDoll(false);

				pBody->SetMatrix(cMath::MatrixMul(a_mtxOffset, mtxBodyWorld));
				pBoneState->SetBody(pBody);
				pBoneState->SetBodyMatrix(mtxBoneToBody);

				if (apBodyVec)
					apBodyVec->push_back(pBody);
				vBodies.push_back(pBody);

				/////////////////////////////////////
				// Create collider body
				iPhysicsBody *pColliderBody = apPhysicsWorld->CreateBody(apEntity->GetName() + "_collider_" + pBoneState->GetName(),
																		 pShape);
				pColliderBody->SetMass(0);
				pColliderBody->SetActive(false);
				pColliderBody->SetCollideCharacter(false);

				pBoneState->SetColliderBody(pColliderBody);

				if (apBodyVec)
					apBodyVec->push_back(pColliderBody);
			}
		}

		// TODO: Reset root node matrix
	}
	///////////////////////////////////
	// Create bodies for the sub meshes.
	else {
		for (int sub = 0; sub < GetSubMeshNum(); sub++) {
			cSubMesh *pSubMesh = GetSubMesh(sub);
			cSubMeshEntity *pSubEntity = apEntity->GetSubMeshEntity(sub);

			bool bGroupShare = false;

			// Check if the submesh is a child of another mesh,
			// if so skip it.
			for (int i = 0; i < GetSubMeshNum(); ++i) {
				if (i == sub)
					continue;
				cSubMesh *pExtraSub = GetSubMesh(i);
				cSubMeshEntity *pExtraEntity = apEntity->GetSubMeshEntity(i);

				if (pSubMesh->GetGroup() == pExtraSub->GetNodeName()) {
					pExtraEntity->AddChild(pSubEntity);
					pSubEntity->SetMatrix(pSubMesh->GetLocalTransform());

					bGroupShare = true;
					break;
				}
			}
			if (bGroupShare)
				continue;

			// Extra check to see if any other sub object chairs the group.
			for (int i = 0; i < GetSubMeshNum(); ++i) {
				if (i == sub)
					continue;
				cSubMesh *pExtraSub = GetSubMesh(i);

				if (pExtraSub->GetGroup() == pSubMesh->GetGroup()) {
					Error("SubMesh %s shares group with %s\n", pSubMesh->GetName().c_str(),
						  pExtraSub->GetName().c_str());
					bGroupShare = true;
				}
			}

			if (bGroupShare)
				continue;

			cMatrixf mtxOldOffset;

			// Log("Sub: %s group: '%s'\n",pSubMesh->GetName().c_str(),pSubMesh->GetGroup().c_str());

			// Iterate the colliders and search for the colliders for each object.
			tCollideShapeVec vShapes;
			for (int shape = 0; shape < GetColliderNum(); shape++) {
				cMeshCollider *pColl = GetCollider(shape);
				if (pColl->msGroup == pSubMesh->GetGroup()) {
					mtxOldOffset = pColl->m_mtxOffset;

					// Remove the scale
					cMatrixf mtxSub = pSubEntity->GetWorldMatrix();

					// Log("SubEntity '%s' : %s\n",pSubEntity->GetName().c_str(),mtxSub.ToString().c_str());
					// if(pSubEntity->GetParent())
					//	Log("Node parent: %s\n",static_cast<cNode3D*>(pSubEntity->GetParent())->GetName());

					// The scale should already been removed.
					/*cMatrixf mtxScale = cMath::MatrixScale(pSubMesh->GetModelScale());
					mtxSub = cMath::MatrixMul(mtxSub, cMath::MatrixInverse(mtxScale));*/

					// Get the local offset of the collider, relative to the sub mesh
					pColl->m_mtxOffset = cMath::MatrixMul(cMath::MatrixInverse(mtxSub),
														  pColl->m_mtxOffset);

					iCollideShape *pShape = CreateCollideShapeFromCollider(pColl, apPhysicsWorld);
					vShapes.push_back(pShape);
					// Log("Created shapes!\n");

					// Setting old offset
					pColl->m_mtxOffset = mtxOldOffset;
				}
			}

			// Create the compound shape if needed.
			iCollideShape *pShape;
			if (vShapes.size() > 1) {
				pShape = apPhysicsWorld->CreateCompundShape(vShapes);
			} else if (vShapes.size() == 1) {
				pShape = vShapes[0];
			} else {
				Error("No shapes for submesh '%s' with group: '%s' in mesh '%s'\n", pSubMesh->GetName().c_str(),
					  pSubMesh->GetGroup().c_str(), msName.c_str());
				continue;
			}

			// Create body and set mass to 1 for now.
			iPhysicsBody *pBody = apPhysicsWorld->CreateBody(apEntity->GetName() + "_" + pSubMesh->GetName(),
															 pShape);
			pBody->CreateNode()->AddEntity(pSubEntity);
			pBody->SetMass(1);

			pBody->SetMatrix(cMath::MatrixMul(a_mtxOffset, pSubEntity->GetWorldMatrix()));
			pSubEntity->SetMatrix(cMatrixf::Identity);
			pSubEntity->SetBody(pBody);

			if (apBodyVec)
				apBodyVec->push_back(pBody);
			vBodies.push_back(pBody);
		}
	}

	/////////////////////////////////////////////////////
	// Iterate the mesh joints and create physics joints
	for (int joint = 0; joint < GetPhysicsJointNum(); joint++) {
		cMeshJoint *pMeshJoint = GetPhysicsJoint(joint);

		// Convert name to global
		tString sChildBody = apEntity->GetName() + "_" + pMeshJoint->msChildBody;
		tString sParentBody = apEntity->GetName() + "_" + pMeshJoint->msParentBody;

		// Get Parent and Child body
		iPhysicsBody *pChildBody = NULL;
		iPhysicsBody *pParentBody = NULL;
		for (int body = 0; body < (int)vBodies.size(); body++) {
			iPhysicsBody *pBody = vBodies[body];
			if (pBody->GetName() == sChildBody) {
				pChildBody = pBody;
			} else if (pBody->GetName() == sParentBody) {
				pParentBody = pBody;
			}
		}
		if (pParentBody == NULL && pMeshJoint->msParentBody != "") {
			Warning("Parent body '%s' for joint '%s' in mesh '%s' does not exist!\n", pMeshJoint->msParentBody.c_str(),
					pMeshJoint->msName.c_str(), msName.c_str());
			continue;
		}
		if (pChildBody == NULL) {
			Error("Child body '%s' for joint '%s' in mesh '%s' does not exist!\n", pMeshJoint->msChildBody.c_str(),
				  pMeshJoint->msName.c_str(), msName.c_str());
			continue;
		}

		iPhysicsJoint *pJoint = CreateJointInWorld(apEntity->GetName() + "_", pMeshJoint,
												   pParentBody, pChildBody,
												   a_mtxOffset, apPhysicsWorld);

		if (apJointVec)
			apJointVec->push_back(pJoint);
	}

	if (GetSkeleton())
		apEntity->SetMatrix(a_mtxOffset);
}

//-----------------------------------------------------------------------

bool cMesh::HasSeveralBodies() {
	if (GetColliderNum() <= 0)
		return false;

	tString sPrevGroup = GetCollider(0)->msGroup;

	for (int shape = 1; shape < GetColliderNum(); shape++) {
		cMeshCollider *pColl = GetCollider(shape);
		if (pColl->msGroup != sPrevGroup)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------

cMeshCollider *cMesh::CreateCollider(eCollideShapeType aType) {
	cMeshCollider *pColl = hplNew(cMeshCollider, ());
	pColl->mType = aType;

	mvColliders.push_back(pColl);

	return pColl;
}

cMeshCollider *cMesh::GetCollider(int alIdx) {
	return mvColliders[alIdx];
}

int cMesh::GetColliderNum() {
	return (int)mvColliders.size();
}

iCollideShape *cMesh::CreateCollideShapeFromCollider(cMeshCollider *pCollider, iPhysicsWorld *apWorld) {
	// WORKAROUND: Bug #14570: "HPL1: helmet stuck inside a locker"
	// Collider creation and updating changed between the version of the Newton physics library we are using and the original version. The changes are probably in dgBody::UpdateCollisionMatrix
	// (or in one of the functions called inside of it), called when at the creation of the helmet's PhysicsBodyNewton and after modifying the body's transformation matrix later in the loading process.
	if (GetName() == "iron_mine_helmet.dae") {
		return apWorld->CreateBoxShape(pCollider->mvSize - cVector3f(0.04f, 0.0f, 0.04f), &pCollider->m_mtxOffset);
	}
	switch (pCollider->mType) {
	case eCollideShapeType_Box:
		return apWorld->CreateBoxShape(pCollider->mvSize, &pCollider->m_mtxOffset);
	case eCollideShapeType_Sphere:
		return apWorld->CreateSphereShape(pCollider->mvSize, &pCollider->m_mtxOffset);
	case eCollideShapeType_Cylinder:
		return apWorld->CreateCylinderShape(pCollider->mvSize.x, pCollider->mvSize.y, &pCollider->m_mtxOffset);
	case eCollideShapeType_Capsule:
		return apWorld->CreateCapsuleShape(pCollider->mvSize.x, pCollider->mvSize.y, &pCollider->m_mtxOffset);
	default:
		return NULL;
	}
}

iCollideShape *cMesh::CreateCollideShape(iPhysicsWorld *apWorld) {
	if (mvColliders.empty())
		return NULL;

	// Create a single object
	if (mvColliders.size() == 1) {
		return CreateCollideShapeFromCollider(mvColliders[0], apWorld);
	}
	// Create compound object
	else {
		tCollideShapeVec vShapes;
		vShapes.reserve(mvColliders.size());

		for (size_t i = 0; i < mvColliders.size(); ++i) {
			vShapes.push_back(CreateCollideShapeFromCollider(mvColliders[i], apWorld));
		}

		return apWorld->CreateCompundShape(vShapes);
	}
}

//-----------------------------------------------------------------------

cMeshLight *cMesh::CreateLight(eLight3DType aType) {
	cMeshLight *pLight = hplNew(cMeshLight, ());

	mvLights.push_back(pLight);

	return pLight;
}

cMeshLight *cMesh::GetLight(int alIdx) {
	return mvLights[alIdx];
}

int cMesh::GetLightNum() {
	return (int)mvLights.size();
}

iLight3D *cMesh::CreateLightInWorld(const tString &sNamePrefix, cMeshLight *apMeshLight,
									cMeshEntity *apMeshEntity, cWorld3D *apWorld) {
	iLight3D *pLight = NULL;

	////////////////////////////////
	// Spot
	if (apMeshLight->mType == eLight3DType_Spot) {
		cLight3DSpot *pLightSpot = apWorld->CreateLightSpot(sNamePrefix + "_" + apMeshLight->msName);
		pLightSpot->SetDiffuseColor(apMeshLight->mColor);
		pLightSpot->SetFarAttenuation(apMeshLight->mfRadius);
		pLightSpot->SetFOV(apMeshLight->mfFOV);
		if (apMeshLight->msFile != "")
			pLightSpot->LoadXMLProperties(apMeshLight->msFile);

		pLight = pLightSpot;
	}
	////////////////////////////////
	// Point
	else if (apMeshLight->mType == eLight3DType_Point) {
		cLight3DPoint *pLightPoint = apWorld->CreateLightPoint(sNamePrefix + "_" + apMeshLight->msName);
		pLightPoint->SetDiffuseColor(apMeshLight->mColor);
		pLightPoint->SetFarAttenuation(apMeshLight->mfRadius);
		pLightPoint->SetCastShadows(apMeshLight->mbCastShadows);
		if (apMeshLight->msFile != "")
			pLightPoint->LoadXMLProperties(apMeshLight->msFile);

		pLight = pLightPoint;
	} else {
		return NULL;
	}

	pLight->SetMatrix(apMeshLight->m_mtxTransform);

	apMeshEntity->AttachEntityToParent(pLight, apMeshLight->msParent);

	return pLight;
}

//-----------------------------------------------------------------------

cMeshBillboard *cMesh::CreateBillboard() {
	cMeshBillboard *pBillboard = hplNew(cMeshBillboard, ());
	mvBillboards.push_back(pBillboard);
	return pBillboard;
}

cMeshBillboard *cMesh::GetBillboard(int alIdx) {
	return mvBillboards[alIdx];
}

int cMesh::GetBillboardNum() {
	return (int)mvBillboards.size();
}

cBillboard *cMesh::CreateBillboardInWorld(const tString &sNamePrefix, cMeshBillboard *apMeshBillboard,
										  cMeshEntity *apMeshEntity, cWorld3D *apWorld) {
	cBillboard *pBillboard = apWorld->CreateBillboard(sNamePrefix + "_" + apMeshBillboard->msName,
													  apMeshBillboard->mvSize);
	pBillboard->SetAxis(apMeshBillboard->mvAxis);
	pBillboard->SetPosition(apMeshBillboard->mvPosition);
	pBillboard->SetForwardOffset(apMeshBillboard->mfOffset);

	pBillboard->LoadXMLProperties(apMeshBillboard->msFile);

	apMeshEntity->AttachEntityToParent(pBillboard, apMeshBillboard->msParent);

	return pBillboard;
}

//-----------------------------------------------------------------------

cMeshBeam *cMesh::CreateBeam() {
	cMeshBeam *pBeam = hplNew(cMeshBeam, ());
	mvBeams.push_back(pBeam);
	return pBeam;
}

cMeshBeam *cMesh::GetBeam(int alIdx) {
	return mvBeams[alIdx];
}

int cMesh::GetBeamNum() {
	return (int)mvBeams.size();
}

cBeam *cMesh::CreateBeamInWorld(const tString &sNamePrefix, cMeshBeam *apMeshBeam,
								cMeshEntity *apMeshEntity, cWorld3D *apWorld) {
	cBeam *pBeam = apWorld->CreateBeam(sNamePrefix + "_" + apMeshBeam->msName);

	pBeam->SetPosition(apMeshBeam->mvStartPosition);
	pBeam->GetEnd()->SetPosition(apMeshBeam->mvEndPosition);

	pBeam->LoadXMLProperties(apMeshBeam->msFile);

	apMeshEntity->AttachEntityToParent(pBeam, apMeshBeam->msStartParent);
	apMeshEntity->AttachEntityToParent(pBeam->GetEnd(), apMeshBeam->msEndParent);

	return pBeam;
}

//-----------------------------------------------------------------------

cMeshReference *cMesh::CreateReference() {
	cMeshReference *pRef = hplNew(cMeshReference, ());
	mvReferences.push_back(pRef);
	return pRef;
}

cMeshReference *cMesh::GetReference(int alIdx) {
	return mvReferences[alIdx];
}

int cMesh::GetReferenceNum() {
	return (int)mvReferences.size();
}

iEntity3D *cMesh::CreateReferenceInWorld(const tString &sNamePrefix,
										 cMeshReference *apMeshRef,
										 cMeshEntity *apMeshEntity, cWorld3D *apWorld,
										 const cMatrixf &a_mtxOffset) {
	if (apMeshRef->msParent != "") {

		tString sName = sNamePrefix + "_" + apMeshRef->msName;
		iEntity3D *pEntity = apWorld->CreateEntity(sName,
												   apMeshRef->m_mtxTransform,
												   apMeshRef->msFile, true);

		if (pEntity)
			apMeshEntity->AttachEntityToParent(pEntity, apMeshRef->msParent);
		return pEntity;
	} else {
		tString sName = sNamePrefix + "_" + apMeshRef->msName;
		iEntity3D *pEntity = apWorld->CreateEntity(sName,
												   cMath::MatrixMul(a_mtxOffset, apMeshRef->m_mtxTransform),
												   apMeshRef->msFile, true);

		// Log("Created ref: %s\n",sName.c_str());

		return pEntity;
	}
}

//-----------------------------------------------------------------------

cMeshParticleSystem *cMesh::CreateParticleSystem() {
	cMeshParticleSystem *pPS = hplNew(cMeshParticleSystem, ());
	mvParticleSystems.push_back(pPS);
	return pPS;
}

cMeshParticleSystem *cMesh::GetParticleSystem(int alIdx) {
	return mvParticleSystems[alIdx];
}

int cMesh::GetParticleSystemNum() {
	return (int)mvParticleSystems.size();
}

cParticleSystem3D *cMesh::CreateParticleSystemInWorld(const tString &sNamePrefix,
													  cMeshParticleSystem *apMeshPS,
													  cMeshEntity *apMeshEntity, cWorld3D *apWorld) {
	cParticleSystem3D *pPS = apWorld->CreateParticleSystem(sNamePrefix + "_" + apMeshPS->msName,
														   apMeshPS->msType, apMeshPS->mvSize, apMeshPS->m_mtxTransform);
	if (pPS == NULL) {
		Error("Couldn't create particle system '%s'\n", apMeshPS->msType.c_str());
		return NULL;
	}

	apMeshEntity->AttachEntityToParent(pPS, apMeshPS->msParent);

	return pPS;
}

//-----------------------------------------------------------------------

cMeshSoundEntity *cMesh::CreateSoundEntity() {
	cMeshSoundEntity *pSound = hplNew(cMeshSoundEntity, ());
	mvSoundEntities.push_back(pSound);
	return pSound;
}
cMeshSoundEntity *cMesh::GetSoundEntity(int alIdx) {
	return mvSoundEntities[alIdx];
}
int cMesh::GetSoundEntityNum() {
	return (int)mvSoundEntities.size();
}
cSoundEntity *cMesh::CreateSoundEntityInWorld(const tString &sNamePrefix, cMeshSoundEntity *apMeshSound,
											  cMeshEntity *apMeshEntity, cWorld3D *apWorld) {
	tString sName = sNamePrefix + "_" + apMeshSound->msName;
	cSoundEntity *pSound = apWorld->CreateSoundEntity(sName,
													  apMeshSound->msType, false);

	// Log("Created sound entity: '%s'\n",sName.c_str());
	if (pSound == NULL) {
		Error("Couldn't create sound entity '%s'\n", apMeshSound->msType.c_str());
		return NULL;
	}
	pSound->SetPosition(apMeshSound->mvPosition);
	apMeshEntity->AttachEntityToParent(pSound, apMeshSound->msParent);
	return pSound;
}

//-----------------------------------------------------------------------

cNode3D *cMesh::GetRootNode() {
	return mpRootNode;
}

void cMesh::AddNode(cNode3D *apNode) {
	mvNodes.push_back(apNode);
}

int cMesh::GetNodeNum() {
	return (int)mvNodes.size();
}

cNode3D *cMesh::GetNode(int alIdx) {
	return mvNodes[alIdx];
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIAVTE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
} // namespace hpl
