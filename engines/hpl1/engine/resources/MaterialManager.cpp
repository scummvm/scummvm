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

#include "hpl1/engine/resources/MaterialManager.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/MaterialHandler.h"
#include "hpl1/engine/impl/tinyXML/tinyxml.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/System.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMaterialManager::cMaterialManager(cGraphics *apGraphics, cResources *apResources)
	: iResourceManager(apResources->GetFileSearcher(), apResources->GetLowLevel(),
					   apResources->GetLowLevelSystem()) {
	mpGraphics = apGraphics;
	mpResources = apResources;

	mlTextureSizeLevel = 0;
	mTextureFilter = eTextureFilter_Bilinear;
	mfTextureAnisotropy = 1.0f;

	mlIdCounter = 0;
}

cMaterialManager::~cMaterialManager() {
	DestroyAll();

	Log(" Done with materials\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iMaterial *cMaterialManager::CreateMaterial(const tString &asName) {
	tString sPath;
	iMaterial *pMaterial;
	tString asNewName;

	BeginLoad(asName);

	asNewName = cString::SetFileExt(asName, "mat");

	pMaterial = static_cast<iMaterial *>(this->FindLoadedResource(asNewName, sPath));

	if (pMaterial == NULL && sPath != "") {
		pMaterial = LoadFromFile(asNewName, sPath);

		if (pMaterial == NULL) {
			Error("Couldn't load material '%s'\n", asNewName.c_str());
			EndLoad();
			return NULL;
		}

		AddResource(pMaterial);
	}

	if (pMaterial)
		pMaterial->IncUserCount();
	else
		Error("Couldn't create material '%s'\n", asNewName.c_str());

	EndLoad();
	return pMaterial;
}

//-----------------------------------------------------------------------

iResourceBase *cMaterialManager::Create(const tString &asName) {
	return CreateMaterial(asName);
}

//-----------------------------------------------------------------------

void cMaterialManager::Update(float afTimeStep) {
	tResourceHandleMapIt it = m_mapHandleResources.begin();
	for (; it != m_mapHandleResources.end(); ++it) {
		iResourceBase *pBase = it->second;
		iMaterial *pMat = static_cast<iMaterial *>(pBase);

		pMat->Update(afTimeStep);
	}
}

//-----------------------------------------------------------------------

void cMaterialManager::Unload(iResourceBase *apResource) {
}
//-----------------------------------------------------------------------

void cMaterialManager::Destroy(iResourceBase *apResource) {
	apResource->DecUserCount();

	if (apResource->HasUsers() == false) {
		RemoveResource(apResource);
		hplDelete(apResource);
	}
}

//-----------------------------------------------------------------------

void cMaterialManager::SetTextureFilter(eTextureFilter aFilter) {
	if (aFilter == mTextureFilter)
		return;
	mTextureFilter = aFilter;

	tResourceHandleMapIt it = m_mapHandleResources.begin();
	for (; it != m_mapHandleResources.end(); ++it) {
		iMaterial *pMat = static_cast<iMaterial *>(it->second);

		for (int i = 0; i < eMaterialTexture_LastEnum; ++i) {
			iTexture *pTex = pMat->GetTexture((eMaterialTexture)i);
			if (pTex)
				pTex->SetFilter(aFilter);
		}
	}
}

//-----------------------------------------------------------------------

void cMaterialManager::SetTextureAnisotropy(float afX) {
	if (afX < 1.0 || mpGraphics->GetLowLevel()->GetCaps(eGraphicCaps_AnisotropicFiltering) == 0) {
		return;
	}
	if (afX > (float)mpGraphics->GetLowLevel()->GetCaps(eGraphicCaps_MaxAnisotropicFiltering)) {
		return;
	}

	if (mfTextureAnisotropy == afX)
		return;
	mfTextureAnisotropy = afX;

	tResourceHandleMapIt it = m_mapHandleResources.begin();
	for (; it != m_mapHandleResources.end(); ++it) {
		iMaterial *pMat = static_cast<iMaterial *>(it->second);

		for (int i = 0; i < eMaterialTexture_LastEnum; ++i) {
			iTexture *pTex = pMat->GetTexture((eMaterialTexture)i);
			if (pTex)
				pTex->SetAnisotropyDegree(mfTextureAnisotropy);
		}
	}
}

//-----------------------------------------------------------------------

tString cMaterialManager::GetPhysicsMaterialName(const tString &asName) {
	tString sPath;
	iMaterial *pMaterial;
	tString asNewName;

	asNewName = cString::SetFileExt(asName, "mat");

	pMaterial = static_cast<iMaterial *>(this->FindLoadedResource(asNewName, sPath));

	if (pMaterial == NULL && sPath != "") {
		TiXmlDocument *pDoc = hplNew(TiXmlDocument, (sPath.c_str()));
		if (!pDoc->LoadFile()) {
			return "";
		}

		TiXmlElement *pRoot = pDoc->RootElement();

		TiXmlElement *pMain = pRoot->FirstChildElement("Main");
		if (pMain == NULL) {
			hplDelete(pDoc);
			Error("Main child not found in '%s'\n", sPath.c_str());
			return "";
		}

		tString sPhysicsName = cString::ToString(pMain->Attribute("PhysicsMaterial"), "Default");

		hplDelete(pDoc);

		return sPhysicsName;
	}

	if (pMaterial)
		return pMaterial->GetPhysicsMaterial();
	else
		return "";
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
iMaterial *cMaterialManager::LoadFromFile(const tString &asName, const tString &asPath) {
	TiXmlDocument *pDoc = hplNew(TiXmlDocument, (asPath.c_str()));
	if (!pDoc->LoadFile()) {
		hplDelete(pDoc);
		return NULL;
	}

	TiXmlElement *pRoot = pDoc->RootElement();

	///////////////////////////
	// Main
	TiXmlElement *pMain = pRoot->FirstChildElement("Main");
	if (pMain == NULL) {
		hplDelete(pDoc);
		Error("Main child not found.\n");
		return NULL;
	}

	const char *sType = pMain->Attribute("Type");
	if (sType == NULL) {
		hplDelete(pDoc);
		Error("Type not found.\n");
		return NULL;
	}

	bool bUseAlpha = cString::ToBool(pMain->Attribute("UseAlpha"), false);
	bool bDepthTest = cString::ToBool(pMain->Attribute("DepthTest"), true);
	float fValue = cString::ToFloat(pMain->Attribute("Value"), 1);
	tString sPhysicsMatName = cString::ToString(pMain->Attribute("PhysicsMaterial"), "Default");

	iMaterial *pMat = mpGraphics->GetMaterialHandler()->Create(asName, sType, eMaterialPicture_Texture);
	if (pMat == NULL) {
		hplDelete(pDoc);
		Error("Invalid material type '%s'\n", sType);
		return NULL;
	}

	pMat->SetHasAlpha(bUseAlpha);
	pMat->SetDepthTest(bDepthTest);
	pMat->SetValue(fValue);
	pMat->SetPhysicsMaterial(sPhysicsMatName);

	///////////////////////////
	// Textures
	TiXmlElement *pTexRoot = pRoot->FirstChildElement("TextureUnits");
	if (pTexRoot == NULL) {
		hplDelete(pDoc);
		Error("TextureUnits child not found.\n");
		return NULL;
	}

	tTextureTypeList lstTexTypes = pMat->GetTextureTypes();
	tTextureTypeListIt it = lstTexTypes.begin();
	for (; it != lstTexTypes.end(); it++) {
		iTexture *pTex = nullptr;

		TiXmlElement *pTexChild = pTexRoot->FirstChildElement(GetTextureString(it->mType).c_str());
		if (pTexChild == NULL) {
			/*Error("Texture unit missing!");
			hplDelete(pMat);
			return NULL;*/
			continue;
		}

		eTextureTarget target = GetTarget(cString::ToString(pTexChild->Attribute("Type"), ""));
		tString sFile = cString::ToString(pTexChild->Attribute("File"), "");
		bool bMipMaps = cString::ToBool(pTexChild->Attribute("Mipmaps"), true);
		bool bCompress = cString::ToBool(pTexChild->Attribute("Compress"), false);
		eTextureWrap wrap = GetWrap(cString::ToString(pTexChild->Attribute("Wrap"), ""));

		eTextureAnimMode animMode = GetAnimMode(cString::ToString(pTexChild->Attribute("AnimMode"), "None"));
		float fFrameTime = cString::ToFloat(pTexChild->Attribute("AnimFrameTime"), 1.0f);

		if (sFile == "") {
			continue;
		}

		if (animMode != eTextureAnimMode_None) {
			pTex = mpResources->GetTextureManager()->CreateAnim2D(sFile, bMipMaps, bCompress,
																  eTextureType_Normal,
																  mlTextureSizeLevel);
		} else {
			if (target == eTextureTarget_2D) {
				pTex = mpResources->GetTextureManager()->Create2D(sFile, bMipMaps, bCompress,
																  eTextureType_Normal,
																  mlTextureSizeLevel);
			} else if (target == eTextureTarget_1D) {
				pTex = mpResources->GetTextureManager()->Create1D(sFile, bMipMaps, bCompress,
																  eTextureType_Normal,
																  mlTextureSizeLevel);
			} else if (target == eTextureTarget_CubeMap) {
				pTex = mpResources->GetTextureManager()->CreateCubeMap(sFile, bMipMaps, bCompress,
																	   eTextureType_Normal,
																	   mlTextureSizeLevel);
			}
		}

		pTex->SetFrameTime(fFrameTime);
		pTex->SetAnimMode(animMode);

		if (pTex == NULL) {
			hplDelete(pDoc);
			hplDelete(pMat);
			return NULL;
		}

		pTex->SetWrapS(wrap);
		pTex->SetWrapT(wrap);

		pTex->SetFilter(mTextureFilter);
		pTex->SetAnisotropyDegree(mfTextureAnisotropy);

		pMat->SetTexture(pTex, it->mType);
	}

	///////////////////////////
	// Custom
	pMat->LoadData(pRoot);

	hplDelete(pDoc);

	pMat->Compile();

	return pMat;
}

//-----------------------------------------------------------------------

eTextureTarget cMaterialManager::GetTarget(const tString &asType) {
	if (cString::ToLowerCase(asType) == "cube")
		return eTextureTarget_CubeMap;
	else if (cString::ToLowerCase(asType) == "1D")
		return eTextureTarget_1D;
	else if (cString::ToLowerCase(asType) == "2D")
		return eTextureTarget_2D;
	else if (cString::ToLowerCase(asType) == "3D")
		return eTextureTarget_3D;

	return eTextureTarget_2D;
}
//-----------------------------------------------------------------------

tString cMaterialManager::GetTextureString(eMaterialTexture aType) {
	switch (aType) {
	case eMaterialTexture_Diffuse:
		return "Diffuse";
	case eMaterialTexture_Alpha:
		return "Alpha";
	case eMaterialTexture_NMap:
		return "NMap";
	case eMaterialTexture_Illumination:
		return "Illumination";
	case eMaterialTexture_Specular:
		return "Specular";
	case eMaterialTexture_CubeMap:
		return "CubeMap";
	case eMaterialTexture_Refraction:
		return "Refraction";
	default:
		break;
	}

	return "";
}

//-----------------------------------------------------------------------

eTextureWrap cMaterialManager::GetWrap(const tString &asType) {
	if (cString::ToLowerCase(asType) == "repeat")
		return eTextureWrap_Repeat;
	else if (cString::ToLowerCase(asType) == "clamp")
		return eTextureWrap_Clamp;
	else if (cString::ToLowerCase(asType) == "clamptoedge")
		return eTextureWrap_ClampToEdge;

	return eTextureWrap_Repeat;
}

eTextureAnimMode cMaterialManager::GetAnimMode(const tString &asType) {
	if (cString::ToLowerCase(asType) == "none")
		return eTextureAnimMode_None;
	else if (cString::ToLowerCase(asType) == "loop")
		return eTextureAnimMode_Loop;
	else if (cString::ToLowerCase(asType) == "oscillate")
		return eTextureAnimMode_Oscillate;

	return eTextureAnimMode_None;
}

//-----------------------------------------------------------------------
} // namespace hpl
