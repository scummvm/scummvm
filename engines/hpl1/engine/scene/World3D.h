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

#ifndef HPL_WORLD3D_H
#define HPL_WORLD3D_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

#include "hpl1/engine/scene/Entity3D.h"
#include "hpl1/engine/scene/SoundSource.h"

#include "hpl1/engine/game/GameTypes.h"

#include "hpl1/engine/game/SaveGame.h"

class TiXmlElement;

namespace hpl {

class cGraphics;
class cResources;
class cSound;
class cPhysics;
class cScene;
class cSystem;
class cAI;

class iCamera;
class cCamera3D;
class cNode3D;
class cLight3DSpot;
class cLight3DPoint;
class iLight3D;
class cImageEntity;
class cParticleManager;
class cParticleSystem3D;
class iScript;
class cPortalContainer;
class iRenderableContainer;
class cMeshEntity;
class cMesh;
class cBillboard;
class cBeam;
class iPhysicsWorld;
class cColliderEntity;
class iPhysicsBody;
class cSoundEntity;
class cAINodeContainer;
class cAStarHandler;
class cAINodeGeneratorParams;

typedef Common::List<iLight3D *> tLight3DList;
typedef Common::List<iLight3D *>::iterator tLight3DListIt;

typedef Common::List<cMeshEntity *> tMeshEntityList;
typedef Common::List<cMeshEntity *>::iterator tMeshEntityListIt;

typedef Common::List<cBillboard *> tBillboardList;
typedef Common::List<cBillboard *>::iterator tBillboardListIt;

typedef Common::List<cBeam *> tBeamList;
typedef Common::List<cBeam *>::iterator tBeamListIt;

typedef Common::List<cParticleSystem3D *> tParticleSystem3DList;
typedef tParticleSystem3DList::iterator tParticleSystem3DListIt;

typedef Common::List<cColliderEntity *> tColliderEntityList;
typedef Common::List<cColliderEntity *>::iterator tColliderEntityListIt;

typedef Common::List<cSoundEntity *> tSoundEntityList;
typedef Common::List<cSoundEntity *>::iterator tSoundEntityListIt;

typedef Common::List<cAINodeContainer *> tAINodeContainerList;
typedef Common::List<cAINodeContainer *>::iterator tAINodeContainerListIt;

typedef Common::List<cAStarHandler *> tAStarHandlerList;
typedef Common::List<cAStarHandler *>::iterator tAStarHandlerIt;

//-------------------------------------------------------------------

typedef cSTLIterator<cMeshEntity *, tMeshEntityList, tMeshEntityListIt> cMeshEntityIterator;
typedef cSTLIterator<cBillboard *, tBillboardList, tBillboardListIt> cBillboardIterator;
typedef cSTLIterator<iLight3D *, tLight3DList, tLight3DListIt> cLight3DListIterator;
typedef cSTLIterator<cParticleSystem3D *, tParticleSystem3DList, tParticleSystem3DListIt> cParticleSystem3DIterator;
typedef cSTLIterator<cSoundEntity *, tSoundEntityList, tSoundEntityListIt> cSoundEntityIterator;
typedef cSTLIterator<cBeam *, tBeamList, tBeamListIt> cBeamIterator;

//-------------------------------------------------------------------

class cTempAiNode {
public:
	cTempAiNode(const cVector3f &avPos, const tString &asName) : mvPos(avPos), msName(asName) {}
	cVector3f mvPos;
	tString msName;
};

typedef Common::List<cTempAiNode> tTempAiNodeList;
typedef Common::List<cTempAiNode>::iterator tTempAiNodeListIt;

class cTempNodeContainer {
public:
	tString msName;
	tTempAiNodeList mlstNodes;
};

typedef Common::StableMap<tString, cTempNodeContainer *> tTempNodeContainerMap;
typedef Common::StableMap<tString, cTempNodeContainer *>::iterator tTempNodeContainerMapIt;

//-------------------------------------------------------------------

class cAreaEntity : public iSerializable {
	kSerializableClassInit(cAreaEntity) public : tString msName;
	tString msType;
	cMatrixf m_mtxTransform;
	cVector3f mvSize;
};

typedef Common::StableMap<tString, cAreaEntity *> tAreaEntityMap;
typedef tAreaEntityMap::iterator tAreaEntityMapIt;

//-------------------------------------------------------------------

class cStartPosEntity : public iSerializable {
	kSerializableClassInit(cStartPosEntity) public : cStartPosEntity() {}
	cStartPosEntity(const tString &asName) : msName(asName) {}

