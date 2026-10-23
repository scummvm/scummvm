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
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

/*
 * Altered for ScummVM: removed the fixed-point and Nintendo DS code; uses the integer types of ScummVM and its system headers, spells out the float limits, and B2_NOT_USED no longer makes statements without effect.
 */

#ifndef B2_SETTINGS_H
#define B2_SETTINGS_H

#include "common/scummsys.h"

#define B2_NOT_USED(x) ((void)(x))
#define b2Assert(A) assert(A)


// The integer types are those of ScummVM

typedef float float32;
#define	B2_FLT_MAX	3.402823466e+38F
#define	B2_FLT_EPSILON	1.192092896e-07F
#define	B2FORCE_SCALE(x)	(x)
#define	B2FORCE_INV_SCALE(x)	(x)

const float32 b2_pi = 3.14159265359f;

/// @file
/// Global tuning constants based on meters-kilograms-seconds (MKS) units.
///

// Collision
const int32 b2_maxManifoldPoints = 2;
const int32 b2_maxPolygonVertices = 8;
const int32 b2_maxProxies = 512;				// this must be a power of two
const int32 b2_maxPairs = 8 * b2_maxProxies;	// this must be a power of two

// Dynamics

/// A small length used as a collision and constraint tolerance. Usually it is
/// chosen to be numerically significant, but visually insignificant.
const float32 b2_linearSlop = 0.005f;	// 0.5 cm

/// A small angle used as a collision and constraint tolerance. Usually it is
/// chosen to be numerically significant, but visually insignificant.
const float32 b2_angularSlop = 2.0f / 180.0f * b2_pi;			// 2 degrees

/// Continuous collision detection (CCD) works with core, shrunken shapes. This is the
/// amount by which shapes are automatically shrunk to work with CCD. This must be
/// larger than b2_linearSlop.
const float32 b2_toiSlop = 8.0f * b2_linearSlop;

/// Maximum number of contacts to be handled to solve a TOI island.
const int32 b2_maxTOIContactsPerIsland = 32;

/// A velocity threshold for elastic collisions. Any collision with a relative linear
/// velocity below this threshold will be treated as inelastic.
const float32 b2_velocityThreshold = 1.0f;		// 1 m/s

/// The maximum linear position correction used when solving constraints. This helps to
/// prevent overshoot.
const float32 b2_maxLinearCorrection = 0.2f;	// 20 cm

/// The maximum angular position correction used when solving constraints. This helps to
/// prevent overshoot.
const float32 b2_maxAngularCorrection = 8.0f / 180.0f * b2_pi;			// 8 degrees

/// The maximum linear velocity of a body. This limit is very large and is used
/// to prevent numerical problems. You shouldn't need to adjust this.
const float32 b2_maxLinearVelocity = 200.0f;
const float32 b2_maxLinearVelocitySquared = b2_maxLinearVelocity * b2_maxLinearVelocity;

/// The maximum angular velocity of a body. This limit is very large and is used
/// to prevent numerical problems. You shouldn't need to adjust this.
const float32 b2_maxAngularVelocity = 250.0f;
const float32 b2_maxAngularVelocitySquared = b2_maxAngularVelocity * b2_maxAngularVelocity;

/// This scale factor controls how fast overlap is resolved. Ideally this would be 1 so
/// that overlap is removed in one time step. However using values close to 1 often lead
/// to overshoot.
const float32 b2_contactBaumgarte = 0.2f;

// Sleep

/// The time that a body must be still before it will go to sleep.
const float32 b2_timeToSleep = 0.5f;									// half a second

/// A body cannot sleep if its linear velocity is above this tolerance.
const float32 b2_linearSleepTolerance = 0.01f;		// 1 cm/s

/// A body cannot sleep if its angular velocity is above this tolerance.
const float32 b2_angularSleepTolerance = 2.0f / 180.0f;		// 2 degrees/s

// Memory Allocation

/// The current number of bytes allocated through b2Alloc.
extern int32 b2_byteCount;

/// Implement this function to use your own memory allocator.
void* b2Alloc(int32 size);

/// If you implement b2Alloc, you should also implement this function.
void b2Free(void* mem);

/// Version numbering scheme.
/// See http://en.wikipedia.org/wiki/Software_versioning
struct b2Version
{
	int32 major;		///< significant changes
	int32 minor;		///< incremental changes
	int32 revision;		///< bug fixes
};

/// Current version.
extern b2Version b2_version;

/// Friction mixing law. Feel free to customize this.
inline float32 b2MixFriction(float32 friction1, float32 friction2)
{
	return sqrtf(friction1 * friction2);
}

/// Restitution mixing law. Feel free to customize this.
inline float32 b2MixRestitution(float32 restitution1, float32 restitution2)
{
	return restitution1 > restitution2 ? restitution1 : restitution2;
}

#endif
