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

#include "hpl1/engine/resources/EntityLoader_Object.h"

#include "hpl1/engine/impl/tinyXML/tinyxml.h"
#include "hpl1/engine/scene/World3D.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/physics/CollideShape.h"
#include "hpl1/engine/physics/PhysicsBody.h"
#include "hpl1/engine/physics/PhysicsController.h"
#include "hpl1/engine/physics/PhysicsJoint.h"
#include "hpl1/engine/physics/PhysicsJointBall.h"
#include "hpl1/engine/physics/PhysicsJointHinge.h"
#include "hpl1/engine/physics/PhysicsJointScrew.h"
#include "hpl1/engine/physics/PhysicsJointSlider.h"
#include "hpl1/engine/physics/PhysicsWorld.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/resources/AnimationManager.h"
#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/MaterialManager.h"
#include "hpl1/engine/resources/MeshLoaderHandler.h"
#include "hpl1/engine/resources/MeshManager.h"

#include "hpl1/engine/graphics/Animation.h"
#include "hpl1/engine/graphics/Mesh.h"
#include "hpl1/engine/graphics/SubMesh.h"
#include "hpl1/engine/scene/AnimationState.h"
#include "hpl1/engine/scene/MeshEntity.h"
#include "hpl1/engine/scene/Node3D.h"

