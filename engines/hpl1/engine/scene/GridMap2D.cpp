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

#include "hpl1/engine/scene/GridMap2D.h"
#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/scene/Entity2D.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGridMap2D::cGridMap2D(cVector2l avSize, cVector2l avGridSize, cVector2l avMaxGridSpan) {
	mvSize = avSize;
	mvGridSize = avGridSize;
	mvMaxGridSpan = avMaxGridSpan;
	mvGridNum = avSize / avGridSize + 1;
	mlGlobalCount = 0;

	mvGrids.resize(mvGridNum.x * mvGridNum.y);

	for (int i = 0; i < (int)mvGrids.size(); i++)
		mvGrids[i] = cGrid2D();
}

//-----------------------------------------------------------------------

cGridMap2D::~cGridMap2D() {
	for (tGrid2DObjectMapIt it = m_mapAllObjects.begin(); it != m_mapAllObjects.end(); it++) {
		it->second->Destroy();
		hplDelete(it->second);
	}

	m_mapAllObjects.clear();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// GRIDMAP PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iGridMap2DIt *cGridMap2D::GetRectIterator(const cRect2f &aRect) {
	cVector2l vPos = cVector2l((int)floor(aRect.x / (float)mvGridSize.x),
							   (int)floor(aRect.y / (float)mvGridSize.y));
	cVector2l vSize = cVector2l((int)(aRect.w / (float)mvGridSize.x) + 1,
								(int)(aRect.h / (float)mvGridSize.y) + 1);

	// Check if we need yet another grid for x and y
	if (aRect.x + aRect.w >= (vPos.x + vSize.x) * mvGridSize.x)
		vSize.x++;
	if (aRect.y + aRect.h >= (vPos.y + vSize.y) * mvGridSize.y)
		vSize.y++;

	// Log("\nPos: %d:%d\n",vPos.x,vPos.y);
	// Log("Size: %d:%d\n\n",vSize.x,vSize.y);

	return hplNew(cGridMap2DRectIt, (this, vPos, vSize));
}

//-----------------------------------------------------------------------

bool cGridMap2D::AddEntity(iEntity2D *apEntity) {
	cGrid2DObject *pObject = hplNew(cGrid2DObject, (apEntity, this, mlHandleCount));
	apEntity->SetGrid2DObject(pObject);

	// Insert into the all map.
	m_mapAllObjects.insert(tGrid2DObjectMap::value_type(mlHandleCount, pObject));

	mlHandleCount++;
	return true;
}

//-----------------------------------------------------------------------

bool cGridMap2D::RemoveEntity(iEntity2D *apEntity) {
	cGrid2DObject *pObject = apEntity->GetGrid2DObject();
	if (pObject == NULL)
		return false;

	pObject->Destroy();
	m_mapAllObjects.erase(pObject->GetHandle());

	hplDelete(pObject);
	apEntity->SetGrid2DObject(NULL);

	return true;
}

//-----------------------------------------------------------------------

cGrid2D *cGridMap2D::GetAt(int alX, int alY) {
	if (alX >= mvGridNum.x || alX < 0 || alY >= mvGridNum.y || alY < 0)
		return NULL;

	return &mvGrids[alX + alY * mvGridNum.x];
}

//-----------------------------------------------------------------------

void cGridMap2D::DrawGrid(iLowLevelGraphics *apLowLevel, float afZ, cColor aCol) {
	for (int x = 0; x < mvGridNum.x; x++)
		for (int y = 0; y < mvGridNum.y; y++) {
			apLowLevel->DrawLineRect2D(cRect2f((float)(x * mvGridSize.x), (float)(y * mvGridSize.y),
											   (float)(mvGridSize.x), (float)(mvGridSize.y)),
									   afZ, aCol);
		}
}

//-----------------------------------------------------------------------

