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

#include "hpl1/engine/scene/World3D.h"

#include "hpl1/engine/impl/tinyXML/tinyxml.h"

#include "hpl1/engine/system/Script.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/math/MathTypes.h"

#include "hpl1/engine/game/Game.h"

#include "hpl1/engine/graphics/Beam.h"
#include "hpl1/engine/graphics/BillBoard.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/Mesh.h"
#include "hpl1/engine/graphics/ParticleEmitter3D.h"
#include "hpl1/engine/graphics/ParticleSystem3D.h"
#include "hpl1/engine/graphics/Renderer3D.h"

#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/MaterialManager.h"
#include "hpl1/engine/resources/ParticleManager.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/ScriptManager.h"
#include "hpl1/engine/resources/SoundEntityManager.h"
#include "hpl1/engine/resources/TextureManager.h"

#include "hpl1/engine/scene/Camera.h"
#include "hpl1/engine/scene/ColliderEntity.h"
#include "hpl1/engine/scene/Light3DPoint.h"
#include "hpl1/engine/scene/Light3DSpot.h"
#include "hpl1/engine/scene/MeshEntity.h"
#include "hpl1/engine/scene/Node3D.h"
#include "hpl1/engine/scene/PortalContainer.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/SoundEntity.h"
#include "hpl1/engine/scene/SoundSource.h"

#include "hpl1/engine/system/System.h"

#include "hpl1/engine/sound/Sound.h"
#include "hpl1/engine/sound/SoundEntityData.h"
#include "hpl1/engine/sound/SoundHandler.h"

#include "hpl1/engine/physics/Physics.h"
#include "hpl1/engine/physics/PhysicsBody.h"
#include "hpl1/engine/physics/PhysicsWorld.h"

#include "hpl1/engine/ai/AI.h"
#include "hpl1/engine/ai/AINodeContainer.h"
#include "hpl1/engine/ai/AINodeGenerator.h"
#include "hpl1/engine/ai/AStar.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cWorld3D::cWorld3D(tString asName, cGraphics *apGraphics, cResources *apResources, cSound *apSound,
				   cPhysics *apPhysics, cScene *apScene, cSystem *apSystem, cAI *apAI) {
	mpGraphics = apGraphics;
	mpResources = apResources;
	mpSound = apSound;
	mpPhysics = apPhysics;
	mpScene = apScene;
	mpSystem = apSystem;
	mpAI = apAI;

	mpRootNode = hplNew(cNode3D, ());

	mpScript = NULL;

	msName = asName;

	mAmbientColor = cColor(0, 0);

	mpPortalContainer = hplNew(cPortalContainer, ());

	mpPhysicsWorld = NULL;
	mbAutoDeletePhysicsWorld = false;

	msFileName = "";
}

//-----------------------------------------------------------------------

