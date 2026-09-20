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

#include "actor_directional_animation_catalog.h"

#include "common/scummsys.h"

#include "common/array.h"

namespace Scooby {
namespace {
const int kPositionCount = 4;

const Common::Array<int16> kCompanionAnimationOffsets = {
	0x0000, 0x0027, 0x002C, 0x002A, 0x0026, 0x0001, 0x002D, 0x002B, 0x0030, 0x0031, 0x0002, 0x0029,
	0x002E, 0x002F, 0x0028, 0x0003};

const Common::Array<int16> kLeadAnimationOffsets = {
	0x0000, 0x0015, 0x001A, 0x0018, 0x0014, 0x0001, 0x001B, 0x0019, 0x001E, 0x001F, 0x0002, 0x0017,
	0x001C, 0x001D, 0x0016, 0x0003};

int16 Resolve(const Common::Array<int16> &offsets, int16 previousPositionIndex,
			  int16 nextPositionIndex) {
	int16 byteOffset = static_cast<int16>(
		previousPositionIndex * kPositionCount * static_cast<int>(sizeof(int16)) +
		nextPositionIndex * static_cast<int>(sizeof(int16)));
	return offsets[static_cast<std::size_t>(byteOffset / static_cast<int>(sizeof(int16)))];
}
} // namespace

int16 ResolveCompanion(int16 previousPositionIndex, int16 nextPositionIndex) {
	return Resolve(kCompanionAnimationOffsets, previousPositionIndex, nextPositionIndex);
}

int16 ResolveLead(int16 previousPositionIndex, int16 nextPositionIndex) {
	return Resolve(kLeadAnimationOffsets, previousPositionIndex, nextPositionIndex);
}
} // namespace Scooby