#include "hpl1/engine/scene/Light3DSpot.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iEntity3D *cEntityLoader_Object::Load(const tString &asName, TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform,
									  cWorld3D *apWorld, const tString &asFileName, bool abLoadReference) {
	////////////////////////////////////////
	// Init
	mvBodies.clear();
	mvJoints.clear();

	mvParticleSystems.clear();
	mvBillboards.clear();
	mvSoundEntities.clear();
	mvLights.clear();
	mvBeams.clear();

	mpEntity = NULL;
	mpMesh = NULL;

	msFileName = asFileName;

	////////////////////////////////////////
	// Load MAIN

	TiXmlElement *pMainElem = apRootElem->FirstChildElement("MAIN");
	if (pMainElem == NULL) {
		Error("Couldn't load element MAIN");
		return NULL;
	}

	msName = cString::ToString(pMainElem->Attribute("Name"), "");
	msSubType = cString::ToString(pMainElem->Attribute("Subtype"), "");

	// Before load virtual call.
	BeforeLoad(apRootElem, a_mtxTransform, apWorld);

	////////////////////////////////////////
	// Load GRAPHICS
	TiXmlElement *pGfxElem = apRootElem->FirstChildElement("GRAPHICS");
	if (pGfxElem == NULL) {
		Error("Couldn't load element GRAPHICS");
		return NULL;
	}

	// load XML properties
	tString sMeshFile = cString::ToString(pGfxElem->Attribute("ModelFile"), "");
	bool bShadows = cString::ToBool(pGfxElem->Attribute("CastShadows"), true);

	bool bAnimationLoop = cString::ToBool(pGfxElem->Attribute("AnimationLoop"), true);
	bool bAnimationRandStart = cString::ToBool(pGfxElem->Attribute("AnimationRandStart"), true);

	mpMesh = apWorld->GetResources()->GetMeshManager()->CreateMesh(sMeshFile);
	if (mpMesh == NULL)
		return NULL;

	////////////////////////////////////////
	// CREATE ENTITY

	mpEntity = apWorld->CreateMeshEntity(asName, mpMesh);

	// Set entity properties
	mpEntity->SetCastsShadows(bShadows);

	////////////////////////////////////////
	// Load SUBMESHES
	TiXmlElement *pSubMeshElem = apRootElem->FirstChildElement("SUBMESH");
	for (; pSubMeshElem != NULL; pSubMeshElem = pSubMeshElem->NextSiblingElement("SUBMESH")) {
		tString sName = cString::ToString(pSubMeshElem->Attribute("Name"), "");
		tString sMaterialFile = cString::ToString(pSubMeshElem->Attribute("MaterialFile"), "");

		cSubMeshEntity *pSubEntity = mpEntity->GetSubMeshEntityName(sName);
		if (pSubEntity == NULL) {
			Warning("Sub mesh '%s' does not exist in mesh '%s'!\n", sName.c_str(),
					mpMesh->GetName().c_str());
			continue;
		}

		if (sMaterialFile != "") {
			iMaterial *pMaterial = apWorld->GetResources()->GetMaterialManager()->CreateMaterial(sMaterialFile);
			if (pMaterial) {
				pSubEntity->SetCustomMaterial(pMaterial);
			}
		}
	}

	////////////////////////////////////////
	// Load ANIMATIONS
	TiXmlElement *pAnimRootElem = apRootElem->FirstChildElement("ANIMATIONS");
	if (pAnimRootElem) {
		mpEntity->ClearAnimations();

		TiXmlElement *pAnimElem = pAnimRootElem->FirstChildElement("Animation");
		for (; pAnimElem != NULL; pAnimElem = pAnimElem->NextSiblingElement("Animation")) {
			tString sFile = cString::ToString(pAnimElem->Attribute("File"), "");
			tString sName = cString::ToString(pAnimElem->Attribute("Name"), "Unknown");
			float fSpeed = cString::ToFloat(pAnimElem->Attribute("Speed"), 1.0f);
			float fSpecialEventTime = cString::ToFloat(pAnimElem->Attribute("SpecialEventTime"), 0.0f);

			cAnimationManager *pAnimManager = apWorld->GetResources()->GetAnimationManager();

			cAnimation *pAnimation = pAnimManager->CreateAnimation(sFile);

			if (pAnimation) {
				cAnimationState *pState = mpEntity->AddAnimation(pAnimation, sName, fSpeed);
				pState->SetSpecialEventTime(fSpecialEventTime);

				////////////////////////////////
				// Get Events
				TiXmlElement *pAnimEventElem = pAnimElem->FirstChildElement("Event");
				for (; pAnimEventElem != NULL; pAnimEventElem = pAnimEventElem->NextSiblingElement("Event")) {
					cAnimationEvent *pEvent = pState->CreateEvent();

					pEvent->mfTime = cString::ToFloat(pAnimEventElem->Attribute("Time"), 1.0f);
					pEvent->mType = GetAnimationEventType(pAnimEventElem->Attribute("Type"));
					pEvent->msValue = cString::ToString(pAnimEventElem->Attribute("Value"), "");
				}
			}
		}
	}

	////////////////////////////////////////
	// Load PHYSICS
	bool bUsingNodeBodies = false;

	//////////////////////////////////
	// Properties for entities with joints
	if ((mpMesh->GetPhysicsJointNum() > 0 || mpMesh->HasSeveralBodies()) &&
		(mpMesh->GetAnimationNum() <= 0 || mpMesh->GetSkeleton())) {
		mpMesh->CreateJointsAndBodies(&mvBodies, mpEntity, &mvJoints, a_mtxTransform, apWorld->GetPhysicsWorld());

		////////////////////////////////////
		// Properties for bodies
		TiXmlElement *pPhysicsElem = apRootElem->FirstChildElement("PHYSICS");
		for (; pPhysicsElem != NULL; pPhysicsElem = pPhysicsElem->NextSiblingElement("PHYSICS")) {
			iPhysicsBody *pBody = NULL;

			// load XML properties
			tString sSubName = asName + "_" + cString::ToString(pPhysicsElem->Attribute("SubName"), "");

			tString sMaterial = cString::ToString(pPhysicsElem->Attribute("Material"), "Default");

			bool bStaticMeshCollider = cString::ToBool(pPhysicsElem->Attribute("StaticMeshCollider"), false);
			tString sColliderMesh = cString::ToString(pPhysicsElem->Attribute("ColliderMesh"), "");

			if (bStaticMeshCollider) {
				cSubMesh *pSubMesh = mpMesh->GetSubMeshName(sColliderMesh);
				if (pSubMesh) {
					iCollideShape *pShape = apWorld->GetPhysicsWorld()->CreateMeshShape(
						pSubMesh->GetVertexBuffer());
					pBody = apWorld->GetPhysicsWorld()->CreateBody(sColliderMesh, pShape);
				} else {
					Error("Couldn't find sub mesh '%s' to create collision mesh\n", sColliderMesh.c_str());
					continue;
				}

				cSubMeshEntity *pSubEntity = mpEntity->GetSubMeshEntityName(sColliderMesh);

				pBody->CreateNode()->AddEntity(pSubEntity);
				pBody->SetMass(0);
				pBody->SetMatrix(a_mtxTransform);

				pSubEntity->SetBody(pBody);

				mvBodies.push_back(pBody);
			} else {
				pBody = (iPhysicsBody *)STLFindByName(mvBodies, sSubName);
				if (pBody == NULL) {
					Error("Cannot find sub entity '%s' in mesh '%s'\n", sSubName.c_str(), mpMesh->GetName().c_str());
					continue;
				}
			}

			SetBodyProperties(pBody, pPhysicsElem);

			iPhysicsMaterial *pMaterial = apWorld->GetPhysicsWorld()->GetMaterialFromName(sMaterial);
			if (pMaterial) {
				pBody->SetMaterial(pMaterial);
			} else {
				Error("Physics Material '%s' for body '%s' in mesh '%s' doesn't exist!\n",
					  sMaterial.c_str(), pBody->GetName().c_str(), mpMesh->GetName().c_str());
			}
		}

		////////////////////////////////////
		// Properties for joints
		TiXmlElement *pJointElem = apRootElem->FirstChildElement("JOINT");
		for (; pJointElem != NULL; pJointElem = pJointElem->NextSiblingElement("JOINT")) {
			tString sName = asName + "_" + cString::ToString(pJointElem->Attribute("Name"), "");

			iPhysicsJoint *pJoint = (iPhysicsJoint *)STLFindByName(mvJoints, sName);
			if (pJoint) {
				SetJointProperties(pJoint, pJointElem, apWorld);
			} else {
				Error("Joint '%s' not found in mesh '%s'\n", sName.c_str(), mpMesh->GetName().c_str());
			}
		}

		// for(size_t i=0; i< vJoints.size(); i++)
		//{
		//	Log("Created joint: %s\n",vJoints[i]->GetName().c_str());
		// }
	}
	//////////////////////////////////
	// Properties for entities nodes and animation
	else if (mpMesh->GetNodeNum() > 0) {
		// Log("nodes and anim\n");
		iPhysicsBody *pRootBody = NULL;

		bUsingNodeBodies = true;

		mpMesh->CreateNodeBodies(&pRootBody, &mvBodies, mpEntity, apWorld->GetPhysicsWorld(),
								 a_mtxTransform);

		TiXmlElement *pPhysicsElem = apRootElem->FirstChildElement("PHYSICS");
		for (; pPhysicsElem != NULL; pPhysicsElem = pPhysicsElem->NextSiblingElement("PHYSICS")) {
			iPhysicsBody *pBody = NULL;

			// load XML properties
			tString sSubName = cString::ToString(pPhysicsElem->Attribute("SubName"), "");
			if (sSubName != "")
				sSubName = asName + "_" + sSubName;
			else
				sSubName = asName;

			tString sMaterial = cString::ToString(pPhysicsElem->Attribute("Material"), "Default");

			bool bStaticMeshCollider = cString::ToBool(pPhysicsElem->Attribute("StaticMeshCollider"), false);
			tString sColliderMesh = cString::ToString(pPhysicsElem->Attribute("ColliderMesh"), "");

			if (bStaticMeshCollider) {
				cSubMesh *pSubMesh = mpMesh->GetSubMeshName(sColliderMesh);
				if (pSubMesh) {
					iCollideShape *pShape = apWorld->GetPhysicsWorld()->CreateMeshShape(
						pSubMesh->GetVertexBuffer());
					pBody = apWorld->GetPhysicsWorld()->CreateBody(sColliderMesh, pShape);
				} else {
					Error("Couldn't find sub mesh '%s' to create collision mesh\n", sColliderMesh.c_str());
					continue;
				}
			} else {
				pBody = (iPhysicsBody *)STLFindByName(mvBodies, sSubName);
				if (pBody == NULL) {
					Error("Cannot find sub entity '%s' in mesh '%s'\n", sSubName.c_str(), mpMesh->GetName().c_str());
					continue;
				}
			}

			SetBodyProperties(pBody, pPhysicsElem);

			// pBody->SetMass(0);
			// pBody->SetGravity(false);

			iPhysicsMaterial *pMaterial = apWorld->GetPhysicsWorld()->GetMaterialFromName(sMaterial);
			if (pMaterial) {
				pBody->SetMaterial(pMaterial);
			} else {
				Error("Physics Material '%s' for body '%s' in mesh '%s' doesn't exist!\n",
					  sMaterial.c_str(), pBody->GetName().c_str(), mpMesh->GetName().c_str());
			}
		}

		// Only allow for one joint
		if (mpMesh->GetPhysicsJointNum() == 1 && mvBodies.size() == 1) {
			iPhysicsJoint *pJoint = mpMesh->CreateJointInWorld(asName, mpMesh->GetPhysicsJoint(0),
															   NULL, mvBodies[0], a_mtxTransform,
															   apWorld->GetPhysicsWorld());
			if (pJoint) {
				TiXmlElement *pJointElem = apRootElem->FirstChildElement("JOINT");
				if (pJointElem) {
					SetJointProperties(pJoint, pJointElem, apWorld);
				}
			}
		} else if (mpMesh->GetPhysicsJointNum() > 1) {
			Error("Over 1 joints in '%s'!Animated body meshes only allow for one joint!\n",
				  mpMesh->GetName().c_str());
		}

	}
	//////////////////////////////////
	// Properties for other entities
	else {
		TiXmlElement *pPhysicsElem = apRootElem->FirstChildElement("PHYSICS");
		if (pPhysicsElem) {
			iPhysicsBody *pBody = NULL;

			// load XML properties
			tString sSubName = cString::ToString(pPhysicsElem->Attribute("SubName"), "");

			bool bCollides = cString::ToBool(pPhysicsElem->Attribute("Collides"), false);
			bool bHasPhysics = cString::ToBool(pPhysicsElem->Attribute("HasPhysics"), false);
			tString sMaterial = cString::ToString(pPhysicsElem->Attribute("Material"), "Default");

			bool bStaticMeshCollider = cString::ToBool(pPhysicsElem->Attribute("StaticMeshCollider"), false);
			tString sColliderMesh = cString::ToString(pPhysicsElem->Attribute("ColliderMesh"), "");

			// Check if this entity should have a body.
			if (bCollides) {
				if (bStaticMeshCollider) {
					cSubMesh *pSubMesh = mpMesh->GetSubMeshName(sColliderMesh);
					if (pSubMesh) {
						iCollideShape *pShape = apWorld->GetPhysicsWorld()->CreateMeshShape(
							pSubMesh->GetVertexBuffer());
						pBody = apWorld->GetPhysicsWorld()->CreateBody(sColliderMesh, pShape);
					} else {
						Error("Couldn't find sub mesh '%s' to create collision mesh\n", sColliderMesh.c_str());
					}
				} else {
					if (mpMesh->GetColliderNum() > 0) {
						pBody = apWorld->GetPhysicsWorld()->CreateBody(asName,
																	   mpMesh->CreateCollideShape(apWorld->GetPhysicsWorld()));
					} else {
						Warning("No collider found for '%s'\n", asFileName.c_str());
					}
				}

				if (pBody) {
					iPhysicsMaterial *pMaterial = apWorld->GetPhysicsWorld()->GetMaterialFromName(sMaterial);
					if (pMaterial) {
						pBody->SetMaterial(pMaterial);
					} else {
						Error("Physics Material '%s' for body '%s' in mesh '%s' doesn't exist!\n",
							  sMaterial.c_str(), pBody->GetName().c_str(), mpMesh->GetName().c_str());
					}

					SetBodyProperties(pBody, pPhysicsElem);

					if (bHasPhysics) {
					} else {
						pBody->SetMass(0);
					}

					pBody->CreateNode()->AddEntity(mpEntity);
					// cMatrixf mtxTemp = a_mtxTransform;
					// Log("-- Body: %s Mtx: %s\n", pBody->GetName().c_str(),
					//	mtxTemp.ToString().c_str());
					// pBody->GetBV()->GetSize().ToString().c_str());

					pBody->SetMatrix(a_mtxTransform);

					mpEntity->SetBody(pBody);

					mvBodies.push_back(pBody);
				}
			}
		}
	}

	////////////////////////////////////////
	// Create lights
	for (int i = 0; i < mpMesh->GetLightNum(); i++) {
		iLight3D *pLight = mpMesh->CreateLightInWorld(asName, mpMesh->GetLight(i), mpEntity, apWorld);
		if (pLight)
			mvLights.push_back(pLight);
	}

	// Iterate light elements
	TiXmlElement *pLightElem = apRootElem->FirstChildElement("LIGHT");
	for (; pLightElem != NULL; pLightElem = pLightElem->NextSiblingElement("LIGHT")) {
		tString sName = cString::ToString(pLightElem->Attribute("Name"), "");

		iLight3D *pLight = (iLight3D *)STLFindByName(mvLights, asName + "_" + sName);
		if (pLight == NULL) {
			Error("Couldn't find light %s among entity %s type: %s lights\n", sName.c_str(), asName.c_str(), asFileName.c_str());
			continue;
		}

		pLight->SetFarAttenuation(cString::ToFloat(pLightElem->Attribute("Attenuation"),
												   pLight->GetFarAttenuation()));
		pLight->SetDiffuseColor(cString::ToColor(pLightElem->Attribute("Color"),
												 pLight->GetDiffuseColor()));
		pLight->SetCastShadows(cString::ToBool(pLightElem->Attribute("CastShadows"),
											   pLight->GetCastShadows()));

		if (pLight->GetLightType() == eLight3DType_Spot) {
			cLight3DSpot *pSpotLight = static_cast<cLight3DSpot *>(pLight);

			pSpotLight->SetFOV(cString::ToFloat(pLightElem->Attribute("FOV"),
												pSpotLight->GetFOV()));
			pSpotLight->SetAspect(cString::ToFloat(pLightElem->Attribute("Aspect"),
												   pSpotLight->GetAspect()));
		}
	}

	////////////////////////////////////////
	// Create billboards
	for (int i = 0; i < mpMesh->GetBillboardNum(); i++) {
		cBillboard *pBill = mpMesh->CreateBillboardInWorld(asName, mpMesh->GetBillboard(i), mpEntity, apWorld);
		if (pBill)
			mvBillboards.push_back(pBill);
	}

	////////////////////////////////////////
	// Create beams
	for (int i = 0; i < mpMesh->GetBeamNum(); i++) {
		cBeam *pBeam = mpMesh->CreateBeamInWorld(asName, mpMesh->GetBeam(i), mpEntity, apWorld);
		if (pBeam)
			mvBeams.push_back(pBeam);
	}

	////////////////////////////////////////
	// Create particle systems
	for (int i = 0; i < mpMesh->GetParticleSystemNum(); i++) {
		cParticleSystem3D *pPS = mpMesh->CreateParticleSystemInWorld(asName, mpMesh->GetParticleSystem(i), mpEntity, apWorld);
		if (pPS)
			mvParticleSystems.push_back(pPS);
	}

	////////////////////////////////////////
	// Create sound entities
	for (int i = 0; i < mpMesh->GetSoundEntityNum(); i++) {
		cSoundEntity *pSound = mpMesh->CreateSoundEntityInWorld(asName, mpMesh->GetSoundEntity(i), mpEntity, apWorld);
		if (pSound)
			mvSoundEntities.push_back(pSound);
	}

	////////////////////////////////////////
	// ATTACH BILLBOARDS
	TiXmlElement *pAttachElem = apRootElem->FirstChildElement("ATTACH_BILLBOARDS");
	if (pAttachElem != NULL) {
		TiXmlElement *pPairElem = pAttachElem->FirstChildElement();
		for (; pPairElem != NULL; pPairElem = pPairElem->NextSiblingElement()) {
			tString sLight = asName + "_" + cString::ToString(pPairElem->Attribute("Light"), "");
			tString sBillboard = asName + "_" + cString::ToString(pPairElem->Attribute("Billboard"), "");

			iLight3D *pLight = apWorld->GetLight(sLight);
			if (pLight == NULL) {
				Warning("Couldn't find light '%s'\n", sLight.c_str());
				continue;
			}

			cBillboard *pBillboard = apWorld->GetBillboard(sBillboard);
			if (pBillboard == NULL) {
				Warning("Couldn't find billboard '%s'\n", sBillboard.c_str());
				continue;
			}
			pLight->AttachBillboard(pBillboard);
		}
	}

	////////////////////////////////////////
	// Set matrix on entity if there are no bodies.
	if ((mvBodies.size() <= 0 || bUsingNodeBodies) && mpEntity->GetBody() == NULL) {
		mpEntity->SetMatrix(a_mtxTransform);

		// to make sure everything is in place.
		mpEntity->UpdateLogic(0);
	}

	// Play animation if there is any.
	if (mpEntity->GetAnimationStateNum() > 0) {
		cAnimationState *pAnimState = mpEntity->GetAnimationState(0);

		pAnimState->SetActive(true);
		pAnimState->SetLoop(bAnimationLoop);
		if (bAnimationRandStart)
			pAnimState->SetTimePosition(cMath::RandRectf(0, pAnimState->GetLength()));
		else
			pAnimState->SetTimePosition(0);
	} else if (mpMesh->GetSkeleton()) {
		mpEntity->SetSkeletonPhysicsActive(true);
	}

	// After load virtual call.
	// This is where the user adds extra stuff.
	AfterLoad(apRootElem, a_mtxTransform, apWorld);

	////////////////////////////////////////
	// Create references
	if (abLoadReference) {
		cMeshEntity *pEntityCopy = mpEntity;
		cMesh *pMeshCopy = mpMesh;
		for (int i = 0; i < pMeshCopy->GetReferenceNum(); i++) {
			/*iEntity3D *pRef = */ mpMesh->CreateReferenceInWorld(asName, pMeshCopy->GetReference(i), pEntityCopy,
																  apWorld, a_mtxTransform);
			// if(pPS) mvParticleSystems.push_back(pPS);
		}
		return pEntityCopy;
	} else {
		return mpEntity;
	}
}

