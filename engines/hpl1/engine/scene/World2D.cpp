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

#include "hpl1/engine/scene/World2D.h"

#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/ImageEntityData.h"
#include "hpl1/engine/graphics/Mesh2d.h"
#include "hpl1/engine/impl/tinyXML/tinyxml.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/physics/Body2D.h"
#include "hpl1/engine/physics/Collider2D.h"
#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/ParticleManager.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/ScriptManager.h"
#include "hpl1/engine/resources/TileSetManager.h"
#include "hpl1/engine/scene/Area2D.h"
#include "hpl1/engine/scene/Camera.h"
#include "hpl1/engine/scene/GridMap2D.h"
#include "hpl1/engine/scene/ImageEntity.h"
#include "hpl1/engine/scene/Light2DPoint.h"
#include "hpl1/engine/scene/Node2D.h"
#include "hpl1/engine/scene/SoundSource.h"
#include "hpl1/engine/scene/TileMap.h"
#include "hpl1/engine/system/MemoryManager.h"
#include "hpl1/engine/system/Script.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/graphics/Renderer2D.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cWorld2D::cWorld2D(tString asName, cGraphics *apGraphics, cResources *apResources, cSound *apSound, cCollider2D *apCollider) {
	mpGraphics = apGraphics;
	mpResources = apResources;
	mpSound = apSound;
	mpCollider = apCollider;

	mpRootNode = hplNew(cNode2D, ());
	mpMapLights = NULL;
	mpMapImageEntities = NULL;
	mpMapBodies = NULL;
	mpTileMap = NULL;

	mpScript = NULL;

	msName = asName;

	mfLightZ = 10;
	mAmbientColor = cColor(0, 0);

	mlBodyIDCount = 0;
}

//-----------------------------------------------------------------------