void cGridMap2D::DrawEntityGrids(iLowLevelGraphics *apLowLevel, cVector2f avWorldPos, float afZ, cColor aCol) {
	tGrid2DObjectMapIt it = m_mapAllObjects.begin();
	while (it != m_mapAllObjects.end()) {
		iEntity2D *pEntity = it->second->GetEntity();
		cRect2f Rect = pEntity->GetBoundingBox();
		Rect.x -= avWorldPos.x;
		Rect.y -= avWorldPos.y;
		apLowLevel->DrawLineRect2D(Rect, afZ, aCol);
		it++;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// GRIDMAP PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGridMap2DRectIt::cGridMap2DRectIt(cGridMap2D *apGridMap, cVector2l avPos, cVector2l avSize) {
	mpGridMap = apGridMap;
	mvPos = avPos;
	mvSize = avSize;
	mlType = 0;
	mpObject = NULL;
	mlGridNum = 0;
	mlGridAdd = 0;
	mlGridRowCount = 0;
	mbUpdated = false;

	// Increase the counter so you can load all objects.
	mpGridMap->mlGlobalCount++;

	mIt = mpGridMap->m_mapGlobalObjects.begin();
}

//-----------------------------------------------------------------------

bool cGridMap2DRectIt::HasNext() {
	if (!mbUpdated) {
		GetGridObject();
		mbUpdated = true;
	}

	if (mpObject)
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------

iEntity2D *cGridMap2DRectIt::Next() {
	iEntity2D *pEntity = NULL;
	if (!mbUpdated) {
		GetGridObject();
		mbUpdated = true;
	}

	if (mpObject) {
		pEntity = mpObject->GetEntity();
		mIt++;
		mbUpdated = false;
	}
	return pEntity;
}

//-----------------------------------------------------------------------
/////////////////////////// PRIVATE //////////////////////////////////////
//-----------------------------------------------------------------------

void cGridMap2DRectIt::GetGridObject() {
	// bool bEnd = false;

	switch (mlType) {
	//// GLOBAL OBJECTS //////////////////
	case 0: {
		// No need to check if the GridObject has been used.
		// All objects in globla only occur once
		if (mIt == mpGridMap->m_mapGlobalObjects.end()) {
			// Log("Reached end of global\n");
			mlType = 1;
			// Check if the coord is in an outer bound
			if (mvPos.x < 0 || mvPos.y < 0 ||
				mvPos.x + mvSize.x > mpGridMap->mvGridNum.x ||
				mvPos.y + mvSize.y > mpGridMap->mvGridNum.y) {
				// Log("Has outer!\n");
				mIt = mpGridMap->m_mapOuterObjects.begin();
			} else {
				// Log("Has NO outer!\n");
				mIt = mpGridMap->m_mapOuterObjects.end();
			}
			// FIXME: fallthrough intended from here?
			// fallthrough
		} else {
			mpObject = mIt->second;
			// Log("Found the object in global!\n");
			break;
		}
	}
	// FIXME: fallthrough intended?
	// fallthrough

	//// OUTER OBJECTS //////////////////
	case 1: {
		// No need to loop this one, since they only occur once and
		// no other calls to the objects inside the grid has been made.
		if (mIt == mpGridMap->m_mapOuterObjects.end()) {
			// Log("End of outer!\n");
			// Log("Pos: %d:%d\n",mvPos.x,mvPos.y);
			// Log("Size: %d:%d\n",mvSize.x,mvSize.y);
			// Log("----------\n");
			mlType = 2;
			// Check if the rect is outside of the grid map
			// If so, we are done
			if (mvPos.x >= mpGridMap->mvGridNum.x || mvPos.y >= mpGridMap->mvGridNum.y ||
				mvPos.x + mvSize.x - 1 < 0 || mvPos.y + mvSize.y - 1 < 0) {
				// Log("Outside Grid!\n");
				mpObject = NULL;
				mlType = -1;
				break;
			}
			// Remove grids outside of the grid.
			// Pos below (0,0):
			if (mvPos.x < 0) {
				mvSize.x += mvPos.x;
				mvPos.x = 0;
			}
			if (mvPos.y < 0) {
				mvSize.y += mvPos.y;
				mvPos.y = 0;
			}

			// Size large than grid map
			if (mvPos.x + mvSize.x > mpGridMap->mvGridNum.x) {
				mvSize.x -= (mvPos.x + mvSize.x) - (mpGridMap->mvGridNum.x);
			}
			if (mvPos.y + mvSize.y > mpGridMap->mvGridNum.y) {
				mvSize.y -= (mvPos.y + mvSize.y) - (mpGridMap->mvGridNum.y);
			}

			// Log("Pos: %d:%d\n",mvPos.x,mvPos.y);
			// Log("Size: %d:%d\n",mvSize.x,mvSize.y);

			// Calculate numbers needed to iterate the grid
			mlGridNum = mvPos.x + mvPos.y * mpGridMap->mvGridNum.x;
			mlGridAdd = mpGridMap->mvGridNum.x - mvSize.x;
			mlGridRowCount = mvSize.y;
			mlGridColCount = mvSize.x;

			// Log("Num: %d Add: %d Row: %d Col: %d\n",mlGridNum,mlGridAdd,
			//		mlGridRowCount,mlGridColCount);

			// Rect is outsize of map.
			if (mvSize.x <= 0 || mvSize.y <= 0) {
				mlGridNum = 0;
				mIt = mpGridMap->mvGrids[mlGridNum].m_mapObjects.end();
				mlGridColCount = 0;
				mlGridRowCount = 0;
			} else {
				mIt = mpGridMap->mvGrids[mlGridNum].m_mapObjects.begin();
			}
			// FIXME: fallthrough intended from here?
			// fallthrough
		} else {
			// Log("Found the object in outer!\n");
			mpObject = mIt->second;

			// Set that is object has been used.
			mpObject->FirstTime(mpGridMap->mlGlobalCount);
			break;
		}
	}
	// FIXME: fallthrough intended?
	// fallthrough

	//// GRID OBJECTS //////////////////
	case 2: {
		// Search through the grids til one with GridObjects is found
		// or we are at the end
		while (true) {
			if (mIt == mpGridMap->mvGrids[mlGridNum].m_mapObjects.end()) {
				// Log("End of grid[%d] list!\n",mlGridNum);
				mlGridNum++;
				mlGridColCount--;
				if (mlGridColCount <= 0) {
					// Log("New row!\n\n");
					mlGridColCount = mvSize.x;
					mlGridRowCount--;
					if (mlGridRowCount <= 0) {
						mpObject = NULL;
						break;
					}
					mlGridNum += mlGridAdd;
				}
				mIt = mpGridMap->mvGrids[mlGridNum].m_mapObjects.begin();
			} else {
				mpObject = mIt->second;

				// Check if object allready have been loaded.
				if (mpObject->FirstTime(mpGridMap->mlGlobalCount)) {
					// Log("Found the object in grid[%d]!\n",mlGridNum);
					break;
				} else {
					// Log("Found OLD object in grid[%d]!\n",mlGridNum);
					mIt++;
				}
			}
		}
	}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// GRIDOBJECT PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGrid2DObject::cGrid2DObject(iEntity2D *apEntity, cGridMap2D *apGridMap, unsigned int alHandle) {
	mpEntity = apEntity;
	mpGridMap = apGridMap;

	mvGridParents.resize(mpGridMap->GetMaxArraySize());
	for (int i = 0; i < (int)mvGridParents.size(); i++)
		mvGridParents[i] = NULL;

	mvPosition = cVector2l(-1000);
	mvGridSpan = cVector2l(-1, -1);
	mlHandle = alHandle;

	mbIsInOuter = false;
	mbIsInGLobal = false;
	mlCount = 0;

	mpEntity->UpdateBoundingBox();
	Update(mpEntity->GetBoundingBox());
}

//-----------------------------------------------------------------------

void cGrid2DObject::Destroy() {
	// Remove all old grids. This should only remove stuff that are necessary.
	for (int x = 0; x < mvGridSpan.x; x++)
		for (int y = 0; y < mvGridSpan.y; y++) {
			int lNum = x + y * mpGridMap->GetMaxGridSpan().x;
			if (mvGridParents[lNum]) {
				mvGridParents[lNum]->Erase(mlHandle);
				mvGridParents[lNum] = NULL;
			}
		}

	if (mbIsInGLobal) {
		mpGridMap->m_mapGlobalObjects.erase(mlHandle);
		mbIsInGLobal = false;
	} else if (mbIsInOuter) {
		mpGridMap->m_mapOuterObjects.erase(mlHandle);
		mbIsInOuter = false;
	}
}

//-----------------------------------------------------------------------

/**
 * \todo This function should be as optimized as possible.
 * \param &aRect
 */
void cGrid2DObject::Update(const cRect2f &aRect) {
	cVector2f vGSize((float)mpGridMap->GetGridSize().x, (float)mpGridMap->GetGridSize().y);
	cVector2l vPos((int)floor(aRect.x / vGSize.x), (int)floor(aRect.y / vGSize.y));

	// Calculate the num of extra grids in x,y direction that are needed.
	cVector2l vGridSpan((int)(aRect.w / vGSize.x), (int)(aRect.h / vGSize.y));
	vGridSpan += 1;

	// Log("Rect: %f:%f:%f:%f\n",aRect.x,aRect.y,aRect.w,aRect.h);
	// Log("GSize: %f:%f\n",vGSize.x,vGSize.y);
	// Log("Pos: %d:%d\n",vPos.x,vPos.y);

	// Test if the rect is to large or has negative size.
	// If so put it in the global map.
	// TODO: this is a long test and it would good to skip it for most stuff
	//  Fix that!
	if (aRect.h < 0 || aRect.w < 0 || vGridSpan.x >= mpGridMap->GetMaxGridSpan().x ||
		vGridSpan.y >= mpGridMap->GetMaxGridSpan().y) {
		if (mbIsInGLobal == false) {
			mbIsInGLobal = true;

			// Erase old positions
			for (int x = 0; x < mvGridSpan.x; x++)
				for (int y = 0; y < mvGridSpan.y; y++) {
					int lNum = x + y * mpGridMap->GetMaxGridSpan().x;
					if (mvGridParents[lNum]) {
						mvGridParents[lNum]->Erase(mlHandle);
						mvGridParents[lNum] = NULL;
					}
				}

			mpGridMap->m_mapGlobalObjects.insert(tGrid2DObjectMap::value_type(mlHandle, this));
			// Log("Added to global\n");
		}

		return;
	}

	// The amount not covered in the span calc above
	// Then check if this piece is out side of the last grid.
	if (aRect.x + aRect.w >= (vPos.x + vGridSpan.x) * vGSize.x)
		vGridSpan.x++;
	if (aRect.y + aRect.h >= (vPos.y + vGridSpan.y) * vGSize.y)
		vGridSpan.y++;

	// Log("GridSpan: %d:%d\n",vGridSpan.x,vGridSpan.y);

	if (vPos != mvPosition || vGridSpan != mvGridSpan) {
		// Remove all old grids. This should only remove stuff that are necessary.
		for (int x = 0; x < mvGridSpan.x; x++)
			for (int y = 0; y < mvGridSpan.y; y++) {
				int lNum = x + y * mpGridMap->GetMaxGridSpan().x;
				if (mvGridParents[lNum] != NULL) {
					mvGridParents[lNum]->Erase(mlHandle);
					mvGridParents[lNum] = NULL;
				}
			}

		if (mbIsInGLobal) {
			mpGridMap->m_mapGlobalObjects.erase(mlHandle);
			mbIsInGLobal = false;
		} else if (mbIsInOuter) {
			mpGridMap->m_mapOuterObjects.erase(mlHandle);
			mbIsInOuter = false;
		}

		mvGridSpan = vGridSpan;

		cGrid2D *pGrid = NULL;
		for (int x = 0; x < mvGridSpan.x; x++)
			for (int y = 0; y < mvGridSpan.y; y++) {
				pGrid = mpGridMap->GetAt(vPos.x + x, vPos.y + y);
				if (pGrid == NULL) {
					if (!mbIsInOuter) {
						mpGridMap->m_mapOuterObjects.insert(tGrid2DObjectMap::value_type(
							mlHandle, this));
						mbIsInOuter = true;
					}

				} else {
					pGrid->Add(this);
					mvGridParents[x + y * mpGridMap->GetMaxGridSpan().x] = pGrid;
				}
			}
	}
}

//-----------------------------------------------------------------------

} // namespace hpl
