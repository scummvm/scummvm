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

#ifndef MOHAWK_ZOOMBINI_RANDOM_H
#define MOHAWK_ZOOMBINI_RANDOM_H

#include "common/random.h"
#include "common/scummsys.h"

#include "mohawk/zoombini_metaengine.h"

namespace Mohawk {

/**
 * Reimplement the original Z1 bounded generator or use ScummVM's default PRNG.
 *
 * The compatibility stream uses the MSVC LCG transition constants, but its
 * game-owned bounded helper consumes all 16 high state bits rather than the
 * 15-bit result returned by the MSVC CRT rand() function.
 * The active algorithm is selected by the "prng_algorithm" config option.
 */
class ZoombiniRandom {
private:
	/** State of the original PRNG. */
	uint32 _randState;
	/** ScummVM standard PRNG. */
	Common::RandomSource _scummRnd;
	/** Currently selected PRNG algorithm. */
	MohawkMetaEngine_Zoombini::PrngAlgorithm _prngAlgorithm;

	/**
	 * Generate one inclusive bounded value with the compatibility algorithm.
	 * A zero upper bound returns zero without advancing @ref ZoombiniRandom::_randState.
	 */
	uint16 getOriginalRandomNumber(uint32 max);

public:
	/**
	 * Construct a new randomness source with the specific @p name.
	 * The name must be globally unique.
	 * It registers the randomness source with the active event recorder, if any.
	 *
	 * Reads "prng_algorithm" from ConfMan to select the algorithm.
	 * Defaults to the compatibility PRNG if the key is absent.
	 */
	ZoombiniRandom(const Common::String &name);

	/** Generates new seed based on the current date/time */
	static uint32 generateNewSeed();
	/** Switch algorithms and start a new random sequence only when the selection changes. */
	void setAlgorithm(MohawkMetaEngine_Zoombini::PrngAlgorithm prngAlgorithm);
	/** Seed the selected stream after applying zero normalization. */
	void setSeed(uint32 seed);
	/** Return the current state of the selected stream. */
	uint32 getSeed() const;

	/**
	 * Generate a random signed integer in the interval [0, max].
	 * @param max The upper bound
	 * @return A random number in the interval [0, max].
	 */
	int16 getRandomNumber(int16 max);
	/**
	 * Generate a random signed integer in the interval [min, max].
	 * @param min The lower bound.
	 * @param max The upper bound.
	 * @return A random number in the interval [min, max].
	 */
	int16 getRandomNumber(int16 min, int16 max);
	/**
	 * Generate a random Boolean value from one inclusive 0..1 draw.
	 * Returns @c true when the draw is one.
	 */
	bool getRandomBool();
	/**
	 * Pick a non-repeating random index from a pool of size @p poolSize.
	 * Uses a @p bitmask to track which indices have been used; resets when all exhausted.
	 * @param poolSize Number of items in the pool (max 32)
	 * @param bitmask Caller-owned state tracking which indices have been picked
	 * @return A randomly chosen index in [0, poolSize-1]
	 */
	uint16 getNonRepeatRandom(uint16 poolSize, uint32 &bitmask);
};

} // End of namespace Mohawk

#endif