//-----------------------------------------------------------------------

void cEntityLoader_Object::SetBodyProperties(iPhysicsBody *apBody, TiXmlElement *apPhysicsElem) {
	float fMass = cString::ToFloat(apPhysicsElem->Attribute("Mass"), 1);
	tString sInertiaVec = cString::ToString(apPhysicsElem->Attribute("InertiaScale"), "1 1 1");

	float fAngluarDamping = cString::ToFloat(apPhysicsElem->Attribute("AngularDamping"), 0.1f);
	float fLinearDamping = cString::ToFloat(apPhysicsElem->Attribute("LinearDamping"), 0.1f);

	bool bBlocksSound = cString::ToBool(apPhysicsElem->Attribute("BlocksSound"), false);
	bool bCollideCharacter = cString::ToBool(apPhysicsElem->Attribute("CollideCharacter"), true);
	bool bCollide = cString::ToBool(apPhysicsElem->Attribute("CollideNonCharacter"), true);

	bool bHasGravity = cString::ToBool(apPhysicsElem->Attribute("HasGravity"), true);

	float fMaxAngluarSpeed = cString::ToFloat(apPhysicsElem->Attribute("MaxAngluarSpeed"), 0);
	float fMaxLinearSpeed = cString::ToFloat(apPhysicsElem->Attribute("MaxLinearSpeed"), 0);

	bool bContinuousCollision = cString::ToBool(apPhysicsElem->Attribute("ContinuousCollision"), true);

	bool bPushedByCharacterGravity = cString::ToBool(apPhysicsElem->Attribute("PushedByCharacterGravity"), false);
#if 0
	float fAutoDisableLinearThreshold = cString::ToFloat(apPhysicsElem->Attribute("AutoDisableLinearThreshold"), 0.1f);
	float fAutoDisableAngularThreshold = cString::ToFloat(apPhysicsElem->Attribute("AutoDisableAngularThreshold"), 0.1f);
	int lAutoDisableNumSteps = cString::ToInt(apPhysicsElem->Attribute("AutoDisableNumSteps"), 10);
#endif
	bool bVolatile = cString::ToBool(apPhysicsElem->Attribute("Volatile"), false);

	bool bCanAttachCharacter = cString::ToBool(apPhysicsElem->Attribute("CanAttachCharacter"), false);

	tFloatVec vInertiaScale;
	cString::GetFloatVec(sInertiaVec, vInertiaScale);

	apBody->SetMass(fMass);
	apBody->SetAngularDamping(fAngluarDamping);
	apBody->SetLinearDamping(fLinearDamping);
	apBody->SetBlocksSound(bBlocksSound);
	apBody->SetCollideCharacter(bCollideCharacter);
	apBody->SetCollide(bCollide);
	apBody->SetGravity(bHasGravity);

	apBody->SetVolatile(bVolatile);

	apBody->SetCanAttachCharacter(bCanAttachCharacter);

	apBody->SetContinuousCollision(bContinuousCollision);
#if 0
	apBody->SetAutoDisableLinearThreshold(fAutoDisableLinearThreshold);
	apBody->SetAutoDisableAngularThreshold(fAutoDisableAngularThreshold);
	apBody->SetAutoDisableNumSteps(lAutoDisableNumSteps);
#endif
	apBody->SetPushedByCharacterGravity(bPushedByCharacterGravity);

	// Log("Body %s contin: %d\n",apBody->GetName().c_str(), apBody->GetContinuousCollision()?1:0);

	apBody->SetMaxAngularSpeed(fMaxAngluarSpeed);
	apBody->SetMaxLinearSpeed(fMaxLinearSpeed);
}

