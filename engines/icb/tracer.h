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

#ifndef ICB_TRACER_H_INCLUDED
#define ICB_TRACER_H_INCLUDED

#include "engines/icb/game_volume.h"
#include "engines/icb/p4.h"
#include "engines/icb/p4_generic.h"

#include "engines/icb/common/px_3drealpoint.h"
#include "engines/icb/common/px_2drealline.h"

namespace ICB {

class _floor_world;

class _tracer : public _game_volume {
public:
	// Definitions used by this class.
	enum FaceID { NO_FACE, LEFT, RIGHT, FRONT, BACK, TOP, BOTTOM };

	// Default constructor and destructor.
	inline _tracer() { m_pyBarrierMemFile = NULL; m_nPadding[0] = 0; }
	virtual inline ~_tracer() { ; }

	// This checks a line through game-world space and returns the point of its first impact.
	bool8 Trace(const px3DRealPoint &oFrom, const px3DRealPoint &oTo, _barrier_ray_type eRayType, px3DRealPoint &oImpact, _barrier_logic_value eImpactType);

	// Call this before using the tracer, to point it at its barriers.
	void SetBarrierPointer(_linked_data_file *pyBarriers) { m_pyBarrierMemFile = pyBarriers; }

	// Call this to give the tracer access to the floor data.
	void SetFloorsPointer(_floor_world *pFloorWorld) { m_pFloorWorld = pFloorWorld; }

private:
	_linked_data_file *m_pyBarrierMemFile; // The memory image of the barrier file.
	_floor_world *m_pFloorWorld;           // The floors data (loaded by the routing code).
	PXreal m_fXDiff, m_fYDiff, m_fZDiff;   // Delta x, y and z for the line we're tracing.
	PXreal m_fSqrLength;                   // Square length of the vector we're tracing.
	bool8 m_bXPositiveGoing;               // These 3 get set once for each
	bool8 m_bYPositiveGoing;               // call to the tracer, and indicate
	bool8 m_bZPositiveGoing;               // which way the line is going.
	uint8 m_nPadding[1];

	// Here I block the use of the default '='.
	_tracer(const _tracer &) { ; }
	void operator=(const _tracer &) { ; }

	// Private functions used only by this class.
	void GetBarriersForCube(const _XYZ_index &oCubeIndices, uint32 *oThisCubesBarriers, int32 &nNumBarriers, int32 nExtraSliceIndex) const;

	px3DRealPoint CalculateEntryToNextCube(const px3DRealPoint &oCurrentPoint, const px3DRealPoint &oTo, const _bullet_cube &oThisCube, FaceID &eCubeLeavingFace) const;

	bool8 CheckRayHeightAgainstBarrier(const px3DRealPoint &oFrom, const px3DRealPoint &oTo, const _route_barrier *pBarrier, px3DRealPoint &o3DImpactPoint) const;

	uint32 FindClosest(const px3DRealPoint &oFrom, px3DRealPoint *oImpactList, uint32 nNumImpacts) const;

	px3DRealPoint CalculateRayIntersectionWithCubeWall(const px3DRealPoint &oCurrentPoint, const px3DRealPoint &oTo, const _bullet_cube &oThisCube, FaceID eLeavingFace) const;

	inline const _route_barrier *GetBarrier(uint32 i) const;
};

inline const _route_barrier *_tracer::GetBarrier(uint32 i) const {
	_route_barrier *pBarriers;

	if (!m_pyBarrierMemFile)
		Fatal_error("No barrier file in _tracer::GetBarrier()");

	pBarriers = (_route_barrier *)m_pyBarrierMemFile->Fetch_item_by_name("Data");
	return &(pBarriers[i]);
}

extern _tracer g_oTracer; // Object for doing the plotting of bullets and line-of-sight.

} // End of namespace ICB

#endif // #if !defined( TRACER_H_INCLUDED )
