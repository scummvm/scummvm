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

#ifndef ICB_P4_GENERIC_PC_H
#define ICB_P4_GENERIC_PC_H

#include "engines/icb/non_ad_module.h"
#include "engines/icb/debug_pc.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_anims.h"

#include "common/textconsole.h"

namespace ICB {

// Useful typedef's for types

// These are now done in px_common.h
/*
// Might just be int's on the PSX
typedef float PXreal;
// Is likely to be done using fixed-point maths on PSX
typedef float PXfloat;

*/

// The camera
typedef PCcamera PXcamera;
// The animation, frame, marker
typedef PXframe_PSX PXframe;
typedef PXmarker_PSX PXmarker;
typedef PXanim_PSX PXanim;
// typedef PCactor PXactor;

// Very hacky #defines for function overloading
#define PXWorldToFilm WorldToFilm

// Very hacky #defines for function overloading
// #define PXFrameEnOfAnim ADFrameEnOfAnim
#define PXFrameEnOfAnim(n, pAnim) psxFrameEnOfAnim(n, pAnim, __FILE__, __LINE__)
#define PXAngleOfVector AngleOfVector
#define PXfabs fabs
#define PXfmod fmod
#define PXsqrt sqrt
#define PXsin sin
#define PXcos cos
#define PXfloor floor

// screen dimensions
#define SCREEN_WIDTH 640
#define SCREEN_DEPTH 480

// screen coordinate limits
#define SCREEN_LEFT_EDGE 0
#define SCREEN_RIGHT_EDGE SCREEN_WIDTH - 1
#define SCREEN_TOP_EDGE 0
#define SCREEN_BOTTOM_EDGE SCREEN_DEPTH - 1

// Definition and function for comparing floats.
#define FLOAT_COMPARE_TOLERANCE ((float)0.000001)
inline bool8 FloatsEqual(PXfloat fA, PXfloat fB, PXfloat tolerance = FLOAT_COMPARE_TOLERANCE) { return ((bool8)((fabs(fA - fB) < tolerance))); }

// This is the name of a bitmap which contains a reference palette entry to be used for transparency.
#define BITMAP_TRANSPARENCY_REFERENCE "transparent_ref"

} // End of namespace ICB

#include "engines/icb/gfx/psx_anims.h"

#endif // #ifndef P4_GENERIC_PC_H