//-----------------------------------------------------------------------

void cEntityLoader_Object::SetJointProperties(iPhysicsJoint *pJoint, TiXmlElement *pJointElem,
											  cWorld3D *apWorld) {
	float fMinValue = cString::ToFloat(pJointElem->Attribute("MinValue"), -1);
	float fMaxValue = cString::ToFloat(pJointElem->Attribute("MaxValue"), -1);

	tString sMoveSound = cString::ToString(pJointElem->Attribute("MoveSound"), "");
	float fMinMoveSpeed = cString::ToFloat(pJointElem->Attribute("MinMoveSpeed"), 0.5f);
	float fMinMoveFreq = cString::ToFloat(pJointElem->Attribute("MinMoveFreq"), 0.9f);
	float fMinMoveVolume = cString::ToFloat(pJointElem->Attribute("MinMoveVolume"), 0.3f);
	float fMinMoveFreqSpeed = cString::ToFloat(pJointElem->Attribute("MinMoveFreqSpeed"), 0.9f);
	float fMaxMoveFreq = cString::ToFloat(pJointElem->Attribute("MaxMoveFreq"), 1.1f);
	float fMaxMoveVolume = cString::ToFloat(pJointElem->Attribute("MaxMoveVolume"), 1.0f);
	float fMaxMoveFreqSpeed = cString::ToFloat(pJointElem->Attribute("MaxMoveFreqSpeed"), 1.1f);
	float fMiddleMoveSpeed = cString::ToFloat(pJointElem->Attribute("MiddleMoveSpeed"), 1.0f);
	float fMiddleMoveVolume = cString::ToFloat(pJointElem->Attribute("MiddleMoveVolume"), 1.0f);
	tString sMoveType = cString::ToString(pJointElem->Attribute("MoveType"), "Linear");
	sMoveType = cString::ToLowerCase(sMoveType);

	float fStickyMinDistance = cString::ToFloat(pJointElem->Attribute("StickyMinDistance"), 0.0f);
	float fStickyMaxDistance = cString::ToFloat(pJointElem->Attribute("StickyMaxDistance"), 0.0f);

	bool bBreakable = cString::ToBool(pJointElem->Attribute("Breakable"), false);
	tString sBreakSound = cString::ToString(pJointElem->Attribute("BreakSound"), "");
	float fBreakForce = cString::ToFloat(pJointElem->Attribute("BreakForce"), 1000);

	bool bLimitAutoSleep = cString::ToBool(pJointElem->Attribute("LimitAutoSleep"), false);
	float fLimitAutoSleepDist = cString::ToFloat(pJointElem->Attribute("LimitAutoSleepDist"), 0.02f);
	int lLimitAutoSleepNumSteps = cString::ToInt(pJointElem->Attribute("LimitAutoSleepNumSteps"), 10);

	pJoint->SetMoveSound(sMoveSound);
	pJoint->SetMinMoveSpeed(fMinMoveSpeed);
	pJoint->SetMinMoveFreq(fMinMoveFreq);
	pJoint->SetMinMoveVolume(fMinMoveVolume);
	pJoint->SetMinMoveFreqSpeed(fMinMoveFreqSpeed);
	pJoint->SetMaxMoveFreq(fMaxMoveFreq);
	pJoint->SetMaxMoveVolume(fMaxMoveVolume);
	pJoint->SetMaxMoveFreqSpeed(fMaxMoveFreqSpeed);
	pJoint->SetMiddleMoveSpeed(fMiddleMoveSpeed);
	pJoint->SetMiddleMoveVolume(fMiddleMoveVolume);
	pJoint->SetMoveSpeedType(sMoveType == "angular" ? ePhysicsJointSpeed_Angular : ePhysicsJointSpeed_Linear);

	pJoint->GetMaxLimit()->msSound = cString::ToString(pJointElem->Attribute("MaxLimit_Sound"), "");
	pJoint->GetMaxLimit()->mfMaxSpeed = cString::ToFloat(pJointElem->Attribute("MaxLimit_MaxSpeed"), 10.0f);
	pJoint->GetMaxLimit()->mfMinSpeed = cString::ToFloat(pJointElem->Attribute("MaxLimit_MinSpeed"), 20.0f);

	pJoint->GetMinLimit()->msSound = cString::ToString(pJointElem->Attribute("MinLimit_Sound"), "");
	pJoint->GetMinLimit()->mfMaxSpeed = cString::ToFloat(pJointElem->Attribute("MinLimit_MaxSpeed"), 10.0f);
	pJoint->GetMinLimit()->mfMinSpeed = cString::ToFloat(pJointElem->Attribute("MinLimit_MinSpeed"), 20.0f);

	pJoint->SetStickyMinDistance(fStickyMinDistance);
	pJoint->SetStickyMaxDistance(fStickyMaxDistance);

	pJoint->SetBreakable(bBreakable);
	pJoint->SetBreakForce(fBreakForce);
	pJoint->SetBreakSound(sBreakSound);

	pJoint->SetLimitAutoSleep(bLimitAutoSleep);
	pJoint->SetLimitAutoSleepDist(fLimitAutoSleepDist);
	pJoint->SetLimitAutoSleepNumSteps(lLimitAutoSleepNumSteps);

	// Set min/max values
	if (fMaxValue >= 0 && fMinValue >= 0) {
		switch (pJoint->GetType()) {
		case ePhysicsJointType_Ball: {
			iPhysicsJointBall *pBallJoint = static_cast<iPhysicsJointBall *>(pJoint);
			pBallJoint->SetConeLimits(pBallJoint->GetPinDir(), cMath::ToRad(fMinValue), cMath::ToRad(fMaxValue));
			break;
		}
		case ePhysicsJointType_Hinge: {
			iPhysicsJointHinge *pHingeJoint = static_cast<iPhysicsJointHinge *>(pJoint);
			pHingeJoint->SetMaxAngle(cMath::ToRad(fMaxValue));
			pHingeJoint->SetMinAngle(cMath::ToRad(-fMinValue));
			break;
		}
		case ePhysicsJointType_Screw: {
			iPhysicsJointScrew *pScrewJoint = static_cast<iPhysicsJointScrew *>(pJoint);
			pScrewJoint->SetMinDistance(-fMinValue / 100);
			pScrewJoint->SetMaxDistance(fMaxValue / 100);
			break;
		}
		case ePhysicsJointType_Slider: {
			iPhysicsJointSlider *pSliderJoint = static_cast<iPhysicsJointSlider *>(pJoint);
			pSliderJoint->SetMinDistance(-fMinValue / 100);
			pSliderJoint->SetMaxDistance(fMaxValue / 100);
			break;
		}
		default:
			break;
		}
	}

	// Load all controllers
	TiXmlElement *pControllerElem = pJointElem->FirstChildElement("Controller");
	for (; pControllerElem != NULL; pControllerElem = pControllerElem->NextSiblingElement("Controller")) {
		LoadController(pJoint, apWorld->GetPhysicsWorld(), pControllerElem);
	}
}

