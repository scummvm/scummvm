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

#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/debug.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/Texture.h"
#include "hpl1/engine/graphics/bitmap2D.h"
#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/low_level_resources.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cTextureManager::cTextureManager(cGraphics *apGraphics, cResources *apResources)
	: iResourceManager(apResources->GetFileSearcher(), apResources->GetLowLevel(),
					   apResources->GetLowLevelSystem()) {
	mpGraphics = apGraphics;
	mpResources = apResources;

	mpLowLevelResources->getSupportedImageFormats(mlstFileFormats);

	mvCubeSideSuffixes.push_back("_pos_x");
	mvCubeSideSuffixes.push_back("_neg_x");
	mvCubeSideSuffixes.push_back("_pos_y");
	mvCubeSideSuffixes.push_back("_neg_y");
	mvCubeSideSuffixes.push_back("_pos_z");
	mvCubeSideSuffixes.push_back("_neg_z");
}

cTextureManager::~cTextureManager() {
	STLMapDeleteAll(m_mapAttenuationTextures);
	DestroyAll();
	Log(" Destroyed all textures\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iTexture *cTextureManager::Create1D(const tString &asName, bool abUseMipMaps, bool abCompress,
									eTextureType aType, unsigned int alTextureSizeLevel) {
	return CreateFlatTexture(asName, abUseMipMaps, abCompress, aType, eTextureTarget_1D, alTextureSizeLevel);
}

//-----------------------------------------------------------------------

iTexture *cTextureManager::Create2D(const tString &asName, bool abUseMipMaps, bool abCompress,
									eTextureType aType, unsigned int alTextureSizeLevel, eTextureTarget aTarget) {
	return CreateFlatTexture(asName, abUseMipMaps, abCompress, aType, aTarget, alTextureSizeLevel);
}

//-----------------------------------------------------------------------

iTexture *cTextureManager::CreateAnim2D(const tString &asName, bool abUseMipMaps, bool abCompress,
										eTextureType aType, unsigned int alTextureSizeLevel) {
	BeginLoad(asName);

	iTexture *pTexture = static_cast<iTexture *>(GetByName(asName));

	if (pTexture == NULL) {
		tString sFileExt = cString::GetFileExt(asName);
		tString sFileName = cString::SetFileExt(cString::GetFileName(asName), "");

		tStringVec mvFileNames;

		tString sTest = sFileName + "01." + sFileExt;
		int lNum = 2;
		tStringVec vPaths;

		while (true) {
			tString sPath = mpFileSearcher->GetFilePath(sTest);

			if (sPath == "") {
				break;
			} else {
				vPaths.push_back(sPath);
				if (lNum < 10)
					sTest = sFileName + "0" + cString::ToString(lNum) + "." + sFileExt;
				else
					sTest = sFileName + cString::ToString(lNum) + "." + sFileExt;

				++lNum;
			}
		}

		if (vPaths.empty()) {
			Error("No textures found for animation %s\n", asName.c_str());
			Error("Couldn't texture '%s'\n", asName.c_str());
			EndLoad();
			return NULL;
		}

		tBitmap2DVec vBitmaps;
		for (size_t i = 0; i < vPaths.size(); ++i) {
			Bitmap2D *pBmp = mpResources->GetLowLevel()->loadBitmap2D(vPaths[i]);
			if (pBmp == NULL) {
				Error("Couldn't load bitmap '%s'!\n", vPaths[i].c_str());

				for (int j = 0; j < (int)vBitmaps.size(); j++)
					hplDelete(vBitmaps[j]);

				EndLoad();
				return NULL;
			}

			vBitmaps.push_back(pBmp);
		}

		// Create the animated texture
		pTexture = mpGraphics->GetLowLevel()->CreateTexture(asName, abUseMipMaps, aType,
															eTextureTarget_2D);

		pTexture->SetSizeLevel(alTextureSizeLevel);

		if (pTexture->CreateAnimFromBitmapVec(&vBitmaps) == false) {
			Error("Couldn't create animated texture '%s'!\n", asName.c_str());
			hplDelete(pTexture);
			for (int j = 0; j < (int)vBitmaps.size(); j++)
				hplDelete(vBitmaps[j]);
			EndLoad();
			return NULL;
		}

		// Bitmaps no longer needed.
		for (int j = 0; j < (int)vBitmaps.size(); j++)
			hplDelete(vBitmaps[j]);

		AddResource(pTexture);
	}

	if (pTexture)
		pTexture->IncUserCount();
	else
		Error("Couldn't texture '%s'\n", asName.c_str());

	EndLoad();
	return pTexture;
}

//-----------------------------------------------------------------------

iTexture *cTextureManager::CreateCubeMap(const tString &asPathName, bool abUseMipMaps,
										 bool abCompress, eTextureType aType,
										 unsigned int alTextureSizeLevel) {
	tString sName = cString::SetFileExt(asPathName, "");

	iTexture *pTexture = static_cast<iTexture *>(GetByName(sName));

	BeginLoad(asPathName);

	if (pTexture == NULL) {
		// See if files for all faces exist
		tStringVec vPaths;
		tString sPath = "";
		for (int i = 0; i < 6; i++) {
			for (tStringListIt it = mlstFileFormats.begin(); it != mlstFileFormats.end(); ++it) {
				tString sNewName = sName + mvCubeSideSuffixes[i] + "." + *it;
				sPath = mpFileSearcher->GetFilePath(sNewName);

				if (sPath != "")
					break;
			}

			if (sPath == "") {
				tString sNewName = sName + mvCubeSideSuffixes[i];
				Error("Couldn't find %d-face '%s', for cubemap '%s'\n", i, sNewName.c_str(), sName.c_str());
				return NULL;
			}

			vPaths.push_back(sPath);
		}

		// Load bitmaps for all faces
		tBitmap2DVec vBitmaps;
		for (int i = 0; i < 6; i++) {
			Bitmap2D *pBmp = mpResources->GetLowLevel()->loadBitmap2D(vPaths[i]);
			if (pBmp == NULL) {
				Error("Couldn't load bitmap '%s'!\n", vPaths[i].c_str());
				for (int j = 0; j < (int)vBitmaps.size(); j++)
					hplDelete(vBitmaps[j]);
				EndLoad();
				return NULL;
			}

			vBitmaps.push_back(pBmp);
		}

		// Create the cubemap
		pTexture = mpGraphics->GetLowLevel()->CreateTexture(sName, abUseMipMaps, aType,
															eTextureTarget_CubeMap);
		pTexture->SetSizeLevel(alTextureSizeLevel);

		if (pTexture->CreateCubeFromBitmapVec(&vBitmaps) == false) {
			Error("Couldn't create cubemap '%s'!\n", sName.c_str());
			hplDelete(pTexture);
			for (int j = 0; j < (int)vBitmaps.size(); j++)
				hplDelete(vBitmaps[j]);
			EndLoad();
			return NULL;
		}

		// Bitmaps no longer needed.
		for (int j = 0; j < (int)vBitmaps.size(); j++)
			hplDelete(vBitmaps[j]);

		AddResource(pTexture);
	}

	if (pTexture)
		pTexture->IncUserCount();
	else
		Error("Couldn't texture '%s'\n", sName.c_str());

	EndLoad();
	return pTexture;
}

//-----------------------------------------------------------------------

iResourceBase *cTextureManager::Create(const tString &asName) {
	return Create2D(asName, true);
}

//-----------------------------------------------------------------------

void cTextureManager::Unload(iResourceBase *apResource) {
}
//-----------------------------------------------------------------------

void cTextureManager::Destroy(iResourceBase *apResource) {
	apResource->DecUserCount();

	if (apResource->HasUsers() == false) {
		RemoveResource(apResource);
		// Log("Deleting1 '%s'-%d\n",apResource->GetName().c_str() ,apResource);
		// Log("Deleting2 '%s'-%d\n",apResource->GetName().c_str() ,(iTexture*)apResource);
		// Log("Deleting1 %d\n",apResource);
		// Log("Deleting2 %d\n",(iTexture*)apResource);
		hplDelete(apResource);
	}
}

//-----------------------------------------------------------------------

void cTextureManager::Update(float afTimeStep) {
	tResourceHandleMapIt it = m_mapHandleResources.begin();
	for (; it != m_mapHandleResources.end(); ++it) {
		iResourceBase *pBase = it->second;
		iTexture *pTexture = static_cast<iTexture *>(pBase);

		pTexture->Update(afTimeStep);
	}
}

//-----------------------------------------------------------------------

iTexture *cTextureManager::CreateAttenuation(const tString &asFallOffName) {
	tString sName = cString::ToLowerCase(asFallOffName);
	tTextureAttenuationMapIt it = m_mapAttenuationTextures.find(sName);
	if (it != m_mapAttenuationTextures.end())
		return it->second;

	tString sPath = "";

	if (cString::GetFileExt(asFallOffName) != "") {
		sPath = mpFileSearcher->GetFilePath(asFallOffName);
	} else {
		for (tStringListIt it2 = mlstFileFormats.begin(); it2 != mlstFileFormats.end(); ++it2) {
			tString sFileName = cString::SetFileExt(asFallOffName, *it2);
			sPath = mpFileSearcher->GetFilePath(sFileName);
			if (sPath != "")
				break;
		}
	}
	if (sPath == "") {
		Log("Couldn't find falloff map file '%s'\n", asFallOffName.c_str());
		return NULL;
	}

	Bitmap2D *pBmp = mpResources->GetLowLevel()->loadBitmap2D(sPath);
	if (pBmp == NULL) {
		Log("Couldn't load bitmap '%s'\n", asFallOffName.c_str());
		return NULL;
	}

	int lBmpChannels = pBmp->getNumChannels();
	int lWidth = pBmp->getWidth();
	const unsigned char *pPixels = static_cast<const unsigned char *>(pBmp->getRawData());

	iTexture *pTexture = mpGraphics->GetLowLevel()->CreateTexture("Attenuation", false, eTextureType_Normal, eTextureTarget_3D);
	int lSize = 16;
	int lAttChannels = 2;

	cVector3f vCentre = ((float)lSize) / 2.0f;
	float fMaxDist = ((float)lSize) / 2.0f; // radius of sphere

	Common::Array<unsigned char> vAttenMap;
	vAttenMap.resize(lSize * lSize * lSize * lAttChannels);

	// Log("CREATTING ATTENUTAION MAP\n");
	for (int z = 0; z < lSize; ++z)
		for (int y = 0; y < lSize; ++y)
			for (int x = 0; x < lSize; ++x) {
				cVector3f vPos((float)x, (float)y, (float)z);
				vPos = vPos - vCentre;

				float fDist = vPos.Length();
				if (fDist > fMaxDist)
					fDist = fMaxDist;
				float fNormDist = fDist / fMaxDist;

				// unsigned char val = 255 - (unsigned char)(fNormDist * 255.0f);
				int lTexPos = (int)(fNormDist * (float)lWidth);
				if (lTexPos >= lWidth)
					lTexPos = lWidth - 1;
				unsigned char val = pPixels[lTexPos * lBmpChannels];

				for (int i = 0; i < lAttChannels; ++i) {
					vAttenMap[z * lSize * lSize * lAttChannels + y * lSize * lAttChannels + x * lAttChannels + i] = val;
				}
			}

	pTexture->CreateFromArray(&vAttenMap[0], lAttChannels, cVector3l(16, 16, 16));
	pTexture->SetWrapS(eTextureWrap_ClampToBorder);
	pTexture->SetWrapT(eTextureWrap_ClampToBorder);
	pTexture->SetWrapR(eTextureWrap_ClampToBorder);

	hplDelete(pBmp);

	m_mapAttenuationTextures.insert(tTextureAttenuationMap::value_type(sName, pTexture));

	return pTexture;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iTexture *cTextureManager::CreateFlatTexture(const tString &asName, bool abUseMipMaps,
											 bool abCompress, eTextureType aType, eTextureTarget aTarget,
											 unsigned int alTextureSizeLevel) {
	tString sPath;

	BeginLoad(asName);

	Common::ScopedPtr<iTexture> pTexture(FindTexture2D(asName, sPath));

	if (!pTexture && sPath != "") {
		// Load the bitmaps
		Common::ScopedPtr<Bitmap2D> bmp(mpLowLevelResources->loadBitmap2D(sPath));
		if (!bmp) {
			Hpl1::logError(Hpl1::kDebugResourceLoading, "Texturemanager Couldn't load bitmap '%s'\n", sPath.c_str());
			EndLoad();
			return nullptr;
		}

		// Create the texture and load from bitmap
		pTexture.reset(mpGraphics->GetLowLevel()->CreateTexture(asName, abUseMipMaps, aType,
																aTarget));
		if (!pTexture) {
			EndLoad();
			return nullptr;
		}
		pTexture->SetSizeLevel(alTextureSizeLevel);
		if (!pTexture->CreateFromBitmap(bmp.get())) {
			EndLoad();
			return nullptr;
		}

		AddResource(pTexture.get());
	}

	if (pTexture)
		pTexture->IncUserCount();
	else
		Hpl1::logError(Hpl1::kDebugResourceLoading, "texture '%s' is invalid\n", asName.c_str());

	EndLoad();
	return pTexture.release();
}

//-----------------------------------------------------------------------

iTexture *cTextureManager::FindTexture2D(const tString &asName, tString &asFilePath) {
	iTexture *pTexture = NULL;

	if (cString::GetFileExt(asName) == "") {
		for (tStringListIt it = mlstFileFormats.begin(); it != mlstFileFormats.end(); ++it) {
			tString sNewName = cString::SetFileExt(asName, *it);
			pTexture = static_cast<iTexture *>(FindLoadedResource(sNewName, asFilePath));

			if ((pTexture == NULL && asFilePath != "") || pTexture != NULL)
				break;
		}
	} else {
		pTexture = static_cast<iTexture *>(FindLoadedResource(asName, asFilePath));
	}

	return pTexture;
}

//-----------------------------------------------------------------------
} // namespace hpl
