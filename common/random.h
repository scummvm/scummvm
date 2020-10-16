/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_RANDOM_H
#define COMMON_RANDOM_H

#include "common/scummsys.h"

namespace Common {

/**
 * @defgroup common_rng RNG
 * @ingroup common
 *
 * @brief Random number generator (RNG) implementation.
 *
 * @{
 */

class String;

/**
 * Simple random number generator. Although it is definitely not suitable for
 * cryptographic purposes, it serves our purposes just fine.
 */
class RandomSource {
private:
	uint32 _randSeed;

public:
	/**
	 * Construct a new randomness source with the specific @p name.
	 * The name used must be globally unique, and is used to
	 * register the randomness source with the active event recorder,
	 * if any.
	 */
	RandomSource(const String &name);

	void setSeed(uint32 seed); /*!< Set the seed used to initialize the RNG. */

	uint32 getSeed() const { /*!< Get a random seed that can be used to initialize the RNG. */
		return _randSeed;
	}

	/**
	 * Generate a random unsigned integer in the interval [0, max].
	 * @param max	The upper bound
	 * @return	A random number in the interval [0, max].
	 */
	uint getRandomNumber(uint max);

	/**
	 * Generate a random bit, i.e. either 0 or 1.
	 * Identical to @c getRandomNumber(1), but potentially faster.
	 * @return	A random bit, either 0 or 1.
	 */
	uint getRandomBit();

	/**
	 * Generate a random unsigned integer in the interval [min, max].
	 * @param min	The lower bound.
	 * @param max	The upper bound.
	 * @return	A random number in the interval [min, max].
	 */
	uint getRandomNumberRng(uint min, uint max);

	/**
	 * Generates a random signed integer in the interval [min, max].
	 * @param min	the lower bound
	 * @param max	the upper bound
	 * @return	a random number in the interval [min, max]
	 */
	int getRandomNumberRngSigned(int min, int max);
};

/** @} */

} // End of namespace Common

#endif
