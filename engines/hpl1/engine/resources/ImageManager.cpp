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

#include "hpl1/engine/resources/ImageManager.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/resources/FrameBitmap.h"
#include "hpl1/engine/resources/FrameTexture.h"
#include "hpl1/engine/resources/ResourceImage.h"
#include "hpl1/engine/resources/low_level_resources.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cImageManager::cImageManager(cFileSearcher *apFileSearcher, iLowLevelGraphics *apLowLevelGraphics,
							 LowLevelResources *apLowLevelResources, LowLevelSystem *apLowLevelSystem)
	: iResourceManager(apFileSearcher, apLowLevelResources, apLowLevelSystem) {
	mpLowLevelGraphics = apLowLevelGraphics;

	mpLowLevelResources->getSupportedImageFormats(mlstFileFormats);

	mvFrameSize = cVector2l(512, 512);
	mlFrameHandle = 0;
}

cImageManager::~cImageManager() {
	// DeleteAllBitmapFrames();
	DestroyAll();

	Log(" Done with images\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iResourceBase *cImageManager::Create(const tString &asName) {
	return CreateInFrame(asName, -1);
}
//-----------------------------------------------------------------------

iResourceBase *cImageManager::CreateInFrame(const tString &asName, int alFrameHandle) {
	cResourceImage *pImage = NULL;
	tString sPath;

	BeginLoad(asName);

	pImage = FindImage(asName, sPath);
	if (!pImage) {
		if (sPath != "") {
			Bitmap2D *pBmp;
			pBmp = mpLowLevelResources->loadBitmap2D(sPath);
			if (pBmp == NULL) {
				Error("Imagemanager Couldn't load bitmap '%s'\n", sPath.c_str());
				EndLoad();
				return NULL;
			}

			pImage = AddToFrame(pBmp, alFrameHandle);

			hplDelete(pBmp);

			if (pImage == NULL) {
				Error("Imagemanager couldn't create image '%s'\n", asName.c_str());
			}

			if (pImage)
				AddResource(pImage);
		}
	} else {
		// Log("Found '%s' in stock!\n",asName.c_str());
	}

	if (pImage)
		pImage->IncUserCount();
	else
		Error("Couldn't load image '%s'\n", asName.c_str());

	// Log("Loaded image %s, it has %d users!\n", pImage->GetName().c_str(),pImage->GetUserCount());
	// Log(" frame has %d pics\n", pImage->GetFrameTexture()->GetPicCount());

	EndLoad();
	return pImage;
}

//-----------------------------------------------------------------------

cResourceImage *cImageManager::CreateImage(const tString &asName, int alFrameHandle) {
	iResourceBase *pRes = CreateInFrame(asName, alFrameHandle);
	cResourceImage *pImage = static_cast<cResourceImage *>(pRes);

	return pImage;
}

//-----------------------------------------------------------------------

cResourceImage *cImageManager::CreateFromBitmap(const tString &asName, Bitmap2D *apBmp, int alFrameHandle) {
	if (apBmp == NULL)
		return NULL;

	cResourceImage *pImage = AddToFrame(apBmp, alFrameHandle);

	if (pImage) {
		AddResource(pImage, false);
		pImage->IncUserCount();
	}

	return pImage;
}

//-----------------------------------------------------------------------

void cImageManager::Unload(iResourceBase *apResource) {
}

void cImageManager::Destroy(iResourceBase *apResource) {
	// Lower the user num for the the resource. If it is 0 then lower the
	// user num for the TextureFrame and delete the resource. If the Texture
	// frame reaches 0 it is deleted as well.
	cResourceImage *pImage = static_cast<cResourceImage *>(apResource);
	cFrameTexture *pFrame = pImage->GetFrameTexture();
	cFrameBitmap *pBmpFrame = pImage->GetFrameBitmap();

	// pImage->GetFrameBitmap()->FlushToTexture(); Not needed?

	// Log("Users Before: %d\n",pImage->GetUserCount());
	// Log("Framepics Before: %d\n",pFrame->GetPicCount());

	pImage->DecUserCount(); // dec frame count as well.. is that ok?

	// Log("---\n");
	// Log("Destroyed Image: '%s' Users: %d\n",pImage->GetName().c_str(),pImage->GetUserCount());
	// Log("Frame %d has left Pics: %d\n",pFrame,pFrame->GetPicCount());

	if (pImage->HasUsers() == false) {
		pFrame->DecPicCount(); // Doing it here now instead.
		pBmpFrame->DecPicCount();
		RemoveResource(apResource);
		hplDelete(apResource);

		// Log("deleting image and dec fram to %d images!\n",pFrame->GetPicCount());
	}

	if (pFrame->IsEmpty()) {
		// Log(" Deleting frame...");

		// Delete the bitmap frame that has this this frame.
		for (tFrameBitmapListIt it = mlstBitmapFrames.begin(); it != mlstBitmapFrames.end(); ++it) {
			cFrameBitmap *pBmpFrame2 = *it;
			if (pBmpFrame2->GetFrameTexture() == pFrame) {
				// Log("and bitmap...");
				hplDelete(pBmpFrame2);
				mlstBitmapFrames.erase(it);
				break;
			}
		}

		// delete from list
		m_mapTextureFrames.erase(pFrame->GetHandle());
		hplDelete(pFrame);
		// Log(" Deleted frame!\n");
	}
	// Log("---\n");
}

//-----------------------------------------------------------------------

void cImageManager::DeleteAllBitmapFrames() {
	FlushAll();
	for (tFrameBitmapListIt it = mlstBitmapFrames.begin(); it != mlstBitmapFrames.end();) {
		hplDelete(*it);
		it = mlstBitmapFrames.erase(it);
	}
}

//-----------------------------------------------------------------------

int cImageManager::FlushAll() {
	// Log("Flushing...");
	int lNum = 0;
	for (tFrameBitmapListIt it = mlstBitmapFrames.begin(); it != mlstBitmapFrames.end();) {
		if ((*it)->FlushToTexture())
			lNum++;

		if ((*it)->IsFull()) {
			// Do not delete all, probably this struct needs to be here for easy access :S
			// hplDelete(*it);
			// it = mlstBitmapFrames.erase(it);
			it++;
		} else {
			it++;
		}
	}

	// Log("Done!\n");

	return lNum;
}

//-----------------------------------------------------------------------

int cImageManager::CreateFrame(cVector2l avSize) {
	cFrameBitmap *pBFrame = CreateBitmapFrame(avSize);

	if (pBFrame == NULL)
		return -1;

	return pBFrame->GetHandle();
}

//-----------------------------------------------------------------------

void cImageManager::SetFrameLocked(int alHandle, bool abLocked) {
	tFrameBitmapListIt it = mlstBitmapFrames.begin();
	while (it != mlstBitmapFrames.end()) {
		if ((*it)->GetHandle() == alHandle) {
			(*it)->SetLocked(abLocked);
			break;
		}
		it++;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cResourceImage *cImageManager::FindImage(const tString &asName, tString &asFilePath) {
	cResourceImage *pImage = NULL;

	if (cString::GetFileExt(asName) == "") {
		for (tStringListIt it = mlstFileFormats.begin(); it != mlstFileFormats.end(); ++it) {
			tString sNewName = cString::SetFileExt(asName, *it);
			pImage = static_cast<cResourceImage *>(FindLoadedResource(sNewName, asFilePath));

			if ((pImage == NULL && asFilePath != "") || pImage != NULL)
				break;
		}
	} else {
		pImage = static_cast<cResourceImage *>(FindLoadedResource(asName, asFilePath));
	}

	return pImage;
}

//-----------------------------------------------------------------------

cResourceImage *cImageManager::AddToFrame(Bitmap2D *apBmp, int alFrameHandle) {
	bool bFound = false;
	cResourceImage *pImage = NULL;

	if (mlstBitmapFrames.size() == 0) {
		CreateBitmapFrame(mvFrameSize);
	}

	if (alFrameHandle < 0) {
		// Search the frames til one is find that fits the bitmap
		for (tFrameBitmapListIt it = mlstBitmapFrames.begin(); it != mlstBitmapFrames.end(); it++) {
			if (!(*it)->IsFull() && !(*it)->IsLocked()) {
				pImage = (*it)->AddBitmap(apBmp);
				if (pImage != NULL) {
					bFound = true;
					break;
				}
			}
		}

		// If it fitted in none of the frames, create a new and put it in that.
		if (!bFound) {
			// Log("No fit!\n");
			// not 100% it fits in this one...if so maybe the bitmap size of the frame
			// should be changed?

			// pImage = CreateBitmapFrame(mvFrameSize)->AddBitmap(apBmp);
			cFrameBitmap *pFrame = CreateBitmapFrame(mvFrameSize);
			if (pFrame) {
				pImage = pFrame->AddBitmap(apBmp);
				if (pImage == NULL) {
					Log("No fit in new frame!\n");
				}
			}
		}
	} else {
		tFrameBitmapListIt it = mlstBitmapFrames.begin();
		while (it != mlstBitmapFrames.end()) {
			if ((*it)->GetHandle() == alFrameHandle) {
				pImage = (*it)->AddBitmap(apBmp);
				break;
			}
			it++;
		}
		if (pImage == NULL)
			Error("Image didn't fit frame %d!\n", alFrameHandle);
	}

	return pImage;
}

//-----------------------------------------------------------------------

cFrameBitmap *cImageManager::CreateBitmapFrame(cVector2l avSize) {
	iTexture *pTex = mpLowLevelGraphics->CreateTexture(false, eTextureType_Normal, eTextureTarget_2D);
	cFrameTexture *pTFrame = hplNew(cFrameTexture, (pTex, mlFrameHandle));
	Bitmap2D *pBmp = mpLowLevelGraphics->CreateBitmap2D(avSize);
	cFrameBitmap *pBFrame = hplNew(cFrameBitmap, (pBmp, pTFrame, mlFrameHandle));

	mlstBitmapFrames.push_back(pBFrame);

	auto ret = m_mapTextureFrames.insert(tFrameTextureMap::value_type(mlFrameHandle, pTFrame));
	if (ret.second == false) {
		Error("Could not add texture frame %d with handle %d! Handle already exist!\n", pTFrame, mlFrameHandle);
	} else {
		// Log("Added texture frame: %d\n",pTFrame);
	}

	mlFrameHandle++;
	return pBFrame;
}

//-----------------------------------------------------------------------
} // namespace hpl
