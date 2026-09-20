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

#include "companion_actor_position_transition_animation_catalog.h"

#include "common/scummsys.h"

#include "common/array.h"

namespace Scooby {
namespace {
const int kPositionCount = 4;

const Common::Array<int16> kAnimationOffsets = {
	0x0004, 0x0033, 0x0038, 0x0036, 0x0032, 0x0005, 0x0039, 0x0037, 0x003C, 0x003D, 0x0006, 0x0035,
	0x003A, 0x003B, 0x0034, 0x0007};
} // namespace

int16 ResolveCompanionPositionTransition(int16 previousPositionIndex, int16 nextPositionIndex) {
	int16 byteOffset = static_cast<int16>(
		previousPositionIndex * kPositionCount * static_cast<int>(sizeof(int16)) +
		nextPositionIndex * static_cast<int>(sizeof(int16)));
	return kAnimationOffsets[static_cast<std::size_t>(byteOffset / static_cast<int>(sizeof(int16)))];
}
} // namespace Scooby