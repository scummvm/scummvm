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

#ifndef ICB_GAMEVOLUME_H_INCLUDED
#define ICB_GAMEVOLUME_H_INCLUDED

#include "engines/icb/p4.h"
#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_3drealpoint.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/common/px_route_barriers.h"

#include "common/math.h"

namespace ICB {

// Set a limit on how many slices we can have (safe to increase this if we ever hit it).
#define MAX_SLICES 10

// Simply pairs together a top and bottom for a slice.
struct _slice_limit {
	PXreal fTop, fBottom;

	// Initialization.
	_slice_limit() {
		fTop = REAL_ZERO;
		fBottom = REAL_ZERO;
	}
};

// This is used to store the actual size and position of one of the cubes in the game volume.
struct _bullet_cube {
	PXreal fTop, fBottom;
	PXreal fLeft, fRight;
	PXreal fBack, fFront;

	// Initialization.
	_bullet_cube() {
		fTop = REAL_ZERO;
		fBottom = REAL_ZERO;
		fLeft = REAL_ZERO;
		fRight = REAL_ZERO;
		fBack = REAL_ZERO;
		fFront = REAL_ZERO;
	}
};

// Holds a 3D index into the game space.
struct _XYZ_index {
	int32 nX, nY, nZ;

	// Initialization.
	_XYZ_index() {
		nX = 0;
		nY = 0;
		nZ = 0;
	}
};

// Note that this class is abstract.  Holds parameters about the space occupied by a game
// session for use by the line-of-sight code.
class _game_volume {
public:
	// Default constructor and destructor.
	inline _game_volume();
	virtual inline ~_game_volume() = 0;

	// Copy constructor.
	_game_volume(const _game_volume &oX) { CopyObject(oX); }

	// Operator '='.
	inline const _game_volume &operator=(const _game_volume &oOpB);

	// This single function sets up all the parameters.
	void SetUpParameters(_linked_data_file *pyLOSData);

	// Gets and sets.
	PXreal GetAbsoluteTop() const { return (m_fAbsoluteTop); }
	PXreal GetAbsoluteBottom() const { return (m_fAbsoluteBottom); }
	PXreal GetLeftEdge() const { return (m_fLeftEdge); }
	PXreal GetRightEdge() const { return (m_fRightEdge); }
	PXreal GetBackEdge() const { return (m_fBackEdge); }
	PXreal GetFrontEdge() const { return (m_fFrontEdge); }

	uint32 GetNumSlices() const { return (m_nNumSlices); }
	PXreal GetSliceTop(uint32 i) const { return (m_oSliceLimits[i].fTop); }
	PXreal GetSliceBottom(uint32 i) const { return (m_oSliceLimits[i].fBottom); }

	uint32 GetXSize() const { return (m_nXSize); }
	uint32 GetZSize() const { return (m_nZSize); }

	bool8 IsValid() const { return (m_bValid); }

	// This works out the indices for the cube a point is in.
	bool8 GetCubeAndIndices(const px3DRealPoint &oPoint, _XYZ_index &oIndex, _bullet_cube &oCube) const;

protected:
	_linked_data_file *m_pyLOSMemFile; // Pointer to the line-of-sight data file.

private:
	PXreal m_fAbsoluteTop;                   // The 'roof' of the cube.
	PXreal m_fAbsoluteBottom;                // The base of the cube.
	PXreal m_fLeftEdge;                      // The leftmost edge of the cube.
	PXreal m_fRightEdge;                     // The rightmost edge of the cube.
	PXreal m_fBackEdge;                      // The backmost edge of the cube.
	PXreal m_fFrontEdge;                     // The frontmost edge of the cube.
	_slice_limit m_oSliceLimits[MAX_SLICES]; // Top and bottom of each slice.
	uint32 m_nNumSlices;                     // The number of slices in the session.
	uint32 m_nXSize;                         // Number of cubes along X dimension.
	uint32 m_nZSize;                         // Number of cubes along Z dimension.
	int32 m_nMinimumXIndex;                  // Minimum X index in the game space.
	int32 m_nMinimumZIndex;                  // Ditto Z.
	bool8 m_bValid;                          // TRUE when the object has been set up.
	uint8 m_nPadding[3];

	// Private functions used only by this class.
	inline void CopyObject(const _game_volume &oX);
};

inline _game_volume::_game_volume() {
	m_fAbsoluteTop = FLOAT_MAX;
	m_fAbsoluteBottom = FLOAT_MIN;
	m_fLeftEdge = REAL_ZERO;
	m_fRightEdge = REAL_ZERO;
	m_fBackEdge = REAL_ZERO;
	m_fFrontEdge = REAL_ZERO;
	m_nNumSlices = 0;
	m_nXSize = 0;
	m_nZSize = 0;
	m_nMinimumXIndex = 0;
	m_nMinimumZIndex = 0;
	m_bValid = FALSE8;
}

inline _game_volume::~_game_volume() {
	// Doesn't actually need to do anything.
}

inline const _game_volume &_game_volume::operator=(const _game_volume &oOpB) {
	CopyObject(oOpB);

	return (*this);
}

inline void _game_volume::CopyObject(const _game_volume &oX) {
	uint32 i;

	m_fAbsoluteTop = oX.m_fAbsoluteTop;
	m_fAbsoluteBottom = oX.m_fAbsoluteBottom;
	m_fLeftEdge = oX.m_fLeftEdge;
	m_fRightEdge = oX.m_fRightEdge;
	m_fBackEdge = oX.m_fBackEdge;
	m_fFrontEdge = oX.m_fFrontEdge;
	m_nXSize = oX.m_nXSize;
	m_nZSize = oX.m_nZSize;
	m_bValid = oX.m_bValid;
	m_nNumSlices = oX.m_nNumSlices;

	for (i = 0; i < oX.m_nNumSlices; ++i)
		m_oSliceLimits[i] = oX.m_oSliceLimits[i];
}

} // End of namespace ICB

#endif // #if !defined( GAMEVOLUME_H_INCLUDED )
