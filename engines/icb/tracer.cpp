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

#include "engines/icb/tracer.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/debug.h"
#include "engines/icb/floors.h"
#include "engines/icb/mission.h"

namespace ICB {

_tracer g_oTracer;

#define TRACER_COMPARE_TOLERANCE FLOAT_COMPARE_TOLERANCE

#define MAX_BARRIERS (256)

bool8 _tracer::Trace(const px3DRealPoint &oFrom,      // IN:  Point to plot from.
                     const px3DRealPoint &oTo,        // IN:  Point to plot to.
                     _barrier_ray_type eRayType,      // IN:  The type of the ray being plotted.
                     px3DRealPoint &oPointOfImpact,   // OUT: The first point of impact.
                     _barrier_logic_value eImpactType // OUT: For interpretation by high-level logic.
                     ) {
	uint32 i;
	uint32 nLoopCount;
	px2DRealPoint o2DImpactPoint;
	px3DRealPoint oCurrentPoint;
	px3DRealPoint o3DImpactPoint;
	int32 nToX, nToY, nToZ;
	uint32 oThisCubesBarriers[MAX_BARRIERS];
	int32 numberBarriers;
	px2DRealLine oPlanRay, oPlanBarrier;
	_bullet_cube oThisCube;
	_XYZ_index oThisIndex;
	_barrier_logic_value eTempImpactType;
	const _route_barrier *pBarrier;
	bool8 bFinishInThisCube;
	FaceID eCubeLeavingFace;
	_floor *psFloor;
	PXreal distCurrentTo;
	PXreal distCurrentImpact;
	PXreal distX, distY, distZ;
	int32 nExtraSliceIndex;

	// First work out which way the ray is going.
	m_bXPositiveGoing = (oTo.GetX() > oFrom.GetX()) ? TRUE8 : FALSE8;
	m_bYPositiveGoing = (oTo.GetY() > oFrom.GetY()) ? TRUE8 : FALSE8;
	m_bZPositiveGoing = (oTo.GetZ() > oFrom.GetZ()) ? TRUE8 : FALSE8;

	// Now work out the normalised vector for the line, which is used later but doesn't need recalculating each iteration.
	m_fXDiff = (PXreal)(oTo.GetX() - oFrom.GetX());
	m_fYDiff = (PXreal)(oTo.GetY() - oFrom.GetY());
	m_fZDiff = (PXreal)(oTo.GetZ() - oFrom.GetZ());

	m_fSqrLength = (PXreal)((PXdouble)(m_fXDiff * m_fXDiff) + (m_fYDiff * m_fYDiff) + (m_fZDiff * m_fZDiff));

	// Initialize for main loop.
	oCurrentPoint = oFrom;

	oPlanRay.SetX1((PXreal)oFrom.GetX());
	oPlanRay.SetY1((PXreal)oFrom.GetZ());
	oPlanRay.SetX2((PXreal)oTo.GetX());
	oPlanRay.SetY2((PXreal)oTo.GetZ());

	nLoopCount = 0;

	nToX = (int32)oTo.GetX();
	nToY = (int32)oTo.GetY();
	nToZ = (int32)oTo.GetZ();

	// Main loop.
	while (TRUE8) {
		// Work out the cube we're in from the current point in space.
		if (!GetCubeAndIndices(oCurrentPoint, oThisIndex, oThisCube)) {
			// We have gone outside the game world, so we can't have hit anything.
			oPointOfImpact = oCurrentPoint;
			eImpactType = BLOCKS;
			return (FALSE8);
		}

		// See whether or not this is the finishing cube.
		if ((nToX >= (int32)oThisCube.fLeft) && (nToX <= (int32)oThisCube.fRight) && (nToY >= (int32)oThisCube.fBottom) && (nToY <= (int32)oThisCube.fTop) &&
		    (nToZ >= (int32)oThisCube.fBack) && (nToZ <= (int32)oThisCube.fFront))
			bFinishInThisCube = TRUE8;
		else
			bFinishInThisCube = FALSE8;

		// Because animating barriers are allocated to slices based on the position of the bottom
		// of the barrier in the world, we need to include barriers based on the current point
		// gravitised down to the first floor rectangle under it.
		nExtraSliceIndex = MS->floor_def->Project_point_down_through_floors((int32)oCurrentPoint.GetX(), (int32)oCurrentPoint.GetY(), (int32)oCurrentPoint.GetZ());

		// Get all barrier indices for this cube.
		GetBarriersForCube(oThisIndex, oThisCubesBarriers, numberBarriers, nExtraSliceIndex);

		// find distCurrentTo
		distX = (PXreal)oTo.GetX() - (PXreal)oCurrentPoint.GetX();
		distY = (PXreal)oTo.GetY() - (PXreal)oCurrentPoint.GetY();
		distZ = (PXreal)oTo.GetZ() - (PXreal)oCurrentPoint.GetZ();
		distCurrentTo = distX * distX + distY * distY + distZ * distZ;

		// Check any barriers we found for intersection with the ray.
		// numberImpacts=0;
		for (i = 0; i < (uint32)numberBarriers; ++i) {
			// Get the barrier we're dealing with.
			pBarrier = GetBarrier(oThisCubesBarriers[i]);

			// The intersection test is relatively expensive, so do the barrier type check
			// first (eg. if we're firing light at a glass barrier, we know it can go through).
			eTempImpactType = IsBarrierTo(pBarrier->material(), eRayType);
			if (eTempImpactType != ALLOWS) {
				// Right, this barrier doesn't allow this ray to pass, so we must check if it
				// is actually in the path of the ray.
				oPlanBarrier.SetX1(pBarrier->x1());
				oPlanBarrier.SetY1(pBarrier->z1());
				oPlanBarrier.SetX2(pBarrier->x2());
				oPlanBarrier.SetY2(pBarrier->z2());

				if (oPlanRay.Intersects(oPlanBarrier, o2DImpactPoint) == px2DRealLine::DO_INTERSECT) {
					// We've found a barrier that the ray intersects with in the horizontal plane;
					// now we need to check the height of the ray at this point against the barrier.
					o3DImpactPoint.SetX(o2DImpactPoint.GetX());
					o3DImpactPoint.SetZ(o2DImpactPoint.GetY());
					if (CheckRayHeightAgainstBarrier(oFrom, oTo, pBarrier, o3DImpactPoint)) {
						// If we don't finish in this one we can guarantee that we will miss
						if (!bFinishInThisCube) {
							return (FALSE8);
						}

						// find distance from oCurrentPoint to o3DImpactPoint (distCurrentImpact)
						// if that is less than distance from oCurrentPoint to oTo (precomputed as distCurrentTo)
						distX = (PXreal)o3DImpactPoint.GetX() - (PXreal)oCurrentPoint.GetX();
						distY = (PXreal)o3DImpactPoint.GetY() - (PXreal)oCurrentPoint.GetY();
						distZ = (PXreal)o3DImpactPoint.GetZ() - (PXreal)oCurrentPoint.GetZ();
						distCurrentImpact = distX * distX + distY * distY + distZ * distZ;
						if (distCurrentImpact < distCurrentTo) {
							return (FALSE8);
						}
					}
				}
			} // end if
		} // end for

		if (bFinishInThisCube) {
			return (TRUE8);
		} else {
			oCurrentPoint = CalculateEntryToNextCube(oCurrentPoint, oTo, oThisCube, eCubeLeavingFace);

			// The only thing we still need to check is if the ray, in leaving this cube, passes through a
			// floor rectangle.  If it does then the floor acts as a barrier, stopping all rays.
			if ((eCubeLeavingFace == TOP) || (eCubeLeavingFace == BOTTOM)) {
				// Loop through all floor rectangles, checking for impacts.
				for (i = 0; i < m_pFloorWorld->Fetch_number_of_floors(); ++i) {
					// Get the floor.
					psFloor = m_pFloorWorld->Fetch_floor_number(i);

					// Can chuck some based simply on height.  Need to do this plus-or-minus 1 because the
					// calculation of entry into the next cube could be going up or down.
					if ((int32)oCurrentPoint.GetY() <= ((int32)psFloor->base_height + 1) && (int32)oCurrentPoint.GetY() >= ((int32)psFloor->base_height - 1)) {
						// This floor rectangle is at the right height, so we need to do the check properly.
						if (((int32)oCurrentPoint.GetX() >= (int32)psFloor->rect.x1) && ((int32)oCurrentPoint.GetX() <= (int32)psFloor->rect.x2) &&
						    ((int32)oCurrentPoint.GetZ() >= (int32)psFloor->rect.z1) && ((int32)oCurrentPoint.GetZ() <= (int32)psFloor->rect.z2)) {
							// The ray has hit a floor rectangle.
							oPointOfImpact = oCurrentPoint;
							eImpactType = BLOCKS;
							return (FALSE8);
						}
					} // end if
				} // end for
			} // end if
		} // end if

		// This is a safety net.  During development, this loop was running forever.  The bug has been fixed, however
		// the symptom is so serious - a complete game hang - that I have placed this catch here to stop any possibility
		// of this happening.  At worst, activation of this might result in an invalid return value from the tracer,
		// which might mean someone shoots someone they shouldn't, but at least it all won't hang up.  I've put an assert
		// in here so that we will know if this net is ever activated during development.  Hopefully, it never will be.
		if (++nLoopCount == 100) {
			// In final release, we'll let this go back, but it may have the wrong answer.  The effects of
			// this should be benign enough to allow this.
			oPointOfImpact = oTo;
			eImpactType = NO_IMPACT;
			return (FALSE8);
		}
	} // end while

	return (FALSE8);
}

void _tracer::GetBarriersForCube(const _XYZ_index &oCubeIndices, uint32 *oThisCubesBarriers, int32 &nNumBarriers, int32 nExtraSliceIndex) const {
	int32 i;
	_barrier_slice *pSlice;
	_barrier_cube *pBarrierCube;
	uint32 nBarrierCubeOffset;
	uint32 nActualIndex;
	uint32 *pBarrierArray;

	// Get to the right slice.
	pSlice = (_barrier_slice *)m_pyLOSMemFile->Fetch_item_by_number(oCubeIndices.nY);

	// Get to the right cube entry.
	nActualIndex = oCubeIndices.nZ * pSlice->row_length + oCubeIndices.nX;
	nBarrierCubeOffset = pSlice->offset_cubes[nActualIndex];
	pBarrierCube = (_barrier_cube *)((unsigned char *)pSlice + nBarrierCubeOffset);
	pBarrierArray = (uint32 *)((unsigned char *)pSlice + pBarrierCube->barriers);

	// Add the barriers to the array we're returning.
	if (pBarrierCube->num_barriers > MAX_BARRIERS)
		Fatal_error("Too many static barriers in cube (found %d)", pBarrierCube->num_barriers);

	nNumBarriers = 0;

	for (i = 0; i < pBarrierCube->num_barriers; ++i)
		oThisCubesBarriers[nNumBarriers++] = pBarrierArray[i];

	// Get animating barriers for the true cube that the ray is passing through.
	nNumBarriers = MS->session_barriers->Get_anim_barriers(nNumBarriers, oThisCubesBarriers, oCubeIndices.nY);

	// Add animating barriers for the cube calculated by gravitising the point down to the floor below.
	if (nExtraSliceIndex != -1)
		nNumBarriers = MS->session_barriers->Get_anim_barriers(nNumBarriers, oThisCubesBarriers, nExtraSliceIndex);
}

px3DRealPoint _tracer::CalculateEntryToNextCube(const px3DRealPoint &oCurrentPoint, // IN:  Ray start point.
                                                const px3DRealPoint &oTo,           // IN:  Ray is aimed here.
                                                const _bullet_cube &oThisCube,      // IN:  The current cube boundaries.
                                                FaceID &eCubeLeavingFace            // OUT: The face the ray leaves the current cube through.
                                                ) const {
	px3DRealPoint oEntryPoint;
	PXreal fXNormalDistance, fYNormalDistance, fZNormalDistance;
	PXreal fCurrentClosest;
	FaceID eOneOfTwoFaces;

	// Set some starting values for this comparison ladder.
	fCurrentClosest = FLOAT_MAX / REAL_TWO;
	eCubeLeavingFace = NO_FACE;

	// Now work out which face is 'closest' with respect to the vector.
	if (!FloatsEqual(m_fXDiff, REAL_ZERO, TRACER_COMPARE_TOLERANCE)) {
		if (m_bXPositiveGoing) {
			fXNormalDistance = (PXreal)PXfabs(((oThisCube.fRight - oCurrentPoint.GetX()) * m_fSqrLength) / m_fXDiff);
			eOneOfTwoFaces = RIGHT;
		} else {
			fXNormalDistance = (PXreal)PXfabs(((oThisCube.fLeft - oCurrentPoint.GetX()) * m_fSqrLength) / m_fXDiff);
			eOneOfTwoFaces = LEFT;
		}

		fCurrentClosest = fXNormalDistance;
		eCubeLeavingFace = eOneOfTwoFaces;
	}

	if (!FloatsEqual(m_fYDiff, REAL_ZERO, TRACER_COMPARE_TOLERANCE)) {
		if (m_bYPositiveGoing) {
			fYNormalDistance = (PXreal)PXfabs(((oThisCube.fTop - oCurrentPoint.GetY()) * m_fSqrLength) / m_fYDiff);
			eOneOfTwoFaces = TOP;
		} else {
			fYNormalDistance = (PXreal)PXfabs(((oThisCube.fBottom - oCurrentPoint.GetY()) * m_fSqrLength) / m_fYDiff);
			eOneOfTwoFaces = BOTTOM;
		}

		if (fYNormalDistance < fCurrentClosest) {
			fCurrentClosest = fYNormalDistance;
			eCubeLeavingFace = eOneOfTwoFaces;
		}
	}

	if (!FloatsEqual(m_fZDiff, REAL_ZERO, TRACER_COMPARE_TOLERANCE)) {
		if (m_bZPositiveGoing) {
			fZNormalDistance = (PXreal)PXfabs(((oThisCube.fFront - oCurrentPoint.GetZ()) * m_fSqrLength) / m_fZDiff);
			eOneOfTwoFaces = FRONT;
		} else {
			fZNormalDistance = (PXreal)PXfabs(((oThisCube.fBack - oCurrentPoint.GetZ()) * m_fSqrLength) / m_fZDiff);
			eOneOfTwoFaces = BACK;
		}

		if (fZNormalDistance < fCurrentClosest) {
			fCurrentClosest = fZNormalDistance;
			eCubeLeavingFace = eOneOfTwoFaces;
		}
	}

	// We now know which face we're going to pass out of this cube through.  Now need to compute the
	// point at which we pass through that plane.
	oEntryPoint = CalculateRayIntersectionWithCubeWall(oCurrentPoint, oTo, oThisCube, eCubeLeavingFace);

	// Return the entry point into the next cube.
	return (oEntryPoint);
}

px3DRealPoint _tracer::CalculateRayIntersectionWithCubeWall(const px3DRealPoint &oCurrentPoint, // IN:  Ray start point.
                                                            const px3DRealPoint &oTo,           // IN:  Ray is aimed here.
                                                            const _bullet_cube &oThisCube,      // IN:  The current cube boundaries.
                                                            FaceID eLeavingFace                 // IN:  Face we're leaving through.
                                                            ) const {
	px3DRealPoint oNewPoint(FLOAT_MAX / REAL_TWO, FLOAT_MAX / REAL_TWO, FLOAT_MAX / REAL_TWO); // Point outside the game world.
	PXreal fWallDistance, fFraction;
	PXreal fXDistance, fYDistance, fZDistance;

	// Work out the vector change for the line.
	fXDistance = (PXreal)(oTo.GetX() - oCurrentPoint.GetX());
	fYDistance = (PXreal)(oTo.GetY() - oCurrentPoint.GetY());
	fZDistance = (PXreal)(oTo.GetZ() - oCurrentPoint.GetZ());

	// Work out what fraction of the distance CurrentPoint-to-TargetPoint we're covering by going
	// to the cube wall.  By proportional triangles, we go the same amount along the 2 perpendicular
	// axes.  Note that I step on 1cm, to place the point inside the next cube.
	switch (eLeavingFace) {
	case RIGHT:
		fWallDistance = (PXreal)PXfabs(oCurrentPoint.GetX() - (oThisCube.fRight + 1));

		// There is a possibility of the divisor being zero on the PSX.  I discovered this too late to
		// attempt to fix the maths properly, so the best I can do is fail safe.  I do this by returning
		// a point outside the game world, which gets interpreted at a higher level as meaning LOS is
		// false, i.e. the target object cannot be seen.
		if (PXfabs(fXDistance) < REAL_ONE)
			return (oNewPoint);

		fFraction = (PXfloat)PXfabs(PXreal2PXfloat(fWallDistance) / fXDistance);
		oNewPoint.SetX(oThisCube.fRight + 1);
		oNewPoint.SetY(oCurrentPoint.GetY() + PXfloat2PXreal(fFraction * fYDistance));
		oNewPoint.SetZ(oCurrentPoint.GetZ() + PXfloat2PXreal(fFraction * fZDistance));
		break;

	case LEFT:
		fWallDistance = (PXreal)PXfabs(oCurrentPoint.GetX() - (oThisCube.fLeft - 1));

		// See comment above.
		if (PXfabs(fXDistance) < REAL_ONE)
			return (oNewPoint);

		fFraction = (PXfloat)PXfabs(PXreal2PXfloat(fWallDistance) / fXDistance);
		oNewPoint.SetX(oThisCube.fLeft - 1);
		oNewPoint.SetY(oCurrentPoint.GetY() + PXfloat2PXreal(fFraction * fYDistance));
		oNewPoint.SetZ(oCurrentPoint.GetZ() + PXfloat2PXreal(fFraction * fZDistance));
		break;

	case FRONT:
		fWallDistance = (PXreal)PXfabs(oCurrentPoint.GetZ() - (oThisCube.fFront + 1));

		// See comment above.
		if (PXfabs(fZDistance) < REAL_ONE)
			return (oNewPoint);

		fFraction = (PXfloat)PXfabs(PXreal2PXfloat(fWallDistance) / fZDistance);
		oNewPoint.SetX(oCurrentPoint.GetX() + PXfloat2PXreal(fFraction * fXDistance));
		oNewPoint.SetY(oCurrentPoint.GetY() + PXfloat2PXreal(fFraction * fYDistance));
		oNewPoint.SetZ(oThisCube.fFront + 1);
		break;

	case BACK:
		fWallDistance = (PXreal)PXfabs(oCurrentPoint.GetZ() - (oThisCube.fBack - 1));

		// See comment above.
		if (PXfabs(fZDistance) < REAL_ONE)
			return (oNewPoint);

		fFraction = (PXfloat)PXfabs(PXreal2PXfloat(fWallDistance) / fZDistance);
		oNewPoint.SetX(oCurrentPoint.GetX() + PXfloat2PXreal(fFraction * fXDistance));
		oNewPoint.SetY(oCurrentPoint.GetY() + PXfloat2PXreal(fFraction * fYDistance));
		oNewPoint.SetZ(oThisCube.fBack - 1);
		break;

	case TOP:
		fWallDistance = (PXreal)PXfabs(oCurrentPoint.GetY() - (oThisCube.fTop + 1));

		// See comment above.
		if (PXfabs(fYDistance) < REAL_ONE)
			return (oNewPoint);

		fFraction = (PXfloat)PXfabs(PXreal2PXfloat(fWallDistance) / fYDistance);
		oNewPoint.SetX(oCurrentPoint.GetX() + PXfloat2PXreal(fFraction * fXDistance));
		oNewPoint.SetY(oThisCube.fTop + 1);
		oNewPoint.SetZ(oCurrentPoint.GetZ() + PXfloat2PXreal(fFraction * fZDistance));
		break;

	case BOTTOM:
		fWallDistance = (PXreal)PXfabs(oCurrentPoint.GetY() - (oThisCube.fBottom - 1));

		// See comment above.
		if (PXfabs(fYDistance) < REAL_ONE)
			return (oNewPoint);

		fFraction = (PXfloat)PXfabs(PXreal2PXfloat(fWallDistance) / fYDistance);
		oNewPoint.SetX(oCurrentPoint.GetX() + PXfloat2PXreal(fFraction * fXDistance));
		oNewPoint.SetY(oThisCube.fBottom - 1);
		oNewPoint.SetZ(oCurrentPoint.GetZ() + PXfloat2PXreal(fFraction * fZDistance));
		break;

	case NO_FACE:
		break;

	} // end switch

	// Return the new point.
	return (oNewPoint);
}

bool8 _tracer::CheckRayHeightAgainstBarrier(const px3DRealPoint &oFrom, const px3DRealPoint &oTo, const _route_barrier *pBarrier, px3DRealPoint &o3DImpactPoint) const {
	PXreal l_fXDiff, l_fZDiff, l_fYDiff;
	PXfloat fImpactDistance;
	PXreal fTotalDistance;
	PXreal fImpactY;

	// Using Pythagoras, work out 2D distance to impact and 2D total distance to travel to destination point.
	l_fXDiff = (PXreal)(o3DImpactPoint.GetX() - oFrom.GetX());
	l_fZDiff = (PXreal)(o3DImpactPoint.GetZ() - oFrom.GetZ());

	fImpactDistance = (PXfloat)PXsqrt(l_fXDiff * l_fXDiff + l_fZDiff * l_fZDiff);

	l_fXDiff = (PXreal)(oTo.GetX() - oFrom.GetX());
	l_fZDiff = (PXreal)(oTo.GetZ() - oFrom.GetZ());
	l_fYDiff = (PXreal)(oTo.GetY() - oFrom.GetY());
	fTotalDistance = (PXreal)PXsqrt(l_fXDiff * l_fXDiff + l_fZDiff * l_fZDiff);

	// The proportion of the first to the second taken as a fraction of the total z change over the
	// ray is the z of the impact (proportional triangles).
	fImpactY = oFrom.GetY() + (fImpactDistance * l_fYDiff) / fTotalDistance;

	// Check this z against the barrier to see if we have impact.
	if ((fImpactY >= pBarrier->bottom()) && (fImpactY <= pBarrier->top())) {
		o3DImpactPoint.SetY(fImpactY);
		return (TRUE8);
	} else {
		return (FALSE8);
	}
}

uint32 _tracer::FindClosest(const px3DRealPoint &oFrom, px3DRealPoint *oImpactList, uint32 nNumberImpacts) const {
	uint32 i;
	uint32 nIndexOfClosest;
	PXreal fClosestSquareSoFar;
	PXreal fSquareDistance;
	PXreal l_fXDiff, l_fYDiff, l_fZDiff;
	px3DRealPoint oClosestSoFar;

	// Loop through all barriers, looking for the closest impact point to the starting point.
	fClosestSquareSoFar = FLOAT_MAX;
	nIndexOfClosest = nNumberImpacts;
	for (i = 0; i < nNumberImpacts; ++i) {
		l_fXDiff = (PXreal)(oFrom.GetX() - oImpactList[i].GetX());
		l_fYDiff = (PXreal)(oFrom.GetY() - oImpactList[i].GetY());
		l_fZDiff = (PXreal)(oFrom.GetZ() - oImpactList[i].GetZ());
		fSquareDistance = (l_fXDiff * l_fXDiff) + (l_fYDiff * l_fYDiff) + (l_fZDiff * l_fZDiff);

		if (fSquareDistance < fClosestSquareSoFar) {
			nIndexOfClosest = i;
			fClosestSquareSoFar = fSquareDistance;
		}
	}

	// Return the point.
	return (nIndexOfClosest);
}

} // End of namespace ICB
