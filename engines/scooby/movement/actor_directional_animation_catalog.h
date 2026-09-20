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

#ifndef SCOOBY_ACTOR_DIRECTIONAL_ANIMATION_CATALOG_H
#define SCOOBY_ACTOR_DIRECTIONAL_ANIMATION_CATALOG_H

#include "common/scummsys.h"

// Maps lead and companion position changes to their authored directional animation offsets.
//
// Ghidra g_awCompanionActorDirectionalAnimationOffsets at 0x000FC6E0-0x000FC6FF and
// g_awLeadActorDirectionalAnimationOffsets at 0x000FC720-0x000FC73F are independent
// four-by-four word matrices. Rows select the previous signed position and columns select the next.

namespace Scooby {
// Resolves one companion directional transition through the original wrapped word offset.
// previousPositionIndex: signed matrix row. nextPositionIndex: signed matrix column.
// Returns the selected descriptor-relative animation offset.
int16 ResolveCompanion(int16 previousPositionIndex, int16 nextPositionIndex);

// Resolves one lead directional transition through the original wrapped word offset.
// previousPositionIndex: signed matrix row. nextPositionIndex: signed matrix column.
// Returns the selected descriptor-relative animation offset.
int16 ResolveLead(int16 previousPositionIndex, int16 nextPositionIndex);
} // namespace Scooby

#endif // SCOOBY_ACTOR_DIRECTIONAL_ANIMATION_CATALOG_H
