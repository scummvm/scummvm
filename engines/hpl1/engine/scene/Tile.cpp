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
#include "hpl1/engine/scene/Tile.h"
#include "hpl1/engine/scene/TileMap.h"
#include "hpl1/engine/scene/TileData.h"
#include "hpl1/engine/scene/TileSet.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cTile::cTile(iTileData* apTileData, eTileRotation aAngle,
				const cVector3f &avPos, const cVector2f &avSize, cCollisionMesh2D* apCollMesh)
	{
		mvPosition = avPos + avSize/2;

		mpTileData = NULL;
		mAngle = aAngle;

		mpCollMesh = apCollMesh;

		mlFlags =0;

		//SetTile(apTileData,aAngle,avPos,avSize);
	}

	//-----------------------------------------------------------------------

	cTile::~cTile()
	{
		if(mpCollMesh) hplDelete(mpCollMesh);
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cTile::SetTile(iTileData* apTileData, eTileRotation aAngle,
						const cVector3f &avPos, const cVector2f &avSize)
	{
		mpTileData = apTileData;
		mAngle = aAngle;
	}

	//-----------------------------------------------------------------------
}
