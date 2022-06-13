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
#include "hpl1/engine/scene/TileLayer.h"
#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cTileLayer::cTileLayer(unsigned int alW, unsigned int alH, bool abCollision,bool abLit,
							eTileLayerType aType,float afZ)
	{
		mvSize = cVector2l(alW, alH);
		mbCollision = abCollision;
		mbLit = abLit;
		mType = aType;
		mfZ = afZ;

		mvTile.resize(alW*alH);
		mvTile.assign(mvTile.size(),NULL);
	}

	//-----------------------------------------------------------------------

	cTileLayer::~cTileLayer()
	{
		Log(" Deleting tilelayer.\n");

		for(int i=0;i<(int)mvTile.size();i++)
			if(mvTile[i])hplDelete(mvTile[i]);

		mvTile.clear();
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	bool cTileLayer::SetTile(unsigned int alX,unsigned int alY,cTile *aVal)
	{
		int lNum = alY*mvSize.x+alX;
		if(lNum>=(int)mvTile.size()) return false;

		if(aVal == NULL)
		{
			if(mvTile[lNum])hplDelete(mvTile[lNum]);
			mvTile[lNum] = NULL;
		}
		else
		{
			if(mvTile[lNum]==NULL)
				mvTile[lNum] = aVal;
			else
				*mvTile[lNum] = *aVal;
		}

		return true;
	}

	//-----------------------------------------------------------------------

	cTile* cTileLayer::GetAt(int alX, int alY)
	{
		if(alX<0 || alX>=mvSize.x || alY<0 || alY>=mvSize.y) return NULL;

		int lNum = alY*mvSize.x+alX;
		if(lNum>=(int)mvTile.size())return NULL;

		return  mvTile[lNum];
	}

	//-----------------------------------------------------------------------

	cTile* cTileLayer::GetAt(int alNum)
	{
		return  mvTile[alNum];
	}

	//-----------------------------------------------------------------------

}