	cMatrixf &GetWorldMatrix() { return m_mtxTransform; }
	cMatrixf &GetLocalMatrix() { return m_mtxTransform; }
	void SetMatrix(const cMatrixf &a_mtxTrans) { m_mtxTransform = a_mtxTrans; }

	tString &GetName() { return msName; }

	cMatrixf m_mtxTransform;
	tString msName;
};

typedef Common::List<cStartPosEntity *> tStartPosEntityList;
typedef Common::List<cStartPosEntity *>::iterator tStartPosEntityListIt;

//------------------------------------------

kSaveData_BaseClass(cWorld3D) {
	kSaveData_ClassInit(cWorld3D) public : cContainerList<cStartPosEntity> mlstStartpos;
	cContainerList<cAreaEntity> mlstAreaEntities;
	cContainerList<cScriptVar> mlstScriptVars;

	virtual iSaveObject *CreateSaveObject(cSaveObjectHandler * apSaveObjectHandler, cGame * apGame);
	virtual int GetSaveCreatePrio();
};

//-------------------------------------------------------------------

class cWorld3D {
public:
	cWorld3D(tString asName, cGraphics *apGraphics, cResources *apResources, cSound *apSound,
			 cPhysics *apPhysics, cScene *apScene, cSystem *apSystem, cAI *apAI);
	~cWorld3D();

	tString GetName() { return msName; }

	bool CreateFromFile(tString asFile);

	void SetFileName(const tString &asFile) { msFileName = asFile; }
	const tString &GetFileName() { return msFileName; }

	void Update(float afTimeStep);

	void PreUpdate(float afTotalTime, float afTimeStep);

	cVector3f GetWorldSize() { return mvWorldSize; }

	iScript *GetScript() { return mpScript; }
	void SetScript(iScript *apScript) { mpScript = apScript; }

	iRenderableContainer *GetRenderContainer();
	cPortalContainer *GetPortalContainer();

	cPhysics *GetPhysics() { return mpPhysics; }
	cResources *GetResources() { return mpResources; }
	cSound *GetSound() { return mpSound; }
	cSystem *GetSystem() { return mpSystem; }

	iEntity3D *CreateEntity(const tString &asName, const cMatrixf &a_mtxTransform,
							const tString &asFile, bool abLoadReferences);
	/**
	 * Call this when all things have been added to set up things like physics world size.
	 **/
	void SetUpData();

	void AddSaveData(cSaveDataHandler *apHandler);

	///// PHYSICS ////////////////////////////////

	void SetPhysicsWorld(iPhysicsWorld *apWorld, bool abAutoDelete = true);
	iPhysicsWorld *GetPhysicsWorld();

	///// AREA ////////////////////////////////

	cAreaEntity *CreateAreaEntity(const tString &asName);
	cAreaEntity *GetAreaEntity(const tString &asName);
	tAreaEntityMap *GetAreaEntityMap() { return &m_mapAreaEntities; }

	///// MESH ENTITY METHODS ////////////////////

	cMeshEntity *CreateMeshEntity(const tString &asName, cMesh *apMesh, bool abAddToContainer = true);
	void DestroyMeshEntity(cMeshEntity *apMesh);
	cMeshEntity *GetMeshEntity(const tString &asName);

	cMeshEntityIterator GetMeshEntityIterator();

	void DrawMeshBoundingBoxes(const cColor &aColor, bool abStatic);

	///// LIGHT METHODS ////////////////////

	cLight3DPoint *CreateLightPoint(const tString &asName = "", bool abAddToContainer = true);
	cLight3DSpot *CreateLightSpot(const tString &asName = "", const tString &asGobo = "",
								  bool abAddToContainer = true);
	void DestroyLight(iLight3D *apLight);
	iLight3D *GetLight(const tString &asName);

	tLight3DList *GetLightList() { return &mlstLights; }

	cLight3DListIterator GetLightIterator() { return cLight3DListIterator(&mlstLights); }

	///// BILLBOARD METHODS ////////////////////

	cBillboard *CreateBillboard(const tString &asName, const cVector2f &avSize,
								const tString &asMaterial = "",
								bool abAddToContainer = true, cMatrixf *apTransform = NULL);
	void DestroyBillboard(cBillboard *apObject);
	cBillboard *GetBillboard(const tString &asName);
	cBillboardIterator GetBillboardIterator();

