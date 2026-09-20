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

#include "deterministic_random.h"

namespace Scooby {
namespace {
inline uint32 RotateLeft32(uint32 value, int count) {
	return (value << count) | (value >> (32 - count));
}
} // namespace

uint16 DeterministicRandom::scaleNextRandomValue(uint16 scale) {
	// Rotate the complete seed left once, complement it, and persist it.
	_state = ~RotateLeft32(_state, 1);

	// Return the upper word of one unsigned 16x16-bit product.
	uint32 product = uint32(scale) * uint16(_state);
	return static_cast<uint16>(product >> 16);
}

bool DeterministicRandom::advanceDuelImpactFlagChoice() {
	_state = RotateLeft32(RotateLeft32(~_state, 1), 16);
	return (_state & 0x00008000u) != 0;
}
} // namespace Scooby