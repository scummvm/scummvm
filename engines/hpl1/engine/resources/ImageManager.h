/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HPL_IMAGEMANAGER_H
#define HPL_IMAGEMANAGER_H

#include "hpl1/engine/resources/ResourceManager.h"
#include "hpl1/engine/math/MathTypes.h"

namespace hpl {

	class cResourceImage;
	class cFrameTexture;
	class cFrameBitmap;
	class iLowLevelGraphics;
	class iBitmap2D;

	typedef std::list<cFrameBitmap*> tFrameBitmapList;
	typedef tFrameBitmapList::iterator tFrameBitmapListIt;

	typedef std::map<int,cFrameTexture*> tFrameTextureMap;
	typedef tFrameTextureMap::iterator tFrameTextureMapIt;

	class cImageManager :public iResourceManager
	{
	public:
		cImageManager(cFileSearcher *apFileSearcher, iLowLevelGraphics *apLowLevelGraphics,
						iLowLevelResources *apLowLevelResources,iLowLevelSystem *apLowLevelSystem);
		~cImageManager();

		iResourceBase* Create(const tString& asName);

		void Destroy(iResourceBase* apResource);

		void Unload(iResourceBase* apResource);

		//Image specifc
		iResourceBase* CreateInFrame(const tString& asName, int alFrameHandle);
		cResourceImage* CreateImage(const tString& asName, int alFrameHandle=-1);
		/**
		 * Draws all updated content to textures. THis must be done before a loaded image can be used.
		 * Use this as unoften as possible.
		 * \return Number of bitmaps flushes
		 */
		int FlushAll();
		void DeleteAllBitmapFrames();

		cResourceImage* CreateFromBitmap(const tString &asName,iBitmap2D* apBmp, int alFrameHandle=-1);

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
		cResourceImage *AddToFrame(iBitmap2D *apBmp,int alFrameHandle);
		cFrameBitmap *CreateBitmapFrame(cVector2l avSize);

	};

};
#endif // HPL_RESOURCEMANAGER_H
