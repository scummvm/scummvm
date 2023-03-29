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

#ifndef HPL_MESH2D_H
#define HPL_MESH2D_H

#include "common/array.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {
class cMesh2DEdge {
public:
	cMesh2DEdge(cVector2f avNormal, cVector2f avMidPos, unsigned int alStartIndex, unsigned int alEndIndex)
		: mvNormal(avNormal), mvMidPos(avMidPos), mlStartIndex(alStartIndex), mlEndIndex(alEndIndex) {}

	cVector2f mvNormal;
	cVector2f mvMidPos;
	unsigned int mlStartIndex;
	unsigned int mlEndIndex;
};

typedef Common::Array<cMesh2DEdge> tMesh2DEdgeVec;
typedef tMesh2DEdgeVec::iterator tMesh2DEdgeVecIt;

class cCollisionMesh2D {
public:
	cCollisionMesh2D() {}

	tVector2fVec mvPos;
	tVector2fVec mvNormal;
};

/** Mesh2D is stores meshes used by 2d objects for collision and drawing.
@remarks
The main objects in the class is the vertex list, the edge list, the index list and the
edge index list. The edge list is a list calculated by the app it self, it contain normals
for all edges in the vertex list. When the vertexes are defined this must be
in a clockwise order in the edge index list! The Z value in the vertex doesn't matter. The indexes
point to the vertex list.
@see
*/
class cMesh2D {
public:
	cMesh2D();
	~cMesh2D();

	void AddVertex(cVector2f avPos, cVector2f avTexCoord = 0, cColor aCol = 1);
	void AddIndex(unsigned int alIndex);
	void AddEdgeIndex(unsigned int alIndex);

	void CreateVertexVec();
	void CreateTileVertexVec();

	tVertexVec *GetVertexVec(const cRect2f &aImageRect, cVector2f avSize, eTileRotation aRotation = eTileRotation_0);
	tUIntVec *GetIndexVec();
	tMesh2DEdgeVec *GetEdgeVec(eTileRotation aRotation = eTileRotation_0);

	/**
	 * Checks if a point is inside the mesh. Only works on convex edge meshes.
	 * \param avPoint
	 * \param avMeshPos
	 * \param aRotation
	 * \return
	 */
	bool PointIsInside(const cVector2f &avPoint, const cVector2f &avMeshPos,
					   eTileRotation aRotation = eTileRotation_0);

	cCollisionMesh2D *CreateCollisonMesh(const cVector2f &avPos, const cVector2f &avSizeMul,
										 eTileRotation aRotation = eTileRotation_0);

private:
	tVector2fVec mvPos;
	tColorVec mvColor;
	tMesh2DEdgeVec mvEdge[eTileRotation_LastEnum];
	tVector2fVec mvTexCoord;
	tUIntVec mvIndex;
	tUIntVec mvEdgeIndex;
	cRect2f mvBoundingBox[eTileRotation_LastEnum];

	tVertexVec mvVtx[eTileRotation_LastEnum]; // One for each tile angle.

	void CalculateEdges(eTileRotation aRotation, tVertexVec &aVtx, tUIntVec &aIdx);
};

} // namespace hpl

#endif // HPL_MESH2D_H
