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

#ifndef SCOOBY_DETERMINISTIC_RANDOM_H
#define SCOOBY_DETERMINISTIC_RANDOM_H

#include "common/scummsys.h"

// Owns the shared pseudo-random state advanced by original runtime routines.

namespace Scooby {

class DeterministicRandom {
public:
	DeterministicRandom() : _state(0) {
	}

	// Ghidra InitializeRuntimeState writes 0x45C3F2D1 at 0x0000A25E-0x0000A267.
	void initializeStartupSeed() { _state = kStartupSeed; }

	// Ghidra: scaleNextRandomValue (0x000022D6). Advances and scales the
	// shared deterministic state.
	uint16 scaleNextRandomValue(uint16 scale);

	// Ghidra ResolveDuelActorCollision/ResolveDuelFirstActorImpact/
	// ResolveDuelSecondActorImpact all repeat this sequence. Advances the
	// shared state and returns the duel impact flag choice from its low word.
	bool advanceDuelImpactFlagChoice();

private:
	static const uint32 kStartupSeed = 0x45C3F2D1;

	// Replaces Ghidra g_dwRandomState at 0xFF0428-0xFF042B.
	uint32 _state;
};
} // namespace Scooby

#endif // SCOOBY_DETERMINISTIC_RANDOM_H
