/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_maths.h"
#include "engines/icb/softskin_pc.h"
#include "engines/icb/common/px_capri_maths.h"

namespace ICB {

int softskinPC(rap_API *rap, int poseBone, MATRIXPC *lw, SVECTORPC *local, int16 *xminLocal, int16 *xmaxLocal, int16 *yminLocal, int16 *ymaxLocal, int16 *zminLocal,
               int16 *zmaxLocal, int screenShift) {
	// step 1 : make all the local-world and local-screen matrices
	//    This is done prior to this function
	// step 2 : take all the offsets from the mesh file
	//          and make them into screen positions using the
	//          correct co-ordinate system
	// step 3 : draw the polygons using the list of created screen positions
	//

	// step 2
	rap_API *pLink = rap;
	uint32 nNone = pLink->nNone;
	uint32 nSingle = pLink->nSingle;
	uint32 nMulti = pLink->nMultiple;
	uint32 i, vIndex;
	Vertex *noneLink = rap->GetNoneLinkPtr();
	VertexLink *singleLink = rap->GetSingleLinkPtr();
	WeightedVertexLink *multiLink = rap->GetMultiLinkPtr();

	uint32 prim;
	uint32 nVertices = 0;

	uint32 bothScaleShift = rap->bothScaleShift;
	// uint32 bothScale = (1 << bothScaleShift);

	uint32 worldScaleShift = rap->worldScaleShift;
	bothScaleShift -= screenShift;
	worldScaleShift -= screenShift;
	// uint32 worldScale = (1 << worldScaleShift );

	// loop over the vertices
	SVECTORPC *plocal = local;
	VECTOR lvert, lvert2;

	int32 flag;
	uint oldPrim = rap->nBones;

	int xmin = *xminLocal;
	int ymin = *yminLocal;
	int zmin = *zminLocal;

	int xmax = *xmaxLocal;
	int ymax = *ymaxLocal;
	int zmax = *zmaxLocal;

	int lvx, lvy, lvz;

	if (poseBone == -1) {
		for (i = 0; i < nNone; i++) {
			vIndex = noneLink->vertId;
			plocal = local + vIndex;
			if (vIndex > nVertices)
				nVertices = vIndex;
			plocal->vx = noneLink->vx;
			plocal->vy = noneLink->vy;
			plocal->vz = noneLink->vz;

			lvx = plocal->vx;
			lvy = plocal->vy;
			lvz = plocal->vz;

			xmin = PXmin(lvx, xmin);
			ymin = PXmin(lvy, ymin);
			zmin = PXmin(lvz, zmin);

			xmax = PXmax(lvx, xmax);
			ymax = PXmax(lvy, ymax);
			zmax = PXmax(lvz, zmax);

			noneLink++;
		}
	} else {
		// Do the pose vertices
		gte_SetRotMatrix_pc(lw + poseBone);
		gte_SetTransMatrix_pc(lw + poseBone);
		for (i = 0; i < nNone; i++) {
			gte_RotTrans_pc((SVECTOR *)&(noneLink->vx), &lvert2, &flag);

			vIndex = noneLink->vertId;
			plocal = local + vIndex;

			plocal->vx = (short)(lvert2.vx >> worldScaleShift);
			plocal->vy = (short)(lvert2.vy >> worldScaleShift);
			plocal->vz = (short)(lvert2.vz >> worldScaleShift);

			lvx = plocal->vx;
			lvy = plocal->vy;
			lvz = plocal->vz;

			xmin = PXmin(lvx, xmin);
			ymin = PXmin(lvy, ymin);
			zmin = PXmin(lvz, zmin);

			xmax = PXmax(lvx, xmax);
			ymax = PXmax(lvy, ymax);
			zmax = PXmax(lvz, zmax);

			noneLink++;
		}
		nVertices = nNone;
	}

	for (i = 0; i < nSingle; i++) {
		prim = singleLink->primId; // which co-ordinate system to use

		// Put the correct rot and trans matrix in place
		// transform to world space : local2world is in workm
		if (prim != oldPrim) {
			gte_SetRotMatrix_pc(lw + prim);
			gte_SetTransMatrix_pc(lw + prim);
			oldPrim = prim;
		}
		gte_RotTrans_pc((SVECTOR *)&(singleLink->vx), &lvert2, &flag);

		plocal = local + singleLink->vertId;
		if (singleLink->vertId > nVertices)
			nVertices = singleLink->vertId;
		plocal->vx = (short)(lvert2.vx >> worldScaleShift);
		plocal->vy = (short)(lvert2.vy >> worldScaleShift);
		plocal->vz = (short)(lvert2.vz >> worldScaleShift);

		lvx = plocal->vx;
		lvy = plocal->vy;
		lvz = plocal->vz;

		xmin = PXmin(lvx, xmin);
		ymin = PXmin(lvy, ymin);
		zmin = PXmin(lvz, zmin);

		xmax = PXmax(lvx, xmax);
		ymax = PXmax(lvy, ymax);
		zmax = PXmax(lvz, zmax);

		singleLink++;
	}

	uint curVert = multiLink->link.vertId;

	lvert.vx = 0;
	lvert.vy = 0;
	lvert.vz = 0;
	for (i = 0; i < nMulti; i++) {
		u_int weight = multiLink->weight;
		prim = multiLink->link.primId; // which co-ordinate system to use

		// Put the correct rot and trans matrix in place
		// transform to world space : local2world is in workm
		if (prim != oldPrim) {
			gte_SetRotMatrix_pc(lw + prim);
			gte_SetTransMatrix_pc(lw + prim);
			oldPrim = prim;
		}
		gte_RotTrans_pc((SVECTOR *)&(multiLink->link.vx), &lvert2, &flag);

		// Do a weighted average of this vector (lvert2)
		// with the stored weighted average (lvert)
		// gte_LoadAverage0( &lvert, &lvert2, 1, weight, &lvert );
		// gte_LoadAverage0 is not any quicker - and more inaccurate
		lvert.vx += lvert2.vx * weight;
		lvert.vy += lvert2.vy * weight;
		lvert.vz += lvert2.vz * weight;

		multiLink++;
		vIndex = multiLink->link.vertId;
		// A new vertex so tidy up the old one
		if (vIndex != curVert) {
			if (curVert > nVertices)
				nVertices = curVert;

			plocal = local + curVert;
			plocal->vx = (short)(lvert.vx >> bothScaleShift);
			plocal->vy = (short)(lvert.vy >> bothScaleShift);
			plocal->vz = (short)(lvert.vz >> bothScaleShift);
			curVert = vIndex;
			lvert.vx = 0;
			lvert.vy = 0;
			lvert.vz = 0;

			lvx = plocal->vx;
			lvy = plocal->vy;
			lvz = plocal->vz;

			xmin = PXmin(lvx, xmin);
			ymin = PXmin(lvy, ymin);
			zmin = PXmin(lvz, zmin);

			xmax = PXmax(lvx, xmax);
			ymax = PXmax(lvy, ymax);
			zmax = PXmax(lvz, zmax);
		}
	}

	*xminLocal = (short)xmin;
	*yminLocal = (short)ymin;
	*zminLocal = (short)zmin;

	*xmaxLocal = (short)xmax;
	*ymaxLocal = (short)ymax;
	*zmaxLocal = (short)zmax;

	nVertices++;
	return nVertices;
}

} // End of namespace ICB
