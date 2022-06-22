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

#include "hpl1/engine/graphics/Mesh2d.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/MemoryManager.h"
#include <math.h>

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMesh2D::cMesh2D() {
}

//-----------------------------------------------------------------------

cMesh2D::~cMesh2D() {
	mvPos.clear();
	mvTexCoord.clear();
	mvColor.clear();
	mvIndex.clear();

	for (int i = 0; i < eTileRotation_LastEnum; i++)
		mvVtx[i].clear();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cMesh2D::AddVertex(cVector2f avPos, cVector2f avTexCoord, cColor aCol) {
	mvPos.push_back(avPos);
	mvTexCoord.push_back(avTexCoord);
	mvColor.push_back(aCol);
}

//-----------------------------------------------------------------------

void cMesh2D::AddIndex(unsigned int alIndex) {
	mvIndex.push_back(alIndex);
}

//-----------------------------------------------------------------------

void cMesh2D::AddEdgeIndex(unsigned int alIndex) {
	mvEdgeIndex.push_back(alIndex);
}

//-----------------------------------------------------------------------

void cMesh2D::CreateVertexVec() {
	for (int i = 0; i < (int)mvPos.size(); i++) {
		mvVtx[0].push_back(cVertex(cVector3f(mvPos[i].x, mvPos[i].y, 0), mvTexCoord[i], mvColor[i]));
	}

	CalculateEdges(eTileRotation_0, mvVtx[0], mvEdgeIndex);
}

//-----------------------------------------------------------------------

void cMesh2D::CreateTileVertexVec() {
	int i;
	CreateVertexVec();

	for (i = 1; i < eTileRotation_LastEnum; i++) {
		mvVtx[i] = mvVtx[0];
	}

	//--- Create the angles;---
	for (int angle = 1; angle < 4; angle++) {
		for (i = 0; i < (int)mvVtx[0].size(); i++) {
			float fAngle = ((float)angle) * kPi2f;
			mvVtx[angle][i].pos.x = cos(fAngle) * mvVtx[0][i].pos.x -
									sin(fAngle) * mvVtx[0][i].pos.y;

			mvVtx[angle][i].pos.y = sin(fAngle) * mvVtx[0][i].pos.x +
									cos(fAngle) * mvVtx[0][i].pos.y;
		}
		// Can use same edge index here since order has not changed.
		CalculateEdges((eTileRotation)angle, mvVtx[angle], mvEdgeIndex);
	}

	// SKIP THESE FOR NOW
	// The edges are not calculated correctly..and I din't think they are that needed.
	/*//--- Flip horizontally ---
	for(i=0;i<(int)mvVtx[0].size();i++)
		mvVtx[4][i].pos.x = -mvVtx[0][i].pos.x;
	CalculateEdges(eTileRotation_FlipH, mvVtx[4], mvEdgeIndex);


	//--- Flip vertically ---
	for(i=0;i<(int)mvVtx[0].size();i++)
		mvVtx[5][i].pos.y = -mvVtx[0][i].pos.y;
	CalculateEdges(eTileRotation_FlipV,mvVtx[5],InvEdgeIndex);


	//--- Flip vertically and horizontal---
	for(i=0;i<(int)mvVtx[0].size();i++)
	{
		mvVtx[6][i].pos.y = -mvVtx[0][i].pos.y;
		mvVtx[6][i].pos.x = -mvVtx[0][i].pos.x;

	}
	//The two flips make it right again. No need to reverse
	CalculateEdges(eTileRotation_FlipHV,mvVtx[6],mvEdgeIndex);*/
}

//-----------------------------------------------------------------------

tVertexVec *cMesh2D::GetVertexVec(const cRect2f &aImageRect, cVector2f avSize, eTileRotation aRotation) {
	cVector3f vImageStart(aImageRect.x, aImageRect.y, 0);
	cVector3f vImageSize(aImageRect.w, aImageRect.h, 0);

	/*Log("ImageStart: %s\n",vImageStart.ToString().c_str());
	Log("ImageSize: %s\n",vImageSize.ToString().c_str());*/

	for (int j = 0; j < (int)mvVtx[aRotation].size(); j++) {
		// we want the same texture coords for all angles. therefor 0
		cVector3f vPos = (mvVtx[0][j].pos + avSize / 2) / avSize;

		mvVtx[aRotation][j].col = 1;
		mvVtx[aRotation][j].tex = vImageStart + (vPos * vImageSize);
		switch (aRotation) {
		case eTileRotation_0:
			mvVtx[aRotation][j].norm = cVector3f(1, 0, 3);
			break;
		case eTileRotation_90:
			mvVtx[aRotation][j].norm = cVector3f(0, 1, 3);
			break;
		case eTileRotation_180:
			mvVtx[aRotation][j].norm = cVector3f(-1, 0, 3);
			break;
		case eTileRotation_270:
			mvVtx[aRotation][j].norm = cVector3f(0, -1, 3);
			break;
		default:
			break;
		}
	}

	return &mvVtx[aRotation];
}

//-----------------------------------------------------------------------

tUIntVec *cMesh2D::GetIndexVec() {
	return &mvIndex;
}

//-----------------------------------------------------------------------

tMesh2DEdgeVec *cMesh2D::GetEdgeVec(eTileRotation aRotation) {
	return &mvEdge[aRotation];
}

//-----------------------------------------------------------------------

bool cMesh2D::PointIsInside(const cVector2f &avPoint, const cVector2f &avMeshPos, eTileRotation aRotation) {
	cVector2f vLocalPoint;
	cVector2f vNormal;

	for (int i = 0; i < (int)mvEdge[aRotation].size(); i++) {
		vLocalPoint = avPoint - avMeshPos - mvEdge[aRotation][i].mvMidPos;
		vNormal = mvEdge[aRotation][i].mvNormal;

		if ((vLocalPoint.x * vNormal.x + vLocalPoint.y * vNormal.y) >= 0)
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------

cCollisionMesh2D *cMesh2D::CreateCollisonMesh(const cVector2f &avPos, const cVector2f &avSizeMul, eTileRotation aRotation) {
	cCollisionMesh2D *pCollMesh = hplNew(cCollisionMesh2D, ());

	for (int i = 0; i < (int)mvEdgeIndex.size(); i++) {
		cVector3f vPos = mvVtx[aRotation][mvEdgeIndex[i]].pos;
		pCollMesh->mvPos.push_back(cVector2f(vPos.x, vPos.y) * (avSizeMul / 2) + avPos);
	}

	for (int i = 0; i < (int)mvEdge[aRotation].size(); i++) {
		pCollMesh->mvNormal.push_back(mvEdge[aRotation][i].mvNormal);
	}

	return pCollMesh;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIAVTE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

/**
 * Walks through the positions using the indexes in an Clockwise order and calculates the
 * normal for every edge (startpos -> endpos).
 */
void cMesh2D::CalculateEdges(eTileRotation aRotation, tVertexVec &aVtx, tUIntVec &aIdx) {
	cVector2f vLargest = -100000;
	cVector2f vSmallest = 1000000;

	for (int i = 0; i < (int)aIdx.size(); i++) {
		// Do some checks for the BB creation:
		// X:
		if (aVtx[aIdx[i]].pos.x > vLargest.x)
			vLargest.x = aVtx[aIdx[i]].pos.x;
		else if (aVtx[aIdx[i]].pos.x < vSmallest.x)
			vSmallest.x = aVtx[aIdx[i]].pos.x;
		// Y:
		if (aVtx[aIdx[i]].pos.y > vLargest.y)
			vLargest.y = aVtx[aIdx[i]].pos.y;
		else if (aVtx[aIdx[i]].pos.y < vSmallest.y)
			vSmallest.y = aVtx[aIdx[i]].pos.y;

		int start = i;
		int end = i + 1 >= (int)aIdx.size() ? 0 : i + 1;
		cVector2f vNormal;
		vNormal.x = -(aVtx[aIdx[start]].pos.y - aVtx[aIdx[end]].pos.y);
		vNormal.y = aVtx[aIdx[start]].pos.x - aVtx[aIdx[end]].pos.x;
		vNormal.Normalise();

		cVector2f vMidPos;
		vMidPos.x = (aVtx[aIdx[start]].pos.x + aVtx[aIdx[end]].pos.x) / 2;
		vMidPos.y = (aVtx[aIdx[start]].pos.y + aVtx[aIdx[end]].pos.y) / 2;

		mvEdge[aRotation].push_back(cMesh2DEdge(vNormal, vMidPos, mvEdgeIndex[start], mvEdgeIndex[end]));
	}

	// Create the bounding box.
	mvBoundingBox[aRotation].x = vSmallest.x;
	mvBoundingBox[aRotation].y = vSmallest.y;
	mvBoundingBox[aRotation].w = vLargest.x - vSmallest.x;
	mvBoundingBox[aRotation].h = vLargest.y - vSmallest.y;
}

//-----------------------------------------------------------------------
} // namespace hpl
