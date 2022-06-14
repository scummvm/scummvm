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

#include "hpl1/engine/scene/TileSet.h"
#include "hpl1/engine/graphics/MaterialHandler.h"
#include "hpl1/engine/graphics/MeshCreator.h"
#include "hpl1/engine/impl/tinyXML/tinyxml.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/system/LowLevelSystem.h"
#include <assert.h>

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cTileSet::cTileSet(tString asName, cGraphics *apGraphics, cResources *apResources)
	: iResourceBase(asName, 0) {
	mpResources = apResources;
	mfTileSize = 0;
	mpGraphics = apGraphics;

	for (int i = 0; i < eMaterialTexture_LastEnum; i++)
		mvImageHandle[i] = -1;
}

//-----------------------------------------------------------------------

cTileSet::~cTileSet() {
	Log(" Deleting tileset.\n");

	for (tTileDataVecIt it = mvData.begin(); it != mvData.end(); it++) {
		(*it)->Destroy();
		hplDelete(*it);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cTileSet::Add(iTileData *apData) {
	mvData.push_back(apData);
}

//-----------------------------------------------------------------------

iTileData *cTileSet::Get(int alNum) {
	assert(alNum >= 0 && alNum < (int)mvData.size());

	return mvData[alNum];
}

//-----------------------------------------------------------------------

bool cTileSet::CreateFromFile(const tString &asFile) {
	TiXmlDocument *pDoc = hplNew(TiXmlDocument, (asFile.c_str()));
	if (!pDoc->LoadFile()) {
		FatalError("Couldn't load tileset '%s'!\n", asFile.c_str());
		return false;
	}

	GetTileNum(pDoc->RootElement()->FirstChildElement());

	// Add the resources
	mpResources->AddResourceDir(pDoc->RootElement()->Attribute("dir"));
	// Get the tiles size
	mfTileSize = (float)cString::ToInt(pDoc->RootElement()->Attribute("size"), 0);

	// Calculate the best size for the frame:
	double x = ceil(log((double)((float)mlNum) * mfTileSize) / log(2.0f));
	double y = ceil(log((double)mfTileSize) / log(2.0f));

	if (x > kMaxTileFrameWidth) {
		y += x - kMaxTileFrameWidth;
		x = kMaxTileFrameWidth;
	}

	mvFrameSize = cVector2l((int)pow(2.0, x), (int)pow(2.0, y));

	TiXmlElement *pTileElement = pDoc->RootElement()->FirstChildElement();
	while (pTileElement != NULL) {
		LoadData(pTileElement);

		pTileElement = pTileElement->NextSiblingElement();
	}

	hplDelete(pDoc);

	mpResources->GetImageManager()->FlushAll();

	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cTileSet::LoadData(TiXmlElement *pElement) {
	tString sName = pElement->Attribute("name");
	tString sMaterial = pElement->Attribute("material");
	tString sMesh = pElement->Attribute("mesh");
	tString sCollideMesh = cString::ToString(pElement->Attribute("collidemesh"), "");

	iMaterial *pMaterial = NULL;
	cMesh2D *pMesh = NULL;
	cMesh2D *pCollideMesh = NULL;
	cResourceImage *pImage = NULL;

	cTileDataNormal *pTileData = hplNew(cTileDataNormal, (mpResources->GetImageManager(), mfTileSize));

	// Log("Tile %s:\n", sName.c_str());
	// Log("Creating material: %s\n",sMaterial.c_str());
	//  Create material
	pMaterial = mpGraphics->GetMaterialHandler()->Create(sMaterial, eMaterialPicture_Image);
	if (pMaterial == NULL) {
		Error("Error creating material '%s' for '%s'!\n", sMaterial.c_str(), sName.c_str());
		return false;
	}

	// Log("Loading images..\n");
	// Load the images
	tTextureTypeList lstTypes = pMaterial->GetTextureTypes();
	for (tTextureTypeListIt it = lstTypes.begin(); it != lstTypes.end(); it++) {
		if (mvImageHandle[it->mType] == -1) {
			mvImageHandle[it->mType] = mpResources->GetImageManager()->CreateFrame(mvFrameSize);
		}

		pImage = mpResources->GetImageManager()->CreateImage(sName + it->msSuffix, mvImageHandle[it->mType]);
		if (pImage == NULL) {
			Error("Can't load texture '%s%s'!\n", sName.c_str(), it->msSuffix.c_str());
			return false;
		}

		pMaterial->SetImage(pImage, it->mType);
	}

	// Compile material
	pMaterial->Compile();

	// Create the mesh
	pMesh = mpGraphics->GetMeshCreator()->Create2D(sMesh, mfTileSize);
	if (pMesh == NULL) {
		Error("Error creating mesh for '%s'!\n", sName.c_str());
		return false;
	}

	pTileData->SetData(pMesh, pMaterial);

	// Create the collide mesh
	if (sCollideMesh != "") {
		pCollideMesh = mpGraphics->GetMeshCreator()->Create2D(sCollideMesh, mfTileSize);
		if (pCollideMesh == NULL) {
			Error("Error creating collide mesh for '%s'!\n", sName.c_str());
			return false;
		}
		pCollideMesh->CreateTileVertexVec();

		pTileData->SetCollideMesh(pCollideMesh);
	}

	// Set the tilesdata properties:
	pTileData->SetIsSolid(cString::ToInt(pElement->Attribute("solid"), 1) ? true : false);

	eTileCollisionType CType = (eTileCollisionType)cString::ToInt(pElement->Attribute("collision_type"), 1);
	pTileData->SetCollisionType(CType);

	Add(pTileData);

	return true;
}

//-----------------------------------------------------------------------

void cTileSet::GetTileNum(TiXmlElement *apElement) {
	mlNum = 0;
	while (apElement != NULL) {
		mlNum++;

		apElement = apElement->NextSiblingElement();
	}
}

//-----------------------------------------------------------------------

} // namespace hpl
