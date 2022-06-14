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

#include "hpl1/engine/resources/FrameBitmap.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/system/LowLevelSystem.h"
#include "hpl1/engine/resources/FrameTexture.h"
#include "hpl1/engine/resources/ResourceImage.h"
#include "hpl1/engine/graphics/Bitmap2D.h"
#include "hpl1/engine/graphics/Texture.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cFrameBitmap::cFrameBitmap(iBitmap2D *apBitmap,  cFrameTexture *apFrmTex, int alHandle) : iFrameBase()
	{
		mpBitmap = apBitmap;
		mpFrameTexture = apFrmTex;
		mpBitmap->FillRect(cRect2l(0,0,0,0), cColor(1,1));
		mlMinHole = 6;
		mlHandle = alHandle;
		mbIsFull = false;
		mbIsLocked = false;

		//Root node in rect tree
		mRects.Insert(cFBitmapRect(0,0,mpBitmap->GetWidth(), mpBitmap->GetHeight(),-1));
	}

	cFrameBitmap::~cFrameBitmap()
	{
		hplDelete(mpBitmap);
		mpBitmap = NULL;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	#define DEBUG_BTREE (false)

	cResourceImage *cFrameBitmap::AddBitmap(iBitmap2D *apSrc)
	{
		cResourceImage *pImage=NULL;
		//source size
		//+2 because we are gonna have a border to get rid if some antialiasing problems
		int lSW = apSrc->GetWidth()+2;
		int lSH = apSrc->GetHeight()+2;

		//destination size
		int lDW = mpBitmap->GetWidth();
		int lDH = mpBitmap->GetHeight();

		cVector2l vPos;

		bool bFoundEmptyNode = false;
		bool bFoundNode = false;
		//Debug
		int node=0;

		if(DEBUG_BTREE)Log("**** Image %d *****\n",mlPicCount);

		//Get the leaves of the tree and search it for a good pos.
		tRectTreeNodeList lstNodes =  mRects.GetLeafList();
		tRectTreeNodeListIt it;
		for(it = lstNodes.begin();it!=lstNodes.end();++it)
		{
			if(DEBUG_BTREE)Log("Checking node %d:\n",node++);
			tRectTreeNode *TopNode = *it;
			cFBitmapRect* pData = TopNode->GetData();

			//Check if the space is free
			if(pData->mlHandle<0)
			{
				if(DEBUG_BTREE)Log("Found free node\n");
				bFoundEmptyNode = true; //An empty node was found.. bitmap not full yet.

				//Check if the Image fits in the rect
				cRect2l NewRect = cRect2l(pData->mRect.x,pData->mRect.y,lSW, lSH);
				if(DEBUG_BTREE)Log("Fit: [%d:%d:%d:%d] in [%d:%d:%d:%d]\n",
						NewRect.x,NewRect.y,NewRect.w,NewRect.h,
						pData->mRect.x,pData->mRect.y,pData->mRect.w,pData->mRect.h);

				if(cMath::BoxFit(NewRect, pData->mRect))
				{
					if(DEBUG_BTREE)Log("The node fits!\n");
					bFoundNode = true;

					//If the bitmap fits perfectly add the node without splitting
					if(MinimumFit(NewRect,pData->mRect))
					{
						if(DEBUG_BTREE)Log("Minimum fit!\n");
						pData->mRect = NewRect;
						pData->mlHandle = 1;
					}
					//If there is still space left, make new nodes.
					else
					{
						if(DEBUG_BTREE)Log("Normal fit!\n");
						//Insert 2 children for the top node (lower and upper part.
						tRectTreeNode* UpperNode;
						//Upper
						UpperNode = mRects.InsertAt(cFBitmapRect(NewRect.x,NewRect.y,
													pData->mRect.w,NewRect.h,-2),TopNode,
													eBinTreeNode_Left);

						//Lower
						mRects.InsertAt(cFBitmapRect(NewRect.x,NewRect.y+NewRect.h,
										pData->mRect.w,pData->mRect.h-NewRect.h,-3),TopNode,
										eBinTreeNode_Right);

						//Split the Upper Node into 2 nodes.
						pData = UpperNode->GetData();//Get the data for the upper node.
						//Upper split, this is the new bitmap
						mRects.InsertAt(cFBitmapRect(NewRect.x,NewRect.y,
										NewRect.w,NewRect.h,2),UpperNode,
										eBinTreeNode_Left);

						//Lower split, this is empty
						mRects.InsertAt(cFBitmapRect(NewRect.x+NewRect.w,NewRect.y,
										pData->mRect.w-NewRect.w,NewRect.h,-4),UpperNode,
										eBinTreeNode_Right);
					}

					vPos = cVector2l(NewRect.x+1,NewRect.y+1);//+1 for the right pos

					//Draw 4 times so we get a nice extra border
					for(int i=0;i<2;i++)for(int j=0;j<2;j++){
						apSrc->DrawToBitmap(mpBitmap,cVector2l(NewRect.x+i*2,NewRect.y+j*2));
					}
					//Fix the border a little more:
					for(int i=-1;i<2;i++)for(int j=-1;j<2;j++)
						if((i==0 || j==0) && (i!=j)){
						apSrc->DrawToBitmap(mpBitmap,cVector2l(NewRect.x+1+i,NewRect.y+1+j));
					}
					//Draw the final
					apSrc->DrawToBitmap(mpBitmap,cVector2l(NewRect.x+1,NewRect.y+1));


					mlPicCount++;
					mpFrameTexture->SetPicCount(mlPicCount);
					break;
				}
			}
		}

		if(bFoundNode)
		{
			//Create the image resource
			pImage = hplNew( cResourceImage, (apSrc->GetFileName(),mpFrameTexture, this,
				cRect2l(vPos,cVector2l(lSW-2,lSH-2)),//-2 to get the correct size.
				cVector2l(mpBitmap->GetWidth(),mpBitmap->GetHeight()),
				mlHandle) );

			if(!bFoundEmptyNode)
			{
				mbIsFull = true;
			}

			mbIsUpdated = true;
		}



		/// LAST DEBUG ///
		if(DEBUG_BTREE)
		{
			Log("Current Tree begin:\n");
			tRectTreeNodeList lstNodes =  mRects.GetNodeList();
			tRectTreeNodeListIt it;
			int node=0;
			for(it = lstNodes.begin();it!=lstNodes.end();++it)
			{
				cRect2l Rect = (*it)->GetData()->mRect;
				int h = (*it)->GetData()->mlHandle;
				Log(" %d: [%d:%d:%d:%d]:%d\n",node,Rect.x,Rect.y,Rect.w,Rect.h,h);
				node++;
			}
			Log("Current Tree end:\n");
			Log("-----------------\n");

			Log("Current Leaves begin:\n");
			lstNodes =  mRects.GetLeafList();
			node=0;
			for(it = lstNodes.begin();it!=lstNodes.end();++it)
			{
				cRect2l Rect = (*it)->GetData()->mRect;
				int h = (*it)->GetData()->mlHandle;
				Log(" %d: [%d:%d:%d:%d]: %d\n",node,Rect.x,Rect.y,Rect.w,Rect.h,h);
				node++;
			}
			Log("Current Tree end:\n");
			Log("-----------------\n");
		}


		return pImage;
	}

	//-----------------------------------------------------------------------

	bool cFrameBitmap::MinimumFit(cRect2l aSrc,cRect2l aDest)
	{
		if(aDest.w-aSrc.w<mlMinHole && aDest.h-aSrc.h<mlMinHole) return true;

		return false;
	}

	//-----------------------------------------------------------------------

	bool cFrameBitmap::IsUpdated()
	{
		return mbIsUpdated;
	}

	//-----------------------------------------------------------------------

	bool cFrameBitmap::IsFull()
	{
		return mbIsFull;
	}

	//-----------------------------------------------------------------------

	bool cFrameBitmap::FlushToTexture()
	{
		if(mbIsUpdated)
		{
			mpFrameTexture->GetTexture()->CreateFromBitmap(mpBitmap);
			mpFrameTexture->GetTexture()->SetWrapS(eTextureWrap_ClampToEdge);
			mpFrameTexture->GetTexture()->SetWrapT(eTextureWrap_ClampToEdge);

			//mpFrameTexture->SetPicCount(mlPicCount);
			mbIsUpdated = false;
			return true;
		}
		else
		{
			return false;
		}
	}

	//-----------------------------------------------------------------------
}
