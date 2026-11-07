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

#ifndef SCOOBY_ACTOR_FACING_ANIMATION_H
#define SCOOBY_ACTOR_FACING_ANIMATION_H

#include "common/scummsys.h"

// Maps Shaggy's and Scooby's facing changes to their authored walking and turning animation offsets.
//
// The ROM holds four consecutive four-by-four word matrices. Each selects a row with the previous signed
// facing (position index) and a column with the next, and yields a descriptor-relative animation offset:
//   - Scooby walking: Ghidra g_awCompanionActorDirectionalAnimationOffsets at
//     0x000FC6E0-0x000FC6FF.
//   - Scooby turning on the spot: Ghidra g_awCompanionActorPositionTransitionAnimationOffsets at
//     0x000FC700-0x000FC71F, used by ExecuteRoomScriptCommand1E and UpdateRoomActorsAndInterface.
//   - Shaggy walking: Ghidra g_awLeadActorDirectionalAnimationOffsets at 0x000FC720-0x000FC73F.
//   - Shaggy turning on the spot: Ghidra g_awLeadActorPositionTransitionAnimationOffsets at
//     0x000FC740-0x000FC75F, used by commands 0x10 and 0x1E.

namespace Scooby {
// Each resolver selects one exact row-major matrix entry through the original wrapped word offset, without
// changing either position.
// previousPositionIndex: signed matrix row. nextPositionIndex: signed matrix column.
// Returns the descriptor-relative animation offset stored at the selected matrix entry.
int16 resolveShaggyWalkingAnimation(int16 previousPositionIndex, int16 nextPositionIndex);
int16 resolveScoobyWalkingAnimation(int16 previousPositionIndex, int16 nextPositionIndex);
int16 resolveShaggyTurnAnimation(int16 previousPositionIndex, int16 nextPositionIndex);
int16 resolveScoobyTurnAnimation(int16 previousPositionIndex, int16 nextPositionIndex);
} // namespace Scooby

#endif // SCOOBY_ACTOR_FACING_ANIMATION_H