//-----------------------------------------------------------------------

ePhysicsControllerType GetControllerType(const char *apString) {
	if (apString == NULL)
		return ePhysicsControllerType_LastEnum;

	tString sName = apString;

	if (sName == "Pid")
		return ePhysicsControllerType_Pid;
	else if (sName == "Spring")
		return ePhysicsControllerType_Spring;

	return ePhysicsControllerType_LastEnum;
}

/////////////////////////

static ePhysicsControllerInput GetControllerInput(const char *apString) {
	if (apString == NULL)
		return ePhysicsControllerInput_LastEnum;

	tString sName = apString;

	if (sName == "JointAngle")
		return ePhysicsControllerInput_JointAngle;
	else if (sName == "JointDist")
		return ePhysicsControllerInput_JointDist;
	else if (sName == "LinearSpeed")
		return ePhysicsControllerInput_LinearSpeed;
	else if (sName == "AngularSpeed")
		return ePhysicsControllerInput_AngularSpeed;

	return ePhysicsControllerInput_LastEnum;
}

/////////////////////////

static ePhysicsControllerOutput GetControllerOutput(const char *apString) {
	if (apString == NULL)
		return ePhysicsControllerOutput_LastEnum;

	tString sName = apString;

	if (sName == "Force")
		return ePhysicsControllerOutput_Force;
	else if (sName == "Torque")
		return ePhysicsControllerOutput_Torque;

	return ePhysicsControllerOutput_LastEnum;
}

