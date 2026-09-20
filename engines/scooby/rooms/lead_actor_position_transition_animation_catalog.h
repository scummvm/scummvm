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

#ifndef SCOOBY_LEAD_ACTOR_POSITION_TRANSITION_ANIMATION_CATALOG_H
#define SCOOBY_LEAD_ACTOR_POSITION_TRANSITION_ANIMATION_CATALOG_H

#include "common/scummsys.h"

// Maps lead-actor room-position changes to their authored transition animations.
//
// Ghidra g_awLeadActorPositionTransitionAnimationOffsets at 0x000FC740-0x000FC75F is a four-by-four word
// matrix. Commands 0x10 and 0x1E select a row with the previous position and a column with the next position.

namespace Scooby {
// Resolves one exact row-major transition animation without changing either position.
// previousPositionIndex: signed row index used by the original word calculation.
// nextPositionIndex: signed column index used by the original word calculation.
// Returns the descriptor-relative animation offset stored at the selected matrix entry.
int16 ResolveLeadPositionTransition(int16 previousPositionIndex, int16 nextPositionIndex);
} // namespace Scooby

#endif // SCOOBY_LEAD_ACTOR_POSITION_TRANSITION_ANIMATION_CATALOG_H
