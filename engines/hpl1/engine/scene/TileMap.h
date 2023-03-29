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

#ifndef HPL_TILEMAP_H
#define HPL_TILEMAP_H

#include "common/array.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/scene/Camera2D.h"
#include "hpl1/engine/scene/TileLayer.h"
#include "hpl1/engine/scene/TileMapIt.h"
#include "hpl1/engine/scene/TileSet.h"

namespace hpl {

typedef Common::Array<cTileSet *> tTileSetVec;
typedef tTileSetVec::iterator tTileSetVecIt;

typedef Common::Array<cTileLayer *> tTileLayerVec;
typedef tTileLayerVec::iterator tTileLayerVecIt;

class cTileMap {
	friend class cTileMapRectIt;
	friend class cTileMapLineIt;

public:
	cTileMap(cVector2l avSize, float afTileSize, cGraphics *apGraphics, cResources *apResources);
	~cTileMap();

	const cVector2l &GetSize() { return mvSize; }
	float GetTileSize() { return mfTileSize; }

	int GetTileNeighbours4Dir(int alTileNum, int alLayer, bool *avDir);

	void AddTileSet(cTileSet *apSet) { mvTileSet.push_back(apSet); }
	cTileSet *GetTileSet(int alNum) { return mvTileSet[alNum]; }

	void AddTileLayerFront(cTileLayer *apLayer);
	void AddTileLayerBack(cTileLayer *apLayer) { mvTileLayer.push_back(apLayer); }

	cTileLayer *GetTileLayer(int alNum) { return mvTileLayer[alNum]; }
	int GetTileLayerNum() { return (int)mvTileLayer.size(); }

	iTileMapIt *GetRectIterator(const cRect2f &aRect, int alLayer);
	iTileMapIt *GetLineIterator(const cVector2f &avStart, const cVector2f &avEnd, int alLayer);

	void Render(cCamera2D *apCam);

	cTile *GetScreenTile(cVector2f avPos, int alLayer, cCamera2D *apCam);
	void SetScreenTileData(cVector2f avPos, int alLayer, cCamera2D *apCam, int alTileSet, int alTileNum);
	void SetScreenTileAngle(cVector2f avPos, int alLayer, cCamera2D *apCam, int alAngle);

	cTile *GetWorldTile(cVector2f avPos, int alLayer);
	cVector2f GetWorldPos(cVector2f avScreenPos, cCamera2D *apCam);

	void SetShadowLayer(int alShadowLayer) { mlShadowLayer = alShadowLayer; }
	int GetShadowLayer() { return mlShadowLayer; }

private:
	cGraphics *mpGraphics;
	cResources *mpResources;

	tTileSetVec mvTileSet;
	tTileLayerVec mvTileLayer;
	int mlShadowLayer;

	cVector2l mvSize;
	float mfTileSize;
	int mlCurrentLayer;

	inline void RenderTileData(cTile *apTile, int alLayer);
};

} // namespace hpl

#endif // HPL_TILEMAP_H
