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

#include "hpl1/engine/graphics/ImageEntityData.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/MaterialHandler.h"
#include "hpl1/engine/graphics/Mesh2d.h"
#include "hpl1/engine/graphics/MeshCreator.h"
#include "hpl1/engine/impl/tinyXML/tinyxml.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/scene/ImageEntity.h"
#include "hpl1/engine/scene/TileSet.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cImageEntityData::cImageEntityData(tString asName, cGraphics *apGraphics, cResources *apResources)
	: iResourceBase(asName, 0) {
	mpResources = apResources;
	mpGraphics = apGraphics;

	mlFrameNum = 0;

	mbCastShadows = false;
	mbCollidable = false;
	mbLit = true;

	mpMesh = NULL;
	mpCollideMesh = NULL;
}

//-----------------------------------------------------------------------

cImageEntityData::~cImageEntityData() {
	for (int i = 0; i < (int)mvImageFrames.size(); i++) {
		hplDelete(mvImageFrames[i].mpMaterial);
	}

	hplDelete(mpMesh);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cImageAnimation *cImageEntityData::GetAnimationByName(const tString &asName) {
	tImageAnimationMapIt it = m_mapAnimations.find(asName);
	if (it == m_mapAnimations.end())
		return NULL;

	return &it->second;
}

//-----------------------------------------------------------------------

cImageAnimation *cImageEntityData::GetAnimationByHandle(int alHandle) {
	tImageAnimationMapIt it = m_mapAnimations.begin();

	while (it != m_mapAnimations.end()) {
		if (it->second.mlHandle == alHandle)
			return &it->second;
		it++;
	}

	return NULL;
}

//-----------------------------------------------------------------------

bool cImageEntityData::CreateFromFile(const tString &asFile, tIntVec &avImageHandle) {
	bool bGotAnim = false;
	TiXmlDocument *pDoc = hplNew(TiXmlDocument, (asFile.c_str()));
	if (!pDoc->LoadFile()) {
		error("Couldn't load tileset '%s'", asFile.c_str());
		return false;
	}

	TiXmlElement *RootElem = pDoc->RootElement();

	// Temp test::
	Common::fill(avImageHandle.begin(), avImageHandle.end(), -1);

	///////// MAIN ///////////////
	TiXmlElement *MainElem = RootElem->FirstChildElement("MAIN");

	msDataName = cString::ToString(MainElem->Attribute("Name"), "");
	msType = cString::ToString(MainElem->Attribute("Type"), "");
	msSubType = cString::ToString(MainElem->Attribute("Subtype"), "");

	///////// IMAGE ///////////////
	TiXmlElement *ImageElem = RootElem->FirstChildElement("IMAGE");

	tString sImageName = cString::ToString(ImageElem->Attribute("Name"), "");
	tString sDirectory = cString::ToString(ImageElem->Attribute("Dir"), "");
	tString sMaterial = cString::ToString(ImageElem->Attribute("Material"), "");
	tString sMesh = cString::ToString(ImageElem->Attribute("Mesh"), "");

	mvImageSize.x = cString::ToFloat(ImageElem->Attribute("Width"), 512) + 2.0f;
	mvImageSize.y = cString::ToFloat(ImageElem->Attribute("Height"), 512) + 2.0f;

	///////// PROPERTIES ///////////////
	TiXmlElement *PropElem = RootElem->FirstChildElement("PROPERTIES");

	mbCastShadows = cString::ToBool(PropElem->Attribute("CastShadows"), false);
	mbCollidable = cString::ToBool(PropElem->Attribute("Collidable"), false);
	mbCollides = cString::ToBool(PropElem->Attribute("Collides"), false);
	mbLit = cString::ToBool(PropElem->Attribute("Lit"), true);
	tString sCollideMesh = cString::ToString(PropElem->Attribute("CollideMesh"), "square");

	///////// ANIMATIONS ///////////////
	TiXmlElement *AnimationElem = RootElem->FirstChildElement("ANIMATIONS");

	if (AnimationElem != NULL) {
		mlFrameNum = cString::ToInt(AnimationElem->Attribute("Frames"), 1);

		TiXmlElement *AnimChildElem = AnimationElem->FirstChildElement();

		int lCount = 0;
		while (AnimChildElem) {
			cImageAnimation Anim;
			Anim.msName = cString::ToString(AnimChildElem->Attribute("Name"), "");
			Anim.mfSpeed = cString::ToFloat(AnimChildElem->Attribute("Speed"), 1);
			Anim.mbCollidable = cString::ToBool(AnimChildElem->Attribute("Collidable"), false);
			Anim.msSound = cString::ToString(AnimChildElem->Attribute("Sound"), "");
			Anim.mlHandle = lCount;

			tString sData = cString::ToString(AnimChildElem->Attribute("Data"), "");
			cString::GetIntVec(sData, Anim.mvFrameNums);

			m_mapAnimations.insert(tImageAnimationMap::value_type(Anim.msName, Anim));

			AnimChildElem = AnimChildElem->NextSiblingElement();
			lCount++;
		}

		bGotAnim = true;
	} else {
		mlFrameNum = 1;
		bGotAnim = false;
	}

	///////// LOADING /////////////////

	mpResources->AddResourceDir(sDirectory);

	// Create the mesh for drawing
	mpMesh = mpGraphics->GetMeshCreator()->Create2D(sMesh, 2);
	if (mpMesh == NULL) {
		Error("Error creating mesh for '%s'!\n", msName.c_str());
		return false;
	}
	mpMesh->CreateVertexVec();
	mvIdxVec = *mpMesh->GetIndexVec();

	// Create the mesh used for collision
	mpCollideMesh = mpGraphics->GetMeshCreator()->Create2D(sCollideMesh, 2);
	if (mpCollideMesh == NULL) {
		Error("Error creating collide mesh '%s' for '%s'!\n", sCollideMesh.c_str(), msName.c_str());
		return false;
	}
	mpCollideMesh->CreateVertexVec();

	// Determine frame size, there should be some minum size and it should also see to that it
	// can contain all frames for the animations.
	double x = ceil(log((double)((float)mlFrameNum) * mvImageSize.x) / log(2.0f));
	double y = ceil(log((double)mvImageSize.y) / log(2.0f));

	if (x > kMaxImageEntityFrameWidth) {
		y += x - kMaxTileFrameWidth;
		x = kMaxTileFrameWidth;
	}

	mvFrameSize = cVector2l((int)pow(2.0, x), (int)pow(2.0, y));

	// Loop through all animation frames.
	for (int i = 0; i < mlFrameNum; i++) {
		// Get the material
		iMaterial *pMaterial = mpGraphics->GetMaterialHandler()->Create(sMaterial, eMaterialPicture_Image);
		if (pMaterial == NULL) {
			Error("Error creating material '%s' for '%s'!\n", sMaterial.c_str(), msName.c_str());
			return false;
		}

		// Get the textures for the material
		tTextureTypeList lstTypes = pMaterial->GetTextureTypes();
		for (tTextureTypeListIt it = lstTypes.begin(); it != lstTypes.end(); it++) {
			if (avImageHandle[it->mType] == -1) {
				avImageHandle[it->mType] = mpResources->GetImageManager()->CreateFrame(mvFrameSize);
			}

			tString sFile;
			if (bGotAnim) {
				int lNum = i + 1;

				sFile = sImageName;
				if (lNum < 10)
					sFile += "0";
				sFile += Common::String::format("%d", lNum).c_str();
			} else {
				sFile = sImageName;
			}

			cResourceImage *pImage = mpResources->GetImageManager()->CreateImage(
				sFile + it->msSuffix,
				avImageHandle[it->mType]);
			if (pImage == NULL) {
				error("Can't load texture '%s%s'", sFile.c_str(), it->msSuffix.c_str());
				return false;
			}

			pMaterial->SetImage(pImage, it->mType);
		}

		pMaterial->Compile();

		cImageFrame ImageFrame;

		ImageFrame.mpMaterial = pMaterial;
		cRect2f ImageRect = pMaterial->GetTextureOffset(eMaterialTexture_Diffuse);
		ImageFrame.mvVtx = *mpMesh->GetVertexVec(ImageRect, 2, eTileRotation_0);

		mvImageFrames.push_back(ImageFrame);
	}

	///////// CLEAN UP ///////////////

	hplDelete(pDoc);

	mpResources->GetImageManager()->FlushAll();

	return true;
}

//-----------------------------------------------------------------------

cImageFrame *cImageEntityData::GetImageFrame(int alFrame) {
	if (alFrame < 0 || alFrame >= (int)mvImageFrames.size())
		return NULL;

	return &mvImageFrames[alFrame];
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cImageEntityData::GetFrameNum(TiXmlElement *apElement) {
	mlFrameNum = 1;
}

//-----------------------------------------------------------------------

} // namespace hpl
