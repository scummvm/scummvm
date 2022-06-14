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

#include "hpl1/engine/resources/ResourceImage.h"
#include "hpl1/engine/resources/FrameTexture.h"
#include "hpl1/engine/graphics/Texture.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

#define kContractSize (0.001f)

	cResourceImage::cResourceImage(tString asName,cFrameTexture *apFrameTex,
		cFrameBitmap *apFrameBmp, cRect2l aRect,
		cVector2l avSrcSize, int alHandle) : iResourceBase(asName,0)
	{
		mpFrameTexture = apFrameTex;
		mpFrameBitmap = apFrameBmp;
		mRect = aRect;
		mvSourceSize = avSrcSize;
		mlHandle = alHandle;

		cVector2f vTexSize = cVector2f((float)mRect.w,(float)mRect.h ) /
								cVector2f((float)mvSourceSize.x,(float)mvSourceSize.y);
		cVector2f vTexPos = cVector2f((float)mRect.x,(float)mRect.y ) /
								cVector2f((float)mvSourceSize.x,(float)mvSourceSize.y);

		mvVtx.push_back(cVertex(cVector3f(0,0,0),
						cVector3f(vTexPos.x+kContractSize, vTexPos.y+kContractSize,0), cColor(1)));

		mvVtx.push_back(cVertex(cVector3f((float)mRect.w,0,0),
						cVector3f(vTexPos.x+vTexSize.x-kContractSize, vTexPos.y+kContractSize,0),
						cColor(1)));

		mvVtx.push_back(cVertex(cVector3f((float)mRect.w,(float)mRect.h,0),
						cVector3f(vTexPos.x+vTexSize.x-kContractSize, vTexPos.y+vTexSize.y-kContractSize,0),
						cColor(1)));

		mvVtx.push_back(cVertex(cVector3f(0,(float)mRect.h,0),
						cVector3f(vTexPos.x+kContractSize, vTexPos.y+vTexSize.y-kContractSize,0),
						cColor(1)));
	}

	//-----------------------------------------------------------------------

	cResourceImage::~cResourceImage()
	{
		mvVtx.clear();
		//mpFrameTexture->DecPicCount();
		mpFrameTexture = NULL;
		mlHandle = -1;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iTexture *cResourceImage::GetTexture()const{return mpFrameTexture->GetTexture();}

	//-----------------------------------------------------------------------

	tVertexVec cResourceImage::GetVertexVecCopy(const cVector2f &avPos, const cVector2f &avSize)
	{
		tVertexVec vTmpVtx = mvVtx;

		if(avSize == cVector2f(-1,-1)) {
			vTmpVtx[1].pos.x = mvVtx[0].pos.x + mRect.w;
			vTmpVtx[2].pos.x = mvVtx[0].pos.x + mRect.w;
			vTmpVtx[2].pos.y = mvVtx[0].pos.y + mRect.h;
			vTmpVtx[3].pos.y = mvVtx[0].pos.y + mRect.h;
		}
		else {
			vTmpVtx[1].pos.x = mvVtx[0].pos.x + avSize.x;
			vTmpVtx[2].pos.x = mvVtx[0].pos.x + avSize.x;
			vTmpVtx[2].pos.y = mvVtx[0].pos.y + avSize.y;
			vTmpVtx[3].pos.y = mvVtx[0].pos.y + avSize.y;
		}

		for(int i=0;i<4;i++)
			vTmpVtx[i].pos+=avPos;

		return vTmpVtx;
	}

	//-----------------------------------------------------------------------

	bool cResourceImage::Reload()
	{
		return false;
	}

	//-----------------------------------------------------------------------

	void cResourceImage::Unload()
	{
	}

	//-----------------------------------------------------------------------

	void cResourceImage::Destroy()
	{
	}

	//-----------------------------------------------------------------------

}