cWorld2D::~cWorld2D() {
	if (mpTileMap)
		hplDelete(mpTileMap);

	if (mpMapLights)
		hplDelete(mpMapLights);
	if (mpMapImageEntities)
		hplDelete(mpMapImageEntities);
	if (mpMapBodies)
		hplDelete(mpMapBodies);
	if (mpMapParticles)
		hplDelete(mpMapParticles);
	if (mpMapAreas)
		hplDelete(mpMapAreas);

	tSoundSourceListIt it = mlstSoundSources.begin();
	while (it != mlstSoundSources.end()) {
		hplDelete(*it);
		it++;
	}
	mlstSoundSources.clear();

	if (mpScript) {
		mpResources->GetScriptManager()->Destroy(mpScript);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWorld2D::Render(cCamera2D *apCamera) {
	mpTileMap->Render(apCamera);
	RenderImagesEntities(apCamera);
	RenderParticles(apCamera);
}

//-----------------------------------------------------------------------

void cWorld2D::Update(float afTimeStep) {
	UpdateEntities();
	UpdateBodies();
	UpdateParticles();
	UpdateSoundSources();
	UpdateLights();
}

//-----------------------------------------------------------------------

cLight2DPoint *cWorld2D::CreateLightPoint(tString asName) {
	if (mpMapLights == NULL)
		return NULL;

	cLight2DPoint *pLight = hplNew(cLight2DPoint, (asName));

	mpMapLights->AddEntity(pLight);

	// Add the light to the base node awell...

	return pLight;
}

//-----------------------------------------------------------------------

void cWorld2D::DestroyLight(iLight2D *apLight) {
	if (mpMapLights == NULL)
		return;

	mpMapLights->RemoveEntity(apLight);
	hplDelete(apLight);
}

//-----------------------------------------------------------------------

cGridMap2D *cWorld2D::GetGridMapLights() {
	return mpMapLights;
}

//-----------------------------------------------------------------------

iLight2D *cWorld2D::GetLight(const tString &asName) {
	tGrid2DObjectMap *pGridMap = mpMapLights->GetAllMap();
	tGrid2DObjectMapIt it = pGridMap->begin();
	for (; it != pGridMap->end(); it++) {
		cGrid2DObject *pObj = it->second;
		if (pObj->GetEntity()->GetName() == asName) {
			return static_cast<iLight2D *>(pObj->GetEntity());
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------

cSoundSource *cWorld2D::CreateSoundSource(const tString &asName, const tString &asSoundName,
										  bool abVolatile) {
	cSoundSource *pSoundSource = hplNew(cSoundSource, (asName, asSoundName, mpSound, abVolatile));
	mlstSoundSources.push_back(pSoundSource);

	return pSoundSource;
}

//-----------------------------------------------------------------------

void cWorld2D::DestroySoundSource(cSoundSource *apSound) {
	mlstSoundSources.remove(apSound);
	hplDelete(apSound);
}

//-----------------------------------------------------------------------

cImageEntity *cWorld2D::CreateImageEntity(tString asName, tString asDataName) {
	cImageEntity *pEntity = hplNew(cImageEntity, (asName, mpResources, mpGraphics));

	if (pEntity == NULL)
		return NULL;

	if (pEntity->LoadEntityData(asDataName)) {
		mpMapImageEntities->AddEntity(pEntity);
	} else {
		hplDelete(pEntity);
	}

	return pEntity;
}

//-----------------------------------------------------------------------

void cWorld2D::DestroyImageEntity(cImageEntity *apEntity) {
	if (mpMapImageEntities == NULL)
		return;

	mpMapImageEntities->RemoveEntity(apEntity);
	hplDelete(apEntity);
}

//-----------------------------------------------------------------------

cImageEntity *cWorld2D::GetImageEntity(const tString &asName) {
	tGrid2DObjectMap *pGridMap = mpMapImageEntities->GetAllMap();
	tGrid2DObjectMapIt it = pGridMap->begin();
	for (; it != pGridMap->end(); it++) {
		cGrid2DObject *pObj = it->second;
		if (pObj->GetEntity()->GetName() == asName) {
			return static_cast<cImageEntity *>(pObj->GetEntity());
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------

iParticleSystem2D *cWorld2D::CreateParticleSystem(const tString &asName, const cVector3f &avSize) {
	/*iParticleSystem2D* pPS = mpResources->GetParticleManager()->CreatePS2D(asName, avSize);
	if(pPS == NULL){
		Error("Couldn't create particle system '%s'\n",asName.c_str());
	}

	mpMapParticles->AddEntity(pPS);

	return pPS;*/
	return NULL;
}

//-----------------------------------------------------------------------

void cWorld2D::DestroyParticleSystem(iParticleSystem2D *apPS) {
	/*if(apPS==NULL)return;

	mpMapParticles->RemoveEntity(apPS);
	hplDelete(apPS);*/
}

//-----------------------------------------------------------------------

cBody2D *cWorld2D::CreateBody2D(const tString &asName, cMesh2D *apMesh, cVector2f avSize) {
	cBody2D *pBody = hplNew(cBody2D, (asName, apMesh, avSize, mpCollider, mlBodyIDCount++));

	mpMapBodies->AddEntity(pBody);

	return pBody;
}

//-----------------------------------------------------------------------

cArea2D *cWorld2D::GetArea(const tString &asName, const tString &asType) {
	tGrid2DObjectMap *GridMap = mpMapAreas->GetAllMap();
	tGrid2DObjectMapIt it = GridMap->begin();
	while (it != GridMap->end()) {
		cArea2D *pArea = static_cast<cArea2D *>(it->second->GetEntity());

		if (asName == "" || pArea->GetName() == asName) {
			if (asType == "" || pArea->GetType() == asType) {
				return pArea;
			}
		}

		it++;
	}

	return NULL;
}

//-----------------------------------------------------------------------

bool cWorld2D::CreateFromFile(tString asFile) {

	// Load the document
	asFile = cString::SetFileExt(asFile, "hpl");
	tString sPath = mpResources->GetFileSearcher()->GetFilePath(asFile);

	if (sPath == "") {
		error("Couldn't find map '%s'", asFile.c_str());
		return false;
	}

	TiXmlDocument *pDoc = hplNew(TiXmlDocument, (sPath.c_str()));
	if (!pDoc->LoadFile()) {
		error("Couldn't load map '%s'", asFile.c_str());
		return false;
	}

	// Load the script
	asFile = cString::SetFileExt(asFile, "hps");
	mpScript = mpResources->GetScriptManager()->CreateScript(asFile);
	if (mpScript == NULL) {
		Error("Couldn't load script '%s'", asFile.c_str());
	}

	TiXmlElement *pHplMapElem = pDoc->RootElement();

	cVector2l vMapSize;
	msMapName = pHplMapElem->Attribute("Name");
	vMapSize.x = cString::ToInt(pHplMapElem->Attribute("Width"), 0);
	vMapSize.y = cString::ToInt(pHplMapElem->Attribute("Height"), 0);
	float fTileSize = (float)cString::ToInt(pHplMapElem->Attribute("TileSize"), 0);
	mfLightZ = cString::ToFloat(pHplMapElem->Attribute("LightZ"), 0);
	mAmbientColor.r = cString::ToFloat(pHplMapElem->Attribute("AmbColR"), 0) / 255.0f;
	mAmbientColor.g = cString::ToFloat(pHplMapElem->Attribute("AmbColG"), 0) / 255.0f;
	mAmbientColor.b = cString::ToFloat(pHplMapElem->Attribute("AmbColB"), 0) / 255.0f;
	mAmbientColor.a = 0;
	// Log("Amb: %f : %f : %f : %f\n",mAmbientColor.r,mAmbientColor.g,mAmbientColor.b,mAmbientColor.a);

	mpGraphics->GetRenderer2D()->SetAmbientLight(mAmbientColor);
	mpGraphics->GetRenderer2D()->SetShadowZ(mfLightZ);

	// Set up data for objects.
	cVector2l vWorldSize = vMapSize * (int)fTileSize;
	mvWorldSize = cVector2f((float)vWorldSize.x, (float)vWorldSize.y);

	mpMapLights = hplNew(cGridMap2D, (vWorldSize, cVector2l(200, 200), cVector2l(5, 5)));
	mpMapImageEntities = hplNew(cGridMap2D, (vWorldSize, cVector2l(150, 150), cVector2l(5, 5)));
	mpMapBodies = hplNew(cGridMap2D, (vWorldSize, cVector2l(150, 150), cVector2l(5, 5)));
	mpMapParticles = hplNew(cGridMap2D, (vWorldSize, cVector2l(300, 300), cVector2l(3, 3)));
	mpMapAreas = hplNew(cGridMap2D, (vWorldSize, cVector2l(300, 300), cVector2l(3, 3)));

	TiXmlElement *pHplMapChildElem = pHplMapElem->FirstChildElement();
	while (pHplMapChildElem) {
		tString sChildName = pHplMapChildElem->Value();

		/////////////////
		/// LIGHTS //////
		/////////////////
		if (sChildName == "Lights") {
			TiXmlElement *pLightChildElem = pHplMapChildElem->FirstChildElement();
			while (pLightChildElem) {
				cVector3f vPos;
				cColor Col(0, 1);
				cLight2DPoint *pLight = CreateLightPoint(pLightChildElem->Attribute("Name"));
				vPos.x = cString::ToFloat(pLightChildElem->Attribute("X"), 0);
				vPos.y = cString::ToFloat(pLightChildElem->Attribute("Y"), 0);
				vPos.z = cString::ToFloat(pLightChildElem->Attribute("Z"), 0);
				pLight->SetPosition(vPos);
				Col.r = cString::ToFloat(pLightChildElem->Attribute("ColR"), 0) / 255.0f;
				Col.g = cString::ToFloat(pLightChildElem->Attribute("ColG"), 0) / 255.0f;
				Col.b = cString::ToFloat(pLightChildElem->Attribute("ColB"), 0) / 255.0f;
				Col.a = cString::ToFloat(pLightChildElem->Attribute("Specular"), 1);
				pLight->SetDiffuseColor(Col);
				pLight->SetFarAttenuation(cString::ToFloat(pLightChildElem->Attribute("Radius"), 0));
				pLight->SetActive(cString::ToBool(pLightChildElem->Attribute("Active"), true));
				/*LOad some more stuff*/
				pLight->SetAffectMaterial(cString::ToBool(pLightChildElem->Attribute("AffectMaterial"), true));
				pLight->SetCastShadows(cString::ToBool(pLightChildElem->Attribute("CastShadows"), true));

				pLightChildElem = pLightChildElem->NextSiblingElement();
			}
		}
		////////////////////////
		/// ENTITIES ///////////
		////////////////////////
		else if (sChildName == "Entities") {
			TiXmlElement *pEntityElem = pHplMapChildElem->FirstChildElement();
			while (pEntityElem) {
				tString sRenderType = cString::ToString(pEntityElem->Attribute("RenderType"), "Image");

				if (sRenderType == "Image") {
					cImageEntity *pEntity = hplNew(cImageEntity, (cString::ToString(
																	  pEntityElem->Attribute("Name"), "Image"),
																  mpResources, mpGraphics));
					// The the main data
					cVector3f vPos;
					vPos.x = cString::ToFloat(pEntityElem->Attribute("X"), 0);
					vPos.y = cString::ToFloat(pEntityElem->Attribute("Y"), 0);
					vPos.z = cString::ToFloat(pEntityElem->Attribute("Z"), 0);
					pEntity->SetPosition(vPos);
					pEntity->SetActive(cString::ToBool(pEntityElem->Attribute("Active"), true));

					if (pEntity->LoadData(pEntityElem)) {
						mpMapImageEntities->AddEntity(pEntity);

						iEntity2DLoader *pLoader = mpResources->GetEntity2DLoader(
							pEntity->GetEntityData()->GetType());

						if (pLoader) {
							pLoader->Load(pEntity);
						} else {
							/*Maybe delete entity if no type found? */
						}
					} else {
						hplDelete(pEntity);
						Error("Couldn't load data for entity '%s'", pEntity->GetName().c_str());
					}
				} else {
					error("No other Render mode for entity exist!!");
				}

				pEntityElem = pEntityElem->NextSiblingElement();
			}
		}
		/////////////////////////////
		/// SOUND SOURCES ///////////
		/////////////////////////////
		else if (sChildName == "SoundSources") {
			TiXmlElement *pSoundElem = pHplMapChildElem->FirstChildElement();
			while (pSoundElem) {
				cSoundSource *pSound = hplNew(cSoundSource, (
																cString::ToString(pSoundElem->Attribute("Name"), ""),
																cString::ToString(pSoundElem->Attribute("SoundName"), ""),
																mpSound, false));

				pSound->LoadData(pSoundElem);

				mlstSoundSources.push_back(pSound);

				pSoundElem = pSoundElem->NextSiblingElement();
			}
		}
		/////////////////////////////
		/// PARTICLE SYSTEMS ///////////
		/////////////////////////////
		/*else if(sChildName == "ParticleSystems")
		{
			TiXmlElement* pPartElem = pHplMapChildElem->FirstChildElement();
			while(pPartElem)
			{
				iParticleSystem2D* pPS;

				tString sName = cString::ToString(pPartElem->Attribute("Name"),"");
				tString sPartName = cString::ToString(pPartElem->Attribute("PartName"),"");
				cVector3f vSize;
				cVector3f vPos;
				vSize.x = cString::ToFloat(pPartElem->Attribute("SizeX"),0);
				vSize.y = cString::ToFloat(pPartElem->Attribute("SizeY"),0);
				vSize.z = cString::ToFloat(pPartElem->Attribute("SizeZ"),0);

				vPos.x = cString::ToFloat(pPartElem->Attribute("X"),0);
				vPos.y = cString::ToFloat(pPartElem->Attribute("Y"),0);
				vPos.z = cString::ToFloat(pPartElem->Attribute("Z"),0);

				pPS = mpResources->GetParticleManager()->CreatePS2D(sPartName,vSize);

				if(pPS==NULL){
					Error("Couldn't load particle system '%s'!\n",sPartName.c_str());
				}
				else {
					pPS->SetName(sName);
					pPS->SetPosition(vPos);

					mpMapParticles->AddEntity(pPS);
				}

				pPartElem = pPartElem->NextSiblingElement();
			}
		}*/
		/////////////////////////////
		/// AREAS ///////////////////
		/////////////////////////////
		else if (sChildName == "Areas") {
			TiXmlElement *pAreaElem = pHplMapChildElem->FirstChildElement();
			while (pAreaElem) {
				cArea2D *pArea;

				tString sName = cString::ToString(pAreaElem->Attribute("Name"), "");
				tString sAreaType = cString::ToString(pAreaElem->Attribute("AreaType"), "");
				cVector3f vPos;

				vPos.x = cString::ToFloat(pAreaElem->Attribute("X"), 0);
				vPos.y = cString::ToFloat(pAreaElem->Attribute("Y"), 0);
				vPos.z = cString::ToFloat(pAreaElem->Attribute("Z"), 0);

				pArea = hplNew(cArea2D, (sName, sAreaType, mpCollider));

				pArea->LoadData(pAreaElem);

				pArea->SetName(sName);
				pArea->SetPosition(vPos);

				if (pArea->GetType() != "Script") {
					iArea2DLoader *pAreaLoader = mpResources->GetArea2DLoader(pArea->GetType());
					if (pAreaLoader) {
						pAreaLoader->Load(pArea);
					}
				}

				mpMapAreas->AddEntity(pArea);

				pAreaElem = pAreaElem->NextSiblingElement();
			}
		}
		/////////////////
		/// TILEMAP /////
		/////////////////
		else if (sChildName == "TileMap") {
			mpTileMap = hplNew(cTileMap, (vMapSize, fTileSize, mpGraphics, mpResources));
			int lShadowLayer = cString::ToInt(pHplMapChildElem->Attribute("ShadowLayer"), 0);

			TiXmlElement *pTileMapChildElem = pHplMapChildElem->FirstChildElement();
			while (pTileMapChildElem) {
				tString sTileMapChildName = pTileMapChildElem->Value();

				// Log("Tilemap: %s\n",sTileMapChildName.c_str());
				////////////////////////
				/// TILE SETS //////////
				////////////////////////

				if (sTileMapChildName == "TileSets") {
					TiXmlElement *pTileSetChildElem = pTileMapChildElem->FirstChildElement();
					while (pTileSetChildElem) {
						tString sName = pTileSetChildElem->Attribute("Name");

						cTileSet *pTileSet = mpResources->GetTileSetManager()->CreateTileSet(sName);
						mpTileMap->AddTileSet(pTileSet);

						pTileSetChildElem = pTileSetChildElem->NextSiblingElement();
					}
				}
				////////////////////////
				/// TILE LAYERS ////////
				///////////////////////
				else if (sTileMapChildName == "Layers") {
					// Log("Layers\n");
					TiXmlElement *pLayerChildElem = pTileMapChildElem->FirstChildElement();
					while (pLayerChildElem) {
						// Log("Layer Children\n");

						if (pLayerChildElem->Attribute("Width") == NULL)
							error("Can't Load Width");
						int lW = cString::ToInt(pLayerChildElem->Attribute("Width"), 0);

						if (pLayerChildElem->Attribute("Height") == NULL)
							error("Can't Load Height");
						int lH = cString::ToInt(pLayerChildElem->Attribute("Height"), 0);
						bool bCollide = cString::ToBool(pLayerChildElem->Attribute("Collide"), true);
						bool bLit = cString::ToBool(pLayerChildElem->Attribute("Lit"), true);
						float fZ = cString::ToFloat(pLayerChildElem->Attribute("Z"), 0);

						cTileLayer *pTileLayer = hplNew(cTileLayer, (lW, lH,
																	 bCollide, bLit, eTileLayerType_Normal));
						pTileLayer->SetZ(fZ);

						mpTileMap->AddTileLayerFront(pTileLayer);

						//// THE TILES ////////
						TiXmlElement *pTileRowElem = pLayerChildElem->FirstChildElement();

						int lRowCount = 0;
						while (pTileRowElem) {
							// Log("Tile Row: ");
							int lColCount = 0;
							tString sData = pTileRowElem->Attribute("Data");
							int lDataCount = 0;
							while (lDataCount < (int)sData.size()) {
								cTile *pTile = hplNew(cTile, (NULL, eTileRotation_0,
															  cVector3f((float)lColCount * fTileSize, (float)lRowCount * fTileSize, fZ),
															  cVector2f(fTileSize, fTileSize), NULL));

								lDataCount = LoadTileData(pTile, &sData, lDataCount);

								if (pTile->GetTileData()) {

									// Create the collision mesh
									if (pTileLayer->HasCollision()) {
										cCollisionMesh2D *pCollMesh = NULL;
										cTileDataNormal *pTData;
										pTData = static_cast<cTileDataNormal *>(pTile->GetTileData());

										if (pTData->GetCollideMesh()) {
											pCollMesh = pTData->GetCollideMesh()->CreateCollisonMesh(
												cVector2f(pTile->GetPosition().x,
														  pTile->GetPosition().y),
												cVector2f(2),
												pTile->GetAngle());
										}

										pTile->SetCollisionMesh(pCollMesh);
									}

									// Add tile to the layer
									pTileLayer->SetTile(lColCount, lRowCount, pTile);
								} else {
									hplDelete(pTile);
								}

								lColCount++;
							}
							// Log("\n");

							lRowCount++;
							pTileRowElem = pTileRowElem->NextSiblingElement();
						}

						pLayerChildElem = pLayerChildElem->NextSiblingElement();
					}
				}
				pTileMapChildElem = pTileMapChildElem->NextSiblingElement();
			}

			// Set the inverse shadow layer.
			mpTileMap->SetShadowLayer(mpTileMap->GetTileLayerNum() - lShadowLayer - 1);
		}

		pHplMapChildElem = pHplMapChildElem->NextSiblingElement();
	}

	hplDelete(pDoc);

	return true;
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cWorld2D::UpdateSoundSources() {
	tSoundSourceListIt it = mlstSoundSources.begin();
	while (it != mlstSoundSources.end()) {
		(*it)->UpdateLogic(0);

		if ((*it)->IsDead()) {
			it = mlstSoundSources.erase(it);
		} else {
			it++;
		}
	}
}

//-----------------------------------------------------------------------

void cWorld2D::UpdateParticles() {
	/*tGrid2DObjectMapIt it = mpMapParticles->GetAllMap()->begin();

	while(it != mpMapParticles->GetAllMap()->end())
	{
		iParticleSystem2D *pEntity = static_cast<iParticleSystem2D*>(it->second->GetEntity());

		pEntity->UpdateLogic(0);

		it++;

		//Check if the system is alive, else destroy
		if(pEntity->IsDead()){
			DestroyParticleSystem(pEntity);
		}
	}*/
}
//-----------------------------------------------------------------------

void cWorld2D::RenderParticles(cCamera2D *apCamera) {
	/*cRect2f ClipRect;
	apCamera->GetClipRect(ClipRect);

	iGridMap2DIt* pEntityIt = mpMapParticles->GetRectIterator(ClipRect);

	while(pEntityIt->HasNext())
	{
		iParticleSystem2D* pEntity = static_cast<iParticleSystem2D*>(pEntityIt->Next());

		pEntity->Render();
	}


	hplDelete(pEntityIt);*/
}

//-----------------------------------------------------------------------

void cWorld2D::UpdateEntities() {
	tGrid2DObjectMapIt it = mpMapImageEntities->GetAllMap()->begin();

	while (it != mpMapImageEntities->GetAllMap()->end()) {
		iEntity2D *pEntity = static_cast<cImageEntity *>(it->second->GetEntity());

		if (pEntity->IsActive())
			pEntity->UpdateLogic(0);

		it++;
	}
}

//-----------------------------------------------------------------------

void cWorld2D::UpdateBodies() {
	tGrid2DObjectMapIt it = mpMapBodies->GetAllMap()->begin();

	while (it != mpMapBodies->GetAllMap()->end()) {
		cBody2D *pBody = static_cast<cBody2D *>(it->second->GetEntity());

		if (pBody->IsActive())
			pBody->UpdateLogic(0);

		it++;
	}
}
//-----------------------------------------------------------------------

void cWorld2D::UpdateLights() {
	tGrid2DObjectMapIt it = mpMapLights->GetAllMap()->begin();

	while (it != mpMapLights->GetAllMap()->end()) {
		iLight2D *pLight = static_cast<iLight2D *>(it->second->GetEntity());

		if (pLight->IsActive())
			pLight->UpdateLogic(0);

		it++;
	}
}

//-----------------------------------------------------------------------

void cWorld2D::RenderImagesEntities(cCamera2D *apCamera) {
	cRect2f ClipRect;
	apCamera->GetClipRect(ClipRect);

	iGridMap2DIt *pEntityIt = mpMapImageEntities->GetRectIterator(ClipRect);

	while (pEntityIt->HasNext()) {
		cImageEntity *pEntity = static_cast<cImageEntity *>(pEntityIt->Next());

		if (pEntity->IsActive()) {
			pEntity->Render();
		}
	}

	hplDelete(pEntityIt);
}

//-----------------------------------------------------------------------

int cWorld2D::LoadTileData(cTile *apTile, tString *asData, int alStart) {
	int lCount = alStart;
	int lStart = lCount;
	int lValType = 0;
	int lSet = 0;
	int lNum;

	while (true) {
		if (asData->c_str()[lCount] == ':' || asData->c_str()[lCount] == '|') {
			if (lStart != lCount) {
				tString sVal = asData->substr(lStart, lCount - lStart);
				int lVal = cString::ToInt(sVal.c_str(), -1);

				cTileSet *pSet = NULL;
				cTileDataNormal *pData = NULL;
				switch (lValType) {
				case 0:
					lSet = lVal;
					break;
				case 1:
					lNum = lVal;
					if (lSet < 0)
						break;
					if (lNum < 0)
						break;

					pSet = mpTileMap->GetTileSet(lSet);
					if (pSet == NULL)
						error("Error getting tileset%d", lSet);
					pData = static_cast<cTileDataNormal *>(pSet->Get(lNum));

					apTile->SetTileData(pData);

					break;
				case 2:
					apTile->SetAngle((eTileRotation)lVal);
					break;
				case 3:
					apTile->SetFlags(eTileFlag_Breakable);
				}
				lValType++;
			}

			if (asData->c_str()[lCount] == '|') {
				/*if(apTile->GetTileData())
				Log("%d,%d,%d|",lSet,lNum,apTile->GetAngle());
				else
				Log("N|");*/
				break;
			}
			lStart = lCount + 1;
		}

		lCount++;
	}

	return lCount + 1;
}

//-----------------------------------------------------------------------
} // namespace hpl
