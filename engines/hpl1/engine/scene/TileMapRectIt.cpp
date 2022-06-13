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
#include "hpl1/engine/scene/TileMapRectIt.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cTileMapRectIt::cTileMapRectIt(cVector2l avPos, cVector2l avSize, cTileMap* apTileMap, int alLayer)
	{
		mpTileMap = apTileMap;
		mpTile = NULL;

		mvPos = avPos;
		mvSize = avSize;

		mlLayer = alLayer;
		mlLayerCount =0;
		mlCurrentLayer = 0;

		//// Clip the pos and size //////
		//Pos smaller then (0,0)
		if(mvPos.x<0){mvSize.x+=mvPos.x; mvPos.x=0;}
		if(mvPos.y<0){mvSize.y+=mvPos.y; mvPos.y=0;}

		//Size large than grid map
		if(mvPos.x + mvSize.x > mpTileMap->mvSize.x){
			mvSize.x -= (mvPos.x + mvSize.x) - (mpTileMap->mvSize.x);
		}
		if(mvPos.y + mvSize.y > mpTileMap->mvSize.y){
			mvSize.y -= (mvPos.y + mvSize.y) - (mpTileMap->mvSize.y);
		}

		mlTileNum = mvPos.x + mvPos.y*mpTileMap->mvSize.x;
		mlTileColAdd = mpTileMap->mvSize.x-mvSize.x;
		mlTileRowCount = mvSize.y;
		mlTileColCount = mvSize.x;

		//The rect is outside of the tilemap
		if(mvSize.x<=0 || mvSize.y<=0 || mvPos.x>=mpTileMap->mvSize.x || mvPos.y>=mpTileMap->mvSize.y ){
			mlTileColCount=0;
			mlTileRowCount=0;
			mlLayerCount = (int)mpTileMap->mvTileLayer.size();
		}
		else{
		}

		mbUpdated = false;
	}

	//-----------------------------------------------------------------------

	cTileMapRectIt::~cTileMapRectIt()
	{
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	bool cTileMapRectIt::HasNext()
	{
		GetTile();

		return mpTile!=NULL;
	}

	//-----------------------------------------------------------------------

	cTile* cTileMapRectIt::Next()
	{
		GetTile();

		mbUpdated = false;
		return mpTile;
	}

	//-----------------------------------------------------------------------


	cTile* cTileMapRectIt::PeekNext()
	{
		GetTile();

		return mpTile;
	}

	//-----------------------------------------------------------------------

	int cTileMapRectIt::GetNum()
	{
		return mlTileNum;
	}

	//-----------------------------------------------------------------------

	int cTileMapRectIt::GetCurrentLayer()
	{
		return mlCurrentLayer;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cTileMapRectIt::GetTile()
	{
		if(mbUpdated)return;
		mbUpdated = true;

		//We are gonna check till we find a non NULL value or the end.
		while(true)
		{
			//Check if end of the this tile pos
			if( (mlLayer>=0 && mlLayerCount>0) || (mlLayer==-1 && mlLayerCount>= (int)mpTileMap->mvTileLayer.size()) )
			{
				mlLayerCount=0;

				mlTileNum++;
				mlTileColCount--;
				if(mlTileColCount<=0)
				{
					//Log("New row!\n\n");
					mlTileColCount=mvSize.x;
					mlTileRowCount--;
					if(mlTileRowCount<=0)
					{
						mpTile=NULL;
						break;
					}
					mlTileNum+=mlTileColAdd;
				}
			}
			else
			{
				if(mlLayer<0){
					mpTile = mpTileMap->mvTileLayer[mlLayerCount]->mvTile[mlTileNum];
					mlCurrentLayer = mlLayerCount;
				}
				else{
					mpTile = mpTileMap->mvTileLayer[mlLayer]->mvTile[mlTileNum];
					mlCurrentLayer = mlLayer;
				}

				mlLayerCount++;

				if(mpTile!=NULL){
					iTileData* pData = mpTile->GetTileData();
					if(pData && pData->IsSolid()){
						mlLayerCount = (int)mpTileMap->mvTileLayer.size();
					}
					break;
				}
			}
		}
	}

	//-----------------------------------------------------------------------

}