	///// BEAM METHODS ////////////////////

	cBeam *CreateBeam(const tString &asName);
	void DestroyBeam(cBeam *apObject);
	cBeam *GetBeam(const tString &asName);
	cBeamIterator GetBeamIterator();

	///// PARTICLE METHODS ////////////////////

	cParticleSystem3D *CreateParticleSystem(const tString &asName, const tString &asType,
											const cVector3f &avSize, const cMatrixf &a_mtxTransform);
	void DestroyParticleSystem(cParticleSystem3D *apPS);
	cParticleSystem3D *GetParticleSystem(const tString &asName);
	bool ParticleSystemExists(cParticleSystem3D *apPS);

	cParticleSystem3DIterator GetParticleSystemIterator() { return cParticleSystem3DIterator(&mlstParticleSystems); }

	///// COllIDER METHODS ////////////////////

	cColliderEntity *CreateColliderEntity(const tString &asName, iPhysicsBody *apBody);
	void DestroyColliderEntity(cColliderEntity *apCollider);
	cColliderEntity *GetColliderEntity(const tString &asName);

	///// SOUND ENTITY METHODS ////////////////////

	cSoundEntity *CreateSoundEntity(const tString &asName, const tString &asSoundEntity,
									bool abRemoveWhenOver);
	void DestroySoundEntity(cSoundEntity *apEntity);
	cSoundEntity *GetSoundEntity(const tString &asName);
	void DestroyAllSoundEntities();
	bool SoundEntityExists(cSoundEntity *apEntity);

	cSoundEntityIterator GetSoundEntityIterator() { return cSoundEntityIterator(&mlstSoundEntities); }

	///// START POS ENTITY METHODS ////////////////

	cStartPosEntity *CreateStartPos(const tString &asName);
	cStartPosEntity *GetStartPosEntity(const tString &asName);
	cStartPosEntity *GetFirstStartPosEntity();

	///// AI NODE METHODS ////////////////

	void GenerateAINodes(cAINodeGeneratorParams *apParams);

	cAINodeContainer *CreateAINodeContainer(const tString &asName,
											const tString &asNodeName,
											const cVector3f &avSize,
											bool abNodeIsAtCenter,
											int alMinEdges, int alMaxEdges, float afMaxEdgeDistance,
											float afMaxHeight);

	cAStarHandler *CreateAStarHandler(cAINodeContainer *apContainer);

	void AddAINode(const tString &asName, const tString &asType, const cVector3f &avPosition);
	tTempAiNodeList *GetAINodeList(const tString &asType);

	/// NODE METHODS //////////////////////
	// Remove this for the time being, not need it seems.
	// cNode3D* GetRootNode(){ return mpRootNode; }

private:
	iSaveData *CreateSaveData();

	// Update
	void UpdateEntities(float afTimeStep);
	void UpdateBodies(float afTimeStep);
	void UpdateParticles(float afTimeStep);
	void UpdateLights(float afTimeStep);
	void UpdateSoundEntities(float afTimeStep);

	tString msName;
	tString msFileName;
	cGraphics *mpGraphics;
	cSound *mpSound;
	cResources *mpResources;
	cPhysics *mpPhysics;
	cScene *mpScene;
	cSystem *mpSystem;
	cAI *mpAI;

	iPhysicsWorld *mpPhysicsWorld;
	bool mbAutoDeletePhysicsWorld;

	cVector3f mvWorldSize;

	cPortalContainer *mpPortalContainer;

	iScript *mpScript;

	tLight3DList mlstLights;
	tMeshEntityList mlstMeshEntities;
	tBillboardList mlstBillboards;
	tBeamList mlstBeams;
	tParticleSystem3DList mlstParticleSystems;
	tColliderEntityList mlstColliders;
	tSoundEntityList mlstSoundEntities;
	tStartPosEntityList mlstStartPosEntities;
	tAreaEntityMap m_mapAreaEntities;

	tAINodeContainerList mlstAINodeContainers;
	tAStarHandlerList mlstAStarHandlers;
	tTempNodeContainerMap m_mapTempNodes;

	cNode3D *mpRootNode;

	tString msMapName;
	cColor mAmbientColor;
};

} // namespace hpl

#endif // HPL_WOLRD3D_H
