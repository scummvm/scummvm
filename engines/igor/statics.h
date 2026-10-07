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

#ifndef IGOR_STATICS_H
#define IGOR_STATICS_H

#include "common/scummsys.h"

namespace Igor {

// Shared animation frame buffer offsets for Part 10 & Part 11
// Ground truth: code/175_2767.asm cseg175:2815, cseg177:0064, cseg177:0097, cseg177:00CA, cseg177:00FD
const uint32 kPart10_11_Frm1 = 0xB400; // cseg175:2815
const uint32 kPart10_11_Frm2 = 0xBC0A; // cseg177:0064
const uint32 kPart10_11_Frm3 = 0xC5E2; // cseg177:0097
const uint32 kPart10_11_Frm4 = 0xC786; // cseg177:00CA
const uint32 kPart10_11_Frm5 = 0xC8AE; // cseg177:00FD

// Layout of the animation buffer while the park is loaded: FRM_Park1..4 follow each other
// from kParkFrames on, and the frames of Laura's entrance replace them while she is on screen.
const uint32 kParkFrames = 0x5A00;        // FRM_Park1: sparse frames, the old lady and Igor
const uint32 kParkFrameTable = 0x76A8;    // FRM_Park2: frame offset table, relative to kParkFrames
const uint32 kParkPickUpFrames = 0xD0E6;  // FRM_Park3: three raw 27x49 frames of the pick up animation
const uint32 kParkIdleFrames = 0xE067;    // FRM_Park4: five raw 18x35 frames of the old lady's idle animation
const uint32 kParkLauraFrames = 0x5A00;   // FRM_ParkLaura1: sparse frames
const uint32 kParkLauraFrameTable = 0x9EAE; // FRM_ParkLaura2: frame offset table, relative to kParkLauraFrames
const uint32 kParkLauraPalette = 0xF920;  // FRM_ParkLaura3: 16 colors for Laura's sprites

} // End of namespace Igor

#endif // IGOR_STATICS_H
