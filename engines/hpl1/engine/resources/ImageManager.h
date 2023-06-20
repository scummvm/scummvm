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

#ifndef HPL_IMAGEMANAGER_H
#define HPL_IMAGEMANAGER_H

#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/resources/ResourceManager.h"

namespace hpl {

class cResourceImage;
class cFrameTexture;
class cFrameBitmap;
class iLowLevelGraphics;
class Bitmap2D;

typedef Common::List<cFrameBitmap *> tFrameBitmapList;
typedef tFrameBitmapList::iterator tFrameBitmapListIt;

typedef Common::StableMap<int, cFrameTexture *> tFrameTextureMap;
typedef tFrameTextureMap::iterator tFrameTextureMapIt;

class cImageManager : public iResourceManager {
public:
	cImageManager(cFileSearcher *apFileSearcher, iLowLevelGraphics *apLowLevelGraphics,
				  LowLevelResources *apLowLevelResources, LowLevelSystem *apLowLevelSystem);
	~cImageManager();

	iResourceBase *Create(const tString &asName);

	void Destroy(iResourceBase *apResource);

	void Unload(iResourceBase *apResource);

	// Image specifc
	iResourceBase *CreateInFrame(const tString &asName, int alFrameHandle);
	cResourceImage *CreateImage(const tString &asName, int alFrameHandle = -1);
	/**
	 * Draws all updated content to textures. THis must be done before a loaded image can be used.
	 * Use this as unoften as possible.
	 * \return Number of bitmaps flushes
	 */
	int FlushAll();
	void DeleteAllBitmapFrames();

	cResourceImage *CreateFromBitmap(const tString &asName, Bitmap2D *apBmp, int alFrameHandle = -1);

	int CreateFrame(cVector2l avSize);
	void SetFrameLocked(int alHandle, bool abLocked);

private:
	iLowLevelGraphics *mpLowLevelGraphics;

	tFrameBitmapList mlstBitmapFrames;
	tFrameTextureMap m_mapTextureFrames;

	tStringList mlstFileFormats;
	cVector2l mvFrameSize;
	int mlFrameHandle;

	cResourceImage *FindImage(const tString &asName, tString &asFilePath);
	cResourceImage *AddToFrame(Bitmap2D *apBmp, int alFrameHandle);
	cFrameBitmap *CreateBitmapFrame(cVector2l avSize);
};

} // namespace hpl

#endif // HPL_RESOURCEMANAGER_H
