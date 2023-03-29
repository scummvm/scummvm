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

#ifndef HPL_GRIDMAP2D_H
#define HPL_GRIDMAP2D_H

#include "common/array.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/std/map.h"

namespace hpl {

class iEntity2D;
class iLowLevelGraphics;

/////////// GRID OBJECT ////////////
class cGrid2D;
class cGridMap2D;

typedef Common::Array<cGrid2D *> tGrid2DPtrVec;
typedef tGrid2DPtrVec::iterator tGrid2DPtrVecIt;

class cGrid2DObject {

public:
	cGrid2DObject(iEntity2D *apEntity, cGridMap2D *apGridMap, unsigned int alHandle);

	void Update(const cRect2f &aRect);
	void Destroy();

	unsigned int GetHandle() { return mlHandle; }
	iEntity2D *GetEntity() { return mpEntity; }
	bool FirstTime(unsigned int alGlobalCount) {
		if (alGlobalCount < mlCount)
			return false;
		mlCount = alGlobalCount + 1;
		return true;
	}

private:
	cVector2l mvPosition;
	cVector2f mvSize;
	cVector2l mvGridSpan;

	tGrid2DPtrVec mvGridParents;

	iEntity2D *mpEntity;
	cGridMap2D *mpGridMap;

	bool mbIsInOuter;
	bool mbIsInGLobal;
	unsigned int mlCount;
	int mlHandle;
};

/////////// GRID ////////////

typedef Hpl1::Std::map<int, cGrid2DObject *> tGrid2DObjectMap;
typedef tGrid2DObjectMap::iterator tGrid2DObjectMapIt;

class cGrid2D {
	friend class cGridMap2DRectIt;

public:
	void Add(cGrid2DObject *pObject) {
		m_mapObjects.insert(tGrid2DObjectMap::value_type(
			pObject->GetHandle(), pObject));
	}

	void Erase(unsigned int alHandle) {
		m_mapObjects.erase(alHandle);
	}

private:
	tGrid2DObjectMap m_mapObjects;
};

/////////// GRID MAP ITERATOR ////

class cGridMap2D;

class iGridMap2DIt {
public:
	virtual ~iGridMap2DIt() = default;
	virtual bool HasNext() = 0;
	virtual iEntity2D *Next() = 0;
};

class cGridMap2DRectIt : public iGridMap2DIt {
public:
	cGridMap2DRectIt(cGridMap2D *apGridMap, cVector2l avPos, cVector2l avSize);
	virtual ~cGridMap2DRectIt() = default;

	bool HasNext();
	iEntity2D *Next();

private:
	cGridMap2D *mpGridMap;

	int mlType; // 0=global 1=outer 2=grids
	tGrid2DObjectMapIt mIt;

	cVector2l mvPos;
	cVector2l mvSize;
	int mlGridNum;
	int mlGridAdd;
	int mlGridRowCount;
	int mlGridColCount;

	bool mbUpdated;

	cGrid2DObject *mpObject;

	void GetGridObject();
};

/////////// GRID MAP ////////////

typedef Common::Array<cGrid2D> tGrid2DVec;
typedef tGrid2DVec::iterator tGrid2DVecIt;

class cGridMap2D {
	friend class cGrid2DObject;
	friend class cGridMap2DRectIt;

public:
	cGridMap2D(cVector2l avSize, cVector2l avGridSize, cVector2l avMaxGridSpan);
	~cGridMap2D();

	iGridMap2DIt *GetRectIterator(const cRect2f &aRect);

	cGrid2D *GetAt(int alX, int alY);

	bool AddEntity(iEntity2D *apEntity);
	bool RemoveEntity(iEntity2D *apEntity);

	int GetMaxArraySize() { return mvMaxGridSpan.x * mvMaxGridSpan.y; }
	const cVector2l &GetMaxGridSpan() { return mvMaxGridSpan; }
	const cVector2l &GetGridSize() { return mvGridSize; }
	const cVector2l &GetGridNum() { return mvGridNum; }

	tGrid2DObjectMap *GetAllMap() { return &m_mapAllObjects; }

	void DrawGrid(iLowLevelGraphics *apLowLevel, float afZ = 100, cColor aCol = cColor(1));
	void DrawEntityGrids(iLowLevelGraphics *apLowLevel, cVector2f avWorldPos, float afZ = 101, cColor aCol = cColor(1, 0, 1, 1));

private:
	// The Grids
	tGrid2DVec mvGrids;

	// all objects
	// This is a list of all objects in the grid.
	tGrid2DObjectMap m_mapAllObjects;

	// Global Objects
	// These are objects that affect the entire map
	tGrid2DObjectMap m_mapGlobalObjects;

	// Outer Objects
	// These are objects outside the bounds of the grid
	tGrid2DObjectMap m_mapOuterObjects;

	// The count increases every time a fetch on the map is made.
	//  (all call to GetFirstEntity)
	unsigned int mlGlobalCount;

	unsigned int mlHandleCount;

	cVector2l mvSize;
	cVector2l mvGridNum;
	cVector2l mvGridSize;
	cVector2l mvMaxGridSpan;
};

} // namespace hpl

#endif // HPL_GRIDMAP2D_H
