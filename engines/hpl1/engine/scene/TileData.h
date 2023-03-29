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

#ifndef HPL_TILEDATA_H
#define HPL_TILEDATA_H

#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/Mesh2d.h"
#include "hpl1/engine/resources/ImageManager.h"
#include "hpl1/engine/resources/ResourceImage.h"

namespace hpl {

enum eTileCollisionType {
	eTileCollisionType_None,
	eTileCollisionType_Normal,
	eTileCollisionType_OnlyDown,
	eTileCollisionType_LastEnum
};

enum eTileDataType {
	eTileDataType_Normal,
	eTileDataType_LastEnum
};

/////// INTERFACE ///////////////////////
class iTileData {
public:
	virtual ~iTileData() {}

	virtual void Destroy() = 0;
	virtual bool IsSolid() = 0;
	virtual eTileDataType GetType() = 0;

	/*virtual tVertexVec GetVertexVec(const cVector3f &avPos, const cVector2f &avSize,
									unsigned char acAngle)=0;*/
};

/////// IMPLEMENTATIONS OF iTILEDATA ////////////

class cTileDataNormal : public iTileData {
public:
	cTileDataNormal(cImageManager *apImageManager, cVector2f avTileSize);
	~cTileDataNormal();

	void Destroy();
	bool IsSolid() { return mbIsSolid; }
	void SetIsSolid(bool abIsSolid) { mbIsSolid = abIsSolid; }

	eTileCollisionType GetCollisionType() { return mCollisionType; }
	void SetCollisionType(eTileCollisionType aCollisionType) { mCollisionType = aCollisionType; }

	eTileDataType GetType() { return eTileDataType_Normal; }
	tVertexVec *GetVertexVec(eTileRotation aRotation);
	tUIntVec *GetIndexVec(eTileRotation aRotation);

	tVertexVec *GetCollideVertexVec(eTileRotation aRotation);
	tUIntVec *GetCollideIndexVec(eTileRotation aRotation);

	iMaterial *GetMaterial() { return mpMaterial; }

	cMesh2D *GetMesh() { return mpMesh; }

	void SetData(cMesh2D *apMesh, iMaterial *apMaterial);

	cMesh2D *GetCollideMesh();
	void SetCollideMesh(cMesh2D *apCollideMesh);

private:
	cImageManager *mpImageManager;

	tResourceImageVec mvImage;
	iMaterial *mpMaterial;
	tVertexVec *mvVtx[eTileRotation_LastEnum];
	tVertexVec *mvCollideVtx[eTileRotation_LastEnum];
	tUIntVec *mvIdx;
	tUIntVec *mvCollideIdx;
	cMesh2D *mpMesh;
	cVector2f mvTileSize;
	bool mbIsSolid;
	eTileCollisionType mCollisionType;

	cMesh2D *mpCollideMesh;

	void SetMesh(cMesh2D *apMesh);
	void SetMaterial(iMaterial *apMaterial);
};

} // namespace hpl
#endif // HPL_TILEDATA_H
