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

#include "engines/icb/game_volume.h"
#include "engines/icb/debug.h"

namespace ICB {

void _game_volume::SetUpParameters(_linked_data_file *pyLOSData) {
	uint32 i;
	_barrier_slice *pSlice;
	_slice_limit sSliceLimit;

	// Store the pointer to the data file.
	m_pyLOSMemFile = pyLOSData;

	// Get the first slice and use this to set the horizontal sizes, since they all share the
	// same horizontal sizes.
	pSlice = (_barrier_slice *)pyLOSData->Fetch_item_by_number(0);
	m_fLeftEdge = pSlice->left_edge;
	m_fRightEdge = pSlice->right_edge;
	m_fBackEdge = pSlice->back_edge;
	m_fFrontEdge = pSlice->front_edge;

	// Set the array dimensions.
	m_nXSize = pSlice->row_length;
	m_nZSize = pSlice->num_cubes / m_nXSize;

	// Work out minimum X and Z, so I can work out true array indices later.
	m_nMinimumXIndex = (int32)(m_fLeftEdge / (PXreal)FLOOR_CUBE_SIZE);
	m_nMinimumZIndex = (int32)(m_fBackEdge / (PXreal)FLOOR_CUBE_SIZE);

	// Now set the slice heights.
	m_nNumSlices = pyLOSData->Fetch_number_of_items();

	for (i = 0; i < m_nNumSlices; ++i) {
		pSlice = (_barrier_slice *)pyLOSData->Fetch_item_by_number(i);
		sSliceLimit.fTop = pSlice->top;
		sSliceLimit.fBottom = pSlice->bottom;
		m_oSliceLimits[i] = sSliceLimit;
	}

	// Set the absolute top and bottom of the volume.
	m_fAbsoluteTop = m_oSliceLimits[m_nNumSlices - 1].fTop;
	m_fAbsoluteBottom = m_oSliceLimits[0].fBottom;

	// The object is now valid.
	m_bValid = TRUE8;

	m_nPadding[0] = 0;
	m_nPadding[1] = 0;
	m_nPadding[2] = 0;
}

bool8 _game_volume::GetCubeAndIndices(const px3DRealPoint &oPoint, // IN:  The current point in space.
                                      _XYZ_index &oIndex,          // OUT: The XYZ array index containing the point.
                                      _bullet_cube &oCube          // OUT: Dimensions of the cube represented by the index.
                                      ) const {
	int32 nIndex;
	int32 nLeastSide;
	uint32 nSlice;

	// Check if the point is outside the game world.  If it is, we can't have hit the object and we
	// won't be colliding with anymore barriers, so we simply stop tracing.
	if ((oPoint.GetX() < m_fLeftEdge) || (oPoint.GetX() > m_fRightEdge) || (oPoint.GetZ() < m_fBackEdge) || (oPoint.GetZ() > m_fFrontEdge) ||
	    (oPoint.GetY() < m_fAbsoluteBottom) || (oPoint.GetY() > m_fAbsoluteTop))
		return (FALSE8);

	// Work out left and right.
	if (oPoint.GetX() < REAL_ZERO)
		nIndex = (int32)((oPoint.GetX() + REAL_ONE) / (PXreal)FLOOR_CUBE_SIZE) - 1;
	else
		nIndex = (int32)(oPoint.GetX() / (PXreal)FLOOR_CUBE_SIZE);

	nLeastSide = nIndex * FLOOR_CUBE_SIZE;
	oCube.fLeft = (PXreal)nLeastSide;
	oCube.fRight = (PXreal)(nLeastSide + (FLOOR_CUBE_SIZE - 1));

	oIndex.nX = nIndex + (-m_nMinimumXIndex);

	// Now front and back.
	if (oPoint.GetZ() < REAL_ZERO)
		nIndex = (int32)((oPoint.GetZ() + REAL_ONE) / (PXreal)FLOOR_CUBE_SIZE) - 1;
	else
		nIndex = (int32)(oPoint.GetZ() / (PXreal)FLOOR_CUBE_SIZE);

	nLeastSide = nIndex * FLOOR_CUBE_SIZE;
	oCube.fBack = (PXreal)nLeastSide;
	oCube.fFront = (PXreal)(nLeastSide + (FLOOR_CUBE_SIZE - 1));

	oIndex.nZ = nIndex + (-m_nMinimumZIndex);

	// The top and bottom involve finding out which slice the point is in.
	nSlice = 0;
	while ((nSlice < m_nNumSlices) && (oPoint.GetY() > m_oSliceLimits[nSlice].fTop))
		++nSlice;

	oIndex.nY = nSlice;
	oCube.fBottom = m_oSliceLimits[oIndex.nY].fBottom;
	oCube.fTop = m_oSliceLimits[oIndex.nY].fTop;

	// Tell tracer to continue tracing.
	return (TRUE8);
}

} // End of namespace ICB