/////////////////////////

static ePhysicsControllerAxis GetControllerAxis(const char *apString) {
	if (apString == NULL)
		return ePhysicsControllerAxis_LastEnum;

	tString sName = apString;

	if (sName == "X")
		return ePhysicsControllerAxis_X;
	else if (sName == "Y")
		return ePhysicsControllerAxis_Y;
	else if (sName == "Z")
		return ePhysicsControllerAxis_Z;

	return ePhysicsControllerAxis_LastEnum;
}

/////////////////////////

static ePhysicsControllerEnd GetControllerEnd(const char *apString) {
	if (apString == NULL)
		return ePhysicsControllerEnd_Null;

	tString sName = apString;

	if (sName == "OnMax")
		return ePhysicsControllerEnd_OnMax;
	else if (sName == "OnMin")
		return ePhysicsControllerEnd_OnMin;
	else if (sName == "OnDest")
		return ePhysicsControllerEnd_OnDest;

	return ePhysicsControllerEnd_Null;
}

/////////////////////////

void cEntityLoader_Object::LoadController(iPhysicsJoint *apJoint, iPhysicsWorld *apPhysicsWorld,
										  TiXmlElement *apElem) {
	//////////////////////////////
	// Get the properties
	tString sName = cString::ToString(apElem->Attribute("Name"), "");
	bool bActive = cString::ToBool(apElem->Attribute("Active"), false);

	ePhysicsControllerType CtrlType = GetControllerType(apElem->Attribute("Type"));
	float fA = cString::ToFloat(apElem->Attribute("A"), 0);
	float fB = cString::ToFloat(apElem->Attribute("B"), 0);
	float fC = cString::ToFloat(apElem->Attribute("C"), 0);
	int lIntegralSize = cString::ToInt(apElem->Attribute("IntegralSize"), 1);

	ePhysicsControllerInput CtrlInput = GetControllerInput(apElem->Attribute("Input"));
	ePhysicsControllerAxis CtrlInputAxis = GetControllerAxis(apElem->Attribute("InputAxis"));
	float fDestValue = cString::ToFloat(apElem->Attribute("DestValue"), 0);
	float fMaxOutput = cString::ToFloat(apElem->Attribute("MaxOutput"), 0);

	ePhysicsControllerOutput CtrlOutput = GetControllerOutput(apElem->Attribute("Output"));
	ePhysicsControllerAxis CtrlOutputAxis = GetControllerAxis(apElem->Attribute("OutputAxis"));
	bool bMulMassWithOutput = cString::ToBool(apElem->Attribute("MulMassWithOutput"), false);

	ePhysicsControllerEnd CtrlEnd = GetControllerEnd(apElem->Attribute("EndType"));
	tString sNextCtrl = cString::ToString(apElem->Attribute("NextController"), "");

	bool bLogInfo = cString::ToBool(apElem->Attribute("LogInfo"), false);

	// Convert degrees to radians.
	if (CtrlInput == ePhysicsControllerInput_JointAngle) {
		fDestValue = cMath::ToRad(fDestValue);
	}

	//////////////////////////////
	// Create the controller
	iPhysicsController *pController = apPhysicsWorld->CreateController(sName);

	pController->SetType(CtrlType);

	pController->SetA(fA);
	pController->SetB(fB);
	pController->SetC(fC);

	pController->SetPidIntegralSize(lIntegralSize);

	pController->SetActive(bActive);
	pController->SetInputType(CtrlInput, CtrlInputAxis);
	pController->SetDestValue(fDestValue);

	pController->SetOutputType(CtrlOutput, CtrlOutputAxis);
	pController->SetMaxOutput(fMaxOutput);
	pController->SetMulMassWithOutput(bMulMassWithOutput);

	pController->SetEndType(CtrlEnd);
	pController->SetNextController(sNextCtrl);

	pController->SetLogInfo(bLogInfo);

	apJoint->AddController(pController);

	// Log("Controller: %s active: %d val: %f %f %f input: %d %d output: %d %d\n",
	//	sName.c_str(),bActive, fA,fB,fC, (int)CtrlInput, (int)CtrlInputAxis, (int)CtrlOutput,(int)CtrlOutputAxis);
}

//-----------------------------------------------------------------------

eAnimationEventType cEntityLoader_Object::GetAnimationEventType(const char *apString) {
	if (apString == NULL)
		return eAnimationEventType_LastEnum;

	tString sName = apString;
	sName = cString::ToLowerCase(sName);

	if (sName == "playsound") {
		return eAnimationEventType_PlaySound;
	}

	Warning("Animation event type '%s' does not exist!\n", apString);
	return eAnimationEventType_LastEnum;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
} // namespace hpl
