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

#ifndef HPL_TILE_H
#define HPL_TILE_H

#include "common/list.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/scene/TileData.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

#define eTileFlag_Breakable (eFlagBit_0)

class cTile {
public:
	cTile(iTileData *apTileData, eTileRotation aAngle,
		  const cVector3f &avPos, const cVector2f &avSize, cCollisionMesh2D *apCollMesh);
	~cTile();

	iTileData *GetTileData() { return mpTileData; }
	void SetTileData(iTileData *apTileData) { mpTileData = apTileData; }

	eTileRotation GetAngle() { return mAngle; }

	void SetTile(iTileData *apTileData, eTileRotation aAngle,
				 const cVector3f &avPos, const cVector2f &avSize);
	void SetAngle(eTileRotation aAngle) { mAngle = aAngle; }

	const cVector3f &GetPosition() const { return mvPosition; }
	cVector3f *GetPositionPtr() { return &mvPosition; }

	cCollisionMesh2D *GetCollisionMesh() { return mpCollMesh; }
	void SetCollisionMesh(cCollisionMesh2D *apMesh) { mpCollMesh = apMesh; }

	tFlag GetFlags() { return mlFlags; }
	void SetFlags(tFlag alFlags) { mlFlags = alFlags; }

private:
	cCollisionMesh2D *mpCollMesh;

	tFlag mlFlags;
	iTileData *mpTileData;
	cVector3f mvPosition;
	eTileRotation mAngle; // 0:normal.  1:90 CW.  2:180. CW  3:270 CW.
						  // 4: flip horisontally.  5: flip vertically.  6: flip both.
};

typedef Common::List<cTile *> tTileList;
typedef tTileList::iterator tTileListIt;

} // namespace hpl

#endif // HPL_TILE_H