cWorld3D::~cWorld3D() {
	STLDeleteAll(mlstMeshEntities);
	STLDeleteAll(mlstLights);
	STLDeleteAll(mlstBillboards);
	STLDeleteAll(mlstBeams);
	STLDeleteAll(mlstColliders);
	STLDeleteAll(mlstParticleSystems);
	STLDeleteAll(mlstStartPosEntities);
	STLMapDeleteAll(m_mapAreaEntities);

	STLDeleteAll(mlstAINodeContainers);
	STLDeleteAll(mlstAStarHandlers);
	STLMapDeleteAll(m_mapTempNodes);

	if (mpScript) {
		mpResources->GetScriptManager()->Destroy(mpScript);
	}

	if (mpPhysicsWorld && mbAutoDeletePhysicsWorld)
		mpPhysics->DestroyWorld(mpPhysicsWorld);

	// So that bodies can stop sound entities on destruction.
	STLDeleteAll(mlstSoundEntities);

	hplDelete(mpPortalContainer);

	hplDelete(mpRootNode);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWorld3D::Update(float afTimeStep) {
	START_TIMING(Physics);
	if (mpPhysicsWorld)
		mpPhysicsWorld->Update(afTimeStep);
	STOP_TIMING(Physics);

	START_TIMING(Entities);
	UpdateEntities(afTimeStep);
	STOP_TIMING(Entities);

	START_TIMING(Bodies);
	UpdateBodies(afTimeStep);
	STOP_TIMING(Bodies);

	START_TIMING(Particles);
	UpdateParticles(afTimeStep);
	STOP_TIMING(Particles);

	START_TIMING(Lights);
	UpdateLights(afTimeStep);
	STOP_TIMING(Lights);

	START_TIMING(SoundEntities);
	UpdateSoundEntities(afTimeStep);
	STOP_TIMING(SoundEntities);
}

//-----------------------------------------------------------------------

void cWorld3D::PreUpdate(float afTotalTime, float afTimeStep) {
	mpSound->GetSoundHandler()->SetSilent(true);

	while (afTotalTime > 0) {
		if (mpPhysicsWorld)
			mpPhysicsWorld->Update(afTimeStep);
		UpdateParticles(afTimeStep);

		afTotalTime -= afTimeStep;
	}

	mpSound->GetSoundHandler()->SetSilent(false);
}

//-----------------------------------------------------------------------

iRenderableContainer *cWorld3D::GetRenderContainer() {
	return mpPortalContainer;
}

cPortalContainer *cWorld3D::GetPortalContainer() {
	return mpPortalContainer;
}

//-----------------------------------------------------------------------

void cWorld3D::SetPhysicsWorld(iPhysicsWorld *apWorld, bool abAutoDelete) {
	mpPhysicsWorld = apWorld;
	mbAutoDeletePhysicsWorld = abAutoDelete;
	if (mpPhysicsWorld)
		mpPhysicsWorld->SetWorld3D(this);
}

iPhysicsWorld *cWorld3D::GetPhysicsWorld() {
	return mpPhysicsWorld;
}

//-----------------------------------------------------------------------

static void CheckMinMaxUpdate(cVector3f &avMin, cVector3f &avMax,
							  const cVector3f &avLocalMin, const cVector3f &avLocalMax) {
	if (avMin.x > avLocalMin.x)
		avMin.x = avLocalMin.x;
	if (avMax.x < avLocalMax.x)
		avMax.x = avLocalMax.x;

	if (avMin.y > avLocalMin.y)
		avMin.y = avLocalMin.y;
	if (avMax.y < avLocalMax.y)
		avMax.y = avLocalMax.y;

	if (avMin.z > avLocalMin.z)
		avMin.z = avLocalMin.z;
	if (avMax.z < avLocalMax.z)
		avMax.z = avLocalMax.z;
}

void cWorld3D::SetUpData() {
	mpPortalContainer->Compile();

	tSectorMap *pSectorMap = mpPortalContainer->GetSectorMap();

	//////////////////////////////////////////////
	// Get world size by getting sector BV
	if (pSectorMap->size() > 0) {
		tSectorMapIt SectorIt = pSectorMap->begin();
		cSector *pSector = SectorIt->second;

		cVector3f vMin = pSector->GetBV()->GetMin();
		cVector3f vMax = pSector->GetBV()->GetMax();
		++SectorIt;
		for (; SectorIt != pSectorMap->end(); ++SectorIt) {
			pSector = SectorIt->second;

			cVector3f vLocalMin = pSector->GetBV()->GetMin();
			cVector3f vLocalMax = pSector->GetBV()->GetMax();

			CheckMinMaxUpdate(vMin, vMax, vLocalMin, vLocalMax);
		}

		// Log("World Min: (%s) Max: (%s)\n",vMin.ToString().c_str(), vMax.ToString().c_str());

		// Create a 10 m border around the world aswell:
		vMin = vMin - cVector3f(10, 10, 10);
		vMax = vMax + cVector3f(10, 10, 10);

		if (mpPhysicsWorld)
			mpPhysicsWorld->SetWorldSize(vMin, vMax);
	}
	//////////////////////////////////////////////
	// Get world size by getting global objects BV
	else {
		cVector3f vMin = cVector3f(10000, 10000, 100000);
		cVector3f vMax = cVector3f(-10000, -10000, -100000);

		// Dynamic
		tRenderableSet *pRenSet = mpPortalContainer->GetGlobalDynamicObjectSet();
		tRenderableSetIt DynIt = pRenSet->begin();
		for (; DynIt != pRenSet->end(); ++DynIt) {
			iRenderable *pObject = *DynIt;
			cVector3f vLocalMin = pObject->GetBoundingVolume()->GetMin();
			cVector3f vLocalMax = pObject->GetBoundingVolume()->GetMax();

			CheckMinMaxUpdate(vMin, vMax, vLocalMin, vLocalMax);
		}

		// Static
		tRenderableList *pRenList = mpPortalContainer->GetGlobalStaticObjectList();
		tRenderableListIt StaticIt = pRenList->begin();
		for (; StaticIt != pRenList->end(); ++StaticIt) {
			iRenderable *pObject = *StaticIt;
			cVector3f vLocalMin = pObject->GetBoundingVolume()->GetMin();
			cVector3f vLocalMax = pObject->GetBoundingVolume()->GetMax();

			CheckMinMaxUpdate(vMin, vMax, vLocalMin, vLocalMax);
		}

		vMin = vMin - cVector3f(10, 10, 10);
		vMax = vMax + cVector3f(10, 10, 10);
		if (mpPhysicsWorld)
			mpPhysicsWorld->SetWorldSize(vMin, vMax);
	}
}

//-----------------------------------------------------------------------

void cWorld3D::AddSaveData(cSaveDataHandler *apHandler) {
	///////////////////////////////
	// Save mesh entities
	tMeshEntityListIt MeshIt = mlstMeshEntities.begin();
	for (; MeshIt != mlstMeshEntities.end(); MeshIt++) {
		cMeshEntity *pEntity = *MeshIt;

		if (pEntity->IsSaved()) {
			iSaveData *pData = pEntity->CreateSaveData();
			pEntity->SaveToSaveData(pData);
			apHandler->Add(pData);
		}
	}

	///////////////////////////////
	// Save billboards
	tBillboardListIt BillIt = mlstBillboards.begin();
	for (; BillIt != mlstBillboards.end(); BillIt++) {
		cBillboard *pEntity = *BillIt;

		if (pEntity->IsSaved()) {
			iSaveData *pData = pEntity->CreateSaveData();
			pEntity->SaveToSaveData(pData);
			apHandler->Add(pData);
		}
	}

	///////////////////////////////
	// Save lights
	tLight3DListIt LightIt = mlstLights.begin();
	for (; LightIt != mlstLights.end(); LightIt++) {
		iLight3D *pEntity = *LightIt;

		if (pEntity->IsSaved()) {
			iSaveData *pData = pEntity->CreateSaveData();
			pEntity->SaveToSaveData(pData);
			apHandler->Add(pData);
		}
	}

	///////////////////////////////
	// Save sounds entities
	tSoundEntityListIt SoundIt = mlstSoundEntities.begin();
	for (; SoundIt != mlstSoundEntities.end(); SoundIt++) {
		cSoundEntity *pEntity = *SoundIt;

		if (pEntity->IsSaved()) {
			iSaveData *pData = pEntity->CreateSaveData();
			pEntity->SaveToSaveData(pData);
			apHandler->Add(pData);
		}
	}

	///////////////////////////////
	// Save particle systems
	tParticleSystem3DListIt PSIt = mlstParticleSystems.begin();
	for (; PSIt != mlstParticleSystems.end(); PSIt++) {
		cParticleSystem3D *pEntity = *PSIt;

		if (pEntity->IsSaved()) {
			iSaveData *pData = pEntity->CreateSaveData();
			pEntity->SaveToSaveData(pData);
			apHandler->Add(pData);
		}
	}

	///////////////////////////////
	// Add world 3d data
	apHandler->Add(CreateSaveData());
}
//-----------------------------------------------------------------------

cAreaEntity *cWorld3D::CreateAreaEntity(const tString &asName) {
	cAreaEntity *pArea = hplNew(cAreaEntity, ());
	pArea->msName = asName;
	m_mapAreaEntities.insert(tAreaEntityMap::value_type(asName, pArea));
	return pArea;
}

cAreaEntity *cWorld3D::GetAreaEntity(const tString &asName) {
	tAreaEntityMapIt it = m_mapAreaEntities.find(asName);
	if (it == m_mapAreaEntities.end())
		return NULL;

	return it->second;
}

//-----------------------------------------------------------------------

iEntity3D *cWorld3D::CreateEntity(const tString &asName, const cMatrixf &a_mtxTransform,
								  const tString &asFile, bool abLoadReferences) {
	iEntity3D *pEntity = NULL;
	tString sFileName = cString::SetFileExt(asFile, "ent");

	tString sPath = mpResources->GetFileSearcher()->GetFilePath(sFileName);

	if (sPath != "") {
		TiXmlDocument *pEntityDoc = hplNew(TiXmlDocument, ());
		if (pEntityDoc->LoadFile(sPath.c_str()) == false) {
			Error("Couldn't load '%s'!\n", sPath.c_str());
		} else {
			TiXmlElement *pRootElem = pEntityDoc->FirstChildElement();
			TiXmlElement *pMainElem = pRootElem->FirstChildElement("MAIN");

			tString sType = cString::ToString(pMainElem->Attribute("Type"), "");

			iEntity3DLoader *pLoader = mpResources->GetEntity3DLoader(sType);

			if (pLoader) {
				pEntity = pLoader->Load(asName, pRootElem, a_mtxTransform, this, sFileName, abLoadReferences);
				pEntity->SetSourceFile(sFileName);
			} else {
				Error("Couldn't find loader for type '%s' in file '%s'\n", sType.c_str(), sFileName.c_str());
			}
		}
		hplDelete(pEntityDoc);
	} else {
		Error("Entity file '%s' was not found!\n", sFileName.c_str());
	}

	return pEntity;
}

//-----------------------------------------------------------------------

cMeshEntity *cWorld3D::CreateMeshEntity(const tString &asName, cMesh *apMesh, bool abAddToContainer) {
	cMeshEntity *pMesh = hplNew(cMeshEntity, (asName, apMesh, mpResources->GetMaterialManager(),
											  mpResources->GetMeshManager(), mpResources->GetAnimationManager()));
	mlstMeshEntities.push_back(pMesh);

	if (abAddToContainer)
		mpPortalContainer->Add(pMesh, false);

	pMesh->SetWorld(this);

	return pMesh;
}

//-----------------------------------------------------------------------

void cWorld3D::DestroyMeshEntity(cMeshEntity *apMesh) {
	if (apMesh == NULL)
		return;

	tMeshEntityListIt It = mlstMeshEntities.begin();
	for (; It != mlstMeshEntities.end(); ++It) {
		if (apMesh == *It) {
			mlstMeshEntities.erase(It);
			break;
		}
	}

	mpPortalContainer->Remove(apMesh);

	hplDelete(apMesh);
}

//-----------------------------------------------------------------------

cMeshEntity *cWorld3D::GetMeshEntity(const tString &asName) {
	tMeshEntityListIt It = mlstMeshEntities.begin();
	for (; It != mlstMeshEntities.end(); ++It) {
		if ((*It)->GetName() == asName) {
			return *It;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------

cMeshEntityIterator cWorld3D::GetMeshEntityIterator() {
	return cMeshEntityIterator(&mlstMeshEntities);
}

//-----------------------------------------------------------------------

void cWorld3D::DrawMeshBoundingBoxes(const cColor &aColor, bool abStatic) {
	tMeshEntityListIt It = mlstMeshEntities.begin();
	for (; It != mlstMeshEntities.end(); ++It) {
		cMeshEntity *pEntity = *It;

		if (abStatic == false && pEntity->IsStatic())
			continue;

		cBoundingVolume *pBV = pEntity->GetBoundingVolume();
		mpGraphics->GetLowLevel()->DrawBoxMaxMin(pBV->GetMax(), pBV->GetMin(), aColor);
	}
}

//-----------------------------------------------------------------------

cLight3DPoint *cWorld3D::CreateLightPoint(const tString &asName, bool abAddToContainer) {
	cLight3DPoint *pLight = hplNew(cLight3DPoint, (asName, mpResources));
	mlstLights.push_back(pLight);

	if (abAddToContainer)
		mpPortalContainer->Add(pLight, false);

	pLight->SetWorld3D(this);

	return pLight;
}

//-----------------------------------------------------------------------

cLight3DSpot *cWorld3D::CreateLightSpot(const tString &asName, const tString &asGobo,
										bool abAddToContainer) {
	cLight3DSpot *pLight = hplNew(cLight3DSpot, (asName, mpResources));
	mlstLights.push_back(pLight);

	if (asGobo != "") {
		iTexture *pTexture = mpResources->GetTextureManager()->Create2D(asGobo, true);
		if (pTexture != NULL)
			pLight->SetTexture(pTexture);
		else
			Warning("Couldn't load texture '%s' for light '%s'", asGobo.c_str(), asName.c_str());
	}

	if (abAddToContainer)
		mpPortalContainer->Add(pLight, false);

	pLight->SetWorld3D(this);

	return pLight;
}

//-----------------------------------------------------------------------

void cWorld3D::DestroyLight(iLight3D *apLight) {
	mpPortalContainer->Remove(apLight);
	STLFindAndDelete(mlstLights, apLight);
}

//-----------------------------------------------------------------------

iLight3D *cWorld3D::GetLight(const tString &asName) {
	tLight3DListIt LightIt = mlstLights.begin();
	for (; LightIt != mlstLights.end(); ++LightIt) {
		if ((*LightIt)->GetName() == asName) {
			return *LightIt;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------

cBillboard *cWorld3D::CreateBillboard(const tString &asName, const cVector2f &avSize,
									  const tString &asMaterial,
									  bool abAddToContainer, cMatrixf *apTransform) {
	cBillboard *pBillboard = hplNew(cBillboard, (asName, avSize, mpResources, mpGraphics));
	mlstBillboards.push_back(pBillboard);

	if (apTransform)
		pBillboard->SetMatrix(*apTransform);
	if (asMaterial != "") {
		iMaterial *pMat = mpResources->GetMaterialManager()->CreateMaterial(asMaterial);
		pBillboard->SetMaterial(pMat);
	}

	if (abAddToContainer)
		mpPortalContainer->Add(pBillboard, false);

	return pBillboard;
}
//-----------------------------------------------------------------------

void cWorld3D::DestroyBillboard(cBillboard *apObject) {
	mpPortalContainer->Remove(apObject);

	STLFindAndDelete(mlstBillboards, apObject);
}

//-----------------------------------------------------------------------

cBillboard *cWorld3D::GetBillboard(const tString &asName) {
	return (cBillboard *)STLFindByName(mlstBillboards, asName);
}

//-----------------------------------------------------------------------

cBillboardIterator cWorld3D::GetBillboardIterator() {
	return cBillboardIterator(&mlstBillboards);
}

//-----------------------------------------------------------------------

cBeam *cWorld3D::CreateBeam(const tString &asName) {
	cBeam *pBeam = hplNew(cBeam, (asName, mpResources, mpGraphics));
	mlstBeams.push_back(pBeam);

	mpPortalContainer->Add(pBeam, false);

	return pBeam;
}
//-----------------------------------------------------------------------

void cWorld3D::DestroyBeam(cBeam *apObject) {
	mpPortalContainer->Remove(apObject);

	STLFindAndDelete(mlstBeams, apObject);
}

//-----------------------------------------------------------------------

cBeam *cWorld3D::GetBeam(const tString &asName) {
	return (cBeam *)STLFindByName(mlstBeams, asName);
}

//-----------------------------------------------------------------------

cBeamIterator cWorld3D::GetBeamIterator() {
	return cBeamIterator(&mlstBeams);
}

//-----------------------------------------------------------------------

cParticleSystem3D *cWorld3D::CreateParticleSystem(const tString &asName, const tString &asType,
												  const cVector3f &avSize, const cMatrixf &a_mtxTransform) {
	cParticleSystem3D *pPS = mpResources->GetParticleManager()->CreatePS3D(asName, asType,
																		   avSize, a_mtxTransform);
	if (pPS == NULL) {
		Error("Couldn't create particle system '%s' of type '%s'\n", asName.c_str(), asType.c_str());
		return NULL;
	}

	// Log("Created particle system '%s' of type '%s'\n",asName.c_str(), asType.c_str());

	// Add the emitters contained in the system.
	// Do not add the system itself.
	for (int i = 0; i < pPS->GetEmitterNum(); ++i) {
		iParticleEmitter3D *pPE = pPS->GetEmitter(i);

		mpPortalContainer->Add(pPE, false);
		pPE->SetWorld(this);
	}

	mlstParticleSystems.push_back(pPS);

	// Log("Created particle system '%s'\n",asType.c_str());

	return pPS;
}

//-----------------------------------------------------------------------

void cWorld3D::DestroyParticleSystem(cParticleSystem3D *apPS) {
	if (apPS == NULL)
		return;

	for (int i = 0; i < apPS->GetEmitterNum(); ++i) {
		iParticleEmitter3D *pPE = apPS->GetEmitter(i);

		mpPortalContainer->Remove(pPE);
	}

	STLFindAndDelete(mlstParticleSystems, apPS);
}

//-----------------------------------------------------------------------

cParticleSystem3D *cWorld3D::GetParticleSystem(const tString &asName) {
	return (cParticleSystem3D *)STLFindByName(mlstParticleSystems, asName);
}

//-----------------------------------------------------------------------

bool cWorld3D::ParticleSystemExists(cParticleSystem3D *apPS) {
	tParticleSystem3DListIt it = mlstParticleSystems.begin();
	for (; it != mlstParticleSystems.end(); ++it) {
		if (apPS == *it)
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------

cColliderEntity *cWorld3D::CreateColliderEntity(const tString &asName, iPhysicsBody *apBody) {
	cColliderEntity *pCollider = hplNew(cColliderEntity, (asName, apBody, mpPhysicsWorld));

	mlstColliders.push_back(pCollider);

	return pCollider;
}
void cWorld3D::DestroyColliderEntity(cColliderEntity *apCollider) {
	STLFindAndDelete(mlstColliders, apCollider);
}
cColliderEntity *cWorld3D::GetColliderEntity(const tString &asName) {
	return (cColliderEntity *)STLFindByName(mlstColliders, asName);
}

//-----------------------------------------------------------------------

cSoundEntity *cWorld3D::CreateSoundEntity(const tString &asName, const tString &asSoundEntity,
										  bool abRemoveWhenOver) {
	cSoundEntityData *pData = mpResources->GetSoundEntityManager()->CreateSoundEntity(asSoundEntity);
	if (pData == NULL) {
		Error("Cannot find sound entity '%s'\n", asSoundEntity.c_str());
		return NULL;
	}

	cSoundEntity *pSound = hplNew(cSoundEntity, (asName, pData,
												 mpResources->GetSoundEntityManager(),
												 this,
												 mpSound->GetSoundHandler(), abRemoveWhenOver));

	mlstSoundEntities.push_back(pSound);

	return pSound;
}

void cWorld3D::DestroySoundEntity(cSoundEntity *apEntity) {
	// STLFindAndDelete(mlstSoundEntities,apEntity);

	tSoundEntityListIt it = mlstSoundEntities.begin();
	for (; it != mlstSoundEntities.end(); ++it) {
		cSoundEntity *pSound = *it;
		if (pSound == apEntity) {
			mlstSoundEntities.erase(it);
			hplDelete(pSound);
			break;
		}
	}
}

void cWorld3D::DestroyAllSoundEntities() {
	// Make sure no body has any sound entity
	if (mpPhysicsWorld) {
		cPhysicsBodyIterator bodyIt = mpPhysicsWorld->GetBodyIterator();
		while (bodyIt.HasNext()) {
			iPhysicsBody *pBody = bodyIt.Next();
			pBody->SetScrapeSoundEntity(NULL);
			pBody->SetRollSoundEntity(NULL);
		}

		cPhysicsJointIterator jointIt = mpPhysicsWorld->GetJointIterator();
		while (jointIt.HasNext()) {
			iPhysicsJoint *pJoint = jointIt.Next();
			pJoint->SetSound(NULL);
		}
	}

	// Destroy all sound entities
	STLDeleteAll(mlstSoundEntities);
	mlstSoundEntities.clear();
}

cSoundEntity *cWorld3D::GetSoundEntity(const tString &asName) {
	return (cSoundEntity *)STLFindByName(mlstSoundEntities, asName);
}

bool cWorld3D::SoundEntityExists(cSoundEntity *apEntity) {
	tSoundEntityListIt it = mlstSoundEntities.begin();
	tSoundEntityListIt end = mlstSoundEntities.end();
	for (; it != end; ++it) {
		if (*it == apEntity)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------

cStartPosEntity *cWorld3D::CreateStartPos(const tString &asName) {
	cStartPosEntity *pStartPos = hplNew(cStartPosEntity, (asName));

	mlstStartPosEntities.push_back(pStartPos);

	return pStartPos;
}

cStartPosEntity *cWorld3D::GetStartPosEntity(const tString &asName) {
	return (cStartPosEntity *)STLFindByName(mlstStartPosEntities, asName);
}

cStartPosEntity *cWorld3D::GetFirstStartPosEntity() {
	if (mlstStartPosEntities.empty())
		return NULL;

	return mlstStartPosEntities.front();
}

//-----------------------------------------------------------------------

void cWorld3D::GenerateAINodes(cAINodeGeneratorParams *apParams) {
	mpAI->GetNodeGenerator()->Generate(this, apParams);
}

//-----------------------------------------------------------------------

cAINodeContainer *cWorld3D::CreateAINodeContainer(const tString &asName,
												  const tString &asNodeName,
												  const cVector3f &avSize,
												  bool abNodeIsAtCenter,
												  int alMinEdges, int alMaxEdges, float afMaxEdgeDistance, float afMaxHeight) {
	cAINodeContainer *pContainer = NULL;

	// unsigned long lStartTime = mpSystem->GetLowLevel()->GetTime();

	//////////////////////////////////
	// See if the container is allready loaded.
	tAINodeContainerListIt it = mlstAINodeContainers.begin();
	for (; it != mlstAINodeContainers.end(); ++it) {
		cAINodeContainer *pCont = *it;
		if (pCont->GetName() == asName) {
			pContainer = pCont;
		}
	}

	//////////////////////////////////
	// Get file name
	cFileSearcher *pFileSearcher = mpResources->GetFileSearcher();
	tString sMapPath = pFileSearcher->GetFilePath(GetFileName());

	tString sAiFileName = cString::SetFileExt(sMapPath, "");
	sAiFileName += "_" + asName;
	sAiFileName = cString::SetFileExt(sAiFileName, "nodes");

	//////////////////////////////////
	// If there is no container created, create it.
	if (pContainer == NULL) {
		tTempNodeContainerMapIt ContIt = m_mapTempNodes.find(asNodeName);
		if (ContIt == m_mapTempNodes.end()) {
			Warning("AI node type '%s' does not exist!\n", asNodeName.c_str());
			return NULL;
		}
		cTempNodeContainer *pTempCont = ContIt->second;

		pContainer = hplNew(cAINodeContainer, (asName, asNodeName, this, avSize));
		mlstAINodeContainers.push_back(pContainer);

		// Set properties
		pContainer->SetMinEdges(alMinEdges);
		pContainer->SetMaxEdges(alMaxEdges);
		pContainer->SetMaxEdgeDistance(afMaxEdgeDistance);
		pContainer->SetMaxHeight(afMaxHeight);
		pContainer->SetNodeIsAtCenter(abNodeIsAtCenter);

		// Reserve space for the incoming nodes.
		pContainer->ReserveSpace(pTempCont->mlstNodes.size());

		// Add nodes to container
		tTempAiNodeListIt NodeIt = pTempCont->mlstNodes.begin();
		for (; NodeIt != pTempCont->mlstNodes.end(); ++NodeIt) {
			cTempAiNode &pNode = *NodeIt;
			pContainer->AddNode(pNode.msName, pNode.mvPos, NULL);
		}

		bool bLoadedFromFile = false;
		if (FileExists(cString::To16Char(sAiFileName))) {
			cDate dateMapFile = FileModifiedDate(cString::To16Char(sMapPath));
			cDate dateAIFile = FileModifiedDate(cString::To16Char(sAiFileName));

			if (dateAIFile > dateMapFile) {
				bLoadedFromFile = true;
				pContainer->LoadFromFile(sAiFileName);
			}
		}

		if (bLoadedFromFile == false) {
			Log("Rebuilding node connections and saving to '%s'\n", sAiFileName.c_str());

			// Compile
			pContainer->Compile();

			// Save to disk
			pContainer->SaveToFile(sAiFileName);
		}
	}

	// unsigned long lTime = mpSystem->GetLowLevel()->GetTime() - lStartTime;
	// Log("Creating ai nodes took: %d\n",lTime);

	return pContainer;
}

//-----------------------------------------------------------------------

cAStarHandler *cWorld3D::CreateAStarHandler(cAINodeContainer *apContainer) {
	cAStarHandler *pAStar = hplNew(cAStarHandler, (apContainer));

	mlstAStarHandlers.push_back(pAStar);

	return pAStar;
}

//-----------------------------------------------------------------------

void cWorld3D::AddAINode(const tString &asName, const tString &asType, const cVector3f &avPosition) {
	cTempNodeContainer *pContainer = NULL;
	tTempNodeContainerMapIt it = m_mapTempNodes.find(asType);
	if (it != m_mapTempNodes.end()) {
		pContainer = it->second;
	}

	if (pContainer == NULL) {
		pContainer = hplNew(cTempNodeContainer, ());
		m_mapTempNodes.insert(tTempNodeContainerMap::value_type(asType, pContainer));
	}

	pContainer->mlstNodes.push_back(cTempAiNode(avPosition, asName));
}

//-----------------------------------------------------------------------

tTempAiNodeList *cWorld3D::GetAINodeList(const tString &asType) {
	cTempNodeContainer *pContainer = NULL;
	tTempNodeContainerMapIt it = m_mapTempNodes.find(asType);
	if (it != m_mapTempNodes.end()) {
		pContainer = it->second;
	}

	if (pContainer == NULL) {
		pContainer = hplNew(cTempNodeContainer, ());
		m_mapTempNodes.insert(tTempNodeContainerMap::value_type(asType, pContainer));
	}

	return &pContainer->mlstNodes;
}

//-----------------------------------------------------------------------

bool cWorld3D::CreateFromFile(tString asFile) {
	return false;
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWorld3D::UpdateParticles(float afTimeStep) {
	tParticleSystem3DListIt it = mlstParticleSystems.begin();

	while (it != mlstParticleSystems.end()) {
		cParticleSystem3D *pPS = *it;

		// Debug:
		// for(int i=0; i<100;++i) pPS->UpdateLogic(afTimeStep / 100.0f);
		pPS->UpdateLogic(afTimeStep);

		// Check if the system is alive, else destroy
		if (pPS->IsDead()) {
			// Log("Deleting particle system '%s'", pPS->GetName().c_str());

			it = mlstParticleSystems.erase(it);
			for (int i = 0; i < pPS->GetEmitterNum(); ++i) {
				mpPortalContainer->Remove(pPS->GetEmitter(i));
			}
			hplDelete(pPS);
		} else {
			it++;
		}
	}
}
//-----------------------------------------------------------------------

void cWorld3D::UpdateEntities(float afTimeStep) {
	tMeshEntityListIt MeshIt = mlstMeshEntities.begin();
	for (; MeshIt != mlstMeshEntities.end(); MeshIt++) {
		cMeshEntity *pEntity = *MeshIt;

		if (pEntity->IsActive()) {
			// bool bTime = cString::GetLastStringPos(pEntity->GetName(), "infected")>=0;
			// if(bTime) START_TIMING_EX(pEntity->GetName().c_str(),entity);

			// Debug:
			// for(int i=0; i<100;++i) pEntity->UpdateLogic(afTimeStep / 100.0f);
			pEntity->UpdateLogic(afTimeStep);

			// if(bTime) STOP_TIMING(entity);
		}
	}
}

//-----------------------------------------------------------------------

void cWorld3D::UpdateBodies(float afTimeStep) {
}

//-----------------------------------------------------------------------

void cWorld3D::UpdateLights(float afTimeStep) {
	tLight3DListIt it = mlstLights.begin();

	while (it != mlstLights.end()) {
		iLight3D *pLight = *it;

		if (pLight->IsActive())
			pLight->UpdateLogic(afTimeStep);

		++it;
	}
}

//-----------------------------------------------------------------------

void cWorld3D::UpdateSoundEntities(float afTimeStep) {
	tSoundEntityListIt it = mlstSoundEntities.begin();

	while (it != mlstSoundEntities.end()) {
		cSoundEntity *pSound = *it;

		if (pSound->IsActive()) {
			// Debug:
			// for(int i=0; i<100;++i) pSound->UpdateLogic(afTimeStep / 100.0f);
			pSound->UpdateLogic(afTimeStep);
		}

		// Check if the system is stopped, else destroy
		if (pSound->IsStopped() && pSound->GetRemoveWhenOver()) {
			it = mlstSoundEntities.erase(it);
			hplDelete(pSound);
		} else {
			it++;
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------

kBeginSerializeBase(cAreaEntity)
	kSerializeVar(msName, eSerializeType_String)
		kSerializeVar(msType, eSerializeType_String)
			kSerializeVar(m_mtxTransform, eSerializeType_Matrixf)
				kSerializeVar(mvSize, eSerializeType_Vector3f)
					kEndSerialize()

	//-------------------------------------------------------------------

	kBeginSerializeBase(cStartPosEntity)
		kSerializeVar(msName, eSerializeType_String)
			kSerializeVar(m_mtxTransform, eSerializeType_Matrixf)
				kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cSaveData_cWorld3D)
		kSerializeClassContainer(mlstStartpos, cStartPosEntity, eSerializeType_Class)
			kSerializeClassContainer(mlstAreaEntities, cAreaEntity, eSerializeType_Class)
				kSerializeClassContainer(mlstScriptVars, cScriptVar, eSerializeType_Class)
					kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject *cSaveData_cWorld3D::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	cWorld3D *pWorld = apGame->GetScene()->GetWorld3D();

	///////////////////////
	// Start pos
	cContainerListIterator<cStartPosEntity> StartIt = mlstStartpos.GetIterator();
	while (StartIt.HasNext()) {
		cStartPosEntity &tempStart = StartIt.Next();
		cStartPosEntity *pStart = pWorld->CreateStartPos(tempStart.GetName());
		pStart->SetMatrix(tempStart.GetWorldMatrix());
	}

	///////////////////////
	// Area entities
	cContainerListIterator<cAreaEntity> AreaIt = mlstAreaEntities.GetIterator();
	while (AreaIt.HasNext()) {
		cAreaEntity &tempArea = AreaIt.Next();
		cAreaEntity *pArea = pWorld->CreateAreaEntity(tempArea.msName);
		pArea->m_mtxTransform = tempArea.m_mtxTransform;
		pArea->msType = tempArea.msType;
		pArea->mvSize = tempArea.mvSize;
	}

	///////////////////////
	// Script vars
	cContainerListIterator<cScriptVar> VarIt = mlstScriptVars.GetIterator();
	while (VarIt.HasNext()) {
		cScriptVar &tempVar = VarIt.Next();
		cScriptVar *pVar = apGame->GetScene()->CreateLocalVar(tempVar.msName);
		pVar->mlVal = tempVar.mlVal;
	}

	return NULL;
}

//-----------------------------------------------------------------------

int cSaveData_cWorld3D::GetSaveCreatePrio() {
	return 4;
}

//-----------------------------------------------------------------------

iSaveData *cWorld3D::CreateSaveData() {
	cSaveData_cWorld3D *pData = hplNew(cSaveData_cWorld3D, ());

	// Start pos
	tStartPosEntityListIt StartIt = mlstStartPosEntities.begin();
	for (; StartIt != mlstStartPosEntities.end(); ++StartIt) {
		pData->mlstStartpos.Add(*(*StartIt));
	}

	// Area entities
	tAreaEntityMapIt AreaIt = m_mapAreaEntities.begin();
	for (; AreaIt != m_mapAreaEntities.end(); ++AreaIt) {
		pData->mlstAreaEntities.Add(*(AreaIt->second));
	}

	// Local scripts
	tScriptVarMap *pLocalVarMap = mpScene->GetLocalVarMap();
	tScriptVarMapIt VarIt = pLocalVarMap->begin();
	for (; VarIt != pLocalVarMap->end(); ++VarIt) {
		pData->mlstScriptVars.Add(VarIt->second);
	}

	return pData;
}

//-----------------------------------------------------------------------

} // namespace hpl